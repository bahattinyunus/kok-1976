#include "protocol.hpp"
#include <cstring>

namespace kok::radio {

stream_receiver::stream_receiver() {
    reset();
    std::memset(&m_stats, 0, sizeof(m_stats));
}

void stream_receiver::reset() {
    m_state = RxState::SYNC_0;
    m_header_idx = 0;
    m_payload_idx = 0;
    m_expected_payload_len = 0;
    m_crc_high = 0;
    std::memset(&m_cached_header, 0, sizeof(m_cached_header));
}

bool stream_receiver::process_byte(u8 byte, tactical_frame& out_frame) {
    ++m_stats.bytes_processed;

    switch (m_state) {
    case RxState::SYNC_0:
        if (byte == 0x19) m_state = RxState::SYNC_1;
        break;

    case RxState::SYNC_1:
        if (byte == 0x76) m_state = RxState::SYNC_2;
        else {
            m_state = (byte == 0x19) ? RxState::SYNC_1 : RxState::SYNC_0;
            ++m_stats.sync_losses;
        }
        break;

    case RxState::SYNC_2:
        if (byte == 0x4B) m_state = RxState::SYNC_3;
        else {
            m_state = (byte == 0x19) ? RxState::SYNC_1 : RxState::SYNC_0;
            ++m_stats.sync_losses;
        }
        break;

    case RxState::SYNC_3:
        if (byte == 0x4F) m_state = RxState::SYNC_4;
        else {
            m_state = (byte == 0x19) ? RxState::SYNC_1 : RxState::SYNC_0;
            ++m_stats.sync_losses;
        }
        break;

    case RxState::SYNC_4:
        if (byte == 0x4B) {
            m_state = RxState::HEADER;
            m_header_idx = 0;
        } else {
            m_state = (byte == 0x19) ? RxState::SYNC_1 : RxState::SYNC_0;
            ++m_stats.sync_losses;
        }
        break;

    case RxState::HEADER:
        m_header_buf[m_header_idx++] = byte;
        if (m_header_idx >= tactical_frame::HEADER_SIZE) {
            std::memcpy(&m_cached_header, m_header_buf, sizeof(frame_header));
            u8 calc_hcrc = crc::crc8(m_header_buf, sizeof(frame_header) - 1);

            if (calc_hcrc != m_cached_header.header_crc || m_cached_header.payload_len > tactical_frame::MAX_PAYLOAD) {
                // Başlık bozuk!
                ++m_stats.header_crc_errors;
                reset();
                return false;
            }

            m_expected_payload_len = m_cached_header.payload_len;
            m_payload_idx = 0;
            if (m_expected_payload_len == 0) {
                m_state = RxState::CRC_HIGH;
            } else {
                m_state = RxState::PAYLOAD;
            }
        }
        break;

    case RxState::PAYLOAD:
        m_payload_buf[m_payload_idx++] = byte;
        if (m_payload_idx >= m_expected_payload_len) {
            m_state = RxState::CRC_HIGH;
        }
        break;

    case RxState::CRC_HIGH:
        m_crc_high = byte;
        m_state = RxState::CRC_LOW;
        break;

    case RxState::CRC_LOW: {
        u8 crc_low = byte;
        u16 recv_crc = (static_cast<u16>(m_crc_high) << 8) | crc_low;

        // CRC kontrolü için tampon: Başlık + Faydalı Yük
        u8 check_buf[tactical_frame::HEADER_SIZE + tactical_frame::MAX_PAYLOAD];
        std::memcpy(check_buf, &m_cached_header, sizeof(frame_header));
        if (m_expected_payload_len > 0) {
            std::memcpy(&check_buf[sizeof(frame_header)], m_payload_buf, m_expected_payload_len);
        }

        u16 calc_crc = crc::crc16_ccitt(check_buf, sizeof(frame_header) + m_expected_payload_len);
        if (recv_crc == calc_crc) {
            // Başarılı paket!
            out_frame.set_header(m_cached_header.msg_type, m_cached_header.sender_id,
                                 m_cached_header.target_id, m_cached_header.seq_num,
                                 m_cached_header.hop_channel);
            out_frame.set_payload(m_payload_buf, m_expected_payload_len);
            ++m_stats.frames_received;
            reset();
            return true;
        } else {
            ++m_stats.frame_crc_errors;
            reset();
            return false;
        }
    }
    }

    return false;
}

usize stream_receiver::process_buffer(const u8* data, usize len, tactical_frame& out_frame) {
    for (usize i = 0; i < len; ++i) {
        if (process_byte(data[i], out_frame)) {
            return i + 1;
        }
    }
    return 0;
}

} // namespace kok::radio
