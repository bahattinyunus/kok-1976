#!/usr/bin/env python3
"""
KÖK-1976 Stres Testi: CRPA Adaptif Uzamsal Hüzmeleme ve Karıştırıcı Bastırma (Spatial Nulling)
Mehmet Hâcim Kamoy ve ASELSAN Anti-Jamming Elektronik Koruma Doktrini
"""

import sys
import os
import math
import random
import cmath

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from radio.sdr.crpa import CRPABeamformerPy


def test_crpa_nulling():
    print("============================================================================")
    print("  KÖK-1976 CRPA UZAMSAL HÜZMELEME VE KARIŞTIRICI BASTIRMA STRES TESTİ")
    print("  (Kamoy Doktrini 1. ve 3. İlke: Ağır Karıştırmada Fiziksel Katman Bastırma)")
    print("============================================================================")

    # Coğrafi ve Taktik Senaryo:
    # Dost Verici: 0.0° Kuzey İstikameti (İleri Müfreze) - Güç: 1.0 (0 dB)
    # Düşman Jammer: 45.0° Kuzey-Doğu İstikameti (Geniş Bant Baraj Jammer) - Güç: +26 dB (Genlik: 20.0)
    desired_azimuth = 0.0
    jammer_azimuth = 45.0

    beamformer = CRPABeamformerPy(mu=0.1)

    sv_des = beamformer.steering_vector(desired_azimuth)
    sv_jam = beamformer.steering_vector(jammer_azimuth)

    # Başlangıçta anten kazançları (Tekli Referans Anten Durumu)
    initial_gain_des = beamformer.calculate_array_gain_db(desired_azimuth)
    initial_gain_jam = beamformer.calculate_array_gain_db(jammer_azimuth)

    print(f"[*] Dost İstasyon Açısı : {desired_azimuth:5.1f}° | Sinyal Genliği: 1.0 (0 dBm)")
    print(f"[*] Düşman Jammer Açısı : {jammer_azimuth:5.1f}° | Jammer Genliği: 20.0 (+26 dBm Güçlü Baraj)")
    print(f"[*] Ham Anten Giriş SINR: -26.0 dB (Telsiz Tamamen Felç/Karıştırılmış)")
    print("-" * 76)

    # 400 Numunelik Adaptasyon Süreci
    num_samples = 400
    random.seed(1976)

    for k in range(num_samples):
        # Dost sinyali (BPSK sembolü +1 veya -1)
        s_des = 1.0 if (k % 20 < 10) else -1.0
        # Düşman gürültü jammer'ı (Yüksek güçlü rastgele Gauss/faz darbesi)
        jam_phase = random.uniform(0, 2 * math.pi)
        s_jam = 20.0 * cmath.exp(1j * jam_phase)
        # Termal gürültü
        noise = complex(random.gauss(0, 0.1), random.gauss(0, 0.1))

        # 4 anten snapshot'ı: x = s_des * sv_des + s_jam * sv_jam + noise
        snapshot = [
            s_des * sv_des[m] + s_jam * sv_jam[m] + noise
            for m in range(4)
        ]

        beamformer.process_sample(snapshot)

    # Adaptasyon Sonrası Dizi Kazançları
    final_gain_des = beamformer.calculate_array_gain_db(desired_azimuth)
    final_gain_jam = beamformer.calculate_array_gain_db(jammer_azimuth)
    null_depth_db = initial_gain_jam - final_gain_jam

    print(f"[+] Düşman Jammer Yönündeki Bastırma (Null Derinliği) : {null_depth_db:.2f} dB (Derin Uzamsal Çukur)")
    print(f"[+] Dost Sinyal Yönündeki Kalan Kazanç                 : {final_gain_des:.2f} dB")
    print(f"[+] Çıkışta Elde Edilen Efektif SINR                  : +{final_gain_des - final_gain_jam:.2f} dB")
    print("-" * 76)

    # Doğrulama: Jammer en az 25 dB bastırılmış olmalı
    assert null_depth_db > 25.0, f"Jammer bastırma yetersiz: {null_depth_db:.2f} dB"
    # Çıkış SINR pozitif olmalı (Dost sinyal kurtarıldı)
    assert (final_gain_des - final_gain_jam) > 10.0, "Dost sinyal kurtarılamadı!"

    print("----------------------------------------------------------------------------")
    print("[SONUÇ] CRPA 4-Elemanlı Power Inversion Karıştırıcı Bastırma TAM BAŞARILI.")
    print("============================================================================\n")


if __name__ == "__main__":
    test_crpa_nulling()
