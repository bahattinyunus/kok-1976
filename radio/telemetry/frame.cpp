#include "frame.hpp"
#include <cstring>

namespace kok::radio {

tactical_frame::tactical_frame() : m_frame_crc(0) {
    std::memset(&m_header, 0, sizeof(m_header));
    std::memset(m_payload, 0, sizeof(m_payload));
}

void tactical_frame::set_header(u8 msg_type, u16 sender_id, u16 target_id, u16 seq_num, u8 hop_channel) {
    m_header.msg_type = msg_type;
    m_header.sender_id = sender_id;
    m_header.target_id = target_id;
    m_header.seq_num = seq_num;
    m_header.hop_channel = hop_channel;
    m_header.header_crc = crc::crc8(reinterpret_cast<const u8*>(&m_header), sizeof(m_header) - 1);
}

bool tactical_frame::set_payload(const u8* data, u8 len) {
    if (len > MAX_PAYLOAD) return false;
    m_header.payload_len = len;
    if (len > 0 && data != nullptr) {
        std::memcpy(m_payload, data, len);
    }
    // Başlık CRC'sini güncelle
    m_header.header_crc = crc::crc8(reinterpret_cast<const u8*>(&m_header), sizeof(m_header) - 1);
    return true;
}

usize tactical_frame::serialize(u8* out_buffer, usize max_len) const {
    usize total_len = SYNC_SIZE + HEADER_SIZE + m_header.payload_len + CRC_SIZE;
    if (max_len < total_len) return 0;

    // 1. Senkronizasyon Baytları (0x19, 0x76, 0x4B, 0x4F, 0x4B)
    out_buffer[0] = 0x19;
    out_buffer[1] = 0x76;
    out_buffer[2] = 0x4B;
    out_buffer[3] = 0x4F;
    out_buffer[4] = 0x4B;

    // 2. Başlık
    std::memcpy(&out_buffer[SYNC_SIZE], &m_header, HEADER_SIZE);

    // 3. Faydalı Yük
    if (m_header.payload_len > 0) {
        std::memcpy(&out_buffer[SYNC_SIZE + HEADER_SIZE], m_payload, m_header.payload_len);
    }

    // 4. Çerçeve CRC-16 (Başlık + Faydalı Yük üzerinden)
    u16 calc_crc = crc::crc16_ccitt(&out_buffer[SYNC_SIZE], HEADER_SIZE + m_header.payload_len);
    out_buffer[total_len - 2] = static_cast<u8>((calc_crc >> 8) & 0xFF);
    out_buffer[total_len - 1] = static_cast<u8>(calc_crc & 0xFF);

    return total_len;
}

Status tactical_frame::deserialize(const u8* in_buffer, usize len, tactical_frame& out_frame) {
    if (len < SYNC_SIZE + HEADER_SIZE + CRC_SIZE) {
        return Status::ERROR_INVALID_PARAM;
    }

    // 1. Senkronizasyon Kontrolü
    if (in_buffer[0] != 0x19 || in_buffer[1] != 0x76 || in_buffer[2] != 0x4B ||
        in_buffer[3] != 0x4F || in_buffer[4] != 0x4B) {
        return Status::ERROR_SYNC_LOST;
    }

    // 2. Başlık Kopyalama
    std::memcpy(&out_frame.m_header, &in_buffer[SYNC_SIZE], HEADER_SIZE);

    // 3. Başlık CRC-8 Kontrolü
    u8 expected_hcrc = crc::crc8(reinterpret_cast<const u8*>(&out_frame.m_header), HEADER_SIZE - 1);
    if (expected_hcrc != out_frame.m_header.header_crc) {
        return Status::ERROR_CRC_MISMATCH;
    }

    if (out_frame.m_header.payload_len > MAX_PAYLOAD) {
        return Status::ERROR_INVALID_PARAM;
    }

    usize expected_total = SYNC_SIZE + HEADER_SIZE + out_frame.m_header.payload_len + CRC_SIZE;
    if (len < expected_total) {
        return Status::ERROR_INVALID_PARAM;
    }

    // 4. Faydalı Yük Kopyalama
    if (out_frame.m_header.payload_len > 0) {
        std::memcpy(out_frame.m_payload, &in_buffer[SYNC_SIZE + HEADER_SIZE], out_frame.m_header.payload_len);
    }

    // 5. Çerçeve CRC-16 Kontrolü
    u16 recv_crc = (static_cast<u16>(in_buffer[expected_total - 2]) << 8) | in_buffer[expected_total - 1];
    u16 calc_crc = crc::crc16_ccitt(&in_buffer[SYNC_SIZE], HEADER_SIZE + out_frame.m_header.payload_len);

    out_frame.m_frame_crc = recv_crc;
    if (recv_crc != calc_crc) {
        return Status::ERROR_CRC_MISMATCH;
    }

    return Status::SUCCESS;
}

bool tactical_frame::is_valid() const {
    u8 expected_hcrc = crc::crc8(reinterpret_cast<const u8*>(&m_header), HEADER_SIZE - 1);
    return (expected_hcrc == m_header.header_crc);
}

} // namespace kok::radio
