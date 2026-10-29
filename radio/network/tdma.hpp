#pragma once

#include "kok/types.hpp"
#include "frame.hpp"

namespace kok {
namespace radio {

/**
 * @brief Millî Taktik Veri Bağı TDMA (Time Division Multiple Access) Zaman Çizelgeleyici
 * 
 * Mehmet Hâcim Kamoy ve ASELSAN Taktik Komuta-Kontrol (C4I) Doktrini.
 * Çatışmasız (zero-collision) telemetri, Durumsal Farkındalık (SA) ve
 * Dost Birlik Takibi (Blue Force Tracking - BFT) için 64-slotluk deterministik çerçeve.
 */
class tdma_scheduler {
public:
    static constexpr usize TOTAL_SLOTS = 64;        // Bir TDMA Çağındaki (Epoch) Slot Sayısı
    static constexpr u32 SLOT_DURATION_US = 7812;   // Slot Süresi (~7.812 ms)
    static constexpr usize MAX_NODES = 16;

    enum class slot_type : u8 {
        IDLE = 0,               // Boş Slot
        BROADCAST_SYNC = 1,     // Ağ Senkronizasyonu & Geç Katılım (Late-Net Entry)
        TRANSMIT_ASSIGNED = 2,  // Bu Düğüme Tahsisli Gönderim Slotu
        RECEIVE = 3             // Diğer Dost Birlikleri Dinleme Slotu
    };

    struct slot_assignment {
        u8 node_id;
        slot_type type;
    };

    explicit tdma_scheduler(u8 local_node_id);

    /**
     * @brief Bu düğüm için belirlenen yayın slotunu tahsis eder.
     */
    void assign_transmit_slot(u8 slot_index);

    /**
     * @brief Saat darbesi veya zamanlayıcı kesmesiyle (Timer tick) slotu ilerletir.
     */
    void advance_tick(u32 elapsed_us);

    /**
     * @brief Mevcut slotun türünü döndürür (TX, RX, SYNC, IDLE)
     */
    slot_type current_slot_type() const;

    /**
     * @brief Mevcut aktif slot indeksi (0 .. TOTAL_SLOTS - 1)
     */
    u8 current_slot() const { return current_slot_; }

    /**
     * @brief Mevcut çağ (Epoch / Frame) numarası
     */
    u32 current_epoch() const { return epoch_counter_; }

    /**
     * @brief Geç Katılım (Late-Net Entry) senkronizasyon paketi alındığında yerel saati kilitler.
     */
    void synchronize_network(u32 master_epoch, u8 master_slot, u32 slot_offset_us);

    /**
     * @brief Bu slotta gönderilecek senkronizasyon çerçevesini doldurur.
     */
    bool create_sync_burst(tactical_frame& frame, u16 sequence_num);

    /**
     * @brief Alınan bir çerçevenin zaman slotu ile uyumlu olup olmadığını doğrular (Anti-Collision).
     */
    bool validate_slot_ingress(u8 sender_node_id) const;

private:
    u8 local_node_id_;
    u8 current_slot_;
    u32 slot_timer_us_;
    u32 epoch_counter_;
    slot_type slot_map_[TOTAL_SLOTS];
    u8 slot_owner_[TOTAL_SLOTS];
    bool is_synchronized_;
};

} // namespace radio
} // namespace kok
