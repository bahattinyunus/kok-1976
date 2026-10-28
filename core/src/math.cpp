#include "kok/math.hpp"

namespace kok {

// CORDIC Açı Tablosu (Radyan cinsinden Q16.16 sabit nokta)
// atan(2^-i) for i = 0..15
static const i32 CORDIC_ANGLES_Q16[16] = {
    51472,  // atan(1)       = 0.785398 rad
    30386,  // atan(0.5)     = 0.463648 rad
    16055,  // atan(0.25)    = 0.244979 rad
    8150,   // atan(0.125)   = 0.124355 rad
    4091,   // atan(0.0625)  = 0.062419 rad
    2047,   // atan(0.03125) = 0.031240 rad
    1024,   // atan(2^-6)    = 0.015624 rad
    512,    // atan(2^-7)    = 0.007812 rad
    256,    // atan(2^-8)    = 0.003906 rad
    128,    // atan(2^-9)    = 0.001953 rad
    64,     // atan(2^-10)   = 0.000977 rad
    32,     // atan(2^-11)   = 0.000488 rad
    16,     // atan(2^-12)   = 0.000244 rad
    8,      // atan(2^-13)   = 0.000122 rad
    4,      // atan(2^-14)   = 0.000061 rad
    2       // atan(2^-15)   = 0.000031 rad
};

// CORDIC Kazanç Çarpanı Sabiti: 1 / K ~= 0.607252935 * 65536 = 39797
static constexpr i32 CORDIC_INV_K_Q16 = 39797;
static constexpr i32 PI_Q16 = 205887;         // 3.14159265 * 65536
static constexpr i32 HALF_PI_Q16 = 102944;    // 1.57079632 * 65536

void math::cordic_sincos(fp32_t angle, fp32_t& out_sin, fp32_t& out_cos) {
    i32 theta = angle.raw;

    // Açıyı [-PI, PI] aralığına normalize et
    while (theta > PI_Q16)  theta -= 2 * PI_Q16;
    while (theta < -PI_Q16) theta += 2 * PI_Q16;

    // Açıyı [-PI/2, PI/2] aralığına indirgeme
    i32 x = CORDIC_INV_K_Q16;
    i32 y = 0;

    if (theta > HALF_PI_Q16) {
        theta -= PI_Q16;
        x = -x;
    } else if (theta < -HALF_PI_Q16) {
        theta += PI_Q16;
        x = -x;
    }

    i32 z = theta;

    for (usize i = 0; i < CORDIC_ITERATIONS; ++i) {
        i32 x_new, y_new;
        if (z >= 0) {
            x_new = x - (y >> i);
            y_new = y + (x >> i);
            z -= CORDIC_ANGLES_Q16[i];
        } else {
            x_new = x + (y >> i);
            y_new = y - (x >> i);
            z += CORDIC_ANGLES_Q16[i];
        }
        x = x_new;
        y = y_new;
    }

    out_cos = fp32_t(x, true);
    out_sin = fp32_t(y, true);
}

void math::cordic_cartesian_to_polar(fp32_t in_i, fp32_t in_q, fp32_t& out_magnitude, fp32_t& out_phase) {
    i32 x = in_i.raw;
    i32 y = in_q.raw;
    i32 z = 0;

    // Sol yarı düzlem için açı düzeltmesi
    if (x < 0) {
        x = -x;
        y = -y;
        z = (y >= 0) ? PI_Q16 : -PI_Q16;
    }

    for (usize i = 0; i < CORDIC_ITERATIONS; ++i) {
        i32 x_new, y_new;
        if (y < 0) {
            x_new = x - (y >> i);
            y_new = y + (x >> i);
            z -= CORDIC_ANGLES_Q16[i];
        } else {
            x_new = x + (y >> i);
            y_new = y - (x >> i);
            z += CORDIC_ANGLES_Q16[i];
        }
        x = x_new;
        y = y_new;
    }

    // Genlik CORDIC_K ile çarpılmalı (x / K = x * inv_K)
    i64 mag_calc = (static_cast<i64>(x) * CORDIC_INV_K_Q16) >> fp32_t::FRACTION_BITS;
    out_magnitude = fp32_t(static_cast<i32>(mag_calc), true);
    out_phase = fp32_t(z, true);
}

u32 math::isqrt(u32 val) {
    if (val == 0) return 0;
    u32 res = 0;
    u32 bit = 1u << 30; // 2^30

    while (bit > val) {
        bit >>= 2;
    }

    while (bit != 0) {
        if (val >= res + bit) {
            val -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}

} // namespace kok
