#ifndef KOK_CRC_HPP
#define KOK_CRC_HPP

#include "types.hpp"

namespace kok {

// ============================================================================
// DOĞRULAMA VE HATA BELİRLEME (CRC-8, CRC-16-CCITT, CRC-32)
// Dış Kütüphanesiz, Deterministik Askerî Çerçeve Bütünlük Kontrolleri
// ============================================================================

class crc {
public:
    // CRC-8 (Polinom: 0x07, Başlangıç: 0x00)
    static u8 crc8(const u8* data, usize len, u8 init = 0x00) {
        u8 crc_val = init;
        for (usize i = 0; i < len; ++i) {
            crc_val ^= data[i];
            for (u8 b = 0; b < 8; ++b) {
                if (crc_val & 0x80) {
                    crc_val = static_cast<u8>((crc_val << 1) ^ 0x07);
                } else {
                    crc_val = static_cast<u8>(crc_val << 1);
                }
            }
        }
        return crc_val;
    }

    // CRC-16-CCITT (Polinom: 0x1021, Başlangıç: 0xFFFF)
    // Telsiz Paketleri ve Taktik Çerçeve Kontrolü (FCS)
    static u16 crc16_ccitt(const u8* data, usize len, u16 init = 0xFFFF) {
        u16 crc_val = init;
        for (usize i = 0; i < len; ++i) {
            crc_val ^= static_cast<u16>(data[i] << 8);
            for (u8 b = 0; b < 8; ++b) {
                if (crc_val & 0x8000) {
                    crc_val = static_cast<u16>((crc_val << 1) ^ 0x1021);
                } else {
                    crc_val = static_cast<u16>(crc_val << 1);
                }
            }
        }
        return crc_val;
    }

    // CRC-32 (IEEE 802.3, Polinom: 0xEDB88320)
    static u32 crc32(const u8* data, usize len, u32 init = 0xFFFFFFFF) {
        u32 crc_val = init;
        for (usize i = 0; i < len; ++i) {
            crc_val ^= data[i];
            for (u8 b = 0; b < 8; ++b) {
                if (crc_val & 1) {
                    crc_val = (crc_val >> 1) ^ 0xEDB88320u;
                } else {
                    crc_val >>= 1;
                }
            }
        }
        return ~crc_val;
    }
};

} // namespace kok

#endif // KOK_CRC_HPP
