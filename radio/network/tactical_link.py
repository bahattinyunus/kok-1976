#!/usr/bin/env python3
"""
KÖK-1976 Millî Taktik Veri Bağı ve TDMA Çizelgeleyici (Link-Kamoy)
Dost Birlik Takibi (Blue Force Tracking - BFT) ve Deterministik Zaman Slotları
"""

import math
from typing import Dict, List, Optional, Tuple


class TacticalTDMANode:
    """Tek bir taktik telsiz veya komuta düğümünün TDMA durumu."""

    TOTAL_SLOTS = 64
    SLOT_DURATION_MS = 7.8125  # ~500 ms tam epoch süresi

    def __init__(self, node_id: int, callsign: str, assigned_slot: int):
        self.node_id = node_id
        self.callsign = callsign
        self.assigned_slot = assigned_slot
        self.current_slot = 0
        self.epoch = 0
        self.slot_timer_ms = 0.0
        self.is_synced = (node_id == 0x01)  # 0x01 Komutan / Master baştan senkron

        # BFT Telemetrisi
        self.pos_x_km = 0.0
        self.pos_y_km = 0.0
        self.fuel_pct = 100
        self.ammo_pct = 100
        self.tx_count = 0
        self.rx_count = 0
        self.collision_count = 0

    def advance_time(self, delta_ms: float) -> Optional[dict]:
        """Zamanı ilerletir. Eğer kendi slotundaysa BFT paketi üretir."""
        self.slot_timer_ms += delta_ms
        while self.slot_timer_ms >= self.SLOT_DURATION_MS:
            self.slot_timer_ms -= self.SLOT_DURATION_MS
            self.current_slot = (self.current_slot + 1) % self.TOTAL_SLOTS
            if self.current_slot == 0:
                self.epoch += 1

        if not self.is_synced:
            return None

        # Kendi slotumuz geldiyse yayın yap
        if self.current_slot == self.assigned_slot:
            self.tx_count += 1
            return {
                "epoch": self.epoch,
                "slot": self.current_slot,
                "sender_id": self.node_id,
                "callsign": self.callsign,
                "pos_x": self.pos_x_km,
                "pos_y": self.pos_y_km,
                "fuel": self.fuel_pct,
                "ammo": self.ammo_pct,
            }
        return None

    def sync_to_master(self, master_epoch: int, master_slot: int, offset_ms: float):
        """Geç Katılım (Late-Net Entry) senkronizasyonu."""
        self.epoch = master_epoch
        self.current_slot = master_slot
        self.slot_timer_ms = offset_ms
        self.is_synced = True


class TacticalDataLinkNetwork:
    """64-Slotluk TDMA Taktik Ağ Simülatörü."""

    def __init__(self):
        self.nodes: Dict[int, TacticalTDMANode] = {}
        self.bft_situational_map: Dict[int, dict] = {}

    def add_node(self, node_id: int, callsign: str, slot: int, x: float = 0.0, y: float = 0.0) -> TacticalTDMANode:
        node = TacticalTDMANode(node_id, callsign, slot)
        node.pos_x_km = x
        node.pos_y_km = y
        self.nodes[node_id] = node
        return node

    def run_epoch_simulation(self, duration_epochs: int = 2) -> Dict[str, any]:
        """TDMA ağında belirtilen epoch süresince veri akışını simüle eder."""
        total_time_ms = duration_epochs * (TacticalTDMANode.TOTAL_SLOTS * TacticalTDMANode.SLOT_DURATION_MS)
        step_ms = 1.0  # 1 ms simülasyon adımı
        current_time = 0.0

        transmissions: List[dict] = []
        collisions = 0
        successful_bft_reports = 0

        while current_time < total_time_ms:
            active_tx = []
            for node in self.nodes.values():
                packet = node.advance_time(step_ms)
                if packet:
                    active_tx.append(packet)

            if len(active_tx) == 1:
                pkt = active_tx[0]
                successful_bft_reports += 1
                transmissions.append(pkt)
                self.bft_situational_map[pkt["sender_id"]] = pkt
                for node in self.nodes.values():
                    if node.node_id != pkt["sender_id"]:
                        node.rx_count += 1
            elif len(active_tx) > 1:
                # Slot çatışması (Collision)
                collisions += 1
                for pkt in active_tx:
                    self.nodes[pkt["sender_id"]].collision_count += 1

            current_time += step_ms

        return {
            "total_sim_time_ms": total_time_ms,
            "successful_bft_reports": successful_bft_reports,
            "collisions": collisions,
            "bft_map": self.bft_situational_map,
        }
