#ifndef KOK_ANTI_TAMPER_HPP
#define KOK_ANTI_TAMPER_HPP

#include "types.hpp"

namespace kok {

// ============================================================================
// ELEKTRONİK HARP VE FİZİKSEL ELE GEÇİRİLME KARŞITI KORUMA (ANTI-TAMPER)
// Acil Kripto İmha (Zeroize) ve Çok Aşamalı Bellek Temizleme Standartları
// DoD 5220.22-M ve Millî Kripto Güvenlik Doktrini (Mehmet Hâcim Kamoy)
// ============================================================================

enum class TamperSource : u8 {
    NONE                  = 0x00,
    EMERGENCY_BUTTON      = 0x01, // Operatörün Acil Sıfırlama Mandalı
    CHASSIS_INTRUSION     = 0x02, // Gövde Kapağının Zorla Açılması
    VOLTAGE_GLITCH        = 0x03, // Düşman Güç Analizi / Gerilim Saldırısı
    TEMPERATURE_EXTREME   = 0x04, // Kriyojenik / Termal Dondurma Saldırısı
    CRYPTO_AUTH_FAULT     = 0x05, // Ardışık Kriptografik Kimlik Doğrulama Hatası
    COMMAND_OVER_THE_AIR  = 0x06  // Karargâhtan Gönderilen Uzaktan İmha Emri
};

struct zeroize_audit {
    bool is_zeroized;
    TamperSource trigger_reason;
    u32 timestamp_ticks;
    usize bytes_purged;
    bool verification_passed;
};

class anti_tamper {
public:
    anti_tamper();

    /**
     * @brief Hassas bellek bölgesini askeri standartta çok geçişli olarak imha eder.
     * 1. Geçiş: 0x55 (01010101)
     * 2. Geçiş: 0xAA (10101010)
     * 3. Geçiş: 0xFF (11111111)
     * 4. Geçiş: 0x00 (00000000)
     * Bellek bariyeri ile derleyicinin optimizasyonla silmesini engeller.
     */
    static void secure_wipe(volatile void* ptr, usize length);

    /**
     * @brief Belleğin tamamen 0x00 olduğunu ve kalıntı taşımadığını doğrular.
     */
    static bool verify_clean(const volatile void* ptr, usize length);

    /**
     * @brief Acil durum imha mekanizmasını (Zeroize) tetikler.
     */
    void trigger_zeroize(TamperSource reason, u32 current_time_ticks);

    /**
     * @brief Telsizin imha durumunda olup olmadığını sorgular.
     */
    bool is_zeroized() const { return m_audit.is_zeroized; }

    /**
     * @brief İmha denetim kaydını döndürür.
     */
    const zeroize_audit& get_audit() const { return m_audit; }

    /**
     * @brief Telsiz durumunu yeniden operasyonel hale getirir (Yalnızca Güvenli Bakım Modunda)
     */
    void reset_state();

private:
    zeroize_audit m_audit;
};

} // namespace kok

#endif // KOK_ANTI_TAMPER_HPP
