#ifndef YOKE_CALIBRATION_CONTROLS_H
#define YOKE_CALIBRATION_CONTROLS_H
#include "CalibrationMenu.h"
#if YOKE_CALIBRATION_ENABLED
#include <Arduino.h>
#include <EEPROM.h>

namespace YokeCalibration {
namespace Pins {
constexpr uint8_t all[]={YOKE_LED_NORMAL_PIN,YOKE_LED_CALIBRATION_PIN,
    YOKE_BUTTON_MODE_PIN,YOKE_BUTTON_OK_PIN,YOKE_BUTTON_BACK_PIN,
    YOKE_BUTTON_PLUS_PIN,YOKE_BUTTON_MINUS_PIN};
constexpr bool available(uint8_t p) {
    return (p<=22 || (p>=26 && p<=28)) && p!=ROLL_ENC_A && p!=ROLL_ENC_B &&
        p!=PITCH_ENC_A && p!=PITCH_ENC_B && p!=ROLL_PWM_LEFT && p!=ROLL_PWM_RIGHT &&
        p!=PITCH_PWM_LEFT && p!=PITCH_PWM_RIGHT && p!=ROLL_EN_PIN && p!=PITCH_EN_PIN &&
        p!=LCD_SDA_PIN && p!=LCD_SCL_PIN &&
        (YOKE_USE_SIMULATION!=2 || (p!=ROLL_POT_PIN && p!=PITCH_POT_PIN));
}
constexpr bool valid(uint8_t i=0,uint8_t j=1) {
    return i>=7 ? true : (!available(all[i]) ? false :
        (j>=7 ? valid(i+1,i+2) : (all[i]!=all[j] && valid(i,j+1))));
}
static_assert(valid(),"Pines de calibracion duplicados, no expuestos o en conflicto con el yugo");
}
class Controls {
public:
    Controls(YokeGains::Settings& values,YokePots::Settings& pots)
        :menu(values,&pots),settings(values),potSettings(pots) {}
    void begin() {
        // LEDs reflect mode, not simulator connection or whether forces are armed.
        pinMode(YOKE_LED_NORMAL_PIN,OUTPUT); pinMode(YOKE_LED_CALIBRATION_PIN,OUTPUT);
        leds();
        for(uint8_t i=0;i<5;++i) {
            pinMode(Pins::all[i+2],INPUT_PULLUP);
            buttons[i].begin(digitalRead(Pins::all[i+2])==LOW,millis());
        }
        EEPROM.begin(256);
        uint8_t record[YokeGains::recordSize];
        for(uint8_t i=0;i<sizeof(record);++i) record[i]=EEPROM.read(i);
        YokeGains::decode(record,settings); // erased/old/corrupt record keeps defaults
        uint8_t potRecord[YokePots::recordSize];
        for(uint8_t i=0;i<sizeof(potRecord);++i) potRecord[i]=EEPROM.read(YokePots::recordOffset+i);
        YokePots::decode(potRecord,potSettings);
    }
    void update(uint32_t now,bool paused) {
        menu.update(now,paused);
        bool pressed[5], down[5];
        for(uint8_t i=0;i<5;++i) down[i]=digitalRead(Pins::all[i+2])==LOW;
        const bool conflict=down[3] && down[4];
        const bool repeat=menu.currentScreen()==Screen::Edit && !menu.savePending() && !conflict;
        for(uint8_t i=0;i<5;++i) pressed[i]=buttons[i].update(down[i],now,repeat && i>=3);
        // Conflicting +/- presses cancel. Back has priority over an accidental OK.
        if(pressed[2]) menu.press(Button::Back);
        else if(pressed[0]) menu.press(Button::Mode);
        else if(pressed[1]) menu.press(Button::Ok);
        else if(!conflict && pressed[3]!=pressed[4]) menu.press(pressed[3]?Button::Plus:Button::Minus);
        leds();
    }
    // Caller must stop both motors before flash erase/program stalls execution.
    bool save() {
        if(!menu.savePending()) return true;
        static_assert(YokeGains::recordSize<=YokePots::recordOffset &&
                      YokePots::recordOffset+YokePots::recordSize<=256,"EEPROM solapada");
        uint8_t record[64], previous[64];
        const bool pots=menu.savingPots();
        const uint16_t offset=pots ? YokePots::recordOffset : 0;
        const uint8_t size=pots ? YokePots::recordSize : YokeGains::recordSize;
        if(pots) YokePots::encode(menu.potCandidate(),record);
        else YokeGains::encode(menu.candidate(),record);
        for(uint8_t i=0;i<size;++i) {
            previous[i]=EEPROM.read(offset+i); EEPROM.write(offset+i,record[i]);
        }
        const bool ok=EEPROM.commit();
        // Una edicion fallida/cancelada no debe colarse en otro guardado posterior.
        if(!ok) for(uint8_t i=0;i<size;++i) EEPROM.write(offset+i,previous[i]);
        menu.finishSave(ok); return ok;
    }
    Menu menu;
private:
    void leds() {
        digitalWrite(YOKE_LED_NORMAL_PIN,menu.active()?LOW:HIGH);
        digitalWrite(YOKE_LED_CALIBRATION_PIN,menu.active()?HIGH:LOW);
    }
    YokeGains::Settings& settings;
    YokePots::Settings& potSettings;
    DebouncedButton buttons[5];
};
}
#endif
#endif
