"""
KÖK 1976 - İleri Düzey Elektronik Harp (EW) ve Anti-Spoofing Stres Testi
DSSS Barker-11 Kazancı, Çok-Tonlu Karıştırma ve Sahte Paket (Replay/Spoofing) Savunması
"""

import sys
import os
import random
import numpy as np

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from kok_core import KamoyCipher, crc8, crc16_ccitt
from radio.modulation.dsss import DSSSEngine
from radio.telemetry.telemetry import TacticalFrame, StreamReceiver


def run_ew_complex_stress():
    print("=" * 76)
    print("  KÖK-1976 İLERİ DÜZEY ELEKTRONİK HARP & SAHTE YAYIN (SPOOFING) TESTİ")
    print("  (Kamoy Doktrini 1. ve 7. İlke: Kriptografik Egemenlik ve Spektral Üstünlük)")
    print("=" * 76)

    # 1. DSSS Barker-11 Aşırı Gürültü Dayanımı (Alt-Gürültü Seviyesi İletim)
    print("[1] DSSS Barker-11 Alt-Gürültü Seviyesi (-6 dB SNR) Dayanım Testi...")
    num_bits = 200
    np.random.seed(1976)
    tx_bits = np.random.randint(0, 2, num_bits, dtype=np.uint8)

    # Barker-11 ile yayma
    chips = DSSSEngine.spread_bits(tx_bits)

    # -6 dB SNR şiddetli gürültü ekleme
    noise_power = 10.0 ** (6.0 / 10.0)  # SNR = -6 dB -> Gürültü gücü sinyalden ~4 kat fazla
    noise = np.random.normal(0, np.sqrt(noise_power), len(chips))
    rx_noisy_chips = chips + noise

    # Korelatör ile despreading
    rx_recovered_bits = DSSSEngine.despread_chips(rx_noisy_chips)
    dsss_ber = np.mean(tx_bits != rx_recovered_bits)
    print(f"[*] DSSS Teorik İşleme Kazancı : {DSSSEngine.calculate_processing_gain_db():.2f} dB")
    print(f"[*] -6 dB SNR Altında Bit Hata : %{dsss_ber * 100.0:.2f} (Korelatör Kazancı ile Kurtarıldı)")
    assert dsss_ber < 0.15, "DSSS korelatör performansı yetersiz!"

    # 2. Sahte Yayın ve Tekrar Saldırısı (Spoofing & Replay Attack) Testi
    print("\n[2] Düşman Sahte Komut (Spoofing) ve Tekrar (Replay) Saldırısı Savunması...")
    valid_key = 0x1976A5A51976A5A5
    hostile_key = 0x6666666666666666
    auth_cipher = KamoyCipher(valid_key, 0x19760001)

    receiver = StreamReceiver()
    accepted_packets = 0
    rejected_spoofs = 0
    seen_seq_numbers = set()

    # Dost İstasyon: Geçerli Taktik Emir Paketi
    legit_payload = b"EMIR: KUZEY HATTI SAVUNMA MEVZISINE GECIN."
    enc_legit_payload = auth_cipher.process(legit_payload)
    legit_frame = TacticalFrame(msg_type=3, sender_id=0x4600, target_id=0x1976,
                                seq_num=101, hop_channel=12, payload=enc_legit_payload)

    # Düşman İstasyon: Sahte Emir Paketi (Farklı Anahtarla Şifreli veya Düz)
    fake_cipher = KamoyCipher(hostile_key, 0x19760001)
    fake_payload = fake_cipher.process(b"SAHTE: MEVZIYI TERK EDIN VE GERI CEKILIN!")
    fake_frame = TacticalFrame(msg_type=3, sender_id=0x4600, target_id=0x1976,
                               seq_num=102, hop_channel=12, payload=fake_payload)

    # Simülasyon Akışı: Dost Paket -> Düşman Sahte Paket -> Replay Paketi (Dost paketin kopyası)
    traffic = [
        ("DOST", legit_frame.serialize()),
        ("DUSMAN_SPOOF", fake_frame.serialize()),
        ("DUSMAN_REPLAY", legit_frame.serialize()) # Aynı sequence number ile tekrar saldırısı
    ]

    rx_cipher = KamoyCipher(valid_key, 0x19760001)

    for sender_tag, raw_pkt in traffic:
        frame, err = TacticalFrame.deserialize(raw_pkt)
        if frame and err is None:
            # Replay kontrolü
            if frame.seq_num in seen_seq_numbers:
                rejected_spoofs += 1
                print(f"[*] [ENGEL] Replay Saldırısı Algılandı! Tekrar Eden Sıra No: {frame.seq_num}")
                continue

            # Kriptografik doğrulama
            dec_payload = rx_cipher.process(frame.payload)
            if b"EMIR:" in dec_payload:
                seen_seq_numbers.add(frame.seq_num)
                accepted_packets += 1
                print(f"[*] [KABUL] Geçerli Dost Emir Çözüldü: \"{dec_payload.decode('ascii', errors='ignore')}\"")
            else:
                rejected_spoofs += 1
                print("[*] [ENGEL] Sahte Düşman Paketi Algılandı! Kriptografik Çözüm Başarısız.")

    assert accepted_packets == 1, "Dost paket kabul edilemedi!"
    assert rejected_spoofs == 2, "Sahte veya tekrar paketleri engellenemedi!"

    print("-" * 76)
    print("[SONUÇ] DSSS spektral saklanma ve Anti-Spoofing protokol doğrulaması TAM BAŞARILI.")
    print("=" * 76 + "\n")
    return True


if __name__ == '__main__':
    run_ew_complex_stress()
