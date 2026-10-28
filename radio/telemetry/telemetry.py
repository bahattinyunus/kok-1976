"""
KÖK 1976 - Taktik Telemetri Çerçevesi ve Protokol Katmanı
(0x1976KOK Taktik Senkronizasyon Başlıklı Paket Ayrıştırıcı ve Üretici)
"""

import struct


def crc8(data: bytes, init: int = 0x00) -> int:
    """CRC-8 (Polinom 0x07, Başlangıç 0x00)"""
    crc = init
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc


def crc16_ccitt(data: bytes, init: int = 0xFFFF) -> int:
    """CRC-16-CCITT (Polinom 0x1021, Başlangıç 0xFFFF)"""
    crc = init
    for b in data:
        crc ^= (b << 8) & 0xFFFF
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


class TacticalFrame:
    SYNC_BYTES = bytes([0x19, 0x76, 0x4B, 0x4F, 0x4B])  # 0x1976KOK
    MAX_PAYLOAD = 64

    def __init__(self, msg_type: int = 1, sender_id: int = 0x1976, target_id: int = 0xFFFF,
                 seq_num: int = 0, hop_channel: int = 0, payload: bytes = b''):
        self.msg_type = msg_type
        self.sender_id = sender_id
        self.target_id = target_id
        self.seq_num = seq_num
        self.hop_channel = hop_channel
        self.payload = bytes(payload)[:self.MAX_PAYLOAD]

    def serialize(self) -> bytes:
        payload_len = len(self.payload)
        # Başlık CRC hesabı için ilk 9 baytı paketle (header_crc hariç)
        partial_hdr = struct.pack('<BHHHBB',
                                  self.msg_type,
                                  self.sender_id,
                                  self.target_id,
                                  self.seq_num,
                                  self.hop_channel,
                                  payload_len)
        header_crc = crc8(partial_hdr)
        full_hdr = partial_hdr + bytes([header_crc])

        # Çerçeve CRC hesabı (Başlık + Faydalı Yük)
        body = full_hdr + self.payload
        frame_crc = crc16_ccitt(body)
        crc_bytes = struct.pack('>H', frame_crc)

        return self.SYNC_BYTES + body + crc_bytes

    @classmethod
    def deserialize(cls, data: bytes):
        if len(data) < len(cls.SYNC_BYTES) + 10 + 2:
            return None, "Yetersiz çerçeve uzunluğu"

        # 1. Sync kontrolü
        if not data.startswith(cls.SYNC_BYTES):
            return None, "Senkronizasyon kaybı (Geçersiz Başlık)"

        offset = len(cls.SYNC_BYTES)
        header_bytes = data[offset : offset + 10]
        msg_type, sender_id, target_id, seq_num, hop_channel, payload_len, header_crc = struct.unpack('<BHHHBBB', header_bytes)

        # Başlık CRC kontrolü
        if crc8(header_bytes[:9]) != header_crc:
            return None, "Başlık CRC-8 hatası"

        if payload_len > cls.MAX_PAYLOAD:
            return None, "Geçersiz faydalı yük boyutu"

        total_expected = offset + 10 + payload_len + 2
        if len(data) < total_expected:
            return None, "Eksik paket verisi"

        payload = data[offset + 10 : offset + 10 + payload_len]
        recv_crc = struct.unpack('>H', data[offset + 10 + payload_len : total_expected])[0]

        calc_crc = crc16_ccitt(data[offset : offset + 10 + payload_len])
        if recv_crc != calc_crc:
            return None, "Çerçeve CRC-16 uyumsuzluğu"

        frame = cls(msg_type, sender_id, target_id, seq_num, hop_channel, payload)
        return frame, None


class StreamReceiver:
    """
    Gürültülü akıştan bayt bayt taktik paketleri yakalayan durum makinesi
    """
    def __init__(self):
        self.buffer = bytearray()
        self.packets_received = 0
        self.crc_errors = 0

    def feed(self, byte_chunk: bytes):
        self.buffer.extend(byte_chunk)
        frames = []

        while len(self.buffer) >= 17:  # En küçük paket: 5 sync + 10 hdr + 0 payload + 2 crc
            # Sync kelimesini ara
            idx = self.buffer.find(TacticalFrame.SYNC_BYTES)
            if idx == -1:
                # Sync bulunamadı, son 4 baytı tut gerisini at
                self.buffer = self.buffer[-4:]
                break

            if idx > 0:
                self.buffer = self.buffer[idx:]

            if len(self.buffer) < 15:
                break

            payload_len = self.buffer[5 + 8]  # payload_len alanı
            if payload_len > TacticalFrame.MAX_PAYLOAD:
                # Sahte sync! Bir bayt atlayıp aramaya devam et
                self.buffer = self.buffer[1:]
                continue

            expected_len = 5 + 10 + payload_len + 2
            if len(self.buffer) < expected_len:
                break  # Paketin geri kalanı henüz gelmedi

            candidate = bytes(self.buffer[:expected_len])
            frame, err = TacticalFrame.deserialize(candidate)
            if frame:
                frames.append(frame)
                self.packets_received += 1
                self.buffer = self.buffer[expected_len:]
            else:
                self.crc_errors += 1
                self.buffer = self.buffer[1:]

        return frames
