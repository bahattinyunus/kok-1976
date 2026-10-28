#include "kok/fec.hpp"

namespace kok {

// ============================================================================
// HAMMING(8,4) GERÇEKLEMESİ
// ============================================================================
// Veri: d3, d2, d1, d0
// Eşlik bitleri (Parity):
// p0 = d0 ^ d1 ^ d3
// p1 = d0 ^ d2 ^ d3
// p2 = d1 ^ d2 ^ d3
// p3 = genel eşlik (tüm bitlerin XOR'u)

u8 fec::hamming84_encode(u8 nibble) {
    u8 d0 = (nibble >> 0) & 1;
    u8 d1 = (nibble >> 1) & 1;
    u8 d2 = (nibble >> 2) & 1;
    u8 d3 = (nibble >> 3) & 1;

    u8 p0 = d0 ^ d1 ^ d3;
    u8 p1 = d0 ^ d2 ^ d3;
    u8 p2 = d1 ^ d2 ^ d3;

    u8 c7 = (d3 << 6) | (d2 << 5) | (d1 << 4) | (p2 << 3) | (d0 << 2) | (p1 << 1) | p0;
    
    // Genel eşlik biti (p3): en üst bit (bit 7)
    u8 parity_count = 0;
    for (u8 b = 0; b < 7; ++b) {
        if ((c7 >> b) & 1) ++parity_count;
    }
    u8 p3 = parity_count & 1;

    return static_cast<u8>(c7 | (p3 << 7));
}

Status fec::hamming84_decode(u8 code, u8& out_nibble) {
    u8 p0 = (code >> 0) & 1;
    u8 p1 = (code >> 1) & 1;
    u8 d0 = (code >> 2) & 1;
    u8 p2 = (code >> 3) & 1;
    u8 d1 = (code >> 4) & 1;
    u8 d2 = (code >> 5) & 1;
    u8 d3 = (code >> 6) & 1;
    u8 p3 = (code >> 7) & 1;

    // Sendrom hesabı
    u8 s0 = p0 ^ d0 ^ d1 ^ d3;
    u8 s1 = p1 ^ d0 ^ d2 ^ d3;
    u8 s2 = p2 ^ d1 ^ d2 ^ d3;
    u8 syndrome = (s2 << 2) | (s1 << 1) | s0;

    // Genel eşlik kontrolü
    u8 ones_count = 0;
    for (u8 b = 0; b < 8; ++b) {
        if ((code >> b) & 1) ++ones_count;
    }
    bool overall_parity_err = (ones_count & 1) != 0;

    if (syndrome == 0) {
        if (overall_parity_err) {
            // Sadece genel eşlik biti bozulmuş, veri sağlam
            out_nibble = static_cast<u8>((d3 << 3) | (d2 << 2) | (d1 << 1) | d0);
            return Status::SUCCESS;
        }
        // Hata yok
        out_nibble = static_cast<u8>((d3 << 3) | (d2 << 2) | (d1 << 1) | d0);
        return Status::SUCCESS;
    }

    // Sendrom var
    if (overall_parity_err) {
        // Tek bit hatası düzeltilebilir
        u8 corrected = code ^ (1 << (syndrome - 1));
        u8 cd0 = (corrected >> 2) & 1;
        u8 cd1 = (corrected >> 4) & 1;
        u8 cd2 = (corrected >> 5) & 1;
        u8 cd3 = (corrected >> 6) & 1;
        out_nibble = static_cast<u8>((cd3 << 3) | (cd2 << 2) | (cd1 << 1) | cd0);
        return Status::SUCCESS;
    }

    // Çift bit hatası tespit edildi, düzeltilemez!
    out_nibble = static_cast<u8>((d3 << 3) | (d2 << 2) | (d1 << 1) | d0);
    return Status::ERROR_UNCORRECTABLE;
}

void fec::hamming_encode_buffer(const u8* in_data, usize len, u8* out_encoded) {
    for (usize i = 0; i < len; ++i) {
        u8 low_nibble = in_data[i] & 0x0F;
        u8 high_nibble = (in_data[i] >> 4) & 0x0F;
        out_encoded[2 * i]     = hamming84_encode(low_nibble);
        out_encoded[2 * i + 1] = hamming84_encode(high_nibble);
    }
}

Status fec::hamming_decode_buffer(const u8* in_encoded, usize in_len, u8* out_data, usize& errors_corrected) {
    errors_corrected = 0;
    Status final_status = Status::SUCCESS;

    usize out_len = in_len / 2;
    for (usize i = 0; i < out_len; ++i) {
        u8 low_nibble = 0;
        u8 high_nibble = 0;
        Status s1 = hamming84_decode(in_encoded[2 * i], low_nibble);
        Status s2 = hamming84_decode(in_encoded[2 * i + 1], high_nibble);

        if (s1 == Status::ERROR_UNCORRECTABLE || s2 == Status::ERROR_UNCORRECTABLE) {
            final_status = Status::ERROR_UNCORRECTABLE;
        }

        out_data[i] = static_cast<u8>(low_nibble | (high_nibble << 4));
    }
    return final_status;
}

// ============================================================================
// GENİŞLETİLMİŞ GOLAY(24,12) GERÇEKLEMESİ
// ============================================================================

// Golay(24,12) için sistematik 12x12 B matrisi satırları (G_POLY = 0xC75)
static const u32 GOLAY_PARITY[12] = {
    0x8EB, 0x93E, 0xA97, 0xDC6, 0x367, 0x6CD,
    0xD99, 0x3DA, 0x7B4, 0xF68, 0x63B, 0xC75
};

static inline u8 bit_count(u32 val) {
    u8 count = 0;
    while (val) {
        count += (val & 1);
        val >>= 1;
    }
    return count;
}

u32 fec::golay24_encode(u16 data12) {
    data12 &= 0x0FFF;
    u32 parity12 = 0;
    for (usize i = 0; i < 12; ++i) {
        if ((data12 >> i) & 1) {
            parity12 ^= GOLAY_PARITY[i];
        }
    }
    return (static_cast<u32>(data12) << 12) | (parity12 & 0x0FFF);
}

Status fec::golay24_decode(u32 code24, u16& out_data12, usize& err_count) {
    code24 &= 0x00FFFFFFu;
    u16 recv_data = static_cast<u16>((code24 >> 12) & 0x0FFF);
    u16 recv_parity = static_cast<u16>(code24 & 0x0FFF);

    // Sendrom hesabı: s = p ^ (d * B)
    u16 calc_parity = 0;
    for (usize i = 0; i < 12; ++i) {
        if ((recv_data >> i) & 1) {
            calc_parity ^= GOLAY_PARITY[i];
        }
    }
    u32 s = recv_parity ^ calc_parity;

    if (s == 0) {
        out_data12 = recv_data;
        err_count = 0;
        return Status::SUCCESS;
    }

    // 1. Durum: 0 veri hatası, <= 3 eşlik hatası
    if (bit_count(s) <= 3) {
        out_data12 = recv_data;
        err_count = bit_count(s);
        return Status::SUCCESS;
    }

    // 2. Durum: 1 veri hatası, <= 2 eşlik hatası
    for (usize i = 0; i < 12; ++i) {
        u32 diff = s ^ GOLAY_PARITY[i];
        if (bit_count(diff) <= 2) {
            out_data12 = recv_data ^ (1 << i);
            err_count = 1 + bit_count(diff);
            return Status::SUCCESS;
        }
    }

    // 3. Durum: 2 veri hatası, <= 1 eşlik hatası
    for (usize i = 0; i < 12; ++i) {
        for (usize j = i + 1; j < 12; ++j) {
            u32 diff = s ^ GOLAY_PARITY[i] ^ GOLAY_PARITY[j];
            if (bit_count(diff) <= 1) {
                out_data12 = recv_data ^ (1 << i) ^ (1 << j);
                err_count = 2 + bit_count(diff);
                return Status::SUCCESS;
            }
        }
    }

    // 4. Durum: 3 veri hatası, 0 eşlik hatası
    for (usize i = 0; i < 12; ++i) {
        for (usize j = i + 1; j < 12; ++j) {
            for (usize k = j + 1; k < 12; ++k) {
                if (s == (GOLAY_PARITY[i] ^ GOLAY_PARITY[j] ^ GOLAY_PARITY[k])) {
                    out_data12 = recv_data ^ (1 << i) ^ (1 << j) ^ (1 << k);
                    err_count = 3;
                    return Status::SUCCESS;
                }
            }
        }
    }

    // 3'ten fazla hata: Düzeltilemez
    out_data12 = recv_data;
    err_count = 4;
    return Status::ERROR_UNCORRECTABLE;
}

// ============================================================================
// BLOK SERPİŞTİRME (INTERLEAVING)
// ============================================================================

void fec::interleave(const u8* in_bytes, u8* out_bytes, usize rows, usize cols) {
    for (usize r = 0; r < rows; ++r) {
        for (usize c = 0; c < cols; ++c) {
            out_bytes[c * rows + r] = in_bytes[r * cols + c];
        }
    }
}

void fec::deinterleave(const u8* in_bytes, u8* out_bytes, usize rows, usize cols) {
    for (usize c = 0; c < cols; ++c) {
        for (usize r = 0; r < rows; ++r) {
            out_bytes[r * cols + c] = in_bytes[c * rows + r];
        }
    }
}

} // namespace kok
