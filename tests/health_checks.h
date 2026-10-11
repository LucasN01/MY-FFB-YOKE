#pragma once
#include "../LoopSupervisor.h"

static void healthChecks() {
    using namespace YokeHealth;
    const uint8_t stages[]={Idle,SerialRx,Calibration,FlashSave,Effects,Motion,Force,
        Motor,ModelLog,DebugLog,Lcd,PotRead,SerialTx,TestHang};
    const char* names[]={"INACTIVO","SERIE_RX","CALIB","FLASH","EFECTOS","MOVIM",
        "FUERZA","MOTOR","LOGMOD","LOGDBG","LCD","POTES","SERIE_TX","PRUEBA"};
    for(unsigned i=0;i<sizeof(stages)/sizeof(stages[0]);++i) {
        uint8_t stage=127; bool fault=true;
        CHECK(decode(encode(stages[i]),stage,fault));
        CHECK(stage==stages[i] && !fault);
        CHECK(std::strcmp(stageName(stage),names[i])==0);
    }
    uint8_t stage=0; bool fault=false;
    CHECK(decode(encode(4)|0x80,stage,fault));
    CHECK(stage==4 && fault);
    CHECK(!decode(0,stage,fault));
    CHECK(!decode(0x59C80104u,stage,fault));
    char line[128];
    const char wdt[]="BOOT reset=WDT wd=0x1 pm=0x0 etapa=4:EFECTOS falla=0\n";
    CHECK(formatBoot(line,sizeof(line),4,0x1,0x0,true,4,false)==int(sizeof(wdt)-1));
    CHECK(std::strcmp(line,wdt)==0);
    const char power[]="BOOT reset=ENCENDIDO wd=0x0 pm=0x1234\n";
    CHECK(formatBoot(line,sizeof(line),1,0,0x1234,false,4,true)==int(sizeof(power)-1));
    CHECK(std::strcmp(line,power)==0);
    CHECK(std::strstr(line,"etapa")==nullptr && std::strstr(line,"falla")==nullptr);
    CHECK(formatBoot(line,sizeof(line),4,1,0,true,15,true)>0);
    CHECK(std::strcmp(line,"BOOT reset=WDT wd=0x1 pm=0x0 etapa=15:PRUEBA falla=1\n")==0);
    CHECK(formatBoot(line,8,4,1,0,true,4,false)==-1);
    CHECK(formatBoot(line,sizeof(wdt)-1,4,1,0,true,4,false)==-1);
    CHECK(formatBoot(line,sizeof(wdt),4,1,0,true,4,false)==int(sizeof(wdt)-1));
    CHECK(formatBoot(line,0,4,1,0,true,4,false)==-1);
}
