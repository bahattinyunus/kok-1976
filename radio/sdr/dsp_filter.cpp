#include "dsp_filter.hpp"

namespace kok::radio {

// 31-Taps Kök Yükseltilmiş Kosinüs (RRC, alpha=0.35, Q15 sabit nokta)
const i16 dsp_filter::RRC_COEFFS_Q15[RRC_TAPS] = {
    -12,   -35,   -68,   -89,   -64,     8,   132,   298,
    468,   585,   583,   405,    21,  -543, -1193,  7680,
  -1193,  -543,    21,   405,   583,   585,   468,   298,
    132,     8,   -64,   -89,   -68,   -35,   -12
};

void dsp_filter::apply_fir_filter(const i16* in_samples, usize count, i16* out_samples) {
    if (count == 0) return;

    for (usize i = 0; i < count; ++i) {
        i32 acc = 0;
        for (usize k = 0; k < RRC_TAPS; ++k) {
            if (i >= k) {
                acc += (static_cast<i32>(in_samples[i - k]) * static_cast<i32>(RRC_COEFFS_Q15[k]));
            }
        }
        // Q15 ölçekleme
        out_samples[i] = static_cast<i16>(acc >> 15);
    }
}

} // namespace kok::radio
