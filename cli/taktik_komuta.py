#!/usr/bin/env python3
"""
KÖK 1976 - Taktik Muhabere ve Görev Kontrol Konsolu (Interactive Tactical CLI)
Mehmet Hâcim Kamoy Taktik Komuta ve Muhabere Kontrol İstasyonu
"""

import sys
import os
import time

if hasattr(sys.stdout, 'reconfigure'):
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from tests.kok_core import KamoyCipher, golay24_encode, golay24_decode
from radio.telemetry.telemetry import TacticalFrame, StreamReceiver
from radio.modulation.dsss import DSSSEngine
from radio.sdr.df import WatsonWattDirectionFinder
from radio.network.tactical_link import TacticalDataLinkNetwork


BANNER = r"""
+--------------------------------------------------------------------------+
|  KÖK-1976 TAKTİK MUHABERE VE ELEKTRONİK HARP KONSOLU (MACUNKÖY HQ)        |
|  "Vatan savunmasında 'teknik olarak imkânsız' diye bir şey yoktur!"      |
+--------------------------------------------------------------------------+
"""

NODES = {
    0x1976: "Macunköy Ana Karargâh (Ankara)",
    0x1964: "Antalya Sahil Röle İstasyonu (Toroslar)",
    0xE999: "Kıbrıs Erenköy Mücahit Mevzisi (Beşparmak)",
    0x4600: "İleri Taktik Muhabere Kolu (ASELSAN 4600)",
}


def print_status():
    print("\n--- [AKTİF TAKTİK AĞ VE DÜĞÜM DURUMU] ---")
    for nid, name in NODES.items():
        print(f"  [0x{nid:04X}] -> {name}")
    print("  Kripto Algoritması: Kamoy-76 Çift-LFSR Akış Şifreleyici")
    print("  Kanal Modülasyonu : 64-Kanallı FHSS & 11-Çipli Barker DSSS")
    print("  Hata Düzeltme     : Genişletilmiş Golay(24,12) + Serpiştirici")
    print("-" * 50)


def send_tactical_message(source_id=0x1976, target_id=0xE999, message="GIZLI EMIR: HAREKAT BASLADI."):
    print(f"\n[*] [GÖNDERİM BAŞLATILDI]: {NODES.get(source_id)} -> {NODES.get(target_id)}")
    print(f"[*] Ham Metin: \"{message}\"")

    # 1. Kriptolama
    cipher = KamoyCipher(0x1976A5A51976A5A5, 0x00000042)
    raw_bytes = message.encode("ascii", errors="replace")
    encrypted_bytes = cipher.process(raw_bytes)
    print(f"[*] Kamoy-76 Şifreli Veri : {encrypted_bytes.hex().upper()}")

    # 2. Taktik Çerçeveleme (0x1976KOK)
    frame = TacticalFrame(msg_type=3, sender_id=source_id, target_id=target_id,
                          seq_num=42, hop_channel=7, payload=encrypted_bytes)
    wire_data = frame.serialize()
    print(f"[*] 0x1976KOK Çerçeve     : {wire_data[:5].hex().upper()} ... Toplam {len(wire_data)} bayt")

    # 3. Alıcı Tarafında Ayrıştırma ve Doğrulama
    rx_frame, err = TacticalFrame.deserialize(wire_data)
    if rx_frame and err is None:
        rx_cipher = KamoyCipher(0x1976A5A51976A5A5, 0x00000042)
        decrypted = rx_cipher.process(rx_frame.payload).decode("ascii", errors="replace")
        print(f"[+] [ALICI DOĞRULANDI] Hedef İstasyon Mesajı Çözdü: \"{decrypted}\"")
        return True
    else:
        print(f"[-] [HATA] Paket Bozuldu: {err}")
        return False


def run_demo():
    print(BANNER)
    print_status()

    print("\n>>> SENARYO 1: Macunköy'den Kıbrıs Erenköy Siperlerine Gizli Taktik Emir...")
    send_tactical_message(0x1976, 0xE999, "ANKARA-1964: GIZLI TALIMAT ALINDI. MEVZIYI KORUYUN.")

    print("\n>>> SENARYO 2: İleri Taktik Koldan Telemetri Raporu...")
    send_tactical_message(0x4600, 0x1976, "ASELSAN-4600: BATARYA=24.2V, FREKANS=VHF-FM, GORULEN TEHDIT YOK.")

    print("\n>>> SENARYO 3: DSSS Barker-11 Spektral Güvenlik Analizi...")
    bits = [1, 0, 1, 1, 0]
    chips = DSSSEngine.spread_bits(bits)
    print(f"[*] Giriş Bitleri: {bits}")
    print(f"[*] 11-Çipli Barker Spektrumu ({len(chips)} çip): {list(chips[:22])} ...")
    print(f"[*] Düşük Yakalanma Olasılığı (LPI/LPD) Kazancı: {DSSSEngine.calculate_processing_gain_db():.2f} dB")

    print("\n>>> SENARYO 4: Elektronik Harp Kestirimi (RDF) & Düşman Jammer Tespiti...")
    st1 = WatsonWattDirectionFinder(1, 0.0, 0.0)
    st2 = WatsonWattDirectionFinder(2, 50.0, 0.0)
    st1.simulate_emitter_signal(20.0, 60.0, snr_db=18.0, num_samples=64)
    st2.simulate_emitter_signal(20.0, 60.0, snr_db=18.0, num_samples=64)
    deg1, conf1, _ = st1.calculate_bearing()
    deg2, conf2, _ = st2.calculate_bearing()
    est = WatsonWattDirectionFinder.triangulate(st1, deg1, st2, deg2)
    print(f"[*] İstasyon 1 (Antalya) Kerterizi: {deg1:.1f}° (Güven: %{conf1*100:.0f})")
    print(f"[*] İstasyon 2 (Alanya) Kerterizi : {deg2:.1f}° (Güven: %{conf2*100:.0f})")
    if est:
        print(f"[+] Düşman Jammer Koordinatı Kestirildi: X = {est[0]:.2f} km, Y = {est[1]:.2f} km (Hedef Kilitlendi)")

    print("\n>>> SENARYO 5: Taktik TDMA Veri Bağı & Blue Force Tracking (BFT)...")
    net = TacticalDataLinkNetwork()
    net.add_node(0x01, "KARTAL-1 (Karargah)", slot=1, x=0.0, y=0.0).is_synced = True
    net.add_node(0x02, "PARS-2 (Tank)", slot=5, x=15.0, y=10.0).is_synced = True
    net.add_node(0x03, "BOZKURT-3 (Piyade)", slot=10, x=22.0, y=18.0).is_synced = True
    sim_res = net.run_epoch_simulation(duration_epochs=2)
    print(f"[*] 2 TDMA Çağında (1000 ms) Toplanan BFT Raporu: {sim_res['successful_bft_reports']}")
    print(f"[*] Slot Çatışması (Collisions): {sim_res['collisions']} (Sıfır Çatışma Determinizmi)")
    for nid, bft in sim_res['bft_map'].items():
        print(f"    [BFT MEVZİ] {bft['callsign']:22s} -> X={bft['pos_x']:4.1f}km, Y={bft['pos_y']:4.1f}km | Yakıt=%{bft['fuel']} Mühimmat=%{bft['ammo']}")

    print("\n" + "=" * 76)
    print("  [DEMO TAMAMLANDI] KÖK-1976 Taktik Muhabere Sistemi Harbe Hazır!")
    print("=" * 76 + "\n")


if __name__ == '__main__':
    run_demo()
