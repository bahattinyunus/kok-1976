#ifndef KOK_RADIO_SQUELCH_HPP
#define KOK_RADIO_SQUELCH_HPP

#include "kok/types.hpp"

namespace kok::radio {

// ============================================================================
// 150 HZ ASKERÎ TON SUSTURMA (TONE SQUELCH) VE GÜRÜLTÜ KAPISI (NOISE GATE)
// ASELSAN 4600 Doktrini: Düşman Gürültü Karıştırmasında Hoparlörü Kapalı Tutma
// Goertzel Algoritması ile Tek Frekanslı Sayısal DFT Süzgeci
// ============================================================================

enum class SquelchMode : u8 {
    CARRIER_ONLY    = 0x00, // Sadece taşıyıcı / gürültü sessizleşmesi (Noise quieting)
    TONE_150HZ      = 0x01, // 150 Hz Askerî Pilot Tonlu Susturma (CTCSS)
    COMBINED_STRICT = 0x02  // Hem taşıyıcı hem 150 Hz ton zorunlu (En yüksek koruma)
};

class squelch_detector {
public:
    static constexpr u32 DEFAULT_SAMPLE_RATE = 8000; // 8 kHz taktik ses örnekleme
    static constexpr u32 TONE_FREQ_HZ = 150;         // 150 Hz Askerî Ton

    squelch_detector();

    void configure(u32 sample_rate, SquelchMode mode, i32 tone_threshold, i32 noise_threshold);

    // Bir blok ses örneğini incele ve susturma kapısının (Audio Gate)
    // açık mı kapalı mı olması gerektiğine karar ver (True: AÇIK/DUYULUR, False: SUSTURULDU)
    bool process_block(const i16* samples, usize count);

    bool is_open() const { return m_gate_open; }
    i32 tone_energy() const { return m_last_tone_energy; }
    i32 noise_energy() const { return m_last_noise_energy; }

private:
    u32 m_sample_rate;
    SquelchMode m_mode;
    i32 m_tone_threshold;
    i32 m_noise_threshold;
    bool m_gate_open;

    i32 m_last_tone_energy;
    i32 m_last_noise_energy;

    // Goertzel Sabiti: 2 * cos(2 * PI * f / Fs) Q14 sabit nokta
    i32 m_goertzel_coeff;

    i32 compute_goertzel_energy(const i16* samples, usize count);
    i32 compute_high_freq_noise(const i16* samples, usize count);
};

} // namespace kok::radio

#endif // KOK_RADIO_SQUELCH_HPP
