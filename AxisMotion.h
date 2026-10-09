#ifndef AXIS_MOTION_H
#define AXIS_MOTION_H

#include <stdint.h>
#include "filters.h"

// Etapa 2: medicion solamente; no genera fuerza ni limita el recorrido.
// Los limites son cuentas relativas al centro, con min < 0 y max > 0.
class AxisMotion {
public:
    AxisMotion(long minimum, long maximum, float velocityFilterTauSeconds)
        : _negativeSpan(-(float)minimum), _positiveSpan((float)maximum),
          _velocityFilter(velocityFilterTauSeconds) {}

    void update(long relativeCounts, uint32_t nowUs) {
        const float position = relativeCounts < 0
            ? relativeCounts / _negativeSpan : relativeCounts / _positiveSpan;

        if (!_initialized) {
            _initialized = true;
            _lastTimeUs = nowUs;
            positionCounts = relativeCounts;
            positionNormalized = position;
            return; // primera muestra: velocidad cero, sin salto artificial
        }

        // Resta unsigned: admite el desbordamiento normal de micros().
        const uint32_t elapsedUs = nowUs - _lastTimeUs;
        if (elapsedUs == 0) return;
        const float dt = elapsedUs * 0.000001f;
        velocityRaw = (position - positionNormalized) / dt;
        velocityFiltered = _velocityFilter.update(velocityRaw, dt);

        positionCounts = relativeCounts;
        positionNormalized = position;
        _lastTimeUs = nowUs;
    }

    float boundedPosition() const {
        if (positionNormalized < -1.0f) return -1.0f;
        if (positionNormalized > 1.0f) return 1.0f;
        return positionNormalized;
    }

    long positionCounts = 0;
    // Sin recortar: +/-1 son los extremos configurados, no topes de software.
    float positionNormalized = 0.0f;
    // Unidades: recorridos centro-extremo por segundo (no grados/s).
    float velocityRaw = 0.0f;
    float velocityFiltered = 0.0f;

private:
    float _negativeSpan, _positiveSpan;
    TimeConstantLowPassFilter _velocityFilter;
    uint32_t _lastTimeUs = 0;
    bool _initialized = false;
};

#endif
