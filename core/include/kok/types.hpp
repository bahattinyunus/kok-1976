#ifndef KOK_TYPES_HPP
#define KOK_TYPES_HPP

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace kok {

// ============================================================================
// TEMEL SABİT GENİŞLİKLİ TİPLER VE TAKTİK DURUM KODLARI
// (Mehmet Hâcim Kamoy Mühendislik Standartları - Sıfır Dış Bağımlılık)
// ============================================================================

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using usize = std::size_t;

enum class Status : u8 {
    SUCCESS             = 0x00,
    ERROR_GENERAL       = 0x01,
    ERROR_BUFFER_FULL   = 0x02,
    ERROR_BUFFER_EMPTY  = 0x03,
    ERROR_CRC_MISMATCH  = 0x04,
    ERROR_SYNC_LOST     = 0x05,
    ERROR_UNCORRECTABLE = 0x06,
    ERROR_INVALID_PARAM = 0x07,
    ERROR_TIMEOUT       = 0x08,
    ERROR_JAMMED        = 0x09
};

// ============================================================================
// SABİT NOKTALI MATEMATİK TİPİ (Q16.16) - DONANIM FPU GEREKTİRMEZ
// ============================================================================

struct fp32_t {
    i32 raw;

    static constexpr i32 FRACTION_BITS = 16;
    static constexpr i32 SCALE = 1 << FRACTION_BITS;

    constexpr fp32_t() : raw(0) {}
    constexpr explicit fp32_t(i32 r, bool is_raw) : raw(r) { (void)is_raw; }

    static constexpr fp32_t from_int(i32 val) {
        return fp32_t(val << FRACTION_BITS, true);
    }

    static constexpr fp32_t from_float(float val) {
        return fp32_t(static_cast<i32>(val * static_cast<float>(SCALE) + (val >= 0.0f ? 0.5f : -0.5f)), true);
    }

    constexpr float to_float() const {
        return static_cast<float>(raw) / static_cast<float>(SCALE);
    }

    constexpr i32 to_int() const {
        return raw >> FRACTION_BITS;
    }

    constexpr fp32_t operator+(fp32_t other) const {
        return fp32_t(raw + other.raw, true);
    }

    constexpr fp32_t operator-(fp32_t other) const {
        return fp32_t(raw - other.raw, true);
    }

    constexpr fp32_t operator*(fp32_t other) const {
        i64 prod = static_cast<i64>(raw) * static_cast<i64>(other.raw);
        return fp32_t(static_cast<i32>(prod >> FRACTION_BITS), true);
    }

    constexpr fp32_t operator/(fp32_t other) const {
        if (other.raw == 0) return fp32_t(0, true);
        i64 num = (static_cast<i64>(raw) << FRACTION_BITS);
        return fp32_t(static_cast<i32>(num / other.raw), true);
    }

    constexpr bool operator==(fp32_t other) const { return raw == other.raw; }
    constexpr bool operator!=(fp32_t other) const { return raw != other.raw; }
    constexpr bool operator<(fp32_t other) const { return raw < other.raw; }
    constexpr bool operator<=(fp32_t other) const { return raw <= other.raw; }
    constexpr bool operator>(fp32_t other) const { return raw > other.raw; }
    constexpr bool operator>=(fp32_t other) const { return raw >= other.raw; }
};

// ============================================================================
// DETERMINİSTİK ENDIANNESS YARDIMCILARI
// ============================================================================

constexpr inline u16 bswap16(u16 val) {
    return static_cast<u16>((val << 8) | (val >> 8));
}

constexpr inline u32 bswap32(u32 val) {
    return ((val & 0x000000FFu) << 24) |
           ((val & 0x0000FF00u) << 8)  |
           ((val & 0x00FF0000u) >> 8)  |
           ((val & 0xFF000000u) >> 24);
}

} // namespace kok

#endif // KOK_TYPES_HPP
