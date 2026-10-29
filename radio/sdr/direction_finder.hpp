#pragma once

#include "kok/types.hpp"
#include "kok/math.hpp"

namespace kok {
namespace radio {

/**
 * @brief Watson-Watt ve Faz İnterferometrisi Tabanlı Taktik Radyo Yön Bulucu (RDF / Kestirme)
 * 
 * Mehmet Hâcim Kamoy ve ASELSAN'ın ilk kestirme sistemleri (DF) mimarisi.
 * Düşman karıştırıcılarının (jammer), kaçak vericilerin veya dost pilotların (CSAR)
 * coğrafi kerteriz açısını (Azimuth) ve sinyal güvenilirlik katsayısını hesaplar.
 * 
 * Freestanding C++17, sıfır heap tahsisi, CORDIC açı ayrıştırması.
 */
class direction_finder {
public:
    static constexpr usize MAX_SAMPLES = 64;

    struct bearing_result {
        fp32_t azimuth_deg;   // Kerteriz Açısı (0.0° - 359.9°)
        fp32_t confidence;    // Kerteriz Güvenilirlik Oranı (0.0 - 1.0)
        fp32_t rssi_dbm;      // Alınan Sinyal Gücü (dBm)
        bool valid;           // Kerteriz geçerli mi?
    };

    direction_finder();

    /**
     * @brief 4-Elemanlı Adcock / Çapraz Dipol ve Duyarga (Sense) Anten Örneklerini İşler
     * 
     * @param v_ns North-South (Kuzey-Güney) anten çifti gerilim örneği (Q16.16)
     * @param v_ew East-West (Doğu-Batı) anten çifti gerilim örneği (Q16.16)
     * @param v_sense Merkezi çok yönlü (Sense) anten gerilim örneği (180° belirsizlik çözümü)
     */
    void feed_sample(fp32_t v_ns, fp32_t v_ew, fp32_t v_sense);

    /**
     * @brief Biriken RF örneklerinden nihai kerteriz açısını ve güvenilirliğini hesaplar.
     */
    bearing_result calculate_bearing() const;

    /**
     * @brief Dahili biriktiriciyi sıfırlar.
     */
    void reset();

    /**
     * @brief İki istasyon kerterizinin kesişiminden hedef koordinat kestirimi yapar (Triangulation)
     * 
     * @param x1 İstasyon 1 Doğu koordinatı (km)
     * @param y1 İstasyon 1 Kuzey koordinatı (km)
     * @param theta1 İstasyon 1 Kerteriz Açısı (derece)
     * @param x2 İstasyon 2 Doğu koordinatı (km)
     * @param y2 İstasyon 2 Kuzey koordinatı (km)
     * @param theta2 İstasyon 2 Kerteriz Açısı (derece)
     * @param out_x Kestirilen hedef X koordinatı (km)
     * @param out_y Kestirilen hedef Y koordinatı (km)
     * @return true Kesişim başarıyla hesaplandı
     */
    static bool triangulate_target(fp32_t x1, fp32_t y1, fp32_t theta1,
                                   fp32_t x2, fp32_t y2, fp32_t theta2,
                                   fp32_t& out_x, fp32_t& out_y);

private:
    fp32_t sum_ns_;
    fp32_t sum_ew_;
    fp32_t sum_sense_;
    fp32_t sum_energy_;
    usize sample_count_;
};

} // namespace radio
} // namespace kok
