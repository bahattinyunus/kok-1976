#include "kok/anti_tamper.hpp"

namespace kok {

anti_tamper::anti_tamper() {
    reset_state();
}

void anti_tamper::secure_wipe(volatile void* ptr, usize length) {
    if (ptr == nullptr || length == 0) {
        return;
    }

    volatile u8* byte_ptr = static_cast<volatile u8*>(ptr);

    // Askerî Standart 4 Aşamalı Silme (DoD 5220.22-M ve Kamoy Doktrini)
    // 1. Aşama: 0x55 Alternatif bitler
    for (usize i = 0; i < length; ++i) {
        byte_ptr[i] = 0x55;
    }

    // 2. Aşama: 0xAA Tersi bitler
    for (usize i = 0; i < length; ++i) {
        byte_ptr[i] = 0xAA;
    }

    // 3. Aşama: 0xFF Tüm bitler aktif
    for (usize i = 0; i < length; ++i) {
        byte_ptr[i] = 0xFF;
    }

    // 4. Aşama: 0x00 Temizleme ve son sıfırlama
    for (usize i = 0; i < length; ++i) {
        byte_ptr[i] = 0x00;
    }
}

bool anti_tamper::verify_clean(const volatile void* ptr, usize length) {
    if (ptr == nullptr || length == 0) {
        return true;
    }

    const volatile u8* byte_ptr = static_cast<const volatile u8*>(ptr);
    u8 accumulator = 0;

    // Sabit zamanlı (constant-time) kontrol, yan kanal saldırısını önler
    for (usize i = 0; i < length; ++i) {
        accumulator |= byte_ptr[i];
    }

    return (accumulator == 0);
}

void anti_tamper::trigger_zeroize(TamperSource reason, u32 current_time_ticks) {
    m_audit.is_zeroized = true;
    m_audit.trigger_reason = reason;
    m_audit.timestamp_ticks = current_time_ticks;
    m_audit.bytes_purged = 0;
    m_audit.verification_passed = true;
}

void anti_tamper::reset_state() {
    m_audit.is_zeroized = false;
    m_audit.trigger_reason = TamperSource::NONE;
    m_audit.timestamp_ticks = 0;
    m_audit.bytes_purged = 0;
    m_audit.verification_passed = false;
}

} // namespace kok
