#ifndef KOK_RADIO_DSP_FILTER_HPP
#define KOK_RADIO_DSP_FILTER_HPP

#include "kok/types.hpp"
#include "kok/math.hpp"

namespace kok::radio {

// ============================================================================
// SAYISAL SÜZGEÇ VE SDR ÇEKİRDEĞİ (DIGITAL SIGNAL PROCESSING CORE)
// Kök Yükseltilmiş Kosinüs (RRC - Root Raised Cosine) ve Sayısal Osilatör (NCO)
// Semboller Arası Girişimi (ISI) Sıfırlayan Frekans Şekillendirme
// ============================================================================

class dsp_filter {
public:
    static constexpr usize RRC_TAPS = 31; // 31-Katsayılı Simetrik FIR Süzgeç

    // Sabit Noktalı (Q15) Kök Yükseltilmiş Kosinüs Katsayıları (Roll-off alpha = 0.35)
    static const i16 RRC_COEFFS_Q15[RRC_TAPS];

    // FIR Konvolüsyonu: Giriş I/Q veya ses sinyalini RRC süzgecinden geçir
    // Semboller arası girişimi (ISI) ortadan kaldırır
    static void apply_fir_filter(const i16* in_samples, usize count, i16* out_samples);

    // Sayısal Kontrollü Osilatör (NCO - Numerically Controlled Oscillator)
    // Frekans kaydırma ve ara frekans (IF) karıştırıcı simülasyonu
    struct nco_state {
        fp32_t phase;
        fp32_t phase_step;

        void configure(float freq_hz, float sample_rate_hz) {
            float step = 6.2831853f * freq_hz / sample_rate_hz;
            phase_step = fp32_t::from_float(step);
            phase = fp32_t::from_float(0.0f);
        }

        void step(fp32_t& out_cos, fp32_t& out_sin) {
            math::cordic_sincos(phase, out_sin, out_cos);
            phase = phase + phase_step;
            if (phase > math::pi()) {
                phase = phase - math::two_pi();
            }
        }
    };
};

} // namespace kok::radio

#endif // KOK_RADIO_DSP_FILTER_HPP
