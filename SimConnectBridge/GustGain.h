#ifndef GUST_GAIN_H
#define GUST_GAIN_H
#include <cmath>
inline double scaleGust(double base, double gain) {
    if(!std::isfinite(base)||!std::isfinite(gain)||gain<0||gain>10) return 0;
    const double value=base*gain;
    return value < -1 ? -1 : (value > 1 ? 1 : value);
}
#endif
