#ifndef YOKE_CALIBRATION_CHECKS_H
#define YOKE_CALIBRATION_CHECKS_H
#include "../CalibrationMenu.h"
#include <cstring>
#include <string>

static void recordSettingChecks() {
    using namespace YokeCalibration;
    YokeGains::Settings gains; YokeRecord::Settings record;
    uint8_t bytes[YokeRecord::recordSize];
    for(bool enabled:{false,true}) {
        record.enabled=enabled; YokeRecord::encode(record,bytes);
        YokeRecord::Settings loaded; CHECK(YokeRecord::decode(bytes,loaded) && loaded.enabled==enabled);
    }
    YokeRecord::Settings loaded{false};
    std::memset(bytes,255,sizeof(bytes)); CHECK(!YokeRecord::decode(bytes,loaded) && !loaded.enabled);
    YokeRecord::encode(record,bytes); bytes[6]^=1;
    CHECK(!YokeRecord::decode(bytes,loaded) && !loaded.enabled);
    for(int field:{0,4,5}) {
        YokeRecord::encode(record,bytes); bytes[field]=2; YokeSC::put16(bytes+6,YokeSC::crc(bytes,6));
        CHECK(!YokeRecord::decode(bytes,loaded) && !loaded.enabled);
    }
    YokePots::Settings pots;
    for(bool withRecord:{false,true}) {
        Menu edited(gains,&pots,withRecord ? &record : nullptr); edited.update(0,true);
        edited.potReadings(1000,1000);
        edited.press(Button::Mode); edited.press(Button::Ok); CHECK(edited.currentScreen()==Screen::Edit);
        edited.press(Button::Mode); CHECK(edited.currentScreen()==Screen::BrowseEffects);
        edited.press(Button::Ok); CHECK(edited.currentScreen()==Screen::EditEffect);
        edited.press(Button::Mode); CHECK(edited.currentScreen()==Screen::BrowsePots);
        edited.press(Button::Ok); CHECK(edited.currentScreen()==Screen::CalibratePots);
        edited.press(Button::Mode); CHECK(edited.currentScreen()==(withRecord ? Screen::BrowseRecord : Screen::Browse));
        if(withRecord) { edited.press(Button::Mode); CHECK(edited.currentScreen()==Screen::Browse); }
    }
    Menu menu(gains,nullptr,&record); char top[17],bottom[17];
    auto expectedRows=[&](const char* a,const char* b) {
        menu.rows(top,bottom);
        const std::string first=std::string(a)+std::string(16-std::strlen(a),' ');
        const std::string second=std::string(b)+std::string(16-std::strlen(b),' ');
        CHECK(std::strncmp(top,first.c_str(),17)==0 && std::strncmp(bottom,second.c_str(),17)==0);
    };
    menu.press(Button::Mode); CHECK(!menu.active());
    menu.update(0,true);
    const Screen cycle[]={Screen::Browse,Screen::BrowseEffects,Screen::BrowsePots,Screen::BrowseRecord,Screen::Browse};
    for(Screen screen:cycle) {
        menu.press(Button::Mode); CHECK(menu.currentScreen()==screen);
        if(screen==Screen::Browse) expectedRows("MODO CALIBRACION","* Ganancia total");
        else if(screen==Screen::BrowseEffects) expectedRows("EFECTOS","* Centrado pitch");
        else if(screen==Screen::BrowsePots) expectedRows("CALIBRAR POTES","OK PARA INICIAR");
        else expectedRows("REGISTRO DATOS","* Activado");
    }
    for(int i=0;i<3;++i) menu.press(Button::Mode);
    expectedRows("REGISTRO DATOS","* Activado");
    menu.press(Button::Plus); menu.press(Button::Minus); expectedRows("REGISTRO DATOS","* Activado");
    CHECK(menu.currentScreen()==Screen::BrowseRecord && record.enabled);
    menu.press(Button::Ok); expectedRows("REGISTRO DATOS","Activado");
    menu.press(Button::Ok); CHECK(menu.currentScreen()==Screen::BrowseRecord && !menu.savePending());
    menu.press(Button::Ok); menu.press(Button::Minus); expectedRows("REGISTRO DATOS","Desactivado");
    menu.press(Button::Back); CHECK(menu.currentScreen()==Screen::BrowseRecord && record.enabled);
    menu.press(Button::Ok); menu.press(Button::Minus); menu.press(Button::Ok);
    CHECK(menu.savingRecord() && !menu.recordCandidate().enabled);
    const auto candidate=menu.candidate();
    for(uint8_t i=0;i<YokeGains::Count;++i) CHECK(candidate.raw(YokeGains::Id(i))==gains.raw(YokeGains::Id(i)));
    CHECK(candidate.effects()==gains.effects());
    menu.finishSave(false); expectedRows("ERROR GUARDADO","OK: REINTENTAR");
    CHECK(record.enabled && !menu.savePending() && std::strstr(top,"ERROR GUARDADO") && std::strstr(bottom,"OK: REINTENTAR"));
    menu.press(Button::Ok); CHECK(menu.savingRecord()); menu.finishSave(true); expectedRows("GUARDADO","REGISTRO DATOS");
    CHECK(!record.enabled && menu.currentScreen()==Screen::BrowseRecord);
    CHECK(std::strstr(top,"GUARDADO") && std::strstr(bottom,"REGISTRO DATOS"));
    menu.update(2000,true); expectedRows("REGISTRO DATOS","* Desactivado");
    menu.press(Button::Ok); menu.press(Button::Plus); expectedRows("REGISTRO DATOS","Activado");
    menu.press(Button::Mode); CHECK(menu.currentScreen()==Screen::Browse && !record.enabled);
    for(int i=0;i<3;++i) menu.press(Button::Mode);
    menu.press(Button::Back); CHECK(!menu.active());
    Menu legacy(gains); legacy.update(0,true);
    const Screen oldCycle[]={Screen::Browse,Screen::BrowseEffects,Screen::BrowsePots,Screen::Browse};
    for(Screen screen:oldCycle) { legacy.press(Button::Mode); CHECK(legacy.currentScreen()==screen); }
}
static void calibrationChecks() {
    recordSettingChecks();
    using namespace YokeCalibration;
    using namespace YokeGains;
    Settings settings;
    Menu menu(settings);
    char top[17],bottom[17];
    CHECK(!menu.active());
    menu.press(Button::Ok); menu.press(Button::Plus); menu.press(Button::Back);
    CHECK(!menu.active());
    menu.update(0,true);
    menu.press(Button::Mode); CHECK(menu.active() && menu.currentScreen()==Screen::Browse);
    menu.rows(top,bottom);
    CHECK(std::strcmp(top,"MODO CALIBRACION")==0);
    CHECK(std::strcmp(bottom,"* Ganancia total")==0);
    menu.press(Button::Minus); CHECK(menu.selected()==Friction);
    menu.press(Button::Plus); CHECK(menu.selected()==Total);
    menu.press(Button::Ok); CHECK(menu.currentScreen()==Screen::Edit);
    menu.rows(top,bottom); CHECK(std::strstr(bottom,"90%")!=nullptr);
    menu.press(Button::Plus); CHECK(menu.editValue()==9500 && settings.raw(Total)==9000);
    menu.press(Button::Back); CHECK(menu.currentScreen()==Screen::Browse && settings.raw(Total)==9000);
    menu.press(Button::Ok); menu.press(Button::Plus); menu.press(Button::Ok);
    CHECK(menu.savePending() && settings.raw(Total)==9000);
    CHECK(menu.candidate().raw(Total)==9500);
    menu.press(Button::Back); CHECK(menu.savePending()); // no UI mutation during save
    menu.finishSave(false); CHECK(menu.currentScreen()==Screen::Edit && settings.raw(Total)==9000);
    menu.rows(top,bottom); CHECK(std::strstr(top,"ERROR GUARDADO")!=nullptr);
    menu.press(Button::Ok); menu.finishSave(true);
    CHECK(settings.raw(Total)==9500 && menu.currentScreen()==Screen::Browse);
    menu.press(Button::Ok); menu.press(Button::Ok); CHECK(!menu.savePending()); // no unchanged flash write
    menu.press(Button::Back); CHECK(!menu.active());

    // Pause interlock: boot, live flight, resuming with an unsaved edit, save notices.
    Settings gatedSettings; Menu gated(gatedSettings);
    gated.press(Button::Mode); CHECK(!gated.active() && gated.displayActive());
    gated.rows(top,bottom); CHECK(std::strstr(top,"PAUSE SIMULADOR")!=nullptr);
    gated.update(2000,false); CHECK(!gated.displayActive());
    gated.update(2001,true); gated.press(Button::Mode); CHECK(gated.active());
    gated.press(Button::Ok); gated.press(Button::Plus); gated.press(Button::Ok);
    CHECK(gated.savePending());
    gated.update(2002,false); CHECK(!gated.active() && !gated.savePending());
    gated.finishSave(true); CHECK(gatedSettings.raw(Total)==9000);
    gated.update(2003,true); gated.press(Button::Mode); gated.press(Button::Ok);
    gated.press(Button::Plus); gated.press(Button::Ok); gated.finishSave(true);
    gated.rows(top,bottom); CHECK(std::strstr(top,"GUARDADO")!=nullptr);
    gated.update(4003,true); gated.rows(top,bottom);
    CHECK(std::strstr(top,"MODO CALIBRACION")!=nullptr);
    gated.update(0xfffffff0UL,false); gated.press(Button::Mode);
    gated.update(1984,false); CHECK(!gated.displayActive());

    // Offline mode permits calibration; fresh unpaused/disarmed frames stay locked.
    YokeSC::Receiver pauseRx; uint8_t pausePacket[26];
    frame4(pausePacket,1,0,0,0); pausePacket[3]=0;
    YokeSC::put16(pausePacket+24,YokeSC::crc(pausePacket,24));
    feed4(pauseRx,pausePacket,0); CHECK(!pauseRx.calibrationAllowed(0,true));
    pausePacket[2]=5; pausePacket[3]=2; YokeSC::put16(pausePacket+4,2);
    YokeSC::put16(pausePacket+24,YokeSC::crc(pausePacket,24)); feed4(pauseRx,pausePacket,10);
    CHECK(pauseRx.calibrationAllowed(10,true) && !pauseRx.fresh(10));
    CHECK(pauseRx.calibrationAllowed(10,false));
    CHECK(pauseRx.calibrationAllowed(309,true) && pauseRx.calibrationAllowed(310,true));
    feed4(pauseRx,pausePacket,200); CHECK(pauseRx.calibrationAllowed(310,true) && !pauseRx.fresh(310)); // duplicate cannot rearm
    pausePacket[3]=3; YokeSC::put16(pausePacket+4,3);
    YokeSC::put16(pausePacket+24,YokeSC::crc(pausePacket,24)); feed4(pauseRx,pausePacket,310);
    CHECK(pauseRx.calibrationAllowed(310,true) && !pauseRx.fresh(310)); // invalid packet cannot rearm
    pausePacket[3]=1;
    YokeSC::put16(pausePacket+24,YokeSC::crc(pausePacket,24)); feed4(pauseRx,pausePacket,311);
    CHECK(!pauseRx.calibrationAllowed(311,true) && pauseRx.fresh(311));

    // Boot without a bridge; running reconnect cancels an offline draft.
    YokeSC::Receiver offlineRx;
    CHECK(offlineRx.calibrationAllowed(0,false));
    CHECK(offlineRx.calibrationAllowed(0,true));
    CHECK(!offlineRx.fresh(0));
    Settings offlineSettings; Menu offlineMenu(offlineSettings);
    offlineMenu.update(0,offlineRx.calibrationAllowed(0,false));
    offlineMenu.press(Button::Mode); CHECK(offlineMenu.active());
    offlineMenu.press(Button::Ok); offlineMenu.press(Button::Plus);
    offlineMenu.press(Button::Ok); CHECK(offlineMenu.savePending());
    pausePacket[2]=6; pausePacket[3]=1; YokeSC::put16(pausePacket+4,10);
    YokeSC::put16(pausePacket+24,YokeSC::crc(pausePacket,24)); feed4(offlineRx,pausePacket,100);
    offlineMenu.update(100,offlineRx.calibrationAllowed(100,true));
    CHECK(!offlineMenu.active() && !offlineMenu.savePending());
    CHECK(offlineSettings.raw(Total)==9000);
    CHECK(!offlineRx.calibrationAllowed(399,true) && offlineRx.fresh(399));
    CHECK(offlineRx.calibrationAllowed(400,true) && !offlineRx.fresh(400));
    CHECK(offlineRx.calibrationAllowed(101,false)); // USB link closed
    // Recent unarmed/unpaused state is NOT permission to edit (monitor/slew).
    pausePacket[3]=0; YokeSC::put16(pausePacket+4,11);
    YokeSC::put16(pausePacket+24,YokeSC::crc(pausePacket,24)); feed4(offlineRx,pausePacket,410);
    CHECK(!offlineRx.calibrationAllowed(410,true));

    // Every setting: label fits, +/- steps, bounds, edit/cancel and commit.
    const Id order[]={Total,CenterPitch,CenterRoll,CenterRest,GustPitch,GustRoll,
                      Ground,Stall,Gear,Spoiler,Damper,Friction};
    menu.press(Button::Mode);
    for(uint8_t i=0;i<Count;++i) {
        CHECK(menu.selected()==order[i]);
        CHECK(std::strlen(spec(order[i]).label)<=14);
        menu.rows(top,bottom); CHECK(std::strlen(top)==16 && std::strlen(bottom)==16);
        CHECK(std::strncmp(bottom+2,spec(order[i]).label,std::strlen(spec(order[i]).label))==0);
        const uint16_t previous=settings.raw(order[i]);
        menu.press(Button::Ok);
        for(int n=0;n<200;++n) menu.press(Button::Plus);
        CHECK(menu.editValue()==10000 && settings.raw(order[i])==previous);
        for(int n=0;n<200;++n) menu.press(Button::Minus);
        CHECK(menu.editValue()==0);
        menu.press(Button::Plus); CHECK(menu.editValue()==spec(order[i]).step);
        menu.press(Button::Ok);
        if(menu.savePending()) menu.finishSave(true);
        CHECK(settings.raw(order[i])==spec(order[i]).step);
        menu.press(Button::Plus);
    }
    CHECK(menu.selected()==Total);

    uint8_t record[recordSize]; encode(settings,record);
    Settings restored;
    CHECK(decode(record,restored));
    for(uint8_t i=0;i<Count;++i) CHECK(settings.raw(Id(i))==restored.raw(Id(i)));
    Menu reboot(restored); CHECK(!reboot.active()); // persisted gains, not persisted screen
    for(int bit=0;bit<recordSize*8;++bit) {
        uint8_t broken[recordSize]; std::memcpy(broken,record,recordSize);
        broken[bit/8]^=uint8_t(1<<(bit%8)); Settings old=restored;
        CHECK(!decode(broken,old));
        for(uint8_t i=0;i<Count;++i) CHECK(old.raw(Id(i))==restored.raw(Id(i)));
    }
    // Valid CRC but wrong schema / out-of-range values must also be rejected.
    uint8_t invalid[recordSize]; std::memcpy(invalid,record,recordSize);
    YokeSC::put16(invalid+4,YOKE_SETTINGS_VERSION+1);
    YokeSC::put16(invalid+recordSize-2,YokeSC::crc(invalid,recordSize-2));
    CHECK(!decode(invalid,restored));
    std::memcpy(invalid,record,recordSize); YokeSC::put16(invalid+8,10001);
    YokeSC::put16(invalid+recordSize-2,YokeSC::crc(invalid,recordSize-2));
    CHECK(!decode(invalid,restored));
    std::memset(invalid,255,recordSize); CHECK(!decode(invalid,restored));

    // Old EEPROM layout migrates without resetting tuned gains.
    uint8_t legacy[recordSize]={0};
    encode(settings,legacy); legacy[6]=Count-1; legacy[7]=0;
    YokeSC::put16(legacy+8+2*CenterPitch,7300);
    const uint8_t legacySize=10+2*(Count-1);
    YokeSC::put16(legacy+legacySize-2,YokeSC::crc(legacy,legacySize-2));
    Settings migrated; CHECK(decode(legacy,migrated));
    CHECK(migrated.raw(CenterPitch)==7300 && migrated.raw(CenterRoll)==3650);
    CHECK(migrated.raw(GustPitch)==settings.raw(GustPitch));
    for(uint8_t i=0;i<Count;++i) {
        CHECK(!migrated.set(Id(i),10001));
        CHECK(migrated.set(Id(i),10000));
    }
    DebouncedButton held;
    held.begin(false,0);
    CHECK(!held.update(true,1,true)); CHECK(held.update(true,31,true));
    CHECK(!held.update(true,1030,true)); CHECK(held.update(true,1031,true));
    CHECK(!held.update(true,1180,true)); CHECK(held.update(true,1181,true));
    CHECK(!held.update(false,1182,true)); CHECK(!held.update(false,1400,true));
    held.begin(true,0); CHECK(!held.update(true,2000,true)); // boot-held never repeats
    held.begin(false,0xfffffff0UL);
    CHECK(!held.update(true,0xfffffff1UL,true)); CHECK(held.update(true,15,true));
    CHECK(held.update(true,1015,true)); // delay across rollover
    CHECK(!held.update(true,1016,false)); // navigation/exit cancels repeat
    CHECK(!held.update(true,1017,true));
    DebouncedButton button;
    button.begin(false,0);
    CHECK(!button.update(true,1)); CHECK(!button.update(false,5)); CHECK(!button.update(true,10));
    CHECK(!button.update(true,39)); CHECK(button.update(true,40));
    CHECK(!button.update(true,5000)); // held: one event
    CHECK(!button.update(false,5010)); CHECK(!button.update(false,5040));
    CHECK(!button.update(true,5050)); CHECK(button.update(true,5080));
    button.begin(true,0); CHECK(!button.update(true,100)); // held during power-up stays normal
    button.begin(false,0xfffffff0UL); CHECK(!button.update(true,0xfffffff5UL));
    CHECK(button.update(true,25)); // millis wrap

    // Effects view: draft/cancel/save, failed flash, gains preserved, persistence.
    Settings toggled; Menu toggleMenu(toggled);
    toggleMenu.update(0,true);
    toggleMenu.press(Button::Mode); toggleMenu.press(Button::Mode);
    CHECK(toggleMenu.currentScreen()==Screen::BrowseEffects);
    toggleMenu.rows(top,bottom); CHECK(std::strstr(top,"EFECTOS")!=nullptr);
    for(uint8_t i=1;i<Count;++i) {
        const Id id=order[i]; const uint16_t savedGain=toggled.raw(id);
        CHECK(toggleMenu.selected()==id);
        toggleMenu.press(Button::Ok);
        toggleMenu.rows(top,bottom); CHECK(std::strstr(bottom,"Activado")!=nullptr);
        toggleMenu.press(Button::Minus);
        toggleMenu.rows(top,bottom); CHECK(std::strstr(bottom,"Desactivado")!=nullptr);
        CHECK(toggled.enabled(id)); // draft is not applied
        toggleMenu.press(Button::Back);
        CHECK(toggleMenu.currentScreen()==Screen::BrowseEffects && toggled.enabled(id));
        toggleMenu.press(Button::Ok); toggleMenu.press(Button::Minus); toggleMenu.press(Button::Ok);
        CHECK(toggleMenu.savePending() && !toggleMenu.candidate().enabled(id));
        toggleMenu.finishSave(false);
        CHECK(toggleMenu.currentScreen()==Screen::EditEffect && toggled.enabled(id));
        toggleMenu.press(Button::Ok); toggleMenu.finishSave(true);
        CHECK(!toggled.enabled(id) && toggled.raw(id)==savedGain);
        uint8_t saved[recordSize]; encode(toggled,saved); Settings loaded;
        CHECK(decode(saved,loaded) && !loaded.enabled(id) && loaded.raw(id)==savedGain);
        toggleMenu.press(Button::Ok); toggleMenu.press(Button::Plus); toggleMenu.press(Button::Ok);
        toggleMenu.finishSave(true); CHECK(toggled.enabled(id) && toggled.raw(id)==savedGain);
        toggleMenu.press(Button::Plus);
    }
    CHECK(toggleMenu.selected()==CenterPitch);
    toggleMenu.press(Button::Mode); CHECK(toggleMenu.currentScreen()==Screen::BrowsePots);
    toggleMenu.press(Button::Mode); CHECK(toggleMenu.currentScreen()==Screen::Browse);
    toggleMenu.press(Button::Mode); CHECK(toggleMenu.currentScreen()==Screen::BrowseEffects);
    toggleMenu.press(Button::Back); CHECK(!toggleMenu.active());
    toggleMenu.press(Button::Mode); toggleMenu.press(Button::Mode); toggleMenu.press(Button::Ok);
    toggleMenu.press(Button::Minus); toggleMenu.press(Button::Ok);
    toggleMenu.update(1,false); toggleMenu.finishSave(true);
    CHECK(!toggleMenu.active() && toggled.enabled(CenterPitch));
    CHECK(!toggled.setEnabled(Total,false));
    // Previous 12-gain format migrates with all effects enabled.
    encode(toggled,legacy); legacy[7]=0;
    YokeSC::put16(legacy+10+2*Count-2,YokeSC::crc(legacy,10+2*Count-2));
    CHECK(decode(legacy,migrated) && migrated.effects()==Settings::effectBits);
    encode(toggled,legacy);
    YokeSC::put16(legacy+8+2*Count,0xffff);
    YokeSC::put16(legacy+recordSize-2,YokeSC::crc(legacy,recordSize-2));
    CHECK(!decode(legacy,migrated)); // unknown bits, even with valid CRC

    // Three sections, four captures, atomic cancel/save and persistent ADC settings.
    YokePots::Settings potSettings;
    Settings potGains;
    Menu pots(potGains,&potSettings);
    pots.update(0,true);
    pots.press(Button::Mode); CHECK(pots.currentScreen()==Screen::Browse);
    pots.press(Button::Mode); CHECK(pots.currentScreen()==Screen::BrowseEffects);
    pots.press(Button::Mode); CHECK(pots.currentScreen()==Screen::BrowsePots);
    pots.rows(top,bottom); CHECK(std::strstr(top,"CALIBRAR POTES")!=nullptr);
    pots.press(Button::Ok); // no local ADC input (modes 0/1)
    CHECK(pots.currentScreen()==Screen::BrowsePots && !pots.savePending());
    pots.rows(top,bottom); CHECK(std::strstr(top,"REQUIERE MODO 2")!=nullptr);
    uint8_t originalPots[YokePots::recordSize], actualPots[YokePots::recordSize];
    YokePots::encode(potSettings,originalPots);
    for(int cancelStep=0;cancelStep<4;++cancelStep) {
        pots.potReadings(3500,200); pots.press(Button::Ok);
        CHECK(pots.currentScreen()==Screen::CalibratePots && pots.calibrationStep()==0);
        for(int step=0;step<cancelStep;++step) {
            pots.potReadings(step==3 ? 300 : 3500,step==0 ? 200 : 3800);
            pots.press(Button::Ok);
            CHECK(!pots.savePending());
        }
        pots.press(Button::Back);
        CHECK(pots.currentScreen()==Screen::BrowsePots && !pots.savePending());
        YokePots::encode(potSettings,actualPots);
        CHECK(std::memcmp(originalPots,actualPots,sizeof(actualPots))==0);
    }
    pots.press(Button::Ok);
    pots.update(0,true); pots.rows(top,bottom);
    CHECK(std::strstr(top,"CALIBRAR PITCH") && std::strstr(bottom,"GIRAR ANTIHOR."));
    pots.update(1500,true); pots.rows(top,bottom); CHECK(std::strstr(bottom,"HASTA EL FINAL"));
    pots.update(3000,true); pots.rows(top,bottom); CHECK(std::strstr(bottom,"PRESIONAR OK"));
    CHECK(std::strlen(top)==16 && std::strlen(bottom)==16);
    pots.potReadings(3500,200); pots.press(Button::Ok);
    pots.rows(top,bottom); CHECK(std::strstr(bottom,"GIRAR HORARIO"));
    pots.potReadings(3500,220); pots.press(Button::Ok);
    CHECK(pots.calibrationStep()==1 && !pots.savePending());
    pots.rows(top,bottom); CHECK(std::strstr(bottom,"POCO RECORRIDO"));
    pots.potReadings(3500,3800); pots.press(Button::Ok);
    CHECK(pots.calibrationStep()==2);
    pots.rows(top,bottom); CHECK(std::strstr(top,"CALIBRAR ROLL"));
    pots.press(Button::Ok); CHECK(pots.calibrationStep()==3);
    pots.potReadings(300,3800); pots.press(Button::Ok);
    CHECK(pots.savePending() && pots.savingPots());
    YokePots::Settings captured=pots.potCandidate();
    CHECK(captured.axes[1].negative==200 && captured.axes[1].center==2000 && captured.axes[1].positive==3800);
    CHECK(captured.axes[0].negative==3500 && captured.axes[0].center==1900 && captured.axes[0].positive==300);
    CHECK(pots.candidate().raw(Total)==potGains.raw(Total));
    pots.finishSave(false); CHECK(!pots.savePending());
    pots.rows(top,bottom); CHECK(std::strstr(top,"ERROR GUARDADO"));
    YokePots::encode(potSettings,actualPots); CHECK(std::memcmp(originalPots,actualPots,sizeof(actualPots))==0);
    pots.potReadings(1000,1000); pots.press(Button::Ok); // retry keeps confirmed endpoints
    CHECK(pots.potCandidate().axes[0].positive==300 && pots.savePending());
    pots.finishSave(true);
    CHECK(pots.currentScreen()==Screen::BrowsePots && !pots.savePending());
    pots.rows(top,bottom); CHECK(std::strstr(top,"POTES GUARDADOS"));
    CHECK(potSettings.axes[0].center==1900 && potSettings.axes[1].center==2000);
    CHECK(potGains.raw(Total)==9000 && potGains.effects()==Settings::effectBits);
    YokePots::encode(potSettings,actualPots);
    YokePots::Settings loadedPots;
    CHECK(YokePots::decode(actualPots,loadedPots));
    CHECK(loadedPots.axes[0].negative==3500 && loadedPots.axes[1].positive==3800);
    for(int bit=0;bit<YokePots::recordSize*8;++bit) {
        uint8_t broken[YokePots::recordSize]; std::memcpy(broken,actualPots,sizeof(broken));
        broken[bit/8]^=uint8_t(1<<(bit%8));
        CHECK(!YokePots::decode(broken,loadedPots));
        CHECK(loadedPots.axes[0].center==1900 && loadedPots.axes[1].center==2000);
    }
    uint8_t badPots[YokePots::recordSize];
    std::memset(badPots,255,sizeof(badPots)); CHECK(!YokePots::decode(badPots,loadedPots));
    YokePots::Settings badSettings=potSettings; badSettings.axes[0]={100,110,120};
    YokePots::encode(badSettings,badPots); CHECK(!YokePots::decode(badPots,loadedPots));
    badSettings.axes[0]={100,2000,4096};
    YokePots::encode(badSettings,badPots); CHECK(!YokePots::decode(badPots,loadedPots));
    PotentiometerPosition runtime(0,2048,4095);
    runtime.update(4095);
    runtime.configure(loadedPots.axes[0].negative,loadedPots.axes[0].center,loadedPots.axes[0].positive);
    CHECK(runtime.update(1900)==0); // new calibration resets ADC hysteresis
    // Mode changes and resuming simulator also discard the whole wizard.
    pots.press(Button::Ok); pots.potReadings(700,800); pots.press(Button::Ok);
    pots.press(Button::Mode); CHECK(pots.currentScreen()==Screen::Browse);
    pots.press(Button::Mode); pots.press(Button::Mode); pots.press(Button::Ok);
    CHECK(pots.calibrationStep()==0);
    pots.press(Button::Ok); pots.update(4000,false); pots.finishSave(true);
    CHECK(!pots.active() && !pots.savePending() && potSettings.axes[1].negative==200);
    pots.update(4001,true);
    pots.press(Button::Mode); pots.press(Button::Mode); pots.press(Button::Mode);
    pots.press(Button::Back); CHECK(!pots.active());

    // Runtime settings actually drive each effect, with existing rails unchanged.
    Settings live;
    YokeSC::Effects fx(live); YokeSC::Receiver rx;
    uint8_t packet[26];
    for(uint32_t now=0;now<=1000;now+=2) {
        if(now%20==0) {
            frame4(packet,uint16_t(now/20+1),10000,5000,5000);
            for(int off=12;off<=18;off+=2) YokeSC::put16(packet+off,10000);
            YokeSC::put16(packet+8,10000); YokeSC::put16(packet+10,10000);
            YokeSC::put16(packet+24,YokeSC::crc(packet,24)); feed4(rx,packet,now);
        }
        fx.update(rx,now,true);
    }
    for(uint8_t i=0;i<Count;++i) live.set(Id(i),0);
    CHECK(fx.command(rx,1,1,YokeSC::Centering,1)==0);
    live.set(CenterPitch,8000); CHECK(fx.command(rx,1,1,YokeSC::Centering,1)<-.79f);
    live.set(CenterPitch,4000); CHECK(fx.command(rx,1,1,YokeSC::Centering,1)>-.401f);
    CHECK(fx.command(rx,0,1,YokeSC::Centering,1)==0);
    live.set(CenterRoll,2500);
    CHECK(std::fabs(fx.command(rx,0,1,YokeSC::Centering,1)+.25f)<.001f);
    const float pitchBefore=fx.command(rx,1,1,YokeSC::Centering,1);
    live.set(CenterRoll,7000);
    CHECK(std::fabs(fx.command(rx,0,1,YokeSC::Centering,1)+.7f)<.001f);
    CHECK(near(fx.command(rx,1,1,YokeSC::Centering,1),pitchBefore));
    live.set(Total,0); CHECK(fx.command(rx,1,live.get(Total),YokeSC::Centering,1)==0);
    live.set(GustRoll,2000); CHECK(near(fx.command(rx,0,1,YokeSC::Turbulence),.2f));
    live.set(GustPitch,6000); CHECK(near(fx.command(rx,1,1,YokeSC::Turbulence),.6f));
    live.set(Damper,10000); live.set(Friction,10000);
    CHECK(YokeSC::motionAdjustedForce(-.2f,10,1,true,true,live)<-.2f);
    live.set(Damper,0); live.set(Friction,0);
    CHECK(near(YokeSC::motionAdjustedForce(-.2f,10,1,true,true,live),-.2f));
    const Id envelopeIds[]={Stall,Gear,Spoiler,Ground};
    const uint8_t masks[]={YokeSC::Stall,YokeSC::Gear,YokeSC::Spoiler,YokeSC::Ground};
    float peaks[4]={0,0,0,0};
    for(uint8_t i=0;i<4;++i) live.set(envelopeIds[i],10000);
    for(uint32_t now=1002;now<1298;now+=2) {
        fx.update(rx,now,true);
        for(uint8_t i=0;i<4;++i) {
            const float active=fx.command(rx,0,1,masks[i]);
            if(std::fabs(active)>peaks[i]) peaks[i]=std::fabs(active);
            live.setEnabled(envelopeIds[i],false);
            CHECK(fx.command(rx,0,1,masks[i])==0 && live.raw(envelopeIds[i])==10000);
            live.setEnabled(envelopeIds[i],true);
            CHECK(near(active,fx.command(rx,0,1,masks[i])));
            live.set(envelopeIds[i],0); CHECK(fx.command(rx,0,1,masks[i])==0);
            live.set(envelopeIds[i],10000);
            CHECK(std::fabs(active)<=SIMCONNECT_EXTRA_LIMIT);
            if(i==3) CHECK(std::fabs(active)*255<=40.0001f);
        }
    }
    for(uint8_t i=0;i<4;++i) CHECK(peaks[i]>.1f);
    live.setEnabled(CenterRoll,false);
    CHECK(fx.command(rx,0,1,YokeSC::Centering,1)==0);
    CHECK(fx.command(rx,1,1,YokeSC::Centering,1)<0);
    live.setEnabled(CenterPitch,false);
    CHECK(fx.command(rx,1,1,YokeSC::Centering,1)==0);
    live.setEnabled(GustRoll,false); CHECK(fx.command(rx,0,1,YokeSC::Turbulence)==0);
    CHECK(fx.command(rx,1,1,YokeSC::Turbulence)>0);
    live.setEnabled(GustPitch,false); CHECK(fx.command(rx,1,1,YokeSC::Turbulence)==0);
    live.set(Damper,1200); live.set(Friction,300);
    live.setEnabled(Damper,false); live.setEnabled(Friction,false);
    CHECK(fx.command(rx,1,1,YokeSC::Damper|YokeSC::Friction,0,2)==0);
    live.setEnabled(Damper,true); CHECK(fx.command(rx,1,1,YokeSC::Damper,0,2)<0);
    live.setEnabled(Friction,true);
#if YOKE_FRICTION_POSITION_MODEL
    fx.command(rx,1,1,YokeSC::Friction,0,0);
    CHECK(fx.command(rx,1,1,YokeSC::Friction,-.05f,0)>0);
#else
    CHECK(fx.command(rx,1,1,YokeSC::Friction,0,-2)>0);
#endif
    live.set(CenterRest,1500); live.setEnabled(CenterRest,false);
    CHECK(live.effective(CenterRest)==0 && live.raw(CenterRest)==1500);
    fx.reset(); CHECK(fx.command(rx,1,1,YokeSC::Centering,1)==0);
}
#endif
