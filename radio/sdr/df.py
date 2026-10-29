#!/usr/bin/env python3
"""
KÖK-1976 Taktik Radyo Yön Bulucu ve Kestirme (RDF / Direction Finding) Motoru
Watson-Watt / Adcock Faz Dizisi & Kerteriz Kesişimi (Triangulation)
"""

import math
from typing import Tuple, List, Optional


class WatsonWattDirectionFinder:
    """
    4-Elemanlı Adcock Anten Dizisi ve Watson-Watt Prensibi ile
    Kerteriz Açısı (Azimuth) ve Üçgenleme Kestiricisi.
    """

    def __init__(self, station_id: int, pos_x_km: float, pos_y_km: float):
        self.station_id = station_id
        self.pos_x = pos_x_km
        self.pos_y = pos_y_km
        self.samples_ns: List[float] = []
        self.samples_ew: List[float] = []
        self.samples_sense: List[float] = []

    def reset(self):
        self.samples_ns.clear()
        self.samples_ew.clear()
        self.samples_sense.clear()

    def feed_sample(self, v_ns: float, v_ew: float, v_sense: float):
        """RF anten voltaj örneklerini ekle."""
        self.samples_ns.append(v_ns)
        self.samples_ew.append(v_ew)
        self.samples_sense.append(v_sense)

    def simulate_emitter_signal(self, emitter_x: float, emitter_y: float,
                                snr_db: float = 20.0, num_samples: int = 32) -> float:
        """
        Verilen hedef vericinin yaydığı elektromanyetik dalganın
        bu istasyondaki anten dizisine düşen örneklerini simüle eder.
        Dönen değer gerçek geometrik kerteriz açısıdır (derece).
        """
        import random

        dx = emitter_x - self.pos_x
        dy = emitter_y - self.pos_y
        dist = math.hypot(dx, dy)
        if dist < 0.001:
            return 0.0

        # Gerçek açı: Kuzeyden (Y ekseni) saat yönünde Doğuya (X ekseni)
        true_angle_rad = math.atan2(dx, dy)
        if true_angle_rad < 0:
            true_angle_rad += 2 * math.pi
        true_angle_deg = math.degrees(true_angle_rad)

        # Gürültü seviyesi
        noise_std = 10.0 ** (-snr_db / 20.0)

        # Adcock voltajları
        # V_ns ~ cos(theta), V_ew ~ sin(theta)
        for _ in range(num_samples):
            carrier_phase = random.uniform(0, 2 * math.pi)
            carrier = math.cos(carrier_phase)
            v_ns = carrier * math.cos(true_angle_rad) + random.gauss(0, noise_std)
            v_ew = carrier * math.sin(true_angle_rad) + random.gauss(0, noise_std)
            v_sense = carrier + random.gauss(0, noise_std)
            self.feed_sample(v_ns, v_ew, v_sense)

        return true_angle_deg

    def calculate_bearing(self) -> Tuple[float, float, float]:
        """
        Biriken numunelerden Watson-Watt koherent çarpım (Sense referanslı)
        ile kerteriz açısı, güvenilirlik ve RSSI hesaplar.
        Döndürür: (azimuth_deg, confidence, rssi_dbm)
        """
        if not self.samples_ns:
            return (0.0, 0.0, -120.0)

        n = len(self.samples_ns)
        # Watson-Watt koherent çarpım: <V_NS, V_Sense> ve <V_EW, V_Sense>
        dot_ns = sum(ns * s for ns, s in zip(self.samples_ns, self.samples_sense)) / n
        dot_ew = sum(ew * s for ew, s in zip(self.samples_ew, self.samples_sense)) / n

        mag = math.hypot(dot_ns, dot_ew)
        if mag < 1e-4:
            return (0.0, 0.0, -120.0)

        # Kerteriz açısı: Kuzey 0°, Doğu 90° (atan2(X, Y) = atan2(EW, NS))
        angle_rad = math.atan2(dot_ew, dot_ns)
        if angle_rad < 0:
            angle_rad += 2 * math.pi
        deg = math.degrees(angle_rad)

        confidence = min(0.99, mag / (mag + 0.1))
        rssi_dbm = -90.0 + min(70.0, mag * 40.0)

        return (deg, confidence, rssi_dbm)

    @staticmethod
    def triangulate(station_a: 'WatsonWattDirectionFinder', bearing_a_deg: float,
                    station_b: 'WatsonWattDirectionFinder', bearing_b_deg: float) -> Optional[Tuple[float, float]]:
        """
        İki ayrı kestirme istasyonunun kerteriz kesişiminden hedef koordinatlarını bulur.
        """
        r1 = math.radians(bearing_a_deg)
        r2 = math.radians(bearing_b_deg)

        s1, c1 = math.sin(r1), math.cos(r1)
        s2, c2 = math.sin(r2), math.cos(r2)

        dx = station_b.pos_x - station_a.pos_x
        dy = station_b.pos_y - station_a.pos_y

        det = c1 * s2 - s1 * c2
        if abs(det) < 0.05:
            return None

        t1 = (dy * s2 - dx * c2) / det
        if t1 <= 0.0:
            return None

        target_x = station_a.pos_x + t1 * s1
        target_y = station_a.pos_y + t1 * c1
        return (target_x, target_y)
