#include "crpa_beamformer.hpp"
#include <cmath>

namespace kok {
namespace radio {

crpa_beamformer::crpa_beamformer() {
    reset();
}

void crpa_beamformer::reset() {
    // Referans eleman ağırlığı: w0 = 1.0 + 0j
    weights_[0] = complex_fp(fp32_t::from_float(1.0f), fp32_t::from_raw(0));
    // Diğer elemanlar başlangıçta 0
    for (usize m = 1; m < NUM_ELEMENTS; ++m) {
        weights_[m] = complex_fp(fp32_t::from_raw(0), fp32_t::from_raw(0));
    }
    // Varsayılan öğrenme oranı mu = 0.005
    mu_ = fp32_t::from_float(0.005f);
    is_steered_ = false;
}

void crpa_beamformer::set_adaptation_rate(fp32_t mu) {
    mu_ = mu;
}

crpa_beamformer::complex_fp crpa_beamformer::process_sample(const antenna_snapshot& snapshot) {
    // 1. Çıkış hesabı: y = w0* x0 + w1* x1 + w2* x2 + w3* x3
    complex_fp y(fp32_t::from_raw(0), fp32_t::from_raw(0));
    for (usize m = 0; m < NUM_ELEMENTS; ++m) {
        complex_fp term = weights_[m].conj() * snapshot.elements[m];
        y = y + term;
    }

    // 2. Normalizasyon için anlık güç hesabı: P = sum(|x_m|^2)
    float norm_power = 1e-4f;
    for (usize m = 0; m < NUM_ELEMENTS; ++m) {
        float xr = snapshot.elements[m].r.to_float();
        float xi = snapshot.elements[m].i.to_float();
        norm_power += (xr * xr + xi * xi);
    }
    float step = mu_.to_float() / norm_power;
    fp32_t step_fp = fp32_t::from_float(step);

    // 3. Güç İnversiyonu (Power Inversion) Adaptif Güncelleme:
    // w0 sabit tutulur (referans), w1..w3 karıştırıcıyı yok etmek için güncellenir
    // w_m[k+1] = w_m[k] - (mu / P) * conj(y[k]) * x_m[k]
    complex_fp y_conj = y.conj();
    for (usize m = 1; m < NUM_ELEMENTS; ++m) {
        complex_fp grad = y_conj * snapshot.elements[m];
        complex_fp delta(grad.r * step_fp, grad.i * step_fp);
        weights_[m] = weights_[m] - delta;
    }

    return y;
}

void crpa_beamformer::steer_beam(fp32_t target_azimuth_deg) {
    float deg = target_azimuth_deg.to_float();
    float rad = deg * (3.14159265f / 180.0f);
    float sin_a = std::sin(rad);
    float cos_a = std::cos(rad);

    // Dizi eleman fazları: psi = PI * (x_m * sin + y_m * cos)
    // m=0: (0,0) -> 0
    // m=1: (1,0) -> PI * sin
    // m=2: (0,1) -> PI * cos
    // m=3: (1,1) -> PI * (sin + cos)
    float phases[NUM_ELEMENTS] = {
        0.0f,
        3.14159265f * sin_a,
        3.14159265f * cos_a,
        3.14159265f * (sin_a + cos_a)
    };

    for (usize m = 0; m < NUM_ELEMENTS; ++m) {
        // w_m = (1 / sqrt(4)) * exp(j * psi_m) = 0.5 * (cos + j*sin)
        float c = 0.5f * std::cos(phases[m]);
        float s = 0.5f * std::sin(phases[m]);
        weights_[m] = complex_fp(fp32_t::from_float(c), fp32_t::from_float(s));
    }
    is_steered_ = true;
}

fp32_t crpa_beamformer::calculate_array_gain_db(fp32_t test_azimuth_deg) const {
    float deg = test_azimuth_deg.to_float();
    float rad = deg * (3.14159265f / 180.0f);
    float sin_a = std::sin(rad);
    float cos_a = std::cos(rad);

    float phases[NUM_ELEMENTS] = {
        0.0f,
        3.14159265f * sin_a,
        3.14159265f * cos_a,
        3.14159265f * (sin_a + cos_a)
    };

    // Array faktörü: AF = sum(w_m^* * exp(j * psi_m))
    float total_r = 0.0f;
    float total_i = 0.0f;

    for (usize m = 0; m < NUM_ELEMENTS; ++m) {
        float sv_r = std::cos(phases[m]);
        float sv_i = std::sin(phases[m]);

        float w_r = weights_[m].r.to_float();
        float w_i = -weights_[m].i.to_float(); // conj(w)

        total_r += (w_r * sv_r - w_i * sv_i);
        total_i += (w_r * sv_i + w_i * sv_r);
    }

    float power = total_r * total_r + total_i * total_i;
    if (power < 1e-6f) {
        power = 1e-6f;
    }

    float gain_db = 10.0f * std::log10(power);
    return fp32_t::from_float(gain_db);
}

} // namespace radio
} // namespace kok
