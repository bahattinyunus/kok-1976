"""
KÖK 1976 - Doğrudan Dizili Yayılı Spektrum (DSSS) Simülasyonu
(Barker-11 Dizisi ve Eşleşmiş Süzgeç Korelatör Modeli)
"""

import numpy as np


class DSSSEngine:
    BARKER_11 = np.array([1, -1, 1, 1, -1, 1, 1, 1, -1, -1, -1], dtype=np.int8)

    @classmethod
    def spread_bits(cls, bits: np.ndarray) -> np.ndarray:
        """
        Her 0/1 bitini 11-çiplik Barker dizisiyle genişletir.
        """
        bits = np.array(bits, dtype=np.uint8)
        chips = []
        for b in bits:
            polarity = 1 if b == 1 else -1
            chips.append(polarity * cls.BARKER_11)
        if not chips:
            return np.array([], dtype=np.int8)
        return np.concatenate(chips)

    @classmethod
    def despread_chips(cls, chips: np.ndarray) -> np.ndarray:
        """
        Gelen çipler üzerinde eşleşmiş süzgeç korelasyonu yaparak bit kararı verir.
        """
        chips = np.array(chips, dtype=np.float64)
        num_symbols = len(chips) // 11
        bits = []

        for i in range(num_symbols):
            chunk = chips[i * 11 : (i + 1) * 11]
            corr = np.sum(chunk * cls.BARKER_11)
            bits.append(1 if corr > 0 else 0)

        return np.array(bits, dtype=np.uint8)

    @classmethod
    def calculate_processing_gain_db(cls) -> float:
        """
        11-çipli Barker kodu için işleme kazancı (10.41 dB)
        """
        return 10.0 * np.log10(11.0)
