#include "kok/otar.hpp"
#include <cstring>

namespace kok {

otar_manager::otar_manager()
    : m_kek(0),
      m_active_tek(0),
      m_active_key_id(0),
      m_last_sequence(0),
      m_is_kek_loaded(false) {
}

otar_manager::otar_manager(u64 master_kek)
    : m_kek(master_kek),
      m_active_tek(0),
      m_active_key_id(0),
      m_last_sequence(0),
      m_is_kek_loaded(true) {
}

void otar_manager::load_kek(u64 master_kek) {
    m_kek = master_kek;
    m_is_kek_loaded = true;
    m_tamper.reset_state();
}

bool otar_manager::create_rekey_packet(u64 new_tek, u16 key_id, u16 sequence_num, u32 iv, otar_message& out_msg) {
    if (!m_is_kek_loaded || m_tamper.is_zeroized()) {
        return false;
    }

    out_msg.magic = otar_message::MAGIC_HEADER;
    out_msg.sequence = sequence_num;
    out_msg.key_id = key_id;
    out_msg.iv = iv;

    // 1. Yeni TEK'i bayt dizisine dönüştür
    u8 raw_tek[8];
    for (usize i = 0; i < 8; ++i) {
        raw_tek[i] = static_cast<u8>((new_tek >> (56 - i * 8)) & 0xFF);
    }

    // 2. KEK ile Kamoy-76 akış şifreleyicisi başlat ve TEK'i şifrele
    cipher c(m_kek, iv);
    c.process(raw_tek, 8);
    std::memcpy(out_msg.encrypted_tek, raw_tek, 8);

    // 3. Paketin kimlik doğrulama kodunu (MAC) hesapla
    // Doğrulanacak veri: sequence (2B) + key_id (2B) + iv (4B) + encrypted_tek (8B) = 16B
    u8 auth_data[16];
    auth_data[0] = static_cast<u8>((sequence_num >> 8) & 0xFF);
    auth_data[1] = static_cast<u8>(sequence_num & 0xFF);
    auth_data[2] = static_cast<u8>((key_id >> 8) & 0xFF);
    auth_data[3] = static_cast<u8>(key_id & 0xFF);
    auth_data[4] = static_cast<u8>((iv >> 24) & 0xFF);
    auth_data[5] = static_cast<u8>((iv >> 16) & 0xFF);
    auth_data[6] = static_cast<u8>((iv >> 8) & 0xFF);
    auth_data[7] = static_cast<u8>(iv & 0xFF);
    std::memcpy(&auth_data[8], out_msg.encrypted_tek, 8);

    out_msg.mac = cipher::calculate_mac(m_kek, auth_data, sizeof(auth_data));
    return true;
}

bool otar_manager::process_rekey_packet(const otar_message& in_msg) {
    if (!m_is_kek_loaded || m_tamper.is_zeroized()) {
        return false;
    }

    // 1. Magic Header denetimi
    if (in_msg.magic != otar_message::MAGIC_HEADER) {
        return false;
    }

    // 2. Replay denetimi: Sıra numarası eskisinden büyük olmalıdır
    if (in_msg.sequence <= m_last_sequence && m_last_sequence != 0) {
        return false; // Tekrar saldırısı (Replay Attack) engellendi
    }

    // 3. MAC Doğrulaması
    u8 auth_data[16];
    auth_data[0] = static_cast<u8>((in_msg.sequence >> 8) & 0xFF);
    auth_data[1] = static_cast<u8>(in_msg.sequence & 0xFF);
    auth_data[2] = static_cast<u8>((in_msg.key_id >> 8) & 0xFF);
    auth_data[3] = static_cast<u8>(in_msg.key_id & 0xFF);
    auth_data[4] = static_cast<u8>((in_msg.iv >> 24) & 0xFF);
    auth_data[5] = static_cast<u8>((in_msg.iv >> 16) & 0xFF);
    auth_data[6] = static_cast<u8>((in_msg.iv >> 8) & 0xFF);
    auth_data[7] = static_cast<u8>(in_msg.iv & 0xFF);
    std::memcpy(&auth_data[8], in_msg.encrypted_tek, 8);

    u32 expected_mac = cipher::calculate_mac(m_kek, auth_data, sizeof(auth_data));
    if (in_msg.mac != expected_mac) {
        // Sahte paket / Kripto ihlali
        return false;
    }

    // 4. Yeni TEK'in şifresini çöz
    u8 decrypted_tek[8];
    std::memcpy(decrypted_tek, in_msg.encrypted_tek, 8);
    cipher c(m_kek, in_msg.iv);
    c.process(decrypted_tek, 8);

    u64 new_tek = 0;
    for (usize i = 0; i < 8; ++i) {
        new_tek = (new_tek << 8) | decrypted_tek[i];
    }

    // Eski anahtarı güvenli sil
    anti_tamper::secure_wipe(&m_active_tek, sizeof(m_active_tek));

    // Yeni anahtarı aktif et
    m_active_tek = new_tek;
    m_active_key_id = in_msg.key_id;
    m_last_sequence = in_msg.sequence;

    // Geçici belleği temizle
    anti_tamper::secure_wipe(decrypted_tek, sizeof(decrypted_tek));
    return true;
}

void otar_manager::emergency_zeroize(TamperSource reason, u32 current_ticks) {
    anti_tamper::secure_wipe(&m_kek, sizeof(m_kek));
    anti_tamper::secure_wipe(&m_active_tek, sizeof(m_active_tek));
    m_active_key_id = 0;
    m_last_sequence = 0;
    m_is_kek_loaded = false;

    m_tamper.trigger_zeroize(reason, current_ticks);
}

} // namespace kok
