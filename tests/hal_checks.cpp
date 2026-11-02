#include "../include/hal/hal.h"
#include "../include/hal/pwm_plan.h"
#include "../include/hal/usb_descriptor_data.h"
#include <array>
#include <deque>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits>
#include <type_traits>

#include "hal_fake.h"
#include "../include/MotorDriver.h"
#include "../include/AxisEncoders.h"
#include "../include/AxisPotentiometers.h"
#include "../include/CalibrationControls.h"
#include "../include/LcdTelemetry.h"
#include "../include/AxisAs5600.h"
#include "../include/UsbGuard.h"

static unsigned checks = 0, failures = 0;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; \
    std::printf("FAIL line %d: %s\n", __LINE__, #expr); } } while (0)

static void checkPwm() {
    const auto p = planPwm(150000000, 490, 255);
    CHECK(p.scale == 2040); CHECK(p.shiftUp == 4); CHECK(p.shiftDown == 1);
    CHECK(p.divInt == 150); CHECK(p.divFrac == 1);
    const int values[] = {0, 1, 128, 220, 255, -5, 300};
    const uint32_t levels[] = {0, 8, 1024, 1760, 2040, 0, 2040};
    for (size_t i = 0; i < 7; ++i) CHECK(pwmLevel(p, values[i]) == levels[i]);
    const auto q = planPwm(150000000, 1000, 255);
    CHECK(q.scale == 1020); CHECK(q.shiftUp == 3); CHECK(q.shiftDown == 1);
    CHECK(pwmLevel(p, (std::numeric_limits<int>::max)()) == 2040);
    CHECK(pwmLevel(p, (std::numeric_limits<int>::min)()) == 0);
}
static void checkReset() {
    using hal::ResetReason;
    using hal::decodeResetReason;
    CHECK(decodeResetReason(false, false, 0, 0) == ResetReason::Unknown);
    CHECK(decodeResetReason(true, true, 0, 0) == ResetReason::Wdt);
    CHECK(decodeResetReason(false, false, 1, 0) == ResetReason::Soft);
    CHECK(decodeResetReason(false, false, 0, 0x10000) == ResetReason::PowerOn);
    CHECK(decodeResetReason(false, false, 0, 0x40000) == ResetReason::RunPin);
    CHECK(decodeResetReason(false, false, 0, 0x80000) == ResetReason::Debug);
    CHECK(decodeResetReason(false, false, 0, 0x200000) == ResetReason::Debug);
    CHECK(decodeResetReason(false, false, 0, 0x8000000) == ResetReason::Debug);
    CHECK(decodeResetReason(false, false, 0, 0x4000000) == ResetReason::Glitch);
    CHECK(decodeResetReason(false, false, 0, 0x20000) == ResetReason::Brownout);
    CHECK(decodeResetReason(true, true, 1, 0xFFFFFFFF) == ResetReason::Wdt);
    CHECK(decodeResetReason(false, false, 1, 0xFFFFFFFF) == ResetReason::Soft);
    CHECK(decodeResetReason(false, false, 0, 0x50000) == ResetReason::PowerOn);
    CHECK(decodeResetReason(true, false, 0, 0) == ResetReason::Unknown);
    CHECK(decodeResetReason(false, true, 0, 0) == ResetReason::Unknown);
}
static void checkEeprom() {
    using namespace hal_fake;
    EepromImage<FlashBackend> image;
    CHECK(kEepromFlashOffset == 0x3FE000);
    CHECK(!image.commit()); CHECK(image.read(0) == 0);
    for (size_t i = 0; i < flash.size(); ++i) flash[i] = uint8_t(i);
    image.begin(256); CHECK(readBytes == 256);
    for (int i = 0; i < 256; ++i) CHECK(image.read(i) == uint8_t(i));
    CHECK(image.read(-1) == 0); CHECK(image.read(256) == 0);
    image.write(-1, 17); image.write(256, 17); image.write(42, 42);
    CHECK(image.commit()); CHECK(erases == 0); CHECK(programs == 0);
    image.write(42, 17); CHECK(image.commit());
    CHECK(erases == 1); CHECK(programs == 1);
    CHECK(erasedBytes == 4096); CHECK(programmedBytes == 256);
    CHECK(flash[256] == 0xFF); CHECK(flash[4095] == 0xFF);
    CHECK(image.commit()); CHECK(erases == 1);
    image.begin(256); CHECK(image.read(42) == 17);
    image.write(43, 18); flashFail = true;
    CHECK(!image.commit()); CHECK(flash[43] == 43); CHECK(erases == 1);
    flashFail = false; CHECK(image.commit()); CHECK(flash[43] == 18);
    CHECK(erases == 2); CHECK(programs == 2);
    image.begin(0); CHECK(readBytes == 4096); CHECK(image.read(4095) == 0xFF);
    CHECK(image.read(4096) == 0);
    image.begin(5000); CHECK(readBytes == 4096);
    image.begin(300); CHECK(readBytes == 512);
    image.write(511, 0xA5); image.write(512, 0xA5); CHECK(image.commit());
    CHECK(programmedBytes == 512); CHECK(flash[511] == 0xA5); CHECK(flash[512] == 0xFF);
    hal::eeprom.begin(256); CHECK(hal::eeprom.read(42) == 17);
}
static void checkDescriptors() {
    // Independent golden byte streams: design 4.7, CDC template of TinyUSB 0.20.1.
    const uint8_t device[18] = {0x12,0x01,0x00,0x02,0xEF,0x02,0x01,0x40,
        0x8A,0x2E,0x0F,0x00,0x00,0x01,0x01,0x02,0x03,0x01};
    const uint8_t config[75] = {
        0x09,0x02,0x4B,0x00,0x02,0x01,0x00,0xA0,0x32,
        0x08,0x0B,0x00,0x02,0x02,0x02,0x00,0x00,
        0x09,0x04,0x00,0x00,0x01,0x02,0x02,0x00,0x04,
        0x05,0x24,0x00,0x20,0x01, 0x05,0x24,0x01,0x00,0x01,
        0x04,0x24,0x02,0x06, 0x05,0x24,0x06,0x00,0x01,
        0x07,0x05,0x81,0x03,0x08,0x00,0x01,
        0x09,0x04,0x01,0x00,0x02,0x0A,0x00,0x00,0x00,
        0x07,0x05,0x01,0x02,0x40,0x00,0x00,
        0x07,0x05,0x82,0x02,0x40,0x00,0x00};
    CHECK(sizeof(hal::usbDeviceDescriptor) == sizeof(device));
    CHECK(sizeof(hal::usbConfigurationDescriptor) == sizeof(config));
    CHECK(std::memcmp(device, hal::usbDeviceDescriptor, sizeof(device)) == 0);
    CHECK(std::memcmp(config, hal::usbConfigurationDescriptor, sizeof(config)) == 0);
    const uint8_t id[8] = {0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF};
    char serial[17]; hal::usbSerialString(id, serial);
    CHECK(std::strcmp(serial, "0123456789ABCDEF") == 0);
    CHECK(hal::usbLanguageId == 0x0409);
    CHECK(std::strcmp(hal::usbManufacturer, "Raspberry Pi") == 0);
    CHECK(std::strcmp(hal::usbProduct, "Pico 2") == 0);
    CHECK(std::strcmp(hal::usbCdcName, "TinyUSB Serial") == 0);
}
static void checkConstrain() {
    CHECK(hal::constrain(0.5f, -1, 1) == 0.5f);
    CHECK(hal::constrain(2, -1, 1) == 1);
    CHECK(hal::constrain(-2, -1, 1) == -1);
    CHECK(std::isnan(hal::constrain(std::numeric_limits<float>::quiet_NaN(), -1, 1)));
    CHECK(hal::constrain(int16_t(300), 0, 255) == 255);
    static_assert(std::is_same<decltype(hal::constrain(int16_t(300), 0, 255)), int>::value,
                  "Arduino ternary promotions must be preserved");
}
static void checkFake() {
    using namespace hal_fake;
    timeUs = 0x100000001ULL; CHECK(hal::micros() == 1); CHECK(hal::millis() == 4294967);
    events.clear(); hal::pwmSetup(490, 255);
    hal::pinMode(6, hal::PinMode::Output); hal::pwmWrite(6, 0); hal::digitalWrite(10, true);
    CHECK(events.size() == 4);
    CHECK(events[0] == "pwmSetup:490:255"); CHECK(events[1] == "pinMode:6:2");
    CHECK(events[2] == "pwmWrite:6:0"); CHECK(events[3] == "digitalWrite:10:1");
    CHECK(modes[6] == hal::PinMode::Output); CHECK(pwm[6] == 0); CHECK(hal::digitalRead(10));
    hal::pwmWrite(6, 220); CHECK(pwm[6] == 1760);
    unsigned edges = 0;
    hal::attachChangeInterrupt(4, [](void* ctx) { ++*static_cast<unsigned*>(ctx); }, &edges);
    setInput(4, true); setInput(4, false); CHECK(edges == 2);
    const auto saved = hal::irqSave(); setInput(4, true); CHECK(edges == 2);
    hal::irqRestore(saved); setInput(4, false); CHECK(edges == 3);
    hal::attachChangeInterrupt(4, nullptr, nullptr);
    adc[26] = {123, 4095}; CHECK(hal::analogRead12(26) == 123);
    CHECK(hal::analogRead12(26) == 4095); CHECK(hal::analogRead12(25) == 0);
    auto& bus = hal::wire1;
    CHECK(bus.setSDA(14)); CHECK(bus.setSCL(15)); CHECK(!bus.setSDA(4));
    bus.begin(); CHECK(bus.setSDA(14)); CHECK(!bus.setSDA(26));
    bus.setClock(400000); bus.setTimeout(1, true);
    CHECK(bus.write(1) == 0); bus.beginTransmission(0x36); CHECK(bus.write(0x0C) == 1);
    CHECK(bus.endTransmission(false) == 0); CHECK(i2cTx[1] == std::vector<uint8_t>{0x0C});
    CHECK(events[events.size()-2] == "i2cWrite:1:54"); CHECK(events.back() == "i2cStop:1:0");
    i2cRx[1] = {0x0A, 0xBC}; CHECK(bus.requestFrom(0x36, 2) == 2);
    CHECK(events[events.size()-2] == "i2cRead:1:54"); CHECK(events.back() == "i2cStop:1:1");
    CHECK(bus.available() == 2); CHECK(bus.read() == 0x0A); CHECK(bus.read() == 0xBC);
    CHECK(bus.read() == -1); CHECK(bus.available() == 0);
    bus.beginTransmission(0x36); i2cResults[1].push_back(2); CHECK(bus.endTransmission() == 2);
    bus.beginTransmission(0x36); i2cResults[1].push_back(5); CHECK(bus.endTransmission() == 5);
    CHECK(bus.requestFrom(0x36, 33) == 0); bus.end(); CHECK(bus.requestFrom(0x36, 1) == 0);
    hal::usb::init(); hal::usb::lock(); CHECK(usbInitialized); CHECK(usbLocks == 1);
    hal::usb::unlock(); CHECK(usbLocks == 0); hal::usb::background(); CHECK(usbBackgroundCalls == 1);
    usbRx = {42}; CHECK(hal::usbSerial.available() == 1); CHECK(hal::usbSerial.read() == 42);
    CHECK(hal::usbSerial.read() == -1); CHECK(!hal::usbSerial.dtr());
    CHECK(hal::usbSerial.write("abc", 3) == 0); CHECK(usbTx.empty());
    usbDtr = true; usbCapacity = 2; CHECK(hal::usbSerial.write("abc", 3) == 2);
    CHECK(hal::usbSerial.availableForWrite() == 0); CHECK(usbTx[0] == 'a' && usbTx[1] == 'b');
    hal::watchdogBegin(250); hal::watchdogFeed(); CHECK(watchdogMs == 250); CHECK(watchdogFeeds == 1);
    reset = hal::ResetReason::RunPin; CHECK(hal::resetReason() == hal::ResetReason::RunPin);
}

static void checkHardwareModules() {
    using namespace hal_fake;
    hal::pwmSetup(490,255); events.clear();
    MotorDriver motor(6,7,10); motor.begin();
    CHECK(events.size()==8);
    CHECK(events[2]=="pwmWrite:6:0" && events[3]=="pwmWrite:7:0");
    CHECK(events[5]=="digitalWrite:10:1");
    motor.setCommand(0.5f,255);
    CHECK(motor.getLastAppliedPWM()==128); CHECK(pwm[7]==1024 && pwm[6]==0);
    motor.setCommand(-1,255);
    CHECK(motor.getLastAppliedPWM()==-220); CHECK(pwm[6]==1760 && pwm[7]==0);
    const auto before=pwm; hal::pwmWrite(30,255); hal::pwmWrite(255,255);
    CHECK(pwm==before);

    PicoEncoder encoder(0,1); encoder.begin();
    setInput(0,false); setInput(1,false); encoder.write(0);
    setInput(1,true); setInput(0,true); setInput(1,false); setInput(0,false);
    CHECK(encoder.read()==4);
    encoder.write(0);
    setInput(0,true); setInput(1,true); setInput(0,false); setInput(1,false);
    CHECK(encoder.read()==-4);
    hal::attachChangeInterrupt(0,nullptr,nullptr); hal::attachChangeInterrupt(1,nullptr,nullptr);
    { UsbLock lock; CHECK(usbLocks==1); } CHECK(usbLocks==0);

    auto& bus=hal::wire0; bus.begin(); bus.beginTransmission(0x27); bus.write(7);
    bus.begin(); CHECK(bus.write(8)==0); CHECK(bus.endTransmission()==4);
    i2cRx[0]={1,2}; CHECK(bus.requestFrom(0x27,2)==2);
    bus.begin(); CHECK(bus.available()==0); CHECK(bus.read()==-1);
    events.clear(); bus.beginTransmission(0x27); CHECK(bus.endTransmission()==0);
    CHECK(events[0]=="i2cProbe:0:39");
}
static void checkControls() {
    using namespace hal_fake;
    using namespace YokeCalibration;
    flash.fill(0xFF); flashFail=false;
    YokeGains::Settings gains, defaults; YokePots::Settings pots; YokeRecord::Settings record;
    Controls controls(gains,pots,&record); controls.begin();
    for(uint8_t i=0;i<YokeGains::Count;++i)
        CHECK(gains.raw(YokeGains::Id(i))==defaults.raw(YokeGains::Id(i)));
    CHECK(YokeGains::recordSize<=YokePots::recordOffset);
    CHECK(YokePots::recordOffset+YokePots::recordSize<=YokeRecord::recordOffset);
    CHECK(YokeRecord::recordOffset+YokeRecord::recordSize<=256);
    controls.menu.update(0,true); controls.menu.press(Button::Mode);
    controls.menu.press(Button::Ok); controls.menu.press(Button::Minus);
    controls.menu.press(Button::Ok); const auto wanted=controls.menu.candidate();
    CHECK(controls.menu.savePending()); CHECK(controls.save());
    CHECK(gains.raw(YokeGains::Total)!=defaults.raw(YokeGains::Total));
    YokeGains::Settings loaded; YokePots::Settings loadedPots; YokeRecord::Settings loadedRecord;
    Controls reread(loaded,loadedPots,&loadedRecord); reread.begin();
    for(uint8_t i=0;i<YokeGains::Count;++i)
        CHECK(loaded.raw(YokeGains::Id(i))==wanted.raw(YokeGains::Id(i)));
    for(size_t i=YokePots::recordOffset;i<256;++i) CHECK(flash[i]==0xFF);
}
static void checkLcd() {
    using namespace hal_fake;
    timeUs=0; i2cResults[0].clear(); events.clear();
    setInput(LCD_SDA_PIN,true); setInput(LCD_SCL_PIN,true);
    LcdTelemetry lcd; lcd.begin();
    auto update=[&]() { lcd.update(hal::millis(),0.5f,0,128,0); };
    timeUs=100000; i2cResults[0].push_back(2); update();
    CHECK(events[0]=="i2cProbe:0:39");
    events.clear(); timeUs+=499000; update(); CHECK(events.empty());
    timeUs+=1000; update(); CHECK(events[0]=="i2cProbe:0:63");
    const unsigned waits[]={100,5,5,1,1,1,1,1,1,3};
    const std::vector<std::vector<uint8_t>> writes={
        {8},{0x38,0x3C,0x38},{0x38,0x3C,0x38},{0x38,0x3C,0x38},
        {0x28,0x2C,0x28},{0x28,0x2C,0x28,0x88,0x8C,0x88},
        {8,12,8,0xC8,0xCC,0xC8},{8,12,8,0x68,0x6C,0x68},
        {8,12,8,0x28,0x2C,0x28}};
    for(size_t i=0;i<10;++i) {
        events.clear(); timeUs+=(waits[i]-1)*1000; update(); CHECK(events.empty());
        timeUs+=1000; update();
        if(i<9) { CHECK(events[0]=="i2cWrite:0:63"); CHECK(i2cTx[0]==writes[i]); }
        else CHECK(events.empty());
    }
    update(); CHECK(i2cTx[0]==std::vector<uint8_t>({0x88,0x8C,0x88,8,12,8}));
    std::string row;
    for(unsigned i=0;i<16;++i) {
        update(); CHECK(i2cTx[0].size()==6);
        row+=char((i2cTx[0][0]&0xF0)|(i2cTx[0][3]>>4));
    }
    CHECK(row=="RF+.5000 PW+128 ");
}
static void checkAs5600Hardware() {
    using namespace hal_fake;
    timeUs=0; hal::wire0.end(); hal::wire1.end();
    for(unsigned i=0;i<2;++i) { i2cResults[i].clear(); i2cRx[i]={0x05,0}; }
    AxisAs5600 axes; axes.begin();
    i2cRx[1]={0x20,0x01,0x23}; i2cRx[0]={0x20,0x04,0x56};
    events.clear(); axes.update(1);
    CHECK(axes.raw(0)==0x123); CHECK(axes.raw(1)==0x456);
    CHECK(i2cTx[1]==std::vector<uint8_t>{0x0B}); CHECK(i2cTx[0]==std::vector<uint8_t>{0x0B});
    CHECK(events[0]=="i2cWrite:1:54" && events[2]=="i2cRead:1:54");
    CHECK(events[4]=="i2cWrite:0:54" && events[6]=="i2cRead:0:54");
    CHECK(i2cRx[0].empty() && i2cRx[1].empty());
    i2cResults[0]={2}; i2cResults[1]={2}; axes.update(2);
    CHECK(std::strcmp(axes.statusText(0),"I2C")==0);
    CHECK(std::strcmp(axes.statusText(1),"I2C")==0);
}

int main() {
    checkPwm(); checkReset(); checkEeprom(); checkDescriptors(); checkConstrain(); checkFake();
    checkHardwareModules(); checkControls(); checkLcd(); checkAs5600Hardware();
    std::printf("%s hal_checks (%u casos)\n", failures ? "FAIL" : "PASS", checks);
    return failures ? 1 : 0;
}
