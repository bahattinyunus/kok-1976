#ifndef KOK_CRYPTO_HPP
#define KOK_CRYPTO_HPP

#include "types.hpp"

namespace kok {

// ============================================================================
// TAKTİK AKIŞ ŞİFRELEYİCİSİ (KAMOY-76 TACTICAL CIPHER)
// Harici Kütüphanesiz, Deterministik, Düşük Kapı Sayılı Kripto Modülü
// Çift-LFSR Doğrusal Olmayan Filtreleme ve Karıştırma
// ============================================================================

class cipher {
public:
    cipher();
    cipher(u64 key, u32 iv);

    void init(u64 key, u32 iv);

    // Bir sonraki anahtar akışı (keystream) baytını üret
    u8 next_byte();

    // Veriyi yerinde (in-place) şifrele / şifresini çöz
    void process(u8* data, usize len);

    // Taktik Mesaj Kimlik Doğrulama Kodu (MAC) Hesabı
    static u32 calculate_mac(u64 key, const u8* data, usize len);

private:
    u32 m_lfsr1; // 31-bit Galois LFSR
    u32 m_lfsr2; // 32-bit Fibonacci LFSR
    u32 m_counter;
    u8  m_sbox[16];

    void step();
};

} // namespace kok

#endif // KOK_CRYPTO_HPP
