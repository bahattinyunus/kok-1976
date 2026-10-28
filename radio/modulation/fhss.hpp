#ifndef KOK_RADIO_FHSS_HPP
#define KOK_RADIO_FHSS_HPP

#include "kok/types.hpp"
#include "kok/crypto.hpp"

namespace kok::radio {

// ============================================================================
// FREKANS ATLAMA YAYILI SPEKTRUM MOTORU (FHSS - FREQUENCY HOPPING SPREAD SPECTRUM)
// Karıştırmaya ve Dinlemeye Karşı Millî Atlama Deseni ve Kanal Senkronizasyonu
// ============================================================================

class fhss_engine {
public:
    static constexpr usize NUM_CHANNELS = 64;

    fhss_engine();
    fhss_engine(u64 hop_key, u32 dwell_symbols);

    void configure(u64 hop_key, u32 dwell_symbols);

    // Bir sonraki atlama kanalını (0 .. NUM_CHANNELS - 1) getir
    u16 next_channel();

    // Belirli bir zaman adımı (hop_index) için deterministik frekans kanalı
    u16 get_channel_at(u32 hop_index) const;

    // Frekans kanalını merkez frekansa (Hz cinsinden) dönüştür
    // base_freq: ör. 30.000.000 Hz, channel_spacing: ör. 25.000 Hz
    u32 get_frequency_hz(u16 channel, u32 base_freq = 30000000, u32 channel_spacing = 25000) const;

    // Kanal kalitesi ve düşman karıştırması izleme (Adaptive Frequency Hopping)
    void report_channel_jammed(u16 channel);
    void report_channel_clear(u16 channel);
    bool is_channel_jammed(u16 channel) const;

    u32 dwell_symbols() const { return m_dwell_symbols; }
    u32 current_hop_index() const { return m_current_hop_index; }

private:
    u64 m_hop_key;
    u32 m_dwell_symbols;
    u32 m_current_hop_index;
    u64 m_jammed_mask; // 64 kanal için bit maskesi
};

} // namespace kok::radio

#endif // KOK_RADIO_FHSS_HPP
