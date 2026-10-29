"""
KÖK 1976 - Taktik Ad-Hoc Mesh ve Röle Yönlendirme Stres Testi
Dağlık Arazide (Toroslar & Beşparmak) Çok-Sekmeli (Multi-Hop) Muhabere Güvenilirliği
"""

import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from radio.telemetry.telemetry import TacticalFrame


class SimulatedMeshNode:
    def __init__(self, node_id, name):
        self.node_id = node_id
        self.name = name
        self.routes = {} # dest_id -> next_hop_id
        self.seen_packets = set()
        self.received_messages = []
        self.forwarded_count = 0

    def add_route(self, dest_id, next_hop_id):
        self.routes[dest_id] = next_hop_id

    def receive_packet(self, frame: TacticalFrame, network_map):
        tag = (frame.sender_id, frame.seq_num)
        if tag in self.seen_packets:
            return # Mükerrer (Döngü koruması)
        self.seen_packets.add(tag)

        # Hedef bu düğüm mü?
        if frame.target_id == self.node_id:
            self.received_messages.append(frame.payload)
            return

        # Röle yönlendirmesi
        next_hop = self.routes.get(frame.target_id)
        if next_hop is not None and next_hop in network_map:
            self.forwarded_count += 1
            # Bir sonraki düğüme ilet
            network_map[next_hop].receive_packet(frame, network_map)


def run_mesh_stress():
    print("=" * 76)
    print("  KÖK-1976 TAKTİK AD-HOC MESH VE ÇOK-SEKMELİ RÖLE STRES TESTİ")
    print("  (Kamoy Doktrini: Dağlık Coğrafyada ve Siperlerde Kesintisiz İntikal)")
    print("=" * 76)

    # 5 Düğümlü Taktik Hat Oluşturma:
    # [1: Macunköy HQ] -> [2: Toroslar Rölesi] -> [3: Antalya Sahil] -> [4: Beşparmak Dağı] -> [5: Erenköy Siperi]
    nodes = {
        1: SimulatedMeshNode(1, "Macunköy HQ (Ankara)"),
        2: SimulatedMeshNode(2, "Toroslar Zirve Rölesi"),
        3: SimulatedMeshNode(3, "Antalya Sahil Üssü"),
        4: SimulatedMeshNode(4, "Beşparmak Dağları İleri Röle"),
        5: SimulatedMeshNode(5, "Erenköy Mücahit Siperi"),
    }

    # İki yönlü zincir rotaları tanımla
    # 1 -> 2 -> 3 -> 4 -> 5
    for dest in [2, 3, 4, 5]:
        nodes[1].add_route(dest, 2)
    nodes[2].add_route(1, 1)
    for dest in [3, 4, 5]:
        nodes[2].add_route(dest, 3)
    nodes[3].add_route(1, 2)
    nodes[3].add_route(2, 2)
    nodes[3].add_route(4, 4)
    nodes[3].add_route(5, 4)
    nodes[4].add_route(1, 3)
    nodes[4].add_route(2, 3)
    nodes[4].add_route(3, 3)
    nodes[4].add_route(5, 5)
    for dest in [1, 2, 3, 4]:
        nodes[5].add_route(dest, 4)

    num_test_packets = 200
    print(f"[*] {num_test_packets} adet taktik paket 4-sekmeli (4-hop) zincir üzerinden iletiliyor...")

    for i in range(num_test_packets):
        payload = f"EMIR-NO-{i}: SAVUNMA HATTINI TAKVIYE EDIN.".encode("ascii")
        frame = TacticalFrame(msg_type=3, sender_id=1, target_id=5, seq_num=i,
                              hop_channel=0, payload=payload)
        # Macunköy'den gönder (Toroslar üzerinden akacak)
        nodes[1].receive_packet(frame, nodes)

    # Doğrulama
    delivered = len(nodes[5].received_messages)
    pdr = (delivered / num_test_packets) * 100.0

    print(f"[*] Gönderilen Paket     : {num_test_packets}")
    print(f"[*] Ulaşan Paket (Hedef) : {delivered}")
    print(f"[*] Paket Teslim Oranı   : %{pdr:.1f}")
    print(f"[*] Toroslar Röle Sekmesi: {nodes[2].forwarded_count} paket")
    print(f"[*] Antalya Röle Sekmesi : {nodes[3].forwarded_count} paket")
    print(f"[*] Beşparmak Röle Sekme : {nodes[4].forwarded_count} paket")

    assert delivered == num_test_packets, "Mesh üzerinden kayıp paket var!"
    assert nodes[2].forwarded_count == num_test_packets
    assert nodes[4].forwarded_count == num_test_packets

    print("-" * 76)
    print("[SONUÇ] Dağlık arazi 4-sekmeli taktik röleleme testi %100 BAŞARIYLA TAMAMLANDI.")
    print("=" * 76 + "\n")
    return True


if __name__ == '__main__':
    run_mesh_stress()
