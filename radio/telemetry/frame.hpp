#ifndef KOK_RADIO_FRAME_HPP
#define KOK_RADIO_FRAME_HPP

#include "kok/types.hpp"
#include "kok/crc.hpp"

namespace kok::radio {

// ============================================================================
// TAKTİK TELEMETRİ ÇERÇEVESİ (TACTICAL TELEMETRY FRAME SPECIFICATION)
// 0x1976KOK Taktik Senkronizasyon Başlığı ile Gürültüye Dayanıklı Paket Mimarisi
// ============================================================================

#pragma pack(push, 1)
struct frame_header {
    u8  msg_type;       // Mesaj tipi (0x01: Konum, 0x02: Durum/Batarya, 0x03: Taktik Emir, 0x04: ACK)
    u16 sender_id;      // Gönderen Telsiz/İstasyon Kimliği
    u16 target_id;      // Hedef Telsiz/İstasyon Kimliği
    u16 seq_num;        // Paket Sıra Numarası
    u8  hop_channel;    // Kullanılan Frekans Atlama Kanalı (0..63)
    u8  payload_len;    // Faydalı Yük Boyutu (Azami MAX_PAYLOAD)
    u8  header_crc;     // Başlık Bütünlük Kontrolü (CRC-8)
};
#pragma pack(pop)

class tactical_frame {
public:
    static constexpr u32 SYNC_WORD = 0x19764B4Fu; // "1976 KO"
    static constexpr u8  SYNC_EXTRA = 0x4Bu;       // "K" -> 0x1976KOK
    static constexpr usize SYNC_SIZE = 5;
    static constexpr usize HEADER_SIZE = sizeof(frame_header);
    static constexpr usize MAX_PAYLOAD = 64;
    static constexpr usize CRC_SIZE = sizeof(u16);
    static constexpr usize MAX_FRAME_SIZE = SYNC_SIZE + HEADER_SIZE + MAX_PAYLOAD + CRC_SIZE;

    tactical_frame();

    void set_header(u8 msg_type, u16 sender_id, u16 target_id, u16 seq_num, u8 hop_channel);
    bool set_payload(const u8* data, u8 len);

    // Çerçeveyi bayt dizisine serileştir
    // Dönen değer yazılan toplam bayt sayısıdır (0 ise hata)
    usize serialize(u8* out_buffer, usize max_len) const;

    // Bayt dizisinden çerçeveyi ayrıştır ve CRC kontrolü yap
    static Status deserialize(const u8* in_buffer, usize len, tactical_frame& out_frame);

    const frame_header& header() const { return m_header; }
    const u8* payload() const { return m_payload; }
    u8 payload_length() const { return m_header.payload_len; }
    u16 frame_crc() const { return m_frame_crc; }

    bool is_valid() const;

private:
    frame_header m_header;
    u8  m_payload[MAX_PAYLOAD];
    u16 m_frame_crc;
};

} // namespace kok::radio

#endif // KOK_RADIO_FRAME_HPP
