#include "filters.h"
TimeConstantLowPassFilter::TimeConstantLowPassFilter(float tauSeconds)
    : _tau(tauSeconds), _output(0.0f) {}

float TimeConstantLowPassFilter::update(float input, float deltaTime) {
    if (deltaTime <= 0.0f) return _output;
    // alpha=tau/(tau+dt): tau=18ms da alpha=0.90 con dt=2ms.
    const float weight = deltaTime / (_tau + deltaTime);
    _output += weight * (input - _output);
    return _output;
}
