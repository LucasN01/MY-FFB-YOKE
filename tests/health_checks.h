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
        CHECK((encode(stages[i]) & 0xFF00u)==0);
        CHECK(std::strcmp(stageName(stage),names[i])==0);
    }
    uint8_t stage=0; bool fault=false;
    CHECK(decode(encode(4)|0x80,stage,fault));
    CHECK(stage==4 && fault);
    CHECK(!decode(0,stage,fault));
    CHECK(decode(0x59C80104u,stage,fault) && stage==4 && !fault);
    CHECK(decodeException(0x59C80104u)==1);
    CHECK(decode(encode(TestHang)|kFaultBit|0xFF00u,stage,fault) && stage==TestHang && fault);
    CHECK(decodeException(encode(TestHang)|kFaultBit|0xFF00u)==255);
    CHECK(!decode(0x59C90104u,stage,fault));
    char line[160];
    const char wdt[]="BOOT reset=WDT wd=0x1 pm=0x0 etapa=4:EFECTOS falla=0\n";
    CHECK(formatBoot(line,sizeof(line),4,0x1,0x0,true,4,false,255,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu)==int(sizeof(wdt)-1));
    CHECK(std::strcmp(line,wdt)==0);
    const char power[]="BOOT reset=ENCENDIDO wd=0x0 pm=0x1234\n";
    CHECK(formatBoot(line,sizeof(line),1,0,0x1234,false,4,true,0,0,0,0)==int(sizeof(power)-1));
    CHECK(std::strcmp(line,power)==0);
    CHECK(std::strstr(line,"etapa")==nullptr && std::strstr(line,"falla")==nullptr);
    CHECK(formatBoot(line,sizeof(line),4,1,0,true,15,true,0,0x10001234u,0x10001235u,0x8200u)>0);
    CHECK(std::strcmp(line,"BOOT reset=WDT wd=0x1 pm=0x0 etapa=15:PRUEBA falla=1 exc=0 pc=0x10001234 lr=0x10001235 cfsr=0x00008200\n")==0);
    const uint32_t validPcs[]={0x10000000u,0x103FFFFFu,0x20000000u,0x20081FFFu};
    for(uint32_t pc:validPcs) {
        CHECK(formatBoot(line,sizeof(line),4,1,0,true,15,true,0,pc,0,0)>0);
        CHECK(std::strstr(line,"pc_invalido")==nullptr);
    }
    const uint32_t invalidPcs[]={0,0x0FFFFFFFu,0x10400000u,0x1FFFFFFFu,0x20082000u,0xFFFFFFFFu};
    for(uint32_t pc:invalidPcs) {
        CHECK(formatBoot(line,sizeof(line),4,1,0,true,15,true,0,pc,0,0)>0);
        CHECK(std::strstr(line," pc_invalido=1\n")!=nullptr);
    }
    const int maxLength=formatBoot(line,sizeof(line),0,0xFFFFFFFFu,0xFFFFFFFFu,true,Between,true,
        255,0xFFFFFFFFu,0xFFFFFFFFu,0xFFFFFFFFu);
    CHECK(maxLength>0 && maxLength<159 && std::strlen(line)==size_t(maxLength));
    CHECK(formatBoot(line,8,4,1,0,true,15,true,0,0,0,0)==-1);
    CHECK(formatBoot(line,8,4,1,0,true,4,false,0,0,0,0)==-1);
    CHECK(formatBoot(line,sizeof(wdt)-1,4,1,0,true,4,false,0,0,0,0)==-1);
    CHECK(formatBoot(line,sizeof(wdt),4,1,0,true,4,false,0,0,0,0)==int(sizeof(wdt)-1));
    CHECK(formatBoot(line,0,4,1,0,true,4,false,0,0,0,0)==-1);
}
