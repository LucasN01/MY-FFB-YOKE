/*
  MotorDriver.h / .cpp
  ------------------------------------------------------------
  ARCHIVO NUEVO. Traduce un valor de PWM firmado (-255..255) a
  las dos salidas PWM del puente H, igual esquema que usaba
  AxisCalibration::MoveMotor() del proyecto original.

  No debería necesitar cambios, salvo:
  - Si tu driver espera PWM invertido (0=full, 255=off) -> avisame.
  - Si usás pin de enable, ya está contemplado (ENABLE_PIN = -1 si no aplica).
------------------------------------------------------------
*/

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>
#include "mis_defines.h"

class MotorDriver {
public:
    MotorDriver(uint8_t pinLeft, uint8_t pinRight, int8_t enablePin = -1)
        : _pinLeft(pinLeft), _pinRight(pinRight), _enablePin(enablePin) {}

    void begin() {
        pinMode(_pinLeft, OUTPUT);
        pinMode(_pinRight, OUTPUT);
        stop(); // PWM a cero ANTES de habilitar el puente.
        if (_enablePin >= 0) {
            pinMode(_enablePin, OUTPUT);
            digitalWrite(_enablePin, HIGH);
        }
        stop();
    }

    // Frontera entre referencia normalizada y salida electrica.
    // Redondeo simetrico: command=0 produce exactamente PWM=0.
    void setCommand(float command, int16_t pwmLimit = 255) {
        command = constrain(command, -1.0f, 1.0f);
        pwmLimit = constrain(pwmLimit, 0, 255);
        const float scaled = command * pwmLimit;
        const int16_t pwm = (int16_t)(scaled >= 0.0f ? scaled + 0.5f : scaled - 0.5f);
        setPWM(pwm);
    }

    // Salida PWM firmada; conserva la asignacion de pines existente.
    void setPWM(int16_t pwmSigned) {
        pwmSigned = constrain(pwmSigned, -MOTOR_PWM_HARD_LIMIT, MOTOR_PWM_HARD_LIMIT);
        _lastAppliedPWM = pwmSigned; // guardado para el TorqueMonitor

        if (pwmSigned >= 0) {
            analogWrite(_pinLeft, 0);
            analogWrite(_pinRight, pwmSigned);
        } else {
            analogWrite(_pinRight, 0);
            analogWrite(_pinLeft, -pwmSigned);
        }
    }

    void stop() {
        analogWrite(_pinLeft, 0);
        analogWrite(_pinRight, 0);
        _lastAppliedPWM = 0;
    }

    // Último PWM efectivamente aplicado (con signo), útil para el
    // chequeo de torque esperado vs medido.
    int16_t getLastAppliedPWM() const { return _lastAppliedPWM; }

private:
    uint8_t _pinLeft, _pinRight;
    int8_t  _enablePin;
    int16_t _lastAppliedPWM = 0;
};

#endif
