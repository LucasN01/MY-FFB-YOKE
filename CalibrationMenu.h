#ifndef YOKE_CALIBRATION_MENU_H
#define YOKE_CALIBRATION_MENU_H
#include "GainSettings.h"
#include "PotCalibration.h"
#include <stdio.h>
#include <string.h>

namespace YokeCalibration {
enum class Button : uint8_t { Mode, Ok, Back, Plus, Minus };
enum class Screen : uint8_t { Normal, Browse, Edit, BrowseEffects, EditEffect, BrowsePots, CalibratePots };
class DebouncedButton {
public:
    void begin(bool pressed,uint32_t now) {
        raw=stable=pressed; changed=heldAt=lastRepeat=now; armed=repeating=false;
    }
    bool update(bool pressed,uint32_t now,bool repeat=false) {
        if(pressed!=raw) { raw=pressed; changed=now; }
        if(!raw) { armed=false; repeating=false; } // release stops repeat immediately
        if(raw!=stable && uint32_t(now-changed)>=YOKE_BUTTON_DEBOUNCE_MS) {
            stable=raw;
            if(stable) { armed=true; repeating=false; heldAt=lastRepeat=now; return true; }
        }
        if(!repeat) { heldAt=lastRepeat=now; repeating=false; }
        if(repeat && raw && stable && armed &&
           uint32_t(now-heldAt)>=YOKE_BUTTON_REPEAT_DELAY_MS &&
           (!repeating || uint32_t(now-lastRepeat)>=YOKE_BUTTON_REPEAT_INTERVAL_MS)) {
            lastRepeat=now; repeating=true; return true; // no catch-up burst after a stall
        }
        return false;
    }
private:
    bool raw=false,stable=false,armed=false,repeating=false;
    uint32_t changed=0,heldAt=0,lastRepeat=0;
};
class Menu {
public:
    explicit Menu(YokeGains::Settings& values,YokePots::Settings* pots=nullptr)
        :settings(values),potSettings(pots) {}
    void potReadings(uint16_t roll,uint16_t pitch) {
        adc[0]=roll; adc[1]=pitch; potsAvailable=potSettings && roll<=4095 && pitch<=4095;
    }
    bool savingPots() const { return pending && screen==Screen::CalibratePots; }
    YokePots::Settings potCandidate() const { return potDraft; }
    uint8_t calibrationStep() const { return potStep; }
    void update(uint32_t now,bool paused) {
        clock=now; allowed=paused;
        if(notice!=Notice::None && uint32_t(now-noticeAt)>=2000) notice=Notice::None;
        if(!allowed && active()) {
            screen=Screen::Normal; pending=false; error=false;
            show(Notice::PauseRequired);
        }
    }
    bool displayActive() const { return active() || notice!=Notice::None; }
    bool active() const { return screen!=Screen::Normal; }
    Screen currentScreen() const { return screen; }
    YokeGains::Id selected() const {
        // Display order is independent of persistent EEPROM IDs.
        static const YokeGains::Id order[]={
            YokeGains::Total, YokeGains::CenterPitch, YokeGains::CenterRoll,
            YokeGains::CenterRest, YokeGains::GustPitch, YokeGains::GustRoll,
            YokeGains::Ground, YokeGains::Stall, YokeGains::Gear,
            YokeGains::Spoiler, YokeGains::Damper, YokeGains::Friction
        };
        static_assert(sizeof(order)/sizeof(order[0])==YokeGains::Count,"Menu incompleto");
        return order[index+(effectView() ? 1 : 0)];
    }
    uint16_t editValue() const { return draft; }
    bool savePending() const { return pending; }
    bool effectView() const { return screen==Screen::BrowseEffects || screen==Screen::EditEffect; }
    void press(Button button) {
        if(pending) return;
        if(!allowed) {
            if(button==Button::Mode) show(Notice::PauseRequired);
            return;
        }
        notice=Notice::None;
        if(screen==Screen::Normal) {
            if(button==Button::Mode) { screen=Screen::Browse; index=0; }
            return;
        }
        if(button==Button::Mode) {
            screen=effectView() ? Screen::BrowsePots :
                ((screen==Screen::BrowsePots || screen==Screen::CalibratePots) ? Screen::Browse : Screen::BrowseEffects);
            index=0; error=false; return; // switch lists, discard unconfirmed edit
        }
        if(button==Button::Back) {
            screen=screen==Screen::CalibratePots ? Screen::BrowsePots :
                (screen==Screen::EditEffect ? Screen::BrowseEffects :
                (screen==Screen::Edit ? Screen::Browse : Screen::Normal));
            error=false; return;
        }
        if(screen==Screen::BrowsePots) {
            if(button==Button::Ok) {
                if(!potsAvailable) { show(Notice::PotsRequired); return; }
                potDraft=*potSettings; potStep=0; error=false; rangeError=false; stepAt=clock;
                screen=Screen::CalibratePots;
            }
            return;
        }
        if(screen==Screen::CalibratePots) {
            if(button!=Button::Ok || !potsAvailable) return;
            // Error de flash: reintentar la misma captura completa, sin releer.
            if(error) { pending=true; return; }
            const uint8_t axis=potStep<2 ? 1 : 0; // Pitch primero, despues Roll
            if((potStep&1)==0) potDraft.axes[axis].negative=adc[axis];
            else {
                const YokePots::Axis captured=YokePots::endpoints(potDraft.axes[axis].negative,adc[axis]);
                if(!YokePots::valid(captured)) { rangeError=true; return; }
                potDraft.axes[axis]=captured;
            }
            rangeError=false;
            stepAt=clock;
            if(potStep==3) pending=true;
            else ++potStep;
            return;
        }
        if(screen==Screen::Browse || screen==Screen::BrowseEffects) {
            const bool effects=effectView();
            const uint8_t count=YokeGains::Count-(effects ? 1 : 0);
            if(button==Button::Plus) index=(index+1)%count;
            if(button==Button::Minus) index=(index+count-1)%count;
            if(button==Button::Ok) {
                draft=effects ? settings.enabled(selected()) : settings.raw(selected());
                screen=effects ? Screen::EditEffect : Screen::Edit; error=false;
            }
        } else {
            if(button==Button::Plus) {
                const uint16_t next=draft+YokeGains::spec(selected()).step;
                draft=effectView() ? 1 : (next>10000 ? 10000 : next); error=false;
            }
            if(button==Button::Minus) {
                const uint16_t step=YokeGains::spec(selected()).step;
                draft=effectView() ? 0 : (draft<step ? 0 : draft-step); error=false;
            }
            if(button==Button::Ok) {
                const uint16_t current=effectView() ? settings.enabled(selected()) : settings.raw(selected());
                if(draft==current) { screen=effectView() ? Screen::BrowseEffects : Screen::Browse; error=false; }
                else pending=true;
            }
        }
    }
    YokeGains::Settings candidate() const {
        YokeGains::Settings result=settings;
        if(pending && !savingPots()) {
            if(effectView()) result.setEnabled(selected(),draft!=0);
            else result.set(selected(),draft);
        }
        return result;
    }
    void finishSave(bool success) {
        if(!pending) return;
        if(savingPots()) {
            if(success) { *potSettings=potDraft; screen=Screen::BrowsePots; show(Notice::PotsSaved); }
            error=!success; pending=false; return;
        }
        if(success) {
            settings=candidate();
            screen=effectView() ? Screen::BrowseEffects : Screen::Browse;
            show(Notice::Saved);
        }
        error=!success; pending=false;
    }
    void rows(char* top,char* bottom) const {
        char value[17]={0};
        if(notice==Notice::PauseRequired) {
            padded(top,"PAUSE SIMULADOR"); padded(bottom,"PARA CALIBRAR");
        } else if(notice==Notice::PotsRequired) {
            padded(top,"REQUIERE MODO 2"); padded(bottom,"POTES ADC0/ADC1");
        } else if(notice==Notice::PotsSaved) {
            padded(top,"POTES GUARDADOS"); padded(bottom,"PITCH Y ROLL");
        } else if(screen==Screen::BrowsePots) {
            padded(top,"CALIBRAR POTES"); padded(bottom,"OK PARA INICIAR");
        } else if(screen==Screen::CalibratePots) {
            padded(top,error ? "ERROR GUARDADO" : (potStep<2 ? "CALIBRAR PITCH" : "CALIBRAR ROLL"));
            // Alternar instrucciones completas en la segunda fila de 16 columnas.
            const uint8_t phase=(uint32_t(clock-stepAt)/1500)%3;
            if(error) padded(bottom,"OK: REINTENTAR");
            else if(rangeError) padded(bottom,"POCO RECORRIDO");
            else padded(bottom,phase==0 ? ((potStep&1) ? "GIRAR HORARIO" : "GIRAR ANTIHOR.") :
                               (phase==1 ? "HASTA EL FINAL" : "PRESIONAR OK"));
        } else if(notice==Notice::Saved) {
            padded(top,"GUARDADO"); padded(bottom,YokeGains::spec(selected()).label);
        } else if(screen==Screen::Browse || screen==Screen::BrowseEffects) {
            padded(top,effectView() ? "EFECTOS" : "MODO CALIBRACION");
            snprintf(value,sizeof(value),"* %s",YokeGains::spec(selected()).label);
            padded(bottom,value);
        } else if(screen==Screen::Edit || screen==Screen::EditEffect) {
            padded(top,error ? "ERROR GUARDADO" : YokeGains::spec(selected()).label);
            if(effectView()) snprintf(value,sizeof(value),"%s",draft ? "Activado" : "Desactivado");
            else if(selected()==YokeGains::Total) snprintf(value,sizeof(value),"Valor: %u%%",unsigned(draft/100));
            else snprintf(value,sizeof(value),"Valor: %u.%03u",unsigned(draft/10000),unsigned((draft%10000)/10));
            padded(bottom,value);
        } else { padded(top,""); padded(bottom,""); }
    }
private:
    enum class Notice : uint8_t { None, PauseRequired, Saved, PotsRequired, PotsSaved };
    Notice notice=Notice::None;
    uint32_t clock=0,noticeAt=0;
    bool allowed=false;
    void show(Notice value) { notice=value; noticeAt=clock; }
    static void padded(char* out,const char* text) {
        memset(out,' ',16); out[16]=0;
        for(uint8_t i=0;i<16 && text[i];++i) out[i]=text[i];
    }
    YokeGains::Settings& settings;
    YokePots::Settings* potSettings;
    YokePots::Settings potDraft;
    uint16_t adc[2]={0,0};
    uint8_t potStep=0;
    uint32_t stepAt=0;
    bool potsAvailable=false,rangeError=false;
    Screen screen=Screen::Normal;
    uint8_t index=0;
    uint16_t draft=0;
    bool pending=false,error=false;
};
}
#endif
