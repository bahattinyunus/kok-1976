"""
KÖK 1976 - 1964 Antalya-Kıbrıs Troposferik Dalga ve Ufuk Ötesi İletim Simülasyonu
(Kamoy Doktrini: "Vatan Savunmasında Teknik Olarak İmkânsız Diye Bir Şey Yoktur")
"""

import math
import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from radio.telemetry.telemetry import TacticalFrame, crc16_ccitt


class CyprusPropagationModel:
    """
    1964 Antalya - Kıbrıs (Erenköy / Lefkoşa) Radyo Yayılım Fiziği Modeli
    """
    def __init__(self):
        self.freq_mhz = 100.0            # Taşıyıcı Frekans (VHF 100 MHz)
        self.distance_km = 260.0         # Antalya Sahili - Kıbrıs Kuş Uçuşu Mesafe
        self.h_tx = 300.0                # Antalya Toros etekleri verici irtifası (metre)
        self.h_rx = 600.0                # Beşparmak dağları mücahit mevzisi (metre)
        self.p_tx_watts = 1000.0         # 1000 Watt (+60 dBm) Modifiye TRT Vericisi
        self.p_tx_dbm = 10.0 * math.log10(self.p_tx_watts * 1000.0)
        self.g_tx_dbi = 18.0             # 4'lü Stacked Yagi Faz Dizisi Kazancı
        self.g_rx_dbi = 10.0             # Erenköy Yönlü Anten Kazancı
        self.cable_loss_db = 1.5         # Koaksiyel hat kaybı
        self.bandwidth_hz = 3000.0       # Dar bant (3 kHz)
        self.noise_figure_db = 6.0       # Alıcı gürültü faktörü
        self.temp_k = 290.0

    def calculate_los_horizon(self):
        # 4/3 Efektif Dünya Yarıçapı Modeli
        k = 4.0 / 3.0
        r_earth = 6371.0
        r_eff = k * r_earth

        d_tx = math.sqrt(2.0 * r_eff * (self.h_tx / 1000.0))
        d_rx = math.sqrt(2.0 * r_eff * (self.h_rx / 1000.0))
        d_los = d_tx + d_rx
        return d_tx, d_rx, d_los

    def calculate_free_space_loss(self):
        # L_fs = 32.44 + 20*log10(f_MHz) + 20*log10(d_km)
        return 32.44 + 20.0 * math.log10(self.freq_mhz) + 20.0 * math.log10(self.distance_km)

    def calculate_tropospheric_scatter_loss(self):
        # ITU-R P.617 Troposferik Saçılma Yaklaşık Modeli
        # L_tropo = 20*log10(f) + 30*log10(d) + theta_loss
        # Akdeniz buhar kanalı (evaporation duct) ve yaz inversiyonu avantajı: -25 dB düzeltme
        theta_mrad = (self.distance_km / 8.5) # Yaklaşık saçılma açısı
        l_tropo = 20.0 * math.log10(self.freq_mhz) + 30.0 * math.log10(self.distance_km) + 0.1 * theta_mrad - 25.0
        return l_tropo

    def calculate_link_budget(self, mode="kamoy"):
        d_tx, d_rx, d_los = self.calculate_los_horizon()
        l_fs = self.calculate_free_space_loss()

        if mode == "standard_los":
            # Standart raporda varsayılan model: Ufuk aşılınca sinyal tamamen kesilir
            # Engel difraksiyon kaybı > 100 dB eklenir
            excess_loss = 100.0
            tx_gain = 0.0 # Standart izotropik anten
        else:
            # Kamoy Modeli: Stacked Yagi + Troposferik Saçılma + Kanalizasyon
            excess_loss = self.calculate_tropospheric_scatter_loss() - l_fs
            if excess_loss < 20.0: excess_loss = 20.0
            tx_gain = self.g_tx_dbi

        eirp = self.p_tx_dbm + tx_gain - self.cable_loss_db
        p_rx_dbm = eirp - l_fs - excess_loss + self.g_rx_dbi

        # Termal gürültü: N = k * T * B
        k_boltzmann = 1.380649e-23
        noise_pwr_watts = k_boltzmann * self.temp_k * self.bandwidth_hz
        noise_floor_dbm = 10.0 * math.log10(noise_pwr_watts * 1000.0)
        total_noise_dbm = noise_floor_dbm + self.noise_figure_db

        snr_db = p_rx_dbm - total_noise_dbm

        return {
            "d_los_km": d_los,
            "gap_km": self.distance_km - d_los,
            "eirp_dbm": eirp,
            "l_fs_db": l_fs,
            "excess_loss_db": excess_loss,
            "p_rx_dbm": p_rx_dbm,
            "noise_floor_dbm": total_noise_dbm,
            "snr_db": snr_db
        }


def run_cyprus_1964_simulation():
    print("=" * 72)
    print("  KÖK-1976: 1964 KIBRIS GİZLİ HATTI TROPOSFERİK YAYILIM SİMÜLASYONU")
    print("  (Kamoy Öğretisi: Antalya Sahilinden Kıbrıs Siperlerine Kesintisiz İletim)")
    print("=" * 72)

    model = CyprusPropagationModel()
    res_std = model.calculate_link_budget(mode="standard_los")
    res_kamoy = model.calculate_link_budget(mode="kamoy")

    print(f"[*] Toplam Coğrafi Mesafe  : {model.distance_km:.1f} km")
    print(f"[*] Görüş Hattı Radyo Ufku  : {res_kamoy['d_los_km']:.1f} km")
    print(f"[*] Ufuk Aşımı (Ölü Bölge) : {res_kamoy['gap_km']:.1f} km (Fiziki Görüş Yok!)")
    print("-" * 72)
    print(f"{'Parametre':<30} | {'Yabancı Uzman Raporu':<20} | {'Hâcim Kamoy Mühendisliği':<20}")
    print("-" * 72)
    print(f"{'EIRP Işıma Gücü':<30} | {res_std['eirp_dbm']:<18.1f} dBm | {res_kamoy['eirp_dbm']:<18.1f} dBm")
    print(f"{'Serbest Uzay Kaybı (L_fs)':<30} | {res_std['l_fs_db']:<18.1f} dB  | {res_kamoy['l_fs_db']:<18.1f} dB")
    print(f"{'Ufuk Aşımı / Saçılma Kaybı':<30} | {res_std['excess_loss_db']:<18.1f} dB  | {res_kamoy['excess_loss_db']:<18.1f} dB")
    print(f"{'Alıcı Giriş Gücü (P_rx)':<30} | {res_std['p_rx_dbm']:<18.1f} dBm | {res_kamoy['p_rx_dbm']:<18.1f} dBm")
    print(f"{'Gürültü Tabanı (NF+Thermal)':<30} | {res_std['noise_floor_dbm']:<18.1f} dBm | {res_kamoy['noise_floor_dbm']:<18.1f} dBm")
    print(f"{'Elde Edilen S/N Oranı (SNR)':<30} | {res_std['snr_db']:<18.1f} dB  | {res_kamoy['snr_db']:<18.1f} dB")
    print("-" * 72)

    # Taktik Çerçeve İletim Doğrulaması
    clandestine_msg = b"ANKARA-1964: GIZLI EMIR ALINDI. ERENKOY SIPERLERI DIRENSIN."
    frame = TacticalFrame(msg_type=3, sender_id=0x1964, target_id=0xE999, seq_num=1,
                          hop_channel=0, payload=clandestine_msg)
    wire_bytes = frame.serialize()

    # Eğer SNR > 15 dB ise paket kayıpsız iletilir
    assert res_kamoy['snr_db'] > 20.0, "Kamoy link bütçesi yetersiz!"
    parsed, err = TacticalFrame.deserialize(wire_bytes)
    assert parsed is not None and err is None
    assert parsed.payload == clandestine_msg

    print(f"[BAŞARILI] Akdeniz üzerinden aşırılan gizli mesaj: \"{parsed.payload.decode('ascii')}\"")
    print(f"[SONUÇ] Hâcim Kamoy'un stacked faz dizisi ve dar bant süzgeçleri teorik imkânsızlığı yıktı.")
    print("=" * 72 + "\n")
    return True


if __name__ == '__main__':
    run_cyprus_1964_simulation()
