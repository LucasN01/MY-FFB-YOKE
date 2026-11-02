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

namespace hal_fake {
extern uint64_t timeUs;
extern std::array<hal::PinMode, 30> modes;
extern std::array<uint32_t, 30> pwm;
extern std::array<std::deque<uint16_t>, 30> adc;
extern std::vector<std::string> events;
extern std::array<std::deque<uint8_t>, 2> i2cRx, i2cResults;
extern std::array<std::vector<uint8_t>, 2> i2cTx;
extern std::deque<uint8_t> usbRx, usbTx;
extern bool usbDtr, usbInitialized;
extern size_t usbCapacity;
extern unsigned usbLocks, usbBackgroundCalls;
extern uint32_t watchdogMs, watchdogFeeds;
extern hal::ResetReason reset;
extern std::array<uint8_t, 4096> flash;
extern size_t readBytes, erasedBytes, programmedBytes;
extern unsigned erases, programs;
extern bool flashFail;
void setInput(uint8_t pin, bool high);
}
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
int main() {
    checkPwm(); checkReset(); checkEeprom(); checkDescriptors(); checkConstrain(); checkFake();
    std::printf("%s hal_checks (%u casos)\n", failures ? "FAIL" : "PASS", checks);
    return failures ? 1 : 0;
}
