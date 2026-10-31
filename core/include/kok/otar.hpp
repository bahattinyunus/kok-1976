#ifndef KOK_OTAR_HPP
#define KOK_OTAR_HPP

#include "types.hpp"
#include "crypto.hpp"
#include "anti_tamper.hpp"

namespace kok {

// ============================================================================
// HAVA ÜZERİNDEN ANAHTAR YENİLEME VE ASKERÎ TRANSEC (OTAR) MİMARİSİ
// KEK (Key Encryption Key) ve TEK (Traffic Encryption Key) Hiyerarşisi
// ASELSAN 4600/9600 Taktik Telsiz Kripto Yönetim Doktrini
// ============================================================================

struct otar_message {
    static constexpr u32 MAGIC_HEADER = 0x7652454B; // 'vREK' (0x76, 'R', 'E', 'K')
    static constexpr usize SIZE = 24;

    u32 magic;             // 0x7652454B
    u16 sequence;          // Monotonik Artan Sıra Numarası (Anti-Replay)
    u16 key_id;            // Yeni Anahtar İndeksi / Gün Numarası
    u8  encrypted_tek[8];  // KEK ile Şifrelenmiş Yeni Trafik Anahtarı (TEK)
    u32 iv;                // Oturum Başlangıç Vektörü
    u32 mac;               // Mesaj Kimlik Doğrulama Kodu (MAC)
};

class otar_manager {
public:
    otar_manager();
    explicit otar_manager(u64 master_kek);

    /**
     * @brief Kripto Yetkilisi tarafından KEK anahtarını cihaza yükler.
     */
    void load_kek(u64 master_kek);

    /**
     * @brief Karargâh/Merkez İstasyonu için OTAR Yenileme Paketi üretir.
     * 
     * @param new_tek Dağıtılacak yeni operasyonel trafik anahtarı
     * @param key_id Anahtar kimliği
     * @param sequence_num Artan sıra no
     * @param iv Oturum IV
     * @param out_msg Üretilen 24 baytlık OTAR paketi
     */
    bool create_rekey_packet(u64 new_tek, u16 key_id, u16 sequence_num, u32 iv, otar_message& out_msg);

    /**
     * @brief Taktik telsizde havadan gelen OTAR paketini işler, doğrular ve anahtarı günceller.
     * 
     * @param in_msg Havadan alınan OTAR paketi
     * @return true Kimlik doğrulandı ve yeni TEK aktif edildi
     */
    bool process_rekey_packet(const otar_message& in_msg);

    /**
     * @brief Aktif trafik anahtarını (TEK) sorgular.
     */
    u64 get_active_tek() const { return m_active_tek; }

    /**
     * @brief Aktif anahtar kimliğini sorgular.
     */
    u16 get_active_key_id() const { return m_active_key_id; }

    /**
     * @brief Acil Kripto İmha: KEK, TEK ve tüm oturum kayıtlarını anında bellekten siler.
     */
    void emergency_zeroize(TamperSource reason, u32 current_ticks);

    /**
     * @brief Cihazın kriptografik olarak hazır olup olmadığını döndürür.
     */
    bool is_operational() const { return m_is_kek_loaded && !m_tamper.is_zeroized(); }

    /**
     * @brief Anti-tamper durum denetleyicisi
     */
    const anti_tamper& get_tamper_unit() const { return m_tamper; }

private:
    u64 m_kek;
    u64 m_active_tek;
    u16 m_active_key_id;
    u16 m_last_sequence;
    bool m_is_kek_loaded;
    anti_tamper m_tamper;
};

} // namespace kok

#endif // KOK_OTAR_HPP
