#!/usr/bin/env python3
"""
KÖK-1976 Kontrollü Alış Paternli Anten (CRPA) ve Uzamsal Karıştırıcı Bastırma Simülatörü
4-Elemanlı Faz Dizisi, Power Inversion ve LMS Adaptif Uzamsal Hüzmeleme
"""

import math
import cmath
from typing import List, Tuple


class CRPABeamformerPy:
    """4-Elemanlı Düzlemsel CRPA Anten Dizisi ve Adaptif Hüzmeleyici."""

    def __init__(self, mu: float = 0.005):
        self.num_elements = 4
        self.mu = mu
        self.weights = [1.0 + 0j, 0.0 + 0j, 0.0 + 0j, 0.0 + 0j]

    def reset(self):
        self.weights = [1.0 + 0j, 0.0 + 0j, 0.0 + 0j, 0.0 + 0j]

    def steering_vector(self, azimuth_deg: float) -> List[complex]:
        """Verilen geliş açısı için 4 elemanın faz kayma vektörü."""
        rad = math.radians(azimuth_deg)
        sin_a = math.sin(rad)
        cos_a = math.cos(rad)

        # Eleman Geometrisi (d = lambda / 2): (0,0), (d,0), (0,d), (d,d)
        phases = [
            0.0,
            math.pi * sin_a,
            math.pi * cos_a,
            math.pi * (sin_a + cos_a)
        ]
        return [cmath.exp(1j * p) for p in phases]

    def process_sample(self, snapshot: List[complex]) -> complex:
        """
        4 antenden gelen eşzamanlı I/Q örneğini işler.
        Normalize LMS (NLMS) ile güç inversiyonu uygulayarak karıştırıcıyı bastırır.
        """
        # y = w0* x0 + w1* x1 + w2* x2 + w3* x3
        y = sum(w.conjugate() * x for w, x in zip(self.weights, snapshot))

        # Toplam anlık güç (Normalizasyon için)
        norm_power = sum(abs(x) ** 2 for x in snapshot) + 1e-4
        step = self.mu / norm_power

        # NLMS Adaptasyonu: w_m[k+1] = w_m[k] - (mu / P) * conj(y) * x_m
        for m in range(1, self.num_elements):
            delta = step * y.conjugate() * snapshot[m]
            self.weights[m] -= delta

        return y

    def calculate_array_gain_db(self, test_azimuth_deg: float) -> float:
        """Verilen test açısındaki anten dizisi kazancı (dB)."""
        sv = self.steering_vector(test_azimuth_deg)
        # Array Factor = sum(w_m* * sv_m)
        af = sum(w.conjugate() * s for w, s in zip(self.weights, sv))
        power = abs(af) ** 2
        if power < 1e-6:
            power = 1e-6
        return 10.0 * math.log10(power)
