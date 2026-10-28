#ifndef KOK_MATH_HPP
#define KOK_MATH_HPP

#include "types.hpp"

namespace kok {

// ============================================================================
// HESAPLANABİLİRLİK VE CORDIC MOTORU (FPU-SUZ MİKRODENETLEYİCİLER İÇİN)
// Trigonometrik Fonksiyonlar, Faz ve Genlik Determinizmi
// ============================================================================

class math {
public:
    // CORDIC Açı Sabiti (Q1.15 radyan / açı formatı)
    static constexpr usize CORDIC_ITERATIONS = 16;
    
    // CORDIC ile Sinüs ve Kosinüs Hesabı
    // angle: -PI .. +PI aralığı Q16.16 sabit nokta (radyan)
    // out_sin, out_cos: Q16.16 çıktı
    static void cordic_sincos(fp32_t angle, fp32_t& out_sin, fp32_t& out_cos);

    // CORDIC ile Genlik ve Faz Hesabı (Kutupsal Dönüşüm)
    // in_i, in_q: Giriş I/Q bileşenleri
    // out_magnitude: Vektör genliği
    // out_phase: Vektör faz açısı (-PI .. +PI)
    static void cordic_cartesian_to_polar(fp32_t in_i, fp32_t in_q, fp32_t& out_magnitude, fp32_t& out_phase);

    // Hızlı Tamsayı Karekök (Newton-Raphson / Bit-Shift)
    static u32 isqrt(u32 val);

    // Sabit Noktalı Pi Değerleri
    static constexpr fp32_t pi() { return fp32_t::from_float(3.141592653589793f); }
    static constexpr fp32_t two_pi() { return fp32_t::from_float(6.283185307179586f); }
    static constexpr fp32_t half_pi() { return fp32_t::from_float(1.570796326794896f); }
};

} // namespace kok

#endif // KOK_MATH_HPP
