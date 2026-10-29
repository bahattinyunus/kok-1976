#ifndef KOK_RADIO_CVSD_HPP
#define KOK_RADIO_CVSD_HPP

#include "kok/types.hpp"

namespace kok::radio {

// ============================================================================
// ASKERÎ CVSD SES KODLAYICI (CONTINUOUSLY VARIABLE SLOPE DELTA MODULATION)
// STANAG 4209 / MIL-STD-188-113 Taktik Telsiz Ses Sıkıştırma Motoru (16 kbps)
// Bit Hatalarına Karşı Aşırı Dayanıklı 1-Bit Delta-Sigma Ses Sayısallaştırma
// ============================================================================

class cvsd_codec {
public:
    static constexpr i32 DELTA_MIN = 32;       // Asgari adım boyutu
    static constexpr i32 DELTA_MAX = 4096;     // Azami adım boyutu (Slope Overload Koruma)
    static constexpr i32 SYLLABIC_FILTER = 15; // Hece filtresi sönüm katsayısı (15/16)
    static constexpr i32 INTEGRATOR_LEAK = 31; // İntegratör kaçak katsayısı (31/32)

    cvsd_codec();

    void reset();

    // 16-bit PCM ses örneğini 1-bit CVSD çıktısına kodla (Dönen: 0 veya 1)
    u8 encode_sample(i16 pcm_sample);

    // 1-bit CVSD bitini 16-bit PCM ses örneğine deşifre et
    i16 decode_bit(u8 cvsd_bit);

    // Dizi düzeyinde PCM -> CVSD bit akışı (Her bayt 8 adet 1-bit ses örneği taşır)
    usize encode_pcm_buffer(const i16* pcm_in, usize sample_count, u8* cvsd_out_bytes, usize max_bytes);

    // Dizi düzeyinde CVSD bit akışı -> 16-bit PCM ses örneği
    usize decode_cvsd_buffer(const u8* cvsd_in_bytes, usize byte_count, i16* pcm_out, usize max_samples);

private:
    i32 m_accumulator;   // Temsili sinyal integratörü (Q16)
    i32 m_step_size;     // Uyarlanabilir adım büyüklüğü
    u8  m_shift_reg;     // Son 3 veya 4 bitlik ardışıklık kaydırma kaydedicisi

    void update_step_size(u8 current_bit);
};

} // namespace kok::radio

#endif // KOK_RADIO_CVSD_HPP
