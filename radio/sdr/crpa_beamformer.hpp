#pragma once

#include "kok/types.hpp"
#include "kok/math.hpp"

namespace kok {
namespace radio {

/**
 * @brief Kontrollü Alış Paternli Anten (CRPA - Controlled Reception Pattern Antenna)
 * ve Adaptif Uzamsal Karıştırıcı Bastırma (Spatial Nulling) Motoru
 * 
 * Mehmet Hâcim Kamoy ve ASELSAN Anti-Jamming Elektronik Koruma Doktrini.
 * 4-Elemanlı faz dizili anten ile düşman yönlü karıştırıcısına (Jammer)
 * derin uzamsal çukur (Spatial Null > 30 dB) yerleştirir; zayıf dost sinyali kurtarır.
 * 
 * Freestanding C++17, Q16.16 sabit noktalı adaptif LMS / Güç İnversiyonu (Power Inversion).
 */
class crpa_beamformer {
public:
    static constexpr usize NUM_ELEMENTS = 4;

    struct complex_fp {
        fp32_t r; // Gerçel Bileşen (In-phase / I)
        fp32_t i; // Sanal Bileşen (Quadrature / Q)

        constexpr complex_fp() : r(), i() {}
        constexpr complex_fp(fp32_t real_part, fp32_t imag_part) : r(real_part), i(imag_part) {}

        complex_fp operator+(const complex_fp& o) const {
            return complex_fp(r + o.r, i + o.i);
        }

        complex_fp operator-(const complex_fp& o) const {
            return complex_fp(r - o.r, i - o.i);
        }

        complex_fp operator*(const complex_fp& o) const {
            // (a + jb)(c + jd) = (ac - bd) + j(ad + bc)
            return complex_fp((r * o.r) - (i * o.i), (r * o.i) + (i * o.r));
        }

        complex_fp conj() const {
            return complex_fp(r, fp32_t::from_raw(-i.raw));
        }
    };

    struct antenna_snapshot {
        complex_fp elements[NUM_ELEMENTS];
    };

    crpa_beamformer();

    /**
     * @brief Adaptif ağırlıkları varsayılan referans durumuna sıfırlar.
     */
    void reset();

    /**
     * @brief Adaptif LMS öğrenme katsayısını (mu) ayarlar.
     */
    void set_adaptation_rate(fp32_t mu);

    /**
     * @brief 4 antenden gelen eşzamanlı I/Q numunesini işler ve hüzmelenmiş çıkışı üretir.
     * Güç İnversiyonu (Power Inversion) algoritmasıyla karıştırıcıyı anında bastırır.
     * 
     * @param snapshot 4 anten elemanının I/Q örneği
     * @return complex_fp Bastırılmış ve hüzmelenmiş çıkış sinyali y[k]
     */
    complex_fp process_sample(const antenna_snapshot& snapshot);

    /**
     * @brief Belirli bir varış açısına (Azimuth) ana hüzme yönlendirme ağırlığı hesaplar.
     * 
     * @param target_azimuth_deg Dost istasyonun yönü (0.0° - 359.9°)
     */
    void steer_beam(fp32_t target_azimuth_deg);

    /**
     * @brief Verilen bir geliş açısındaki (azimuth) anten dizisi kazancını (dB) hesaplar.
     * 
     * @param test_azimuth_deg Test açısı (derece)
     * @return fp32_t Array kazancı (dB cinsinden)
     */
    fp32_t calculate_array_gain_db(fp32_t test_azimuth_deg) const;

    /**
     * @brief Mevcut adaptif ağırlıkları sorgular.
     */
    const complex_fp* get_weights() const { return weights_; }

private:
    complex_fp weights_[NUM_ELEMENTS];
    fp32_t mu_; // LMS adım büyüklüğü
    bool is_steered_;
};

} // namespace radio
} // namespace kok
