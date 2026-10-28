#include "dsss.hpp"

namespace kok::radio {

const i8 dsss_engine::BARKER_11[CHIPS_PER_BIT] = {
    1, -1, 1, 1, -1, 1, 1, 1, -1, -1, -1
};

usize dsss_engine::spread_bytes(const u8* in_bytes, usize len, i8* out_chips, usize max_chips) {
    usize chip_idx = 0;
    for (usize i = 0; i < len; ++i) {
        for (u8 b = 0; b < 8; ++b) {
            if (chip_idx + CHIPS_PER_BIT > max_chips) {
                return chip_idx;
            }
            u8 bit = (in_bytes[i] >> (7 - b)) & 1;
            i8 polarity = (bit != 0) ? 1 : -1;

            for (usize c = 0; c < CHIPS_PER_BIT; ++c) {
                out_chips[chip_idx++] = static_cast<i8>(polarity * BARKER_11[c]);
            }
        }
    }
    return chip_idx;
}

i32 dsss_engine::correlate_symbol(const i8* chips_11) {
    i32 sum = 0;
    for (usize c = 0; c < CHIPS_PER_BIT; ++c) {
        sum += (chips_11[c] * BARKER_11[c]);
    }
    return sum;
}

usize dsss_engine::despread_chips(const i8* in_chips, usize num_chips, u8* out_bytes, usize max_bytes) {
    usize num_bits = num_chips / CHIPS_PER_BIT;
    usize byte_count = 0;
    u8 current_byte = 0;
    u8 bit_pos = 0;

    for (usize b = 0; b < num_bits; ++b) {
        i32 corr = correlate_symbol(&in_chips[b * CHIPS_PER_BIT]);
        u8 decision_bit = (corr > 0) ? 1 : 0;

        current_byte = static_cast<u8>((current_byte << 1) | decision_bit);
        ++bit_pos;

        if (bit_pos == 8) {
            if (byte_count < max_bytes) {
                out_bytes[byte_count++] = current_byte;
            }
            current_byte = 0;
            bit_pos = 0;
        }
    }

    return byte_count;
}

} // namespace kok::radio
