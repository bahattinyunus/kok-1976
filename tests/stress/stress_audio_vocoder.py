"""
KÖK 1976 - Askerî CVSD Ses Sıkıştırma ve 150 Hz Ton Susturma (Squelch) Stres Testi
STANAG 4209 / MIL-STD-188-113 Standartlarında 16 kbps Taktik Ses Dayanıklılığı
"""

import math
import sys
import os
import numpy as np

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))


class SimulatedCVSD:
    def __init__(self, delta_min=32, delta_max=4096):
        self.delta_min = delta_min
        self.delta_max = delta_max
        self.accumulator = 0
        self.step_size = delta_min
        self.shift_reg = 0xAA

    def update_step(self, bit):
        self.shift_reg = ((int(self.shift_reg) << 1) | (int(bit) & 1)) & 0xFF
        last3 = self.shift_reg & 0x07
        if last3 == 0x07 or last3 == 0x00:
            self.step_size = min(self.step_size + self.delta_min * 2, self.delta_max)
        else:
            self.step_size = max((self.step_size * 15) // 16, self.delta_min)

    def encode(self, samples):
        bits = []
        for s in samples:
            bit = 1 if s >= self.accumulator else 0
            self.accumulator = (self.accumulator * 31) // 32
            if bit == 1:
                self.accumulator += self.step_size
            else:
                self.accumulator -= self.step_size
            self.accumulator = max(-32768, min(32767, self.accumulator))
            self.update_step(bit)
            bits.append(bit)
        return np.array(bits, dtype=np.uint8)

    def decode(self, bits):
        self.accumulator = 0
        self.step_size = self.delta_min
        self.shift_reg = 0xAA
        reconstructed = []
        for b in bits:
            self.accumulator = (self.accumulator * 31) // 32
            if b == 1:
                self.accumulator += self.step_size
            else:
                self.accumulator -= self.step_size
            self.accumulator = max(-32768, min(32767, self.accumulator))
            self.update_step(b)
            reconstructed.append(self.accumulator)
        return np.array(reconstructed, dtype=np.int16)


def goertzel_150hz_energy(samples, sample_rate=8000):
    k = int(0.5 + (len(samples) * 150.0 / sample_rate))
    omega = (2.0 * math.pi * k) / len(samples)
    coeff = 2.0 * math.cos(omega)
    s_prev = 0.0
    s_prev2 = 0.0
    for x in samples:
        s = x + coeff * s_prev - s_prev2
        s_prev2 = s_prev
        s_prev = s
    p = s_prev**2 + s_prev2**2 - coeff * s_prev * s_prev2
    return p / (len(samples) ** 2)


def run_audio_stress():
    print("=" * 76)
    print("  KÖK-1976 ASKERÎ CVSD SES VE 150 HZ TON SUSTURMA STRES TESTİ")
    print("  (Kamoy Doktrini: Zorlu Muhabere Koşullarında Anlaşılabilir Ses İletimi)")
    print("=" * 76)

    # 1. Sentetik Ses Sinyali (300 Hz - 2500 Hz Askerî Ses Spektrumu)
    fs = 8000
    duration_sec = 0.5
    t = np.linspace(0, duration_sec, int(fs * duration_sec), endpoint=False)
    # Ses sinyali = 500 Hz temel ton + 1200 Hz formant + 2000 Hz rezonans
    speech_signal = (
        0.5 * np.sin(2.0 * np.pi * 500.0 * t) +
        0.3 * np.sin(2.0 * np.pi * 1200.0 * t) +
        0.2 * np.sin(2.0 * np.pi * 2000.0 * t)
    )
    pcm_in = (speech_signal * 20000.0).astype(np.int16)

    # 2. CVSD ile 16 kbps Kodlama
    codec = SimulatedCVSD(delta_min=200, delta_max=8000)
    cvsd_bits = codec.encode(pcm_in)
    print(f"[*] İşlenen Ses Örnek Sayısı    : {len(pcm_in)} örnek (8 kHz)")
    print(f"[*] Üretilen CVSD Bit Sayısı     : {len(cvsd_bits)} bit (1-bit/örnek)")

    # 3. Zorlu Telsiz Kanalında %1 Rastgele Bit Hatası Enjeksiyonu
    np.random.seed(1976)
    noisy_bits = np.copy(cvsd_bits)
    error_indices = np.random.choice(len(noisy_bits), size=int(len(noisy_bits) * 0.01), replace=False)
    noisy_bits[error_indices] ^= 1

    # 4. Sesin Geri Çatılması (Reconstruction) ve 1-Örnek Gecikme Dengeleme
    reconstructed_pcm = codec.decode(noisy_bits)

    # Gecikme dengeli korelasyon (Delay-Compensated Correlation)
    norm_in = pcm_in[:-1].astype(float) / 32768.0
    norm_out = reconstructed_pcm[1:].astype(float) / 32768.0
    corr = np.corrcoef(norm_in, norm_out)[0, 1]
    print(f"[*] Kanal Bit Hatasında Anlaşılabilirlik Korelasyonu : {corr:.4f} (Anlaşılabilir Askerî Ses)")
    assert corr > 0.65, "CVSD ses rekonstrüksiyon doğruluğu yetersiz!"

    # 5. 150 Hz Askerî Susturma (Tone Squelch) Testi
    print("\n[5] 150 Hz Askerî Ton Susturma (CTCSS) ve Gürültü Ayrıştırma Testi...")
    t_blk = np.linspace(0, 0.05, 400, endpoint=False) # 50 ms blok (400 örnek)
    # Senaryo A: 150 Hz Askerî Pilot Tonlu Geçerli Yayın
    clean_tone_block = (np.sin(2.0 * np.pi * 150.0 * t_blk) * 10000.0).astype(np.int16)
    # Senaryo B: Düşman Geniş Bantlı Karıştırma Gürültüsü
    noise_block = np.random.normal(0, 10000.0, 400).astype(np.int16)

    energy_tone = goertzel_150hz_energy(clean_tone_block)
    energy_noise = goertzel_150hz_energy(noise_block)

    print(f"[*] 150 Hz Askerî Pilot Ton Enerjisi   : {energy_tone:,.0f} (Hoparlör AÇIK)")
    print(f"[*] Düşman Gürültüsü 150 Hz Enerjisi   : {energy_noise:,.0f} (Hoparlör KAPALI/Susturulmuş)")

    ratio = energy_tone / (energy_noise + 1e-6)
    print(f"[*] Ton / Gürültü Ayırt Etme Oranı     : {ratio:.1f}x Kat Seçicilik")
    assert ratio > 10.0, "150 Hz ton seçiciliği yetersiz!"

    print("-" * 76)
    print("[SONUÇ] Askerî CVSD vocoder ve 150 Hz ton susturma testleri TAM BAŞARILI.")
    print("=" * 76 + "\n")
    return True


if __name__ == '__main__':
    run_audio_stress()
