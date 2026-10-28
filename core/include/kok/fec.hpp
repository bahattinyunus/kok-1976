#ifndef KOK_FEC_HPP
#define KOK_FEC_HPP

#include "types.hpp"

namespace kok {

// ============================================================================
// İLERİ HATA DÜZELTME (FORWARD ERROR CORRECTION - FEC) VE SERPİŞTİRME
// Karıştırma ve Düşük SNR Koşullarında Veri Kurtarma Algoritmaları
// ============================================================================

class fec {
public:
    // ------------------------------------------------------------------------
    // GENİŞLETİLMİŞ HAMMING(8,4) SECDED
    // 4 bit veri -> 8 bit kod sözcüğü
    // Tek bitlik hataları düzeltir, çift bitlik hataları tespit eder.
    // ------------------------------------------------------------------------
    static u8 hamming84_encode(u8 nibble);
    static Status hamming84_decode(u8 code, u8& out_nibble);

    // Dizi düzeyinde Hamming(8,4) kodlama/çözme
    // in_data: N bayt veri -> out_encoded: 2*N bayt kod sözcüğü
    static void hamming_encode_buffer(const u8* in_data, usize len, u8* out_encoded);
    // in_encoded: 2*N bayt -> out_data: N bayt veri
    static Status hamming_decode_buffer(const u8* in_encoded, usize in_len, u8* out_data, usize& errors_corrected);

    // ------------------------------------------------------------------------
    // GENİŞLETİLMİŞ GOLAY(24,12) KODU
    // 12 bit veri -> 24 bit kod sözcüğü
    // 3 bite kadar rastgele hatayı düzeltir, 4 bitlik hataları tespit eder.
    // Askerî VHF/UHF taktik telsizlerin temel koruma katmanı.
    // ------------------------------------------------------------------------
    static u32 golay24_encode(u16 data12);
    static Status golay24_decode(u32 code24, u16& out_data12, usize& err_count);

    // ------------------------------------------------------------------------
    // BLOK SERPİŞTİRİCİ (BLOCK INTERLEAVER)
    // Düşman karıştırması veya ani sönümlemeden kaynaklanan ardışık (burst)
    // hataları dağıtarak FEC bloklarının hataları düzeltebilmesini sağlar.
    // ------------------------------------------------------------------------
    // Matrix: Rows x Cols (len = Rows * Cols)
    static void interleave(const u8* in_bytes, u8* out_bytes, usize rows, usize cols);
    static void deinterleave(const u8* in_bytes, u8* out_bytes, usize rows, usize cols);
};

} // namespace kok

#endif // KOK_FEC_HPP
