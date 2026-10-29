#include "direction_finder.hpp"

namespace kok {
namespace radio {

direction_finder::direction_finder() {
    reset();
}

void direction_finder::reset() {
    sum_ns_ = fp32_t::from_raw(0);
    sum_ew_ = fp32_t::from_raw(0);
    sum_sense_ = fp32_t::from_raw(0);
    sum_energy_ = fp32_t::from_raw(0);
    sample_count_ = 0;
}

void direction_finder::feed_sample(fp32_t v_ns, fp32_t v_ew, fp32_t v_sense) {
    if (sample_count_ >= MAX_SAMPLES) {
        return;
    }

    // Watson-Watt koherent çarpım: <V_NS, V_Sense> ve <V_EW, V_Sense>
    sum_ns_ = sum_ns_ + (v_ns * v_sense);
    sum_ew_ = sum_ew_ + (v_ew * v_sense);
    sum_sense_ = sum_sense_ + v_sense;

    // Enerji = v_ns^2 + v_ew^2 + v_sense^2
    fp32_t energy = (v_ns * v_ns) + (v_ew * v_ew) + (v_sense * v_sense);
    sum_energy_ = sum_energy_ + energy;
    sample_count_++;
}

direction_finder::bearing_result direction_finder::calculate_bearing() const {
    bearing_result res;
    res.azimuth_deg = fp32_t::from_raw(0);
    res.confidence = fp32_t::from_raw(0);
    res.rssi_dbm = fp32_t::from_float(-120.0f);
    res.valid = false;

    if (sample_count_ == 0) {
        return res;
    }

    // Ortalama koherent genlikler: dot_ns (Kuzey-Güney) ve dot_ew (Doğu-Batı)
    fp32_t count_fp = fp32_t::from_int(static_cast<i32>(sample_count_));
    fp32_t dot_ns = sum_ns_ / count_fp;
    fp32_t dot_ew = sum_ew_ / count_fp;

    // CORDIC ile kutupsal dönüşüm: x = Kuzey-Güney (Y ekseni), y = Doğu-Batı (X ekseni)
    fp32_t mag, phase_rad;
    math::cordic_cartesian_to_polar(dot_ns, dot_ew, mag, phase_rad);

    if (mag.to_float() < 0.0001f) {
        // Sinyal çok zayıf, kerteriz güvenilmez
        return res;
    }

    // Radyanı dereceye çevir (180 / PI ≈ 57.2957795)
    fp32_t rad_to_deg = fp32_t::from_float(57.2957795f);
    fp32_t deg = phase_rad * rad_to_deg;

    // Negatif açıları [0, 360) aralığına sar
    if (deg.to_float() < 0.0f) {
        deg = deg + fp32_t::from_float(360.0f);
    }

    res.azimuth_deg = deg;

    // Güvenilirlik: Sinyal genliğinin doygunluk oranı
    float mag_val = mag.to_float();
    float conf_val = mag_val / (mag_val + 0.1f);
    if (conf_val > 0.99f) conf_val = 0.99f;
    res.confidence = fp32_t::from_float(conf_val);

    // Yaklaşık RSSI kestirimi (dBm)
    float rssi_est = -90.0f + (mag_val * 40.0f);
    if (rssi_est > -20.0f) rssi_est = -20.0f;
    res.rssi_dbm = fp32_t::from_float(rssi_est);
    res.valid = (res.confidence.to_float() > 0.20f);

    return res;
}

bool direction_finder::triangulate_target(fp32_t x1, fp32_t y1, fp32_t theta1,
                                          fp32_t x2, fp32_t y2, fp32_t theta2,
                                          fp32_t& out_x, fp32_t& out_y) {
    // Dereceyi radyana çevir
    float deg_to_rad = 3.14159265f / 180.0f;
    float r1 = theta1.to_float() * deg_to_rad;
    float r2 = theta2.to_float() * deg_to_rad;

    // CORDIC ile sin/cos hesapla
    fp32_t s1, c1, s2, c2;
    math::cordic_sincos(fp32_t::from_float(r1), s1, c1);
    math::cordic_sincos(fp32_t::from_float(r2), s2, c2);

    // Delta X ve Y
    float dx = (x2 - x1).to_float();
    float dy = (y2 - y1).to_float();

    // Determinant D = sin(r2 - r1) = c1*s2 - s1*c2
    float det = (c1 * s2 - s1 * c2).to_float();
    if (det > -0.05f && det < 0.05f) {
        // Işınlar neredeyse paralel (Kesişim hesaplanamaz)
        return false;
    }

    // Parametre t1
    float t1 = (dy * s2.to_float() - dx * c2.to_float()) / det;
    if (t1 <= 0.0f) {
        // Kesişim ışının arkasında kalıyor
        return false;
    }

    float target_x = x1.to_float() + t1 * s1.to_float();
    float target_y = y1.to_float() + t1 * c1.to_float();

    out_x = fp32_t::from_float(target_x);
    out_y = fp32_t::from_float(target_y);
    return true;
}

} // namespace radio
} // namespace kok
