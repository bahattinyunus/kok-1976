#include "tdma.hpp"

namespace kok {
namespace radio {

tdma_scheduler::tdma_scheduler(u8 local_node_id)
    : local_node_id_(local_node_id),
      current_slot_(0),
      slot_timer_us_(0),
      epoch_counter_(0),
      is_synchronized_(false) {

    for (usize i = 0; i < TOTAL_SLOTS; ++i) {
        if (i == 0) {
            slot_map_[i] = slot_type::BROADCAST_SYNC;
            slot_owner_[i] = 0x00; // Ağ Komutanı / Senkron Slotu
        } else {
            slot_map_[i] = slot_type::RECEIVE;
            slot_owner_[i] = 0xFF; // Henüz tahsis edilmemiş
        }
    }
}

void tdma_scheduler::assign_transmit_slot(u8 slot_index) {
    if (slot_index < TOTAL_SLOTS && slot_index != 0) {
        slot_map_[slot_index] = slot_type::TRANSMIT_ASSIGNED;
        slot_owner_[slot_index] = local_node_id_;
    }
}

void tdma_scheduler::advance_tick(u32 elapsed_us) {
    slot_timer_us_ += elapsed_us;

    while (slot_timer_us_ >= SLOT_DURATION_US) {
        slot_timer_us_ -= SLOT_DURATION_US;
        current_slot_ = static_cast<u8>((current_slot_ + 1) % TOTAL_SLOTS);
        if (current_slot_ == 0) {
            epoch_counter_++;
        }
    }
}

tdma_scheduler::slot_type tdma_scheduler::current_slot_type() const {
    if (current_slot_ >= TOTAL_SLOTS) {
        return slot_type::IDLE;
    }
    return slot_map_[current_slot_];
}

void tdma_scheduler::synchronize_network(u32 master_epoch, u8 master_slot, u32 slot_offset_us) {
    epoch_counter_ = master_epoch;
    current_slot_ = static_cast<u8>(master_slot % TOTAL_SLOTS);
    slot_timer_us_ = slot_offset_us % SLOT_DURATION_US;
    is_synchronized_ = true;
}

bool tdma_scheduler::create_sync_burst(tactical_frame& frame, u16 sequence_num) {
    if (current_slot_ != 0) {
        return false;
    }

    // Mesaj Türü: 0x0F (TDMA SYNC), Alıcı: 0xFFFF (Genel Yayın)
    frame.set_header(0x0F, local_node_id_, 0xFFFF, sequence_num, 0);

    u8 payload[9];
    payload[0] = static_cast<u8>((epoch_counter_ >> 24) & 0xFF);
    payload[1] = static_cast<u8>((epoch_counter_ >> 16) & 0xFF);
    payload[2] = static_cast<u8>((epoch_counter_ >> 8) & 0xFF);
    payload[3] = static_cast<u8>(epoch_counter_ & 0xFF);
    payload[4] = current_slot_;
    payload[5] = static_cast<u8>((slot_timer_us_ >> 24) & 0xFF);
    payload[6] = static_cast<u8>((slot_timer_us_ >> 16) & 0xFF);
    payload[7] = static_cast<u8>((slot_timer_us_ >> 8) & 0xFF);
    payload[8] = static_cast<u8>(slot_timer_us_ & 0xFF);

    frame.set_payload(payload, sizeof(payload));
    return true;
}

bool tdma_scheduler::validate_slot_ingress(u8 sender_node_id) const {
    if (current_slot_ >= TOTAL_SLOTS) {
        return false;
    }

    // Senkron slotu herkese açıktır
    if (current_slot_ == 0) {
        return true;
    }

    // Sahibi atanmamış slotta gelen veri çatışma riski taşır
    u8 owner = slot_owner_[current_slot_];
    if (owner == 0xFF || owner == sender_node_id) {
        return true;
    }

    // Başkasının slotuna konuşmaya çalışan düğüm (Collision veya Spoofing)
    return false;
}

} // namespace radio
} // namespace kok
