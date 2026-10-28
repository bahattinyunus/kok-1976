#include "fsk.hpp"

namespace kok::radio {

fsk_modulator::fsk_modulator()
    : m_sample_rate(48000), m_baud_rate(1200), m_f_mark(1200), m_f_space(2200),
      m_samples_per_symbol(40), m_phase(fp32_t::from_float(0.0f)) {
    configure(48000, 1200, 1200, 2200);
}

void fsk_modulator::configure(u32 sample_rate, u32 baud_rate, u32 f_mark, u32 f_space) {
    m_sample_rate = sample_rate;
    m_baud_rate = baud_rate;
    m_f_mark = f_mark;
    m_f_space = f_space;
    m_samples_per_symbol = (sample_rate / baud_rate > 0) ? (sample_rate / baud_rate) : 1;

    // phase_step = 2 * PI * f / Fs
    float step_m = 6.2831853f * static_cast<float>(f_mark) / static_cast<float>(sample_rate);
    float step_s = 6.2831853f * static_cast<float>(f_space) / static_cast<float>(sample_rate);

    m_phase_step_mark  = fp32_t::from_float(step_m);
    m_phase_step_space = fp32_t::from_float(step_s);
}

void fsk_modulator::modulate_bit(u8 bit, iq_sample* out_samples) {
    fp32_t step = (bit != 0) ? m_phase_step_mark : m_phase_step_space;

    for (u32 i = 0; i < m_samples_per_symbol; ++i) {
        fp32_t s, c;
        math::cordic_sincos(m_phase, s, c);
        out_samples[i] = iq_sample(c, s);

        m_phase = m_phase + step;
        if (m_phase > math::pi()) {
            m_phase = m_phase - math::two_pi();
        }
    }
}

usize fsk_modulator::modulate_bytes(const u8* bytes, usize len, iq_sample* out_samples, usize max_samples) {
    usize sample_idx = 0;
    for (usize i = 0; i < len; ++i) {
        for (u8 b = 0; b < 8; ++b) {
            if (sample_idx + m_samples_per_symbol > max_samples) {
                return sample_idx;
            }
            u8 bit = (bytes[i] >> (7 - b)) & 1;
            modulate_bit(bit, &out_samples[sample_idx]);
            sample_idx += m_samples_per_symbol;
        }
    }
    return sample_idx;
}

// ============================================================================
// DEMODÜLATÖR GERÇEKLEMESİ
// ============================================================================

fsk_demodulator::fsk_demodulator()
    : m_sample_rate(48000), m_baud_rate(1200), m_samples_per_symbol(40),
      m_prev_sample(fp32_t::from_float(1.0f), fp32_t::from_float(0.0f)),
      m_integrator(fp32_t::from_float(0.0f)), m_symbol_timer(0) {}

void fsk_demodulator::configure(u32 sample_rate, u32 baud_rate, u32 f_mark, u32 f_space) {
    (void)f_mark;
    (void)f_space;
    m_sample_rate = sample_rate;
    m_baud_rate = baud_rate;
    m_samples_per_symbol = (sample_rate / baud_rate > 0) ? (sample_rate / baud_rate) : 1;
    m_prev_sample = iq_sample(fp32_t::from_float(1.0f), fp32_t::from_float(0.0f));
    m_integrator = fp32_t::from_float(0.0f);
    m_symbol_timer = 0;
}

usize fsk_demodulator::demodulate(const iq_sample* samples, usize num_samples, u8* out_bits, usize max_bits) {
    usize bit_count = 0;

    // Eşik açısı: Merkez frekansı orta noktası
    for (usize n = 0; n < num_samples; ++n) {
        iq_sample curr = samples[n];

        // Gecikmeli-Çarpım Ayrıştırıcı (Delay-Multiply FM Discriminator):
        // d[n] = curr * conj(prev) = (curr.i + j curr.q) * (prev.i - j prev.q)
        // d.real = curr.i * prev.i + curr.q * prev.q
        // d.imag = curr.q * prev.i - curr.i * prev.q
        fp32_t d_real = (curr.i * m_prev_sample.i) + (curr.q * m_prev_sample.q);
        fp32_t d_imag = (curr.q * m_prev_sample.i) - (curr.i * m_prev_sample.q);

        fp32_t mag, phase_diff;
        math::cordic_cartesian_to_polar(d_real, d_imag, mag, phase_diff);

        m_integrator = m_integrator + phase_diff;
        m_prev_sample = curr;

        ++m_symbol_timer;
        if (m_symbol_timer >= m_samples_per_symbol) {
            m_symbol_timer = 0;
            if (bit_count < max_bits) {
                // Merkez frekansı orta açısına göre karar
                // mark frekansı daha düşükse (veya yüksekse) integratör ortalaması
                out_bits[bit_count++] = (m_integrator.raw > 0) ? 1 : 0;
            }
            m_integrator = fp32_t::from_float(0.0f);
        }
    }

    return bit_count;
}

} // namespace kok::radio
