#pragma once
#include <stddef.h>
#include <stdint.h>
constexpr uint32_t kEepromFlashOffset = 0x3FE000;
template<class Backend> class EepromImage {
public:
    void begin(size_t requested) {
        if (requested == 0 || requested > 4096) requested = 4096;
        size = (requested + 255) & ~size_t(255);
        Backend::read(data, size);
        dirty = false;
    }
    uint8_t read(int addr) {
        return addr >= 0 && size_t(addr) < size ? data[addr] : 0;
    }
    void write(int addr, uint8_t v) {
        if (addr >= 0 && size_t(addr) < size && data[addr] != v) {
            data[addr] = v;
            dirty = true;
        }
    }
    bool commit() {
        if (!size) return false;
        if (!dirty) return true;
        if (!Backend::eraseProgram(data, size)) return false;
        dirty = false;
        return true;
    }
private:
    uint8_t data[4096] = {};
    size_t size = 0;
    bool dirty = false;
};
