#pragma once
#include <cstddef>
#include <cstring>
#include <cstdint>
namespace YokeLink {
constexpr unsigned long kErrorSemTimeout = 121;   // ERROR_SEM_TIMEOUT (usbser)
constexpr unsigned kMinConsecutiveTimeouts = 3;
constexpr uint64_t kLinkTimeoutMs = 1000;         // > borrado de sector flash (max ~400 ms)
constexpr uint64_t kReconnectPeriodMs = 500;
constexpr unsigned kReconnectHandshakeMs = 600;   // por intento de reconexion
constexpr uint64_t kRearmHoldoffMs = 500;
constexpr uint64_t kRetryLogPeriodMs = 5000;

enum class WriteOutcome { Ok, Timeout, DeviceError };

// TRUE con menos bytes (STATUS_TIMEOUT) o FALSE+121 = vencimiento; otro error = dispositivo.
inline WriteOutcome classifyWrite(bool ok, unsigned long written,
                                  unsigned long size, unsigned long error) {
    if (ok && written == size) return WriteOutcome::Ok;
    if (ok || error == kErrorSemTimeout) return WriteOutcome::Timeout;
    return WriteOutcome::DeviceError;
}

// Vencimientos: reconectar solo si hay >=3 seguidos Y pasaron >=1000 ms desde el primero.
struct LinkMonitor {
    unsigned consecutiveTimeouts = 0;
    uint64_t firstTimeoutMs = 0;
    bool onWrite(WriteOutcome o, uint64_t nowMs) {   // true = enlace perdido
        if (o == WriteOutcome::Ok) { consecutiveTimeouts = 0; return false; }
        if (o == WriteOutcome::DeviceError) return true;
        if (consecutiveTimeouts++ == 0) firstTimeoutMs = nowMs;
        return consecutiveTimeouts >= kMinConsecutiveTimeouts &&
               nowMs - firstTimeoutMs >= kLinkTimeoutMs;
    }
    void reset() { consecutiveTimeouts = 0; firstTimeoutMs = 0; }
};

// Máquina de reconexión (testeable sin Windows).
struct Reconnector {
    bool lost = false;
    uint64_t lostAtMs = 0, nextAttemptMs = 0, rearmAtMs = 0, lastRetryLogMs = 0;
    unsigned reconnections = 0;
    void onLost(uint64_t now) { if (!lost) { lost = true; lostAtMs = now; nextAttemptMs = now; ++reconnections; } }
    bool shouldAttempt(uint64_t now) const { return lost && now >= nextAttemptMs; }
    void onAttemptFailed(uint64_t now) { nextAttemptMs = now + kReconnectPeriodMs; }
    void onReopened(uint64_t now) { lost = false; rearmAtMs = now + kRearmHoldoffMs; }
    bool armAllowed(uint64_t now) const { return !lost && now >= rearmAtMs; }
    bool shouldLogRetry(uint64_t now) { if (now - lastRetryLogMs < kRetryLogPeriodMs && lastRetryLogMs) return false; lastRetryLogMs = now; return true; }
};

// Extrae líneas completas que empiezan con "BOOT " (pueden llegar partidas).
struct BootLineScanner {
    char line[160] = {0};
    size_t n = 0;
    bool overflow = false;
    template <class F> void feed(const char* data, size_t size, F onBoot) {
        for (size_t i = 0; i < size; ++i) {
            const char c = data[i];
            if (c == '\n' || c == '\r') {
                line[n] = 0;
                if (!overflow && n >= 5 && std::strncmp(line, "BOOT ", 5) == 0) onBoot(line);
                n = 0; overflow = false;
            } else if (n + 1 < sizeof(line)) line[n++] = c;
            else overflow = true;
        }
    }
};
}
