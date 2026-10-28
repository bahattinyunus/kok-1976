#include <iostream>
#include <cassert>
#include <cstring>

#include "kok/types.hpp"
#include "kok/static_vector.hpp"
#include "kok/ring_buffer.hpp"
#include "kok/memory_pool.hpp"
#include "kok/math.hpp"
#include "kok/crc.hpp"
#include "kok/fec.hpp"
#include "kok/crypto.hpp"
#include "kok/lpi_lpd.hpp"
#include "fsk.hpp"
#include "fhss.hpp"
#include "dsss.hpp"
#include "frame.hpp"
#include "protocol.hpp"

using namespace kok;
using namespace kok::radio;

void test_types_and_containers() {
    std::cout << "[+] Test Ediliyor: static_vector, ring_buffer, memory_pool..." << std::endl;

    // 1. static_vector testi
    static_vector<u32, 8> vec;
    assert(vec.empty());
    assert(vec.push_back(1976));
    assert(vec.push_back(2026));
    assert(vec.size() == 2);
    assert(vec[0] == 1976);
    assert(vec[1] == 2026);

    // 2. ring_buffer testi
    ring_buffer<u8, 16> rb;
    assert(rb.empty());
    for (u8 i = 0; i < 16; ++i) {
        assert(rb.push(i));
    }
    assert(rb.full());
    assert(!rb.push(99)); // Taşırma engellendi
    u8 popped = 0;
    for (u8 i = 0; i < 16; ++i) {
        assert(rb.pop(popped));
        assert(popped == i);
    }
    assert(rb.empty());

    // 3. memory_pool testi
    memory_pool<64, 4> pool;
    assert(pool.free_count() == 4);
    void* b1 = pool.allocate();
    void* b2 = pool.allocate();
    assert(b1 != nullptr && b2 != nullptr && b1 != b2);
    assert(pool.allocated_count() == 2);
    assert(pool.deallocate(b1));
    assert(pool.deallocate(b2));
    assert(pool.free_count() == 4);

    // 4. Sabit noktalı matematik testi
    fp32_t a = fp32_t::from_float(2.5f);
    fp32_t b = fp32_t::from_float(4.0f);
    fp32_t c = a * b;
    assert(c.to_int() == 10);
}

void test_math_and_cordic() {
    std::cout << "[+] Test Ediliyor: CORDIC ve Matematik Motoru..." << std::endl;

    fp32_t s, c;
    math::cordic_sincos(math::half_pi(), s, c);
    float s_flt = s.to_float();
    float c_flt = c.to_float();
    assert(s_flt >= 0.99f && s_flt <= 1.01f);
    assert(c_flt >= -0.01f && c_flt <= 0.01f);

    fp32_t mag, phase;
    math::cordic_cartesian_to_polar(fp32_t::from_float(3.0f), fp32_t::from_float(4.0f), mag, phase);
    float mag_flt = mag.to_float();
    assert(mag_flt >= 4.95f && mag_flt <= 5.05f); // 3-4-5 üçgeni

    assert(math::isqrt(144) == 12);
    assert(math::isqrt(1000000) == 1000);
}

void test_crc_and_fec() {
    std::cout << "[+] Test Ediliyor: CRC-8/16/32, Hamming(8,4) ve Golay(24,12)..." << std::endl;

    const u8 test_bytes[] = "KOK-1976-MACUNKOY-DEFENSE";
    usize len = sizeof(test_bytes) - 1;

    u8 c8 = crc::crc8(test_bytes, len);
    u16 c16 = crc::crc16_ccitt(test_bytes, len);
    u32 c32 = crc::crc32(test_bytes, len);
    assert(c8 != 0 && c16 != 0 && c32 != 0);

    // Hamming(8,4) testi
    for (u8 nib = 0; nib < 16; ++nib) {
        u8 code = fec::hamming84_encode(nib);
        u8 dec = 0;
        assert(fec::hamming84_decode(code, dec) == Status::SUCCESS);
        assert(dec == nib);

        // 1-bit hata düzeltme
        for (u8 b = 0; b < 8; ++b) {
            u8 corrupted = code ^ (1 << b);
            u8 corr_dec = 0;
            assert(fec::hamming84_decode(corrupted, corr_dec) == Status::SUCCESS);
            assert(corr_dec == nib);
        }
    }

    // Golay(24,12) testi
    u16 test_words[] = {0x000, 0x123, 0xABC, 0xFFF};
    for (u16 w : test_words) {
        u32 code24 = fec::golay24_encode(w);
        u16 out_w = 0;
        usize err_cnt = 0;
        assert(fec::golay24_decode(code24, out_w, err_cnt) == Status::SUCCESS);
        assert(out_w == w && err_cnt == 0);

        // 1, 2, 3 bit hatalarını düzeltme
        u32 c1 = code24 ^ (1 << 3);
        assert(fec::golay24_decode(c1, out_w, err_cnt) == Status::SUCCESS);
        assert(out_w == w && err_cnt == 1);

        u32 c2 = code24 ^ (1 << 4) ^ (1 << 17);
        assert(fec::golay24_decode(c2, out_w, err_cnt) == Status::SUCCESS);
        assert(out_w == w && err_cnt == 2);

        u32 c3 = code24 ^ (1 << 0) ^ (1 << 8) ^ (1 << 22);
        assert(fec::golay24_decode(c3, out_w, err_cnt) == Status::SUCCESS);
        assert(out_w == w && err_cnt == 3);
    }
}

void test_crypto_and_fhss() {
    std::cout << "[+] Test Ediliyor: Kamoy-76 Kripto Akışı, DSSS ve FHSS..." << std::endl;

    cipher c1(0x1976A5A51976A5A5ULL, 0x00000042);
    cipher c2(0x1976A5A51976A5A5ULL, 0x00000042);

    u8 secret_msg[] = "GIZLI TALIMAT: MACUNKOY TELSISI HARBE HAZIR.";
    usize slen = sizeof(secret_msg) - 1;

    u8 encrypted[64];
    std::memcpy(encrypted, secret_msg, slen);
    c1.process(encrypted, slen);
    assert(std::memcmp(secret_msg, encrypted, slen) != 0);

    c2.process(encrypted, slen);
    assert(std::memcmp(secret_msg, encrypted, slen) == 0);

    // DSSS Barker-11 Testi
    const u8 dsss_raw[] = {0xA5, 0x5A};
    i8 spread_chips[16 * dsss_engine::CHIPS_PER_BIT];
    usize num_chips = dsss_engine::spread_bytes(dsss_raw, 2, spread_chips, sizeof(spread_chips));
    assert(num_chips == 16 * 11);

    u8 recovered_dsss[4] = {0};
    usize rec_bytes = dsss_engine::despread_chips(spread_chips, num_chips, recovered_dsss, sizeof(recovered_dsss));
    assert(rec_bytes == 2);
    assert(recovered_dsss[0] == 0xA5 && recovered_dsss[1] == 0x5A);

    // LPI/LPD Metrikleri Testi
    float gp = lpi_lpd::calculate_processing_gain_db(250000.0f, 2400.0f);
    assert(gp > 15.0f && gp < 25.0f);
    float jm = lpi_lpd::calculate_jamming_margin_db(gp, 6.0f);
    assert(jm > 0.0f);

    // FHSS Testi
    fhss_engine fhss(0x1976A5A51976A5A5ULL, 16);
    u16 ch0 = fhss.next_channel();
    u16 ch1 = fhss.next_channel();
    assert(ch0 < fhss_engine::NUM_CHANNELS);
    assert(ch1 < fhss_engine::NUM_CHANNELS);
}

void test_telemetry_and_stream() {
    std::cout << "[+] Test Ediliyor: Taktik Çerçeve ve Akış Ayrıştırıcı Durum Makinesi..." << std::endl;

    tactical_frame tx_frame;
    tx_frame.set_header(0x02, 0x4600, 0x1976, 101, 7);
    const u8 payload[] = "TELEMETRI: AKU=24.5V, SICAKLIK=-15C";
    tx_frame.set_payload(payload, sizeof(payload) - 1);

    u8 wire_buf[tactical_frame::MAX_FRAME_SIZE];
    usize wire_len = tx_frame.serialize(wire_buf, sizeof(wire_buf));
    assert(wire_len > 0);

    // Tek baytlık akış test simülasyonu
    stream_receiver rx;
    tactical_frame rx_frame;
    bool frame_caught = false;

    // Önce hatta rastgele gürültü baytları gönder
    rx.process_byte(0xAA, rx_frame);
    rx.process_byte(0x55, rx_frame);

    for (usize i = 0; i < wire_len; ++i) {
        if (rx.process_byte(wire_buf[i], rx_frame)) {
            frame_caught = true;
            break;
        }
    }

    assert(frame_caught);
    assert(rx_frame.header().sender_id == 0x4600);
    assert(rx_frame.header().seq_num == 101);
    assert(rx_frame.payload_length() == sizeof(payload) - 1);
    assert(std::memcmp(rx_frame.payload(), payload, rx_frame.payload_length()) == 0);
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << "  KOK-1976 C++ CEKIRDEK BIRIM VE DOKTRIN TESTLERI (C++17)   " << std::endl;
    std::cout << "============================================================" << std::endl;

    test_types_and_containers();
    test_math_and_cordic();
    test_crc_and_fec();
    test_crypto_and_fhss();
    test_telemetry_and_stream();

    std::cout << "============================================================" << std::endl;
    std::cout << "  TUM C++ CEKIRDEK TESTLERI BASARIYLA TAMAMLANDI! (OK)      " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
