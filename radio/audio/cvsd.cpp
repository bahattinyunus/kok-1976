#include "cvsd.hpp"

namespace kok::radio {

cvsd_codec::cvsd_codec() {
    reset();
}

void cvsd_codec::reset() {
    m_accumulator = 0;
    m_step_size = DELTA_MIN;
    m_shift_reg = 0xAA; // 10101010 (Durgun kanal)
}

void cvsd_codec::update_step_size(u8 current_bit) {
    m_shift_reg = static_cast<u8>((m_shift_reg << 1) | (current_bit & 1));

    // Son 3 bit kontrolü: 111 veya 000 ise eğim aşırı yüklemesi (slope overload) vardır!
    u8 last3 = m_shift_reg & 0x07;
    if (last3 == 0x07 || last3 == 0x00) {
        // Adım boyutunu hızla artır
        m_step_size = m_step_size + DELTA_MIN * 2;
        if (m_step_size > DELTA_MAX) {
            m_step_size = DELTA_MAX;
        }
    } else {
        // Hece filtresi (Syllabic Filter): Adım boyutunu yavaşça asgari seviyeye sönümlendir
        m_step_size = (m_step_size * SYLLABIC_FILTER) / 16;
        if (m_step_size < DELTA_MIN) {
            m_step_size = DELTA_MIN;
        }
    }
}

u8 cvsd_codec::encode_sample(i16 pcm_sample) {
    i32 target = static_cast<i32>(pcm_sample);
    u8 bit = (target >= m_accumulator) ? 1 : 0;

    // İntegratör güncellemesi: Kaçaklı integratör (Leaky Integrator)
    m_accumulator = (m_accumulator * INTEGRATOR_LEAK) / 32;

    if (bit == 1) {
        m_accumulator += m_step_size;
    } else {
        m_accumulator -= m_step_size;
    }

    // Doygunluk (Saturation clamp) [-32768, 32767]
    if (m_accumulator > 32767) m_accumulator = 32767;
    if (m_accumulator < -32768) m_accumulator = -32768;

    update_step_size(bit);
    return bit;
}

i16 cvsd_codec::decode_bit(u8 cvsd_bit) {
    // Kaçaklı integratör
    m_accumulator = (m_accumulator * INTEGRATOR_LEAK) / 32;

    if ((cvsd_bit & 1) == 1) {
        m_accumulator += m_step_size;
    } else {
        m_accumulator -= m_step_size;
    }

    if (m_accumulator > 32767) m_accumulator = 32767;
    if (m_accumulator < -32768) m_accumulator = -32768;

    update_step_size(cvsd_bit);
    return static_cast<i16>(m_accumulator);
}

usize cvsd_codec::encode_pcm_buffer(const i16* pcm_in, usize sample_count, u8* cvsd_out_bytes, usize max_bytes) {
    usize out_idx = 0;
    u8 current_byte = 0;
    u8 bit_pos = 0;

    for (usize i = 0; i < sample_count; ++i) {
        u8 bit = encode_sample(pcm_in[i]);
        current_byte = static_cast<u8>((current_byte << 1) | bit);
        ++bit_pos;

        if (bit_pos == 8) {
            if (out_idx < max_bytes) {
                cvsd_out_bytes[out_idx++] = current_byte;
            }
            current_byte = 0;
            bit_pos = 0;
        }
    }

    // Kalan bitleri doldur (padding)
    if (bit_pos > 0 && out_idx < max_bytes) {
        current_byte = static_cast<u8>(current_byte << (8 - bit_pos));
        cvsd_out_bytes[out_idx++] = current_byte;
    }

    return out_idx;
}

usize cvsd_codec::decode_cvsd_buffer(const u8* cvsd_in_bytes, usize byte_count, i16* pcm_out, usize max_samples) {
    usize sample_idx = 0;
    for (usize b = 0; b < byte_count; ++b) {
        u8 byte_val = cvsd_in_bytes[b];
        for (i8 bit = 7; bit >= 0; --bit) {
            if (sample_idx >= max_samples) {
                return sample_idx;
            }
            u8 bit_val = static_cast<u8>((byte_val >> bit) & 1);
            pcm_out[sample_idx++] = decode_bit(bit_val);
        }
    }
    return sample_idx;
}

} // namespace kok::radio
