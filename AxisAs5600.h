#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "As5600Position.h"
#include "YokeI2C.h"

class AxisAs5600 {
public:
    void begin() {
        Wire1.setSDA(ROLL_AS5600_SDA_PIN);
        Wire1.setSCL(ROLL_AS5600_SCL_PIN);
        Wire1.begin();
        Wire1.setClock(YOKE_I2C1_HZ);
        Wire1.setTimeout(1, true);
        YokeI2C::configureBus0();
        const uint32_t now=millis();
        for(uint8_t axis=0;axis<2;++axis) {
            sensors[axis].start(now);
            confOk[axis]=writeConf(axis) && checkConf(axis);
        }
        lastConfMs=now;
    }
    void update(uint32_t nowMs) {
        for(uint8_t axis=0;axis<2;++axis) {
            uint8_t bytes[3];
            lastI2cOk[axis]=readRegisters(axis,0x0B,bytes,3);
            if(lastI2cOk[axis]) {
                const YokeAs5600::Frame frame=YokeAs5600::decode(bytes[0],bytes[1],bytes[2]);
                readings[axis]=frame.raw;
                status[axis]=frame.status;
            }
            const bool ok=lastI2cOk[axis] &&
                (!YOKE_AS5600_REQUIRE_MD || (status[axis] & YokeAs5600::StatusMD));
            sensors[axis].update(ok,readings[axis],nowMs);
        }
        if(uint32_t(nowMs-lastConfMs)>=1000) {
            lastConfMs=nowMs;
            const uint8_t axis=confAxis;
            confAxis^=1;
            if(lastI2cOk[axis]) {
                confOk[axis]=checkConf(axis);
                if(!confOk[axis]) confOk[axis]=writeConf(axis) && checkConf(axis);
            }
        }
    }
    bool healthy() const { return sensors[0].healthy() && sensors[1].healthy(); }
    uint16_t raw(uint8_t axis) const { return readings[axis]; }
    float position(uint8_t axis) const { return sensors[axis].position(); }
    const char* statusText(uint8_t axis) const {
        if(sensors[axis].locked()) return "BLQ";
        if(!lastI2cOk[axis]) return "I2C";
        if(!(status[axis] & YokeAs5600::StatusMD)) return "SIN";
        if(status[axis] & YokeAs5600::StatusML) return "DEB";
        if(status[axis] & YokeAs5600::StatusMH) return "FUE";
        return "OK";
    }
    uint8_t agc(uint8_t axis) {
        uint8_t value=0;
        if(!sensors[axis].healthy() || !readRegisters(axis,0x1A,&value,1)) return 0;
        return value;
    }
    void excuseGap() { sensors[0].excuseGap(); sensors[1].excuseGap(); }
private:
    TwoWire& bus(uint8_t axis) { return axis==0 ? Wire1 : Wire; }
    bool readRegisters(uint8_t axis,uint8_t reg,uint8_t* bytes,uint8_t count) {
        TwoWire& wire=bus(axis);
        wire.beginTransmission(AS5600_I2C_ADDRESS);
        wire.write(reg);
        if(wire.endTransmission(false)!=0) return false;
        if(wire.requestFrom(uint8_t(AS5600_I2C_ADDRESS),count)!=count) return false;
        for(uint8_t i=0;i<count;++i) bytes[i]=wire.read();
        return true;
    }
    bool writeConf(uint8_t axis) {
        TwoWire& wire=bus(axis);
        wire.beginTransmission(AS5600_I2C_ADDRESS);
        wire.write(0x07);
        wire.write(YOKE_AS5600_CONF >> 8);
        wire.write(YOKE_AS5600_CONF & 0xFF);
        return wire.endTransmission()==0;
    }
    bool checkConf(uint8_t axis) {
        uint8_t bytes[2];
        return readRegisters(axis,0x07,bytes,2) &&
            (((uint16_t(bytes[0]) << 8 | bytes[1]) & 0x3FFF)==(YOKE_AS5600_CONF & 0x3FFF));
    }
    YokeAs5600::Axis sensors[2]={
        {ROLL_AS5600_CENTER_RAW,ROLL_AS5600_COUNTS_NEG,ROLL_AS5600_COUNTS_POS,ROLL_AS5600_DIRECTION},
        {PITCH_AS5600_CENTER_RAW,PITCH_AS5600_COUNTS_NEG,PITCH_AS5600_COUNTS_POS,PITCH_AS5600_DIRECTION}
    };
    uint16_t readings[2]={0,0};
    uint8_t status[2]={0,0};
    bool lastI2cOk[2]={false,false}, confOk[2]={false,false};
    uint32_t lastConfMs=0;
    uint8_t confAxis=0;
};
