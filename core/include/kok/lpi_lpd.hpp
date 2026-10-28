#ifndef KOK_LPI_LPD_HPP
#define KOK_LPI_LPD_HPP

#include "types.hpp"
#include "math.hpp"

namespace kok {

// ============================================================================
// DÜŞÜK YAKALANMA VE DÜŞÜK TESPİT OLASILIĞI (LPI / LPD) METRİKLERİ
// Elektronik Destek (ED / ESM) Tehditlerine Karşı Spektral Saklanma Hesapları
// ============================================================================

class lpi_lpd {
public:
    // İşleme Kazancı (Processing Gain): G_p = 10 * log10(RF_Bandwidth / Data_Bandwidth)
    // Telsizin düşman gürültüsüne karşı elde ettiği desibel kazancı
    static float calculate_processing_gain_db(float rf_bandwidth_hz, float data_rate_bps) {
        if (data_rate_bps <= 0.0f || rf_bandwidth_hz <= 0.0f) return 0.0f;
        // log10(x) yaklaşık hesabı: ln(x) / 2.302585
        float ratio = rf_bandwidth_hz / data_rate_bps;
        float log_val = 0.0f;
        while (ratio > 10.0f) {
            log_val += 1.0f;
            ratio /= 10.0f;
        }
        while (ratio < 1.0f && ratio > 0.0f) {
            log_val -= 1.0f;
            ratio *= 10.0f;
        }
        // [1..10] aralığında lineer yaklaşım
        log_val += (ratio - 1.0f) / 9.0f;
        return 10.0f * log_val;
    }

    // Karıştırma Marjı (Jamming Margin): M_j = G_p - (S/N)_out - L_sys
    // Telsizin düşman karıştırmasını bastırıp çalışabilmesi için kalan pay (dB)
    static float calculate_jamming_margin_db(float processing_gain_db, float snr_req_db, float system_loss_db = 2.0f) {
        return processing_gain_db - snr_req_db - system_loss_db;
    }

    // Shannon Kanal Kapasitesi Sınırı: C = B * log2(1 + S/N)
    // Düşük SNR koşulunda teorik üst sınır
    static float shannon_capacity_bps(float bandwidth_hz, float snr_linear) {
        if (snr_linear <= 0.0f) return 0.0f;
        // log2(1 + snr)
        float x = 1.0f + snr_linear;
        float log2_val = 0.0f;
        while (x >= 2.0f) {
            log2_val += 1.0f;
            x /= 2.0f;
        }
        log2_val += (x - 1.0f);
        return bandwidth_hz * log2_val;
    }

    // Hızlı Bit Entropisi Ölçer (Shannon Entropisi Yaklaşımı)
    // Keystream veya iletilen paketin rastgelelik derecesini (0..255) ölçer
    static u8 estimate_entropy_score(const u8* data, usize len) {
        if (len == 0) return 0;
        usize byte_counts[16] = {0}; // 4-bit nibble dağılımı
        for (usize i = 0; i < len; ++i) {
            byte_counts[data[i] & 0x0F]++;
            byte_counts[(data[i] >> 4) & 0x0F]++;
        }

        // Homojen dağılım kontrolü
        usize total_samples = len * 2;
        usize ideal_per_bin = total_samples / 16;
        i32 deviation_sum = 0;
        for (usize k = 0; k < 16; ++k) {
            i32 diff = static_cast<i32>(byte_counts[k]) - static_cast<i32>(ideal_per_bin);
            if (diff < 0) diff = -diff;
            deviation_sum += diff;
        }

        // Sapma ne kadar azsa entropi o kadar yüksek (azami 255)
        i32 score = 255 - (deviation_sum * 255 / static_cast<i32>(total_samples));
        return static_cast<u8>(score < 0 ? 0 : (score > 255 ? 255 : score));
    }
};

} // namespace kok

#endif // KOK_LPI_LPD_HPP
