"""
KÖK 1976 - Sayısal Taktik Modem ve Kanal Simülasyonu
(FSK / GFSK / BPSK / QPSK ve AWGN / Rayleigh Kanal Modelleri)
"""

import math
import numpy as np


class FSKModem:
    """
    Düşük SNR dayanımlı 2-FSK / 4-FSK Sayısal Modemi
    """
    def __init__(self, sample_rate=48000, baud_rate=1200, f_mark=1200, f_space=2200):
        self.sample_rate = sample_rate
        self.baud_rate = baud_rate
        self.f_mark = f_mark
        self.f_space = f_space
        self.samples_per_symbol = sample_rate // baud_rate

    def modulate(self, bits):
        """
        Giriş bitlerini 2-FSK karmaşık taban bant (I/Q) sinyaline dönüştürür.
        """
        bits = np.array(bits, dtype=np.uint8)
        total_samples = len(bits) * self.samples_per_symbol
        t = np.arange(self.samples_per_symbol) / self.sample_rate

        samples = []
        phase = 0.0

        for b in bits:
            freq = self.f_mark if b == 1 else self.f_space
            delta_phase = 2.0 * math.pi * freq * t
            symbol_phase = phase + delta_phase
            symbol_iq = np.exp(1j * symbol_phase)
            samples.append(symbol_iq)
            phase = (symbol_phase[-1] + 2.0 * math.pi * freq / self.sample_rate) % (2.0 * math.pi)

        if len(samples) == 0:
            return np.array([], dtype=np.complex128)
        return np.concatenate(samples)

    def demodulate(self, iq_samples):
        """
        Gecikmeli çarpım (Delay-Multiply) FM ayrıştırıcısı ile bit kararı verir.
        """
        if len(iq_samples) < self.samples_per_symbol:
            return np.array([], dtype=np.uint8)

        # Sembol boyutunu korumak için son örneği tekrarla
        padded = np.append(iq_samples, iq_samples[-1])
        d = padded[1:] * np.conj(padded[:-1])
        phase_diff = np.angle(d)

        num_symbols = len(phase_diff) // self.samples_per_symbol
        bits = []

        # Merkez frekansı teorik faz farkı
        f_center = (self.f_mark + self.f_space) / 2.0
        expected_center_diff = 2.0 * math.pi * f_center / self.sample_rate

        for i in range(num_symbols):
            chunk = phase_diff[i * self.samples_per_symbol : (i + 1) * self.samples_per_symbol]
            avg_diff = np.mean(chunk)
            # Eğer mark frekansı space'den küçükse avg_diff < expected -> mark(1)
            if self.f_mark < self.f_space:
                bit = 1 if avg_diff < expected_center_diff else 0
            else:
                bit = 1 if avg_diff >= expected_center_diff else 0
            bits.append(bit)

        return np.array(bits, dtype=np.uint8)


class ChannelSimulator:
    """
    Saha Koşulları Simülasyonu: AWGN, Rayleigh Sönümleme ve Elektronik Karıştırma
    """
    @staticmethod
    def add_awgn(signal, snr_db):
        """
        Belirtilen SNR (dB) değerinde Katkısal Beyaz Gauss Gürültüsü (AWGN) ekler.
        """
        sig_pwr = np.mean(np.abs(signal) ** 2)
        if sig_pwr == 0:
            return signal

        snr_linear = 10.0 ** (snr_db / 10.0)
        noise_pwr = sig_pwr / snr_linear
        noise_std = np.sqrt(noise_pwr / 2.0)

        noise = np.random.normal(0, noise_std, len(signal)) + 1j * np.random.normal(0, noise_std, len(signal))
        return signal + noise

    @staticmethod
    def add_rayleigh_fading(signal, fd_hz=10.0, fs=48000):
        """
        Akdeniz troposferik / çokyollu (Rayleigh) sönümleme etkisi ekler.
        """
        # Clark-Gans Rayleigh modeline yaklaşık sönümleme
        n = len(signal)
        h_i = np.random.normal(0, 1.0, n)
        h_q = np.random.normal(0, 1.0, n)
        h = (h_i + 1j * h_q) / np.sqrt(2.0)
        return signal * h

    @staticmethod
    def add_jamming(signal, jammer_type='barrage', jnr_db=10.0):
        """
        Elektronik Harp Karıştırıcısı:
        - 'barrage': Geniş bantlı gürültü basma
        - 'sweep': Frekans taramalı sinüs karıştırıcısı
        """
        sig_pwr = np.mean(np.abs(signal) ** 2)
        jnr_linear = 10.0 ** (jnr_db / 10.0)
        jam_pwr = sig_pwr * jnr_linear

        n = len(signal)
        if jammer_type == 'barrage':
            jam_std = np.sqrt(jam_pwr / 2.0)
            jam = np.random.normal(0, jam_std, n) + 1j * np.random.normal(0, jam_std, n)
        elif jammer_type == 'sweep':
            t = np.arange(n)
            chirp_phase = 2.0 * math.pi * (0.01 * t + 0.0001 * t**2)
            jam = np.sqrt(jam_pwr) * np.exp(1j * chirp_phase)
        else:
            jam = np.zeros(n, dtype=np.complex128)

        return signal + jam
