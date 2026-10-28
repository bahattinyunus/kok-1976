#ifndef KOK_RADIO_PROTOCOL_HPP
#define KOK_RADIO_PROTOCOL_HPP

#include "frame.hpp"
#include "kok/ring_buffer.hpp"

namespace kok::radio {

// ============================================================================
// TAKTİK TELEMETRİ AKIŞ PROTOKOLÜ (STREAM PROTOCOL STATE MACHINE)
// Gürültülü ve Bayt Kayıplı Kanallarda Donanımsal Senkronizasyon Yakalama
// ============================================================================

enum class RxState : u8 {
    SYNC_0 = 0,
    SYNC_1,
    SYNC_2,
    SYNC_3,
    SYNC_4,
    HEADER,
    PAYLOAD,
    CRC_HIGH,
    CRC_LOW
};

struct protocol_stats {
    u32 frames_received;
    u32 header_crc_errors;
    u32 frame_crc_errors;
    u32 sync_losses;
    u32 bytes_processed;
};

class stream_receiver {
public:
    stream_receiver();

    void reset();

    // Gelen baytı durum makinesine besle.
    // Bir tam çerçeve başarıyla tamamlandığında true döner ve out_frame dolar.
    bool process_byte(u8 byte, tactical_frame& out_frame);

    // Bir bayt dizisini işle
    usize process_buffer(const u8* data, usize len, tactical_frame& out_frame);

    const protocol_stats& stats() const { return m_stats; }
    RxState current_state() const { return m_state; }

private:
    RxState m_state;
    protocol_stats m_stats;

    u8 m_header_buf[tactical_frame::HEADER_SIZE];
    usize m_header_idx;

    u8 m_payload_buf[tactical_frame::MAX_PAYLOAD];
    usize m_payload_idx;
    u8 m_expected_payload_len;

    u8 m_crc_high;
    frame_header m_cached_header;
};

} // namespace kok::radio

#endif // KOK_RADIO_PROTOCOL_HPP
