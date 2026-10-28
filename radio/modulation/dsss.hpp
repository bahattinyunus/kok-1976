#ifndef KOK_RADIO_DSSS_HPP
#define KOK_RADIO_DSSS_HPP

#include "kok/types.hpp"

namespace kok::radio {

// ============================================================================
// DOĞRUDAN DİZİLİ YAYILI SPEKTRUM (DSSS - DIRECT SEQUENCE SPREAD SPECTRUM)
// 11-Çipli Barker Kodu ve Taktik Alt-Gürültü Seviyeli Muhabere Modülü
// ============================================================================

class dsss_engine {
public:
    static constexpr usize CHIPS_PER_BIT = 11;

    // Standart 11-Çipli Askerî Barker Dizisi: [+1, -1, +1, +1, -1, +1, +1, +1, -1, -1, -1]
    // Bit 1 için Barker dizisi, Bit 0 için tersi (Inverted)
    static const i8 BARKER_11[CHIPS_PER_BIT];

    // Giriş bit dizisini çip dizisine genişlet (Her bit -> 11 çip)
    // out_chips dizisi len * 8 * 11 eleman alacak büyüklükte olmalıdır
    static usize spread_bytes(const u8* in_bytes, usize len, i8* out_chips, usize max_chips);

    // Gelen çip dizisini eşleşmiş süzgeç (Matched Filter / Correlator) ile çöz
    // out_bits dizisine bulunan bitler yazılır
    static usize despread_chips(const i8* in_chips, usize num_chips, u8* out_bytes, usize max_bytes);

    // Tek bir 11-çiplik sembolün korelasyon tepe değerini hesapla (-11 .. +11)
    static i32 correlate_symbol(const i8* chips_11);
};

} // namespace kok::radio

#endif // KOK_RADIO_DSSS_HPP
