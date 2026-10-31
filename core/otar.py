#!/usr/bin/env python3
"""
KÖK-1976 Askerî TRANSEC ve Hava Üzerinden Anahtarlama (OTAR) Referans Modülü
DoD 5220.22-M Acil Kripto İmha (Zeroize) ve KEK/TEK Yönetimi
"""

import struct
from typing import Tuple, Optional
from tests.kok_core import KamoyCipher


class AntiTamperUnit:
    """Elektronik Harp ve Ele Geçirilme Karşıtı Acil Kripto İmha Birimi."""

    def __init__(self):
        self.is_zeroized = False
        self.trigger_reason = "NONE"
        self.timestamp = 0

    def secure_wipe(self, byte_array: bytearray):
        """DoD 5220.22-M Askerî 4 Geçişli Bellek İmhası."""
        length = len(byte_array)
        # 1. 0x55
        for i in range(length):
            byte_array[i] = 0x55
        # 2. 0xAA
        for i in range(length):
            byte_array[i] = 0xAA
        # 3. 0xFF
        for i in range(length):
            byte_array[i] = 0xFF
        # 4. 0x00
        for i in range(length):
            byte_array[i] = 0x00

    def trigger_zeroize(self, reason: str, timestamp: int):
        self.is_zeroized = True
        self.trigger_reason = reason
        self.timestamp = timestamp

    def verify_clean(self, byte_array: bytes) -> bool:
        return all(b == 0 for b in byte_array)


class OTARManager:
    """Hava Üzerinden Anahtarlama (Over-The-Air Rekeying) Yöneticisi."""

    MAGIC = 0x7652454B  # 0x76, 'R', 'E', 'K'

    def __init__(self, master_kek: int):
        self.kek = master_kek
        self.active_tek = 0
        self.active_key_id = 0
        self.last_sequence = 0
        self.tamper = AntiTamperUnit()

    def create_rekey_packet(self, new_tek: int, key_id: int, sequence_num: int, iv: int) -> bytes:
        """Karargâh / Kripto Subayı İstasyonu için 24 baytlık OTAR paketi üretir."""
        if self.tamper.is_zeroized:
            raise RuntimeError("Cihaz imha edilmiş durumda (Zeroized)!")

        raw_tek = bytearray(new_tek.to_bytes(8, byteorder='big'))
        cipher = KamoyCipher(self.kek, iv)
        encrypted_tek = cipher.process(bytes(raw_tek))

        # Kimlik doğrulama verisi: sequence(2B) + key_id(2B) + iv(4B) + encrypted_tek(8B)
        auth_data = struct.pack(">HHI", sequence_num, key_id, iv) + encrypted_tek
        mac = KamoyCipher.calculate_mac(self.kek, auth_data)

        # Paket formatı: Magic(4B) + Seq(2B) + KeyID(2B) + EncTek(8B) + IV(4B) + MAC(4B) = 24B
        packet = struct.pack(">IHH8sII", self.MAGIC, sequence_num, key_id, encrypted_tek, iv, mac)
        return packet

    def process_rekey_packet(self, packet_bytes: bytes) -> bool:
        """Taktik telsizde havadan gelen OTAR paketini çözer ve anahtarı günceller."""
        if self.tamper.is_zeroized:
            return False

        if len(packet_bytes) != 24:
            return False

        magic, seq, key_id, enc_tek, iv, mac = struct.unpack(">IHH8sII", packet_bytes)

        if magic != self.MAGIC:
            return False

        if seq <= self.last_sequence and self.last_sequence != 0:
            return False  # Replay koruması

        auth_data = struct.pack(">HHI", seq, key_id, iv) + enc_tek
        expected_mac = KamoyCipher.calculate_mac(self.kek, auth_data)
        if mac != expected_mac:
            return False  # Kriptografik doğrulama hatası

        cipher = KamoyCipher(self.kek, iv)
        dec_tek_bytes = cipher.process(enc_tek)
        self.active_tek = int.from_bytes(dec_tek_bytes, byteorder='big')
        self.active_key_id = key_id
        self.last_sequence = seq
        return True

    def emergency_zeroize(self, reason: str, timestamp: int):
        """Acil Kripto İmha: KEK ve TEK bellekten tamamen silinir."""
        kek_bytes = bytearray(self.kek.to_bytes(8, byteorder='big'))
        tek_bytes = bytearray(self.active_tek.to_bytes(8, byteorder='big'))
        self.tamper.secure_wipe(kek_bytes)
        self.tamper.secure_wipe(tek_bytes)

        self.kek = 0
        self.active_tek = 0
        self.active_key_id = 0
        self.last_sequence = 0
        self.tamper.trigger_zeroize(reason, timestamp)
