#include "mesh_router.hpp"
#include <cstring>

namespace kok::radio {

mesh_router::mesh_router(u16 my_node_id) : m_my_node_id(my_node_id) {
    reset();
}

void mesh_router::reset() {
    m_route_count = 0;
    m_seen_idx = 0;
    std::memset(m_routes, 0, sizeof(m_routes));
    std::memset(m_seen_packets, 0, sizeof(m_seen_packets));
}

bool mesh_router::is_duplicate(u16 sender_id, u16 seq_num) {
    u32 tag = (static_cast<u32>(sender_id) << 16) | seq_num;
    for (usize i = 0; i < DEDUP_CACHE_SIZE; ++i) {
        if (m_seen_packets[i] == tag) {
            return true;
        }
    }
    return false;
}

void mesh_router::mark_seen(u16 sender_id, u16 seq_num) {
    u32 tag = (static_cast<u32>(sender_id) << 16) | seq_num;
    m_seen_packets[m_seen_idx] = tag;
    m_seen_idx = (m_seen_idx + 1) % DEDUP_CACHE_SIZE;
}

void mesh_router::update_route(u16 dest_id, u16 next_hop, u8 hops, u8 link_quality, u32 current_tick) {
    if (dest_id == m_my_node_id) return; // Kendimize rota yazılmaz

    // Mevcut rota var mı?
    for (usize i = 0; i < m_route_count; ++i) {
        if (m_routes[i].destination_id == dest_id) {
            // Daha kısa veya daha kaliteli rota ise güncelle
            if (hops <= m_routes[i].hop_count || link_quality > m_routes[i].link_quality) {
                m_routes[i].next_hop_id = next_hop;
                m_routes[i].hop_count = hops;
                m_routes[i].link_quality = link_quality;
                m_routes[i].last_seen_tick = current_tick;
            }
            return;
        }
    }

    // Yeni rota ekle
    if (m_route_count < MAX_ROUTES) {
        m_routes[m_route_count].destination_id = dest_id;
        m_routes[m_route_count].next_hop_id = next_hop;
        m_routes[m_route_count].hop_count = hops;
        m_routes[m_route_count].link_quality = link_quality;
        m_routes[m_route_count].last_seen_tick = current_tick;
        ++m_route_count;
    }
}

bool mesh_router::get_next_hop(u16 dest_id, u16& out_next_hop, u8& out_hops) const {
    if (dest_id == 0xFFFF) {
        // Genel yayın (Broadcast)
        out_next_hop = 0xFFFF;
        out_hops = 1;
        return true;
    }

    for (usize i = 0; i < m_route_count; ++i) {
        if (m_routes[i].destination_id == dest_id) {
            out_next_hop = m_routes[i].next_hop_id;
            out_hops = m_routes[i].hop_count;
            return true;
        }
    }
    return false;
}

u8 mesh_router::process_incoming_frame(const tactical_frame& in_frame, tactical_frame& out_forward_frame) {
    const frame_header& hdr = in_frame.header();

    // 1. Mükerrer paket (Döngü) kontrolü
    if (is_duplicate(hdr.sender_id, hdr.seq_num)) {
        return 2; // Drop (Mükerrer)
    }
    mark_seen(hdr.sender_id, hdr.seq_num);

    // 2. Paket bize mi ait?
    if (hdr.target_id == m_my_node_id) {
        return 0; // Bu düğüme ait (Local consume)
    }

    // 3. Genel yayın (Broadcast: 0xFFFF)
    if (hdr.target_id == 0xFFFF) {
        // Hem tüket hem ileri yönlendir
        out_forward_frame = in_frame;
        return 0;
    }

    // 4. Başkasına giden paket: Röle / Yönlendirme (Forward)
    // Sıçrama (Hop) sınırını kontrol et
    u8 current_hops = in_frame.header().hop_channel & 0x0F; // hop_channel alt 4 biti sekme sayacı
    if (current_hops >= DEFAULT_MAX_HOPS) {
        return 2; // TTL tükendi, at
    }

    u16 next_hop = 0;
    u8 hops_to_dest = 0;
    if (get_next_hop(hdr.target_id, next_hop, hops_to_dest)) {
        out_forward_frame = in_frame;
        // Sekme sayısını bir artır
        out_forward_frame.set_header(hdr.msg_type, hdr.sender_id, hdr.target_id, hdr.seq_num,
                                     static_cast<u8>((current_hops + 1) | (hdr.hop_channel & 0xF0)));
        out_forward_frame.set_payload(in_frame.payload(), in_frame.payload_length());
        return 1; // Yönlendir (Relay forward)
    }

    // Rota bilinmiyor: Taşkın (Flooding) ile yönlendir
    out_forward_frame = in_frame;
    out_forward_frame.set_header(hdr.msg_type, hdr.sender_id, hdr.target_id, hdr.seq_num,
                                 static_cast<u8>((current_hops + 1) | (hdr.hop_channel & 0xF0)));
    out_forward_frame.set_payload(in_frame.payload(), in_frame.payload_length());
    return 1;
}

} // namespace kok::radio
