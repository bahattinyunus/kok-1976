#!/usr/bin/env python3
"""
KÖK-1976 Stres Testi: Taktik Radyo Yön Bulucu (RDF) ve Hedef Kestirimi (Triangulation)
Mehmet Hâcim Kamoy ve Millî Elektronik Harp Kestirme Sistemleri Doğrulaması
"""

import sys
import os
import math

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from radio.sdr.df import WatsonWattDirectionFinder


def angular_diff(a: float, b: float) -> float:
    """İki açı arasındaki asgari dairesel mesafeyi (0 - 180 derece) hesaplar."""
    diff = abs(a - b) % 360.0
    return 360.0 - diff if diff > 180.0 else diff


def test_direction_finding():
    print("============================================================================")
    print("  KÖK-1976 TAKTİK RADYO YÖN BULUCU (RDF) VE KERTERİZ KESTİRİM TESTİ")
    print("  (Kamoy Doktrini 1. ve 7. İlke: Düşman Jammer ve Hedef Konum Tespiti)")
    print("============================================================================")

    # Coğrafi Senaryo:
    # İstasyon 1: Antalya Sahil Mevzisi (X = 0 km, Y = 0 km)
    # İstasyon 2: Alanya İleri Gözlem İstasyonu (X = 50 km, Y = 0 km)
    # Düşman Hedefi: Akdeniz'deki Düşman Elektronik Harp Gemisi (X = 20 km, Y = 60 km)
    st1 = WatsonWattDirectionFinder(station_id=1, pos_x_km=0.0, pos_y_km=0.0)
    st2 = WatsonWattDirectionFinder(station_id=2, pos_x_km=50.0, pos_y_km=0.0)

    target_true_x = 20.0
    target_true_y = 60.0

    print(f"[*] Gerçek Hedef Koordinatları: X = {target_true_x:.2f} km, Y = {target_true_y:.2f} km")
    print(f"[*] İstasyon 1 (Antalya)      : X = {st1.pos_x:.1f} km, Y = {st1.pos_y:.1f} km")
    print(f"[*] İstasyon 2 (Alanya)       : X = {st2.pos_x:.1f} km, Y = {st2.pos_y:.1f} km")
    print("-" * 76)

    # 1. İstasyon 1 Simülasyonu (+18 dB SNR)
    st1_true_deg = st1.simulate_emitter_signal(target_true_x, target_true_y, snr_db=18.0, num_samples=128)
    deg1, conf1, rss1 = st1.calculate_bearing()
    err1 = angular_diff(deg1, st1_true_deg)
    print(f"[+] İstasyon 1 Kerterizi : {deg1:6.2f}° (Gerçek: {st1_true_deg:6.2f}°, Hata: {err1:.2f}°, Güven: %{conf1*100:.1f})")
    assert err1 < 2.0, f"İstasyon 1 kerteriz hatası çok yüksek: {err1:.2f}°"

    # 2. İstasyon 2 Simülasyonu (+18 dB SNR)
    st2_true_deg = st2.simulate_emitter_signal(target_true_x, target_true_y, snr_db=18.0, num_samples=128)
    deg2, conf2, rss2 = st2.calculate_bearing()
    err2 = angular_diff(deg2, st2_true_deg)
    print(f"[+] İstasyon 2 Kerterizi : {deg2:6.2f}° (Gerçek: {st2_true_deg:6.2f}°, Hata: {err2:.2f}°, Güven: %{conf2*100:.1f})")
    assert err2 < 2.0, f"İstasyon 2 kerteriz hatası çok yüksek: {err2:.2f}°"

    # 3. İki Kerterizin Kesişimi ile Üçgenleme (Triangulation)
    est_target = WatsonWattDirectionFinder.triangulate(st1, deg1, st2, deg2)
    assert est_target is not None, "Hedef kestirim kesişimi bulunamadı!"

    est_x, est_y = est_target
    dist_error = math.hypot(est_x - target_true_x, est_y - target_true_y)
    print("-" * 76)
    print(f"[*] Kestirilen Koordinat : X = {est_x:.2f} km, Y = {est_y:.2f} km")
    print(f"[*] Konumlandırma Hatası : {dist_error:.3f} km (< 2.5 km hedef çemberi içinde)")
    assert dist_error < 2.5, f"Hedef kestirim mesafesi çok yüksek: {dist_error} km"

    # 4. Ağır Düşman Gürültüsü Altında (+3 dB SNR) Dayanım
    st1.reset()
    st1.simulate_emitter_signal(target_true_x, target_true_y, snr_db=3.0, num_samples=128)
    deg_noisy, conf_noisy, _ = st1.calculate_bearing()
    err_noisy = angular_diff(deg_noisy, st1_true_deg)
    print(f"[*] +3 dB Ağır Gürültüde Kerteriz Hatası: {err_noisy:.2f}° (Sinyal başarıyla kurtarıldı)")
    assert err_noisy < 6.0, "Gürültü altında yön kestirimi çöktü!"

    print("----------------------------------------------------------------------------")
    print("[SONUÇ] Watson-Watt ve Adcock faz kestirim sistemi TAM BAŞARILI.")
    print("============================================================================\n")


if __name__ == "__main__":
    test_direction_finding()
