#include "squelch.hpp"
#include "kok/math.hpp"

namespace kok::radio {

squelch_detector::squelch_detector() {
    configure(DEFAULT_SAMPLE_RATE, SquelchMode::TONE_150HZ, 500, 25000);
}

void squelch_detector::configure(u32 sample_rate, SquelchMode mode, i32 tone_threshold, i32 noise_threshold) {
    m_sample_rate = (sample_rate > 0) ? sample_rate : DEFAULT_SAMPLE_RATE;
    m_mode = mode;
    m_tone_threshold = tone_threshold;
    m_noise_threshold = noise_threshold;
    m_gate_open = false;
    m_last_tone_energy = 0;
    m_last_noise_energy = 0;

    // Goertzel katsayısı: coeff = 2 * cos(2 * PI * 150 / Fs)
    // 8 kHz için 2 * cos(2 * PI * 150 / 8000) = 2 * cos(0.1178 rad) = 2 * 0.99307 = 1.9861
    // Q14 formatında: 1.9861 * 16384 = 32541
    float omega = 6.2831853f * static_cast<float>(TONE_FREQ_HZ) / static_cast<float>(m_sample_rate);
    fp32_t s_rad, c_rad;
    math::cordic_sincos(fp32_t::from_float(omega), s_rad, c_rad);
    float cos_val = c_rad.to_float();
    m_goertzel_coeff = static_cast<i32>(2.0f * cos_val * 16384.0f);
}

i32 squelch_detector::compute_goertzel_energy(const i16* samples, usize count) {
    if (count == 0) return 0;

    // Goertzel Filtresi (Q14)
    i64 s_prev = 0;
    i64 s_prev2 = 0;

    for (usize i = 0; i < count; ++i) {
        i64 s = static_cast<i64>(samples[i]) + ((m_goertzel_coeff * s_prev) >> 14) - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }

    // Güç: P = s_prev^2 + s_prev2^2 - (coeff * s_prev * s_prev2)
    i64 p = (s_prev * s_prev) + (s_prev2 * s_prev2) - ((m_goertzel_coeff * s_prev * s_prev2) >> 14);
    if (p < 0) p = 0;

    // Normalize et
    i64 norm_energy = p / static_cast<i64>(count * count);
    return static_cast<i32>(norm_energy);
}

i32 squelch_detector::compute_high_freq_noise(const i16* samples, usize count) {
    if (count < 2) return 0;

    i64 diff_sum = 0;
    for (usize i = 1; i < count; ++i) {
        i32 diff = static_cast<i32>(samples[i]) - static_cast<i32>(samples[i - 1]);
        diff_sum += (diff < 0 ? -diff : diff);
    }

    return static_cast<i32>(diff_sum / static_cast<i64>(count));
}

bool squelch_detector::process_block(const i16* samples, usize count) {
    if (count == 0) return m_gate_open;

    m_last_tone_energy = compute_goertzel_energy(samples, count);
    m_last_noise_energy = compute_high_freq_noise(samples, count);

    bool tone_detected = (m_last_tone_energy >= m_tone_threshold);
    bool noise_quieted = (m_last_noise_energy <= m_noise_threshold);

    switch (m_mode) {
    case SquelchMode::CARRIER_ONLY:
        m_gate_open = noise_quieted;
        break;
    case SquelchMode::TONE_150HZ:
        m_gate_open = tone_detected;
        break;
    case SquelchMode::COMBINED_STRICT:
        m_gate_open = tone_detected && noise_quieted;
        break;
    }

    return m_gate_open;
}

} // namespace kok::radio
