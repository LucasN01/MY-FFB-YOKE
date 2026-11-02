#ifndef LCD_TELEMETRY_H
#define LCD_TELEMETRY_H

#include "hal/hal.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include "mis_defines.h"
#include "YokeI2C.h"

// LCD HD44780 con adaptador PCF8574 habitual:
// P0=RS, P1=RW, P2=E, P3=luz, P4..P7=D4..D7.
// I2C mediante la HAL: no requiere LiquidCrystal_I2C.
class LcdTelemetry {
public:
    void showMenu(bool active,const char* top,const char* bottom) {
        if(active==_menuActive && (!active ||
           (strncmp(top,_menuRows[0],16)==0 && strncmp(bottom,_menuRows[1],16)==0))) return;
        _menuActive=active;
        if(active) {
            for(uint8_t row=0;row<2;++row) {
                const char* text=row?bottom:top;
                memset(_menuRows[row],' ',16); _menuRows[row][16]=0;
                for(uint8_t i=0;i<16 && text[i];++i) _menuRows[row][i]=text[i];
            }
        }
        _step=34;
        _lastRefresh=hal::millis()-LCD_REFRESH_MS;
    }
    void begin() {
        configureBus();
        _address = LCD_I2C_ADDRESS;
        _state = State::Probe;
        _since = hal::millis();
        _waitMs = LCD_POWER_ON_WAIT_MS;
    }

	void update(unsigned long now,
				float commandRoll,
				float commandPitch,
				int16_t pwmRoll,
				int16_t pwmPitch) {
		
		if (_state != State::Ready) {
			
			if ((uint32_t)(now - _since) < _waitMs)
				return;
			
			serviceInitialization(
								  commandRoll,
								  commandPitch,
								  pwmRoll,
								  pwmPitch
								  );
			
			return;
		}
		
		// LCD estaba funcionando pero fue apagado.
		if (!busAvailable()) {
			lcdUnavailable();
			return;
		}
		
		if (_step >= 34) {
			
			if ((uint32_t)(now - _lastInit) >= LCD_RESYNC_MS) {
				schedule(State::IdlePins, 0);
				return;
			}
			
			if ((uint32_t)(now - _lastRefresh) < LCD_REFRESH_MS)
				return;
			
			_lastRefresh = now;
			
			snapshot(
					 commandRoll,
					 commandPitch,
					 pwmRoll,
					 pwmPitch
					 );
		}
		
		const uint8_t row = _step / 17;
		const uint8_t column = _step % 17;
		
		const bool ok =
			column == 0
			? send(row ? 0xC0 : 0x80, false)
			: send(_rows[row][column - 1], true);
		
		if (ok)
			++_step;
	}

private:
    enum class State : uint8_t {
        Probe, IdlePins, Reset1, Reset2, Reset3, FourBits,
        FunctionSet, DisplayOn, EntryMode, Home, Finish, Ready
    };
    uint8_t _address = LCD_I2C_ADDRESS;
    State _state = State::Probe;
    uint8_t _step = 34;
    uint32_t _since = 0;
    uint16_t _waitMs = 0;
    uint32_t _lastInit = 0;
    uint32_t _lastRefresh = 0;
    char _rows[2][17];
    bool _menuActive=false;
    char _menuRows[2][17]={{0},{0}};

    void configureBus() {
        YokeI2C::configureBus0();
    }

    void schedule(State state, uint16_t waitMs) {
        _state = state;
        // Medir desde el fin de la transaccion, no desde el inicio del loop.
        _since = hal::millis();
        _waitMs = waitMs;
    }

	bool finishTransmission() {
		
		const uint8_t result = hal::wire0.endTransmission();
		

		
		if (result == 0)
			return true;
		
		lcdUnavailable();
		return false;
	}

	void serviceInitialization(float cr, float cp,
							   int16_t pr, int16_t pp) {
		
		// LCD apagado o bus I2C bloqueado:
		// salir inmediatamente sin afectar el FFB.
		if (!busAvailable()) {
			lcdUnavailable();
			return;
		}
		
		switch (_state) {
			
		case State::Probe:
			configureBus();
		
		if (!busAvailable()) {
			lcdUnavailable();
			return;
		}
		
		hal::wire0.beginTransmission(_address);
		
		if (finishTransmission()) {
			schedule(State::IdlePins, LCD_POWER_ON_WAIT_MS);
		} else {
			if (_address == 0x27)
				_address = 0x3F;
			else if (_address == 0x3F)
				_address = 0x27;
		}
		break;
		
		case State::IdlePins:
			hal::wire0.beginTransmission(_address);
		hal::wire0.write(0x08);
		
		if (finishTransmission())
			schedule(State::Reset1, 5);
		break;
		
		case State::Reset1:
			if (nibble(0x30))
				schedule(State::Reset2, 5);
		break;
		
		case State::Reset2:
			if (nibble(0x30))
				schedule(State::Reset3, 1);
		break;
		
		case State::Reset3:
			if (nibble(0x30))
				schedule(State::FourBits, 1);
		break;
		
		case State::FourBits:
			if (nibble(0x20))
				schedule(State::FunctionSet, 1);
		break;
		
		case State::FunctionSet:
			if (send(0x28, false))
				schedule(State::DisplayOn, 1);
		break;
		
		case State::DisplayOn:
			if (send(0x0C, false))
				schedule(State::EntryMode, 1);
		break;
		
		case State::EntryMode:
			if (send(0x06, false))
				schedule(State::Home, 1);
		break;
		
		case State::Home:
			if (send(0x02, false))
				schedule(State::Finish, 3);
		break;
		
		case State::Finish:
			snapshot(cr, cp, pr, pp);
		_lastRefresh = _lastInit = hal::millis();
		_state = State::Ready;
		break;
		
		case State::Ready:
			break;
		}
	}

    void pulse(uint8_t value) {
        hal::wire0.write(value);
        hal::wire0.write(value | 0x04);
        hal::wire0.write(value);
    }

    bool nibble(uint8_t value) {
        hal::wire0.beginTransmission(_address);
        pulse(value | 0x08);
        return finishTransmission();
    }

    bool send(uint8_t value, bool data) {
        const uint8_t flags = 0x08 | (data ? 0x01 : 0);
        hal::wire0.beginTransmission(_address);
        pulse((value & 0xF0) | flags);
        pulse(((value << 4) & 0xF0) | flags);
        return finishTransmission();
    }

    void formatRow(char* row, char axis, float command, int16_t pwm) {
        // RF+.7500 PW+191, con espacios finales hasta completar 16 columnas.
        // Cuatro decimales sin printf float ni cero antes del punto.
        command = hal::constrain(command, -1.0f, 1.0f);
        const float scaled = command * 10000.0f;
        const int16_t tenThousandths = (int16_t)(scaled >= 0.0f ? scaled + 0.5f : scaled - 0.5f);
        const unsigned int magnitude = tenThousandths < 0 ? -tenThousandths : tenThousandths;
        const unsigned int pwmMagnitude = pwm < 0 ? -(long)pwm : pwm;
        // Si alguna vez redondea a 1, mostrarlo correctamente (no .0000).
        const char* format = magnitude >= 10000
            ? "%cF%c1.%04u PW%c%3u " : "%cF%c.%04u PW%c%3u  ";
        snprintf(row, 17, format, axis,
                 tenThousandths < 0 ? '-' : '+', magnitude % 10000,
                 pwm < 0 ? '-' : '+', pwmMagnitude);
    }

    void snapshot(float commandRoll, float commandPitch, int16_t pr, int16_t pp) {
        if(_menuActive) { memcpy(_rows,_menuRows,sizeof(_rows)); _step=0; return; }
        formatRow(_rows[0], 'R', commandRoll, pr);
        formatRow(_rows[1], 'P', commandPitch, pp);
        _step = 0;
    }
	
	bool busAvailable() {
		// Si el LCD est� apagado y deja SDA o SCL clavados en false,
		// no intentar una transferencia I2C.
		
		return hal::digitalRead(LCD_SDA_PIN) == true &&
			hal::digitalRead(LCD_SCL_PIN) == true;
	}
	
	void lcdUnavailable() {
		// Abandonar cualquier actualizaci�n actual y volver a probar
		// m�s adelante. El FFB contin�a funcionando normalmente.
		_step = 34;
		schedule(State::Probe, LCD_RETRY_MS);
	}
};

#endif
