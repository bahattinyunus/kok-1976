#!/usr/bin/env python3
"""
KÖK-1976 Stres Testi: Taktik TDMA Zaman Çizelgeleyici ve Late-Net Entry (Geç Katılım)
Mehmet Hâcim Kamoy ve Millî Taktik Veri Bağı (Link-Kamoy) Determinizm Doğrulaması
"""

import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from radio.network.tactical_link import TacticalDataLinkNetwork, TacticalTDMANode


def test_tdma_and_sync():
    print("============================================================================")
    print("  KÖK-1976 TAKTİK TDMA VERİ BAĞI VE LATE-NET ENTRY (GEÇ KATILIM) TESTİ")
    print("  (Kamoy Doktrini 2. ve 6. İlke: Deterministik Slotlar ve Sıfır Çatışma)")
    print("============================================================================")

    network = TacticalDataLinkNetwork()

    # 1. Taktik Düğümlerin Ağa Katılımı ve Slot Tahsisi
    # Slot 0: Senkronizasyon (Master)
    # Slot 1..7: Operasyonel Düğümler
    nodes_info = [
        (0x01, "KARTAL-1 (Karargah)", 1, 0.0, 0.0),
        (0x02, "PARS-2 (Tank Taburu)", 5, 12.4, 8.2),
        (0x03, "BOZKURT-3 (Piyade Bölüğü)", 10, 18.0, 14.5),
        (0x04, "ATMACA-4 (Hava İrtibat)", 15, 35.0, 50.0),
        (0x05, "TAYFUN-5 (Topçu Bataryası)", 20, 5.0, 22.0),
        (0x06, "KORAL-6 (Elektronik Harp)", 25, 2.0, 10.0),
        (0x07, "SIHA-7 (Keşif Dronu)", 30, 28.0, 32.0),
    ]

    for nid, callsign, slot, x, y in nodes_info:
        n = network.add_node(nid, callsign, slot, x, y)
        n.is_synced = True

    print(f"[*] Ağa Eklenen Aktif Düğüm Sayısı: {len(network.nodes)}")
    print("[*] 4 TDMA Çağı (Epoch) Boyunca Deterministik İletim Başlatılıyor (~2000 ms)...")

    res = network.run_epoch_simulation(duration_epochs=4)

    print(f"[+] Toplam Başarılı BFT Telemetrisi : {res['successful_bft_reports']} rapor")
    print(f"[+] Meydana Gelen Slot Çatışması    : {res['collisions']} (Sıfır Çatışma Garantisi)")
    assert res["collisions"] == 0, "TDMA slotlarında çatışma meydana geldi!"
    assert res["successful_bft_reports"] >= 28, "BFT telemetri paketleri eksik!"

    # 2. Geç Katılım (Late-Net Entry - LNE) Testi:
    # Savaşa sonradan katılan veya telsizi resetlenen Düğüm 8 (Özel Kuvvetler)
    print("\n[3] Geç Katılım (Late-Net Entry) Senkronizasyon Testi...")
    node_late = network.add_node(0x08, "BORDO-8 (Özel Kuvvetler)", slot=35, x=40.0, y=18.0)
    node_late.is_synced = False  # Başlangıçta senkron değil

    # 100 ms asenkron ilerleme
    for _ in range(100):
        node_late.advance_time(1.0)
    assert node_late.tx_count == 0, "Senkron olmayan telsiz kaçak yayın yapmamalı!"

    # Master Senkronizasyon Darbesi (Sync Burst) Alındı
    master_node = network.nodes[0x01]
    node_late.sync_to_master(master_node.epoch, master_node.current_slot, master_node.slot_timer_ms)
    print(f"[*] BORDO-8 Telsizi Master Saat Kilitlendi: Epoch {node_late.epoch}, Slot {node_late.current_slot}")
    assert node_late.is_synced, "Geç katılım senkronizasyonu başarısız!"

    # 2 epoch daha koştur
    res2 = network.run_epoch_simulation(duration_epochs=2)
    print(f"[+] Geç Katılım Sonrası Yeni BFT Raporları: {res2['successful_bft_reports']}")
    assert node_late.tx_count > 0, "Geç katılan telsiz yayına başlayamadı!"
    assert res2["collisions"] == 0, "Geç katılım sonrasında ağda çatışma çıktı!"

    print("----------------------------------------------------------------------------")
    print("[SONUÇ] TDMA zamanlama determinizmi ve Geç Katılım (LNE) TAM BAŞARILI.")
    print("============================================================================\n")


if __name__ == "__main__":
    test_tdma_and_sync()
