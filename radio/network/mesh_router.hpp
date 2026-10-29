#ifndef KOK_RADIO_MESH_ROUTER_HPP
#define KOK_RADIO_MESH_ROUTER_HPP

#include "kok/types.hpp"
#include "radio/telemetry/frame.hpp"

namespace kok::radio {

// ============================================================================
// TAKTİK AD-HOC MESH VE RÖLE YÖNLENDİRİCİSİ (MANET ROUTING ENGINE)
// Dağlık Arazide (Toroslar / Beşparmak) ve Karıştırma Altında Kesintisiz İntikal
// Sıfır Altyapı, Telsizden Telsize Çok Sekmeli (Multi-Hop) Taktik Köprü
// ============================================================================

struct route_entry {
    u16 destination_id; // Hedef telsiz kimliği
    u16 next_hop_id;    // Bir sonraki sıçrama (next hop) röle telsizi
    u8  hop_count;      // Sekme mesafesi (1: doğrudan komşu, 2: 1 röle...)
    u8  link_quality;   // Sinyal kalitesi / LQI (0..100)
    u32 last_seen_tick; // Son güncellenme zamanı
};

class mesh_router {
public:
    static constexpr usize MAX_ROUTES = 32;
    static constexpr usize DEDUP_CACHE_SIZE = 64;
    static constexpr u8  DEFAULT_MAX_HOPS = 5;

    explicit mesh_router(u16 my_node_id);

    void reset();

    // Düğüme gelen bir taktik çerçeveyi işle.
    // Dönen değer:
    // 0: Paket bu düğüme ait (Hedefe ulaştı, yerel olarak tüketilecek)
    // 1: Paket başka bir düğüme yönlendirilecek (Röle görevi yapıldı, out_forward_frame dolu)
    // 2: Paket mükerrer (duplicate) veya TTL süresi doldu, atıldı (Drop)
    u8 process_incoming_frame(const tactical_frame& in_frame, tactical_frame& out_forward_frame);

    // Yeni bir taktik rota ekle veya güncelle
    void update_route(u16 dest_id, u16 next_hop, u8 hops, u8 link_quality, u32 current_tick);

    // Belirli bir hedefe gitmek için bir sonraki röleyi bul
    bool get_next_hop(u16 dest_id, u16& out_next_hop, u8& out_hops) const;

    u16 node_id() const { return m_my_node_id; }
    usize route_count() const { return m_route_count; }

private:
    u16 m_my_node_id;

    route_entry m_routes[MAX_ROUTES];
    usize m_route_count;

    // Döngü engelleme: Son görülen paketlerin (sender_id << 16 | seq_num) özeti
    u32 m_seen_packets[DEDUP_CACHE_SIZE];
    usize m_seen_idx;

    bool is_duplicate(u16 sender_id, u16 seq_num);
    void mark_seen(u16 sender_id, u16 seq_num);
};

} // namespace kok::radio

#endif // KOK_RADIO_MESH_ROUTER_HPP
