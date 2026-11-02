#pragma once
namespace hal {
template<class T, class L, class H> constexpr auto constrain(T amt, L low, H high) {
    return amt < low ? low : (amt > high ? high : amt);
}
}
