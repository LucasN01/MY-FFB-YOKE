#pragma once
#include <stddef.h>
#include <stdint.h>
namespace hal {
namespace usb { void init(); void background(); void lock(); void unlock(); }
class CdcSerial {
public:
    int available(); int read(); int availableForWrite(); bool dtr();
    size_t write(const uint8_t* data, size_t n);
    size_t write(const char* data, size_t n);
};
extern CdcSerial usbSerial;
}
