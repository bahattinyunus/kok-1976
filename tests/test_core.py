"""
KÖK 1976 - Temel Çekirdek Birim Testleri
"""

import sys
import os
import unittest

# Proje dizinlerini sys.path'e ekle
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from kok_core import (
    crc8, crc16_ccitt, crc32_ieee,
    hamming84_encode, hamming84_decode,
    golay24_encode, golay24_decode,
    block_interleave, block_deinterleave,
    KamoyCipher
)
from radio.telemetry.telemetry import TacticalFrame, StreamReceiver
from radio.modulation.modem import FSKModem


class TestKokCore(unittest.TestCase):

    def test_crc_calculations(self):
        msg = b"KOK-1976-ASELSAN-MACUNKOY"
        c8 = crc8(msg)
        c16 = crc16_ccitt(msg)
        c32 = crc32_ieee(msg)

        self.assertIsInstance(c8, int)
        self.assertIsInstance(c16, int)
        self.assertIsInstance(c32, int)

        # Değişiklik algılama testi (Hamming mesafesi)
        msg_tampered = b"KOK-1976-ASELSAN-MACUNKOZ"
        self.assertNotEqual(c8, crc8(msg_tampered))
        self.assertNotEqual(c16, crc16_ccitt(msg_tampered))
        self.assertNotEqual(c32, crc32_ieee(msg_tampered))

    def test_hamming84_correction(self):
        # 0..15 arasındaki her 4-bitlik nibble için test
        for nibble in range(16):
            encoded = hamming84_encode(nibble)

            # Hatasız deşifre
            dec, ok = hamming84_decode(encoded)
            self.assertTrue(ok)
            self.assertEqual(dec, nibble)

            # 1 bitlik her olası hatayı düzeltme testi (8 farklı bit pozisyonu)
            for bit_pos in range(8):
                corrupted = encoded ^ (1 << bit_pos)
                dec_corr, ok_corr = hamming84_decode(corrupted)
                self.assertTrue(ok_corr, f"Bit {bit_pos} duzeltilemedi!")
                self.assertEqual(dec_corr, nibble, f"Bit {bit_pos} yanlis cozuldu!")

    def test_golay24_correction(self):
        # 12-bit taktik kelime
        test_words = [0x000, 0x123, 0x456, 0x789, 0xABC, 0xFFF]
        for word in test_words:
            encoded = golay24_encode(word)

            # Hatasız
            dec, errs, ok = golay24_decode(encoded)
            self.assertTrue(ok)
            self.assertEqual(dec, word)
            self.assertEqual(errs, 0)

            # 1 bit hata düzeltme
            for b1 in [0, 5, 11, 23]:
                c1 = encoded ^ (1 << b1)
                dec1, errs1, ok1 = golay24_decode(c1)
                self.assertTrue(ok1)
                self.assertEqual(dec1, word)
                self.assertEqual(errs1, 1)

            # 2 bit hata düzeltme
            c2 = encoded ^ (1 << 2) ^ (1 << 15)
            dec2, errs2, ok2 = golay24_decode(c2)
            self.assertTrue(ok2)
            self.assertEqual(dec2, word)
            self.assertEqual(errs2, 2)

            # 3 bit hata düzeltme
            c3 = encoded ^ (1 << 1) ^ (1 << 8) ^ (1 << 20)
            dec3, errs3, ok3 = golay24_decode(c3)
            self.assertTrue(ok3)
            self.assertEqual(dec3, word)
            self.assertEqual(errs3, 3)

    def test_block_interleaver(self):
        data = b"0123456789ABCDEF"  # 16 bayt (4x4)
        interleaved = block_interleave(data, 4, 4)
        deinterleaved = block_deinterleave(interleaved, 4, 4)
        self.assertEqual(data, deinterleaved)
        self.assertNotEqual(data, interleaved)

    def test_cipher_reversibility(self):
        key = 0xA1B2C3D4E5F60718
        iv = 0x19760042
        plaintext = b"GIZLI TALIMAT: MACUNKOY 1976 PROTOKOLU AKTIF."

        enc_cipher = KamoyCipher(key, iv)
        ciphertext = enc_cipher.process(plaintext)

        self.assertNotEqual(plaintext, ciphertext)

        dec_cipher = KamoyCipher(key, iv)
        recovered = dec_cipher.process(ciphertext)

        self.assertEqual(plaintext, recovered)

    def test_tactical_frame_serialization(self):
        payload = b"LAT=39.95,LON=32.75,BAT=24.2V"
        frame = TacticalFrame(msg_type=2, sender_id=0x4600, target_id=0x1976,
                              seq_num=42, hop_channel=7, payload=payload)
        serialized = frame.serialize()

        # Doğrulama ve deserialization
        recv_frame, err = TacticalFrame.deserialize(serialized)
        self.assertIsNone(err)
        self.assertIsNotNone(recv_frame)
        self.assertEqual(recv_frame.msg_type, 2)
        self.assertEqual(recv_frame.sender_id, 0x4600)
        self.assertEqual(recv_frame.target_id, 0x1976)
        self.assertEqual(recv_frame.seq_num, 42)
        self.assertEqual(recv_frame.hop_channel, 7)
        self.assertEqual(recv_frame.payload, payload)

    def test_stream_receiver(self):
        receiver = StreamReceiver()
        frame1 = TacticalFrame(msg_type=1, sender_id=1, target_id=2, seq_num=101, hop_channel=3, payload=b"PAKET-1")
        frame2 = TacticalFrame(msg_type=1, sender_id=1, target_id=2, seq_num=102, hop_channel=3, payload=b"PAKET-2")

        stream = b"\x00\xFF\xAA" + frame1.serialize() + b"\x55\xDE\xAD\xBE\xEF" + frame2.serialize() + b"\x00"

        # Bayt parça parça beslendiğinde yakalama testi
        received = []
        for i in range(0, len(stream), 7):
            chunk = stream[i : i + 7]
            pkts = receiver.feed(chunk)
            received.extend(pkts)

        self.assertEqual(len(received), 2)
        self.assertEqual(received[0].payload, b"PAKET-1")
        self.assertEqual(received[1].payload, b"PAKET-2")

    def test_fsk_modem_clean_channel(self):
        modem = FSKModem(sample_rate=48000, baud_rate=2400, f_mark=1200, f_space=2400)
        tx_bits = [1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0]
        iq = modem.modulate(tx_bits)
        rx_bits = modem.demodulate(iq)

        self.assertEqual(len(tx_bits), len(rx_bits))
        self.assertTrue((tx_bits == rx_bits).all())


if __name__ == '__main__':
    unittest.main()
