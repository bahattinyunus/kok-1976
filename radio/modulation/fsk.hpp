#ifndef KOK_RADIO_FSK_HPP
#define KOK_RADIO_FSK_HPP

#include "kok/types.hpp"
#include "kok/math.hpp"

namespace kok::radio {

// ============================================================================
// DÜŞÜK SNR KOŞULLU FSK / GFSK MODÜLATÖR VE DEMODÜLATÖRÜ
// (2-FSK / 4-FSK Frekans Kaydırmalı Anahtarlama)
// ============================================================================

struct iq_sample {
    fp32_t i;
    fp32_t q;

    constexpr iq_sample() : i(), q() {}
    constexpr iq_sample(fp32_t in_i, fp32_t in_q) : i(in_i), q(in_q) {}
};

class fsk_modulator {
public:
    fsk_modulator();

    // Parametreleri ayarla:
    // sample_rate: Örnekleme hızı (ör. 48000 Hz)
    // baud_rate: Sembol hızı (ör. 1200 veya 2400 baud)
    // f_mark: Mark frekansı (Bit 1) (ör. 1200 Hz)
    // f_space: Space frekansı (Bit 0) (ör. 2200 Hz)
    void configure(u32 sample_rate, u32 baud_rate, u32 f_mark, u32 f_space);

    // Tek bir sembolü modüle et (out_samples dizisi samples_per_symbol kadar olmalıdır)
    void modulate_bit(u8 bit, iq_sample* out_samples);

    // Bit dizisini modüle et
    usize modulate_bytes(const u8* bytes, usize len, iq_sample* out_samples, usize max_samples);

    u32 samples_per_symbol() const { return m_samples_per_symbol; }

private:
    u32 m_sample_rate;
    u32 m_baud_rate;
    u32 m_f_mark;
    u32 m_f_space;
    u32 m_samples_per_symbol;

    fp32_t m_phase;
    fp32_t m_phase_step_mark;
    fp32_t m_phase_step_space;
};

class fsk_demodulator {
public:
    fsk_demodulator();
    void configure(u32 sample_rate, u32 baud_rate, u32 f_mark, u32 f_space);

    // Gelen I/Q örneklerini faz ayrıştırıcı (discriminator) ile çöz
    // out_bits dizisine bulunan bitler yazılır, dönen değer çıkarılan bit sayısıdır
    usize demodulate(const iq_sample* samples, usize num_samples, u8* out_bits, usize max_bits);

private:
    u32 m_sample_rate;
    u32 m_baud_rate;
    u32 m_samples_per_symbol;
    iq_sample m_prev_sample;
    fp32_t m_integrator;
    u32 m_symbol_timer;
};

} // namespace kok::radio

#endif // KOK_RADIO_FSK_HPP
