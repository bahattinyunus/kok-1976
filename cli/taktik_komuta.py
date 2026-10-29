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

    print("\n" + "=" * 76)
    print("  [DEMO TAMAMLANDI] KÖK-1976 Taktik Muhabere Sistemi Harbe Hazır!")
    print("=" * 76 + "\n")


if __name__ == '__main__':
    run_demo()
