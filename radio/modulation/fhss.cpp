#include "fhss.hpp"

namespace kok::radio {

fhss_engine::fhss_engine()
    : m_hop_key(0x1976A5A51976A5A5ULL), m_dwell_symbols(16),
      m_current_hop_index(0), m_jammed_mask(0) {}

fhss_engine::fhss_engine(u64 hop_key, u32 dwell_symbols)
    : m_hop_key(hop_key), m_dwell_symbols(dwell_symbols),
      m_current_hop_index(0), m_jammed_mask(0) {}

void fhss_engine::configure(u64 hop_key, u32 dwell_symbols) {
    m_hop_key = hop_key;
    m_dwell_symbols = (dwell_symbols > 0) ? dwell_symbols : 1;
    m_current_hop_index = 0;
    m_jammed_mask = 0;
}

u16 fhss_engine::get_channel_at(u32 hop_index) const {
    // Deterministik sözde-rastgele atlama deseni (Kamoy-76 permütasyonu)
    u64 state = m_hop_key ^ (static_cast<u64>(hop_index) * 0x9E3779B97F4A7C15ULL);
    state ^= (state >> 30);
    state *= 0xBF58476D1CE4E5B9ULL;
    state ^= (state >> 27);
    state *= 0x94D049BB133111EBULL;
    state ^= (state >> 31);

    u16 raw_ch = static_cast<u16>(state % NUM_CHANNELS);

    // Eğer kanal karıştırılmışsa, adaptif olarak alternatif bir kanala kay
    if (m_jammed_mask & (1ULL << raw_ch)) {
        for (u16 offset = 1; offset < NUM_CHANNELS; ++offset) {
            u16 alt_ch = static_cast<u16>((raw_ch + offset) % NUM_CHANNELS);
            if (!(m_jammed_mask & (1ULL << alt_ch))) {
                return alt_ch;
            }
        }
    }

    return raw_ch;
}

u16 fhss_engine::next_channel() {
    u16 ch = get_channel_at(m_current_hop_index++);
    return ch;
}

u32 fhss_engine::get_frequency_hz(u16 channel, u32 base_freq, u32 channel_spacing) const {
    return base_freq + (static_cast<u32>(channel % NUM_CHANNELS) * channel_spacing);
}

void fhss_engine::report_channel_jammed(u16 channel) {
    if (channel < NUM_CHANNELS) {
        m_jammed_mask |= (1ULL << channel);
    }
}

void fhss_engine::report_channel_clear(u16 channel) {
    if (channel < NUM_CHANNELS) {
        m_jammed_mask &= ~(1ULL << channel);
    }
}

bool fhss_engine::is_channel_jammed(u16 channel) const {
    if (channel < NUM_CHANNELS) {
        return (m_jammed_mask & (1ULL << channel)) != 0;
    }
    return false;
}

} // namespace kok::radio
