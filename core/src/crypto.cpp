#include "kok/crypto.hpp"

namespace kok {

// Kamoy-76 S-Kutusu (4-bit doğrusal olmayan permütasyon tablosu)
static const u8 DEFAULT_SBOX[16] = {
    0x0C, 0x05, 0x06, 0x0B, 0x09, 0x00, 0x0A, 0x0D,
    0x03, 0x0E, 0x0F, 0x08, 0x04, 0x07, 0x01, 0x02
};

cipher::cipher() : m_lfsr1(0x19761976u), m_lfsr2(0x4D414355u), m_counter(0) {
    for (usize i = 0; i < 16; ++i) {
        m_sbox[i] = DEFAULT_SBOX[i];
    }
}

cipher::cipher(u64 key, u32 iv) {
    init(key, iv);
}

void cipher::init(u64 key, u32 iv) {
    u32 key_high = static_cast<u32>(key >> 32);
    u32 key_low  = static_cast<u32>(key & 0xFFFFFFFFu);

    m_lfsr1 = key_high ^ iv ^ 0x5A5A5A5Au;
    m_lfsr2 = key_low  ^ (~iv) ^ 0xA5A5A5A5u;

    if (m_lfsr1 == 0) m_lfsr1 = 0x19760001u;
    if (m_lfsr2 == 0) m_lfsr2 = 0x20260001u;

    m_counter = 0;

    for (usize i = 0; i < 16; ++i) {
        u8 shift = (key_low >> (i * 2)) & 0x0F;
        m_sbox[i] = DEFAULT_SBOX[(i ^ shift) & 0x0F];
    }

    // Başlangıç ısınma turları (warm-up discard)
    for (usize i = 0; i < 32; ++i) {
        next_byte();
    }
}

void cipher::step() {
    // LFSR1: Polinom x^31 + x^28 + 1 (0x48000000u)
    u32 lsb1 = m_lfsr1 & 1;
    m_lfsr1 >>= 1;
    if (lsb1) {
        m_lfsr1 ^= 0x48000000u;
    }

    // LFSR2: Polinom x^32 + x^22 + x^2 + x^1 + 1 (0x80200003u)
    u32 lsb2 = m_lfsr2 & 1;
    m_lfsr2 >>= 1;
    if (lsb2) {
        m_lfsr2 ^= 0x80200003u;
    }

    ++m_counter;
}

u8 cipher::next_byte() {
    step();

    u8 nibble1 = static_cast<u8>((m_lfsr1 ^ (m_lfsr1 >> 8) ^ (m_lfsr1 >> 16)) & 0x0F);
    u8 nibble2 = static_cast<u8>((m_lfsr2 ^ (m_lfsr2 >> 7) ^ (m_lfsr2 >> 19)) & 0x0F);

    u8 s1 = m_sbox[nibble1];
    u8 s2 = m_sbox[nibble2];

    u8 count_mix = static_cast<u8>((m_counter ^ (m_counter >> 8)) & 0xFF);

    return static_cast<u8>((s1 << 4) | s2) ^ count_mix;
}

void cipher::process(u8* data, usize len) {
    for (usize i = 0; i < len; ++i) {
        data[i] ^= next_byte();
    }
}

u32 cipher::calculate_mac(u64 key, const u8* data, usize len) {
    u32 h = 0x811C9DC5u; // FNV-1a tabanlı taktik MAC
    u32 k0 = static_cast<u32>(key);
    u32 k1 = static_cast<u32>(key >> 32);

    h ^= k0;
    h *= 0x01000193u;

    for (usize i = 0; i < len; ++i) {
        h ^= data[i];
        h *= 0x01000193u;
        h = (h << 13) | (h >> 19); // Rotasyon
    }

    h ^= k1;
    h *= 0x01000193u;
    return h;
}

} // namespace kok
