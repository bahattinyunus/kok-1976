#!/usr/bin/env python3
"""
KÖK-1976 Stres Testi: OTAR (Hava Üzerinden Anahtarlama) ve Acil Kripto İmha (Zeroize)
Mehmet Hâcim Kamoy ve Millî Kriptografik Güvenlik Doktrini Doğrulaması
"""

import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from core.otar import OTARManager


def test_otar_and_zeroize():
    print("============================================================================")
    print("  KÖK-1976 OTAR HAVA ÜZERİNDEN ANAHTARLAMA VE ZEROIZE İMHA TESTİ")
    print("  (Kamoy Doktrini 1. ve 7. İlke: Kriptografik Egemenlik ve Anti-Tamper)")
    print("============================================================================")

    master_kek = 0x1976A5A51976A5A5
    hq_manager = OTARManager(master_kek)
    radio_manager = OTARManager(master_kek)

    print("[*] Karargâh ve Saha Telsizine Master KEK Yüklendi: 0x1976A5A51976A5A5")

    # 1. Havadan Yeni Trafik Anahtarı (TEK) Dağıtımı
    new_tek_day1 = 0x4600BEEFDEADBEEF
    packet1 = hq_manager.create_rekey_packet(new_tek_day1, key_id=101, sequence_num=1, iv=0x00010001)
    print(f"[*] Karargâh 24-Baytlık OTAR Paketi Üretti: {packet1.hex().upper()[:32]}...")

    success = radio_manager.process_rekey_packet(packet1)
    assert success, "OTAR paketi çözülemedi veya doğrulanamadı!"
    assert radio_manager.active_tek == new_tek_day1, "Aktif TEK uyuşmuyor!"
    assert radio_manager.active_key_id == 101, "Key ID uyuşmuyor!"
    print(f"[+] Saha Telsizi OTAR Paketini Doğruladı! Aktif TEK: 0x{radio_manager.active_tek:016X}")

    # 2. Düşman Tekrar Saldırısı (Replay Attack) Savunması
    print("\n[2] Düşman Eski OTAR Paketini Tekrar Yayınlıyor (Replay Saldırısı)...")
    replay_success = radio_manager.process_rekey_packet(packet1)
    assert not replay_success, "Replay saldırısı engellenemedi!"
    print("[+] [ENGEL] Eski sıra numaralı paket anında reddedildi (Replay Kalkanı Aktif).")

    # 3. Sahte Kripto Paketi (Spoofing) Savunması
    print("\n[3] Düşman Sahte MAC İçeren Kripto Paketi Enjekte Ediyor...")
    corrupt_packet = bytearray(packet1)
    corrupt_packet[1] = 2  # Sequence arttırıldı ama MAC uydurma
    corrupt_success = radio_manager.process_rekey_packet(bytes(corrupt_packet))
    assert not corrupt_success, "Sahte MAC paketi engellenemedi!"
    print("[+] [ENGEL] Kriptografik MAC uyuşmazlığı tespit edildi; paket derhal yok edildi.")

    # 4. Acil Kripto İmha (Zeroize / Anti-Tamper)
    print("\n[4] Taktik Acil İmha Testi (Zeroize)...")
    print("[!] Telsiz gövde kapağı zorlandı (CHASSIS_INTRUSION) veya acil butona basıldı!")
    radio_manager.emergency_zeroize(reason="CHASSIS_INTRUSION", timestamp=42000)

    assert radio_manager.tamper.is_zeroized, "Cihaz sıfırlanmadı!"
    assert radio_manager.kek == 0, "KEK bellekten silinmedi!"
    assert radio_manager.active_tek == 0, "TEK bellekten silinmedi!"
    print("[+] DoD 5220.22-M 4 Geçişli Bellek İmhası Tamamlandı.")
    print(f"[+] Telsiz Güvenlik Durumu: ZEROIZED (Sebep: {radio_manager.tamper.trigger_reason})")

    # 5. İmha Edilmiş Telsizde İşlem Yapılamaz
    new_tek_day2 = 0x9999888877776666
    packet2 = hq_manager.create_rekey_packet(new_tek_day2, key_id=102, sequence_num=3, iv=0x00010002)
    post_zeroize_success = radio_manager.process_rekey_packet(packet2)
    assert not post_zeroize_success, "İmha edilmiş cihaz anahtar kabul etti!"
    print("[+] [ONAY] İmha edilmiş telsiz hiçbir kriptografik işlem yapamaz.")

    print("----------------------------------------------------------------------------")
    print("[SONUÇ] OTAR hava üzerinden anahtarlama ve Zeroize imha testleri TAM BAŞARILI.")
    print("============================================================================\n")


if __name__ == "__main__":
    test_otar_and_zeroize()
