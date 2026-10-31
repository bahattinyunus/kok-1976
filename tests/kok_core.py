"""
KÖK 1976 - Python Referans Çekirdeği (C++ Modülleri ile Birebir Bit Uyumlu)
"""

# CRC Fonksiyonları
def crc8(data: bytes, init: int = 0x00) -> int:
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
    crc = init
    for b in data:
        crc ^= (b << 8) & 0xFFFF
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def crc32_ieee(data: bytes, init: int = 0xFFFFFFFF) -> int:
    crc = init
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xEDB88320
            else:
                crc >>= 1
    return (~crc) & 0xFFFFFFFF


# Genişletilmiş Hamming(8,4) SECDED
def hamming84_encode(nibble: int) -> int:
    d0 = (nibble >> 0) & 1
    d1 = (nibble >> 1) & 1
    d2 = (nibble >> 2) & 1
    d3 = (nibble >> 3) & 1

    p0 = d0 ^ d1 ^ d3
    p1 = d0 ^ d2 ^ d3
    p2 = d1 ^ d2 ^ d3

    c7 = (d3 << 6) | (d2 << 5) | (d1 << 4) | (p2 << 3) | (d0 << 2) | (p1 << 1) | p0
    ones = bin(c7).count("1")
    p3 = ones & 1
    return c7 | (p3 << 7)


def hamming84_decode(code: int):
    p0 = (code >> 0) & 1
    p1 = (code >> 1) & 1
    d0 = (code >> 2) & 1
    p2 = (code >> 3) & 1
    d1 = (code >> 4) & 1
    d2 = (code >> 5) & 1
    d3 = (code >> 6) & 1
    p3 = (code >> 7) & 1

    s0 = p0 ^ d0 ^ d1 ^ d3
    s1 = p1 ^ d0 ^ d2 ^ d3
    s2 = p2 ^ d1 ^ d2 ^ d3
    syndrome = (s2 << 2) | (s1 << 1) | s0

    overall_parity_err = (bin(code).count("1") & 1) != 0

    if syndrome == 0:
        return (d3 << 3) | (d2 << 2) | (d1 << 1) | d0, True

    if overall_parity_err:
        # Tek bit hatası düzeltilebilir
        corrected = code ^ (1 << (syndrome - 1))
        cd0 = (corrected >> 2) & 1
        cd1 = (corrected >> 4) & 1
        cd2 = (corrected >> 5) & 1
        cd3 = (corrected >> 6) & 1
        return (cd3 << 3) | (cd2 << 2) | (cd1 << 1) | cd0, True

    # Çift bit hatası tespit edildi, düzeltilemez
    return (d3 << 3) | (d2 << 2) | (d1 << 1) | d0, False


# Golay(24,12) Generator Matrisi (G_POLY = 0xC75)
GOLAY_PARITY = [
    0x8EB, 0x93E, 0xA97, 0xDC6, 0x367, 0x6CD,
    0xD99, 0x3DA, 0x7B4, 0xF68, 0x63B, 0xC75
]

def golay24_encode(data12: int) -> int:
    data12 &= 0x0FFF
    parity12 = 0
    for i in range(12):
        if (data12 >> i) & 1:
            parity12 ^= GOLAY_PARITY[i]
    return ((data12 << 12) | (parity12 & 0x0FFF)) & 0xFFFFFF


def golay24_decode(code24: int):
    code24 &= 0xFFFFFF
    recv_data = (code24 >> 12) & 0x0FFF
    recv_parity = code24 & 0x0FFF

    calc_parity = 0
    for i in range(12):
        if (recv_data >> i) & 1:
            calc_parity ^= GOLAY_PARITY[i]

    s = recv_parity ^ calc_parity
    if s == 0:
        return recv_data, 0, True

    # 1. Durum: 0 veri hatası, <= 3 eşlik hatası
    if bin(s).count("1") <= 3:
        return recv_data, bin(s).count("1"), True

    # 2. Durum: 1 veri hatası, <= 2 eşlik hatası
    for i in range(12):
        diff = s ^ GOLAY_PARITY[i]
        if bin(diff).count("1") <= 2:
            return recv_data ^ (1 << i), 1 + bin(diff).count("1"), True

    # 3. Durum: 2 veri hatası, <= 1 eşlik hatası
    for i in range(12):
        for j in range(i + 1, 12):
            diff = s ^ GOLAY_PARITY[i] ^ GOLAY_PARITY[j]
            if bin(diff).count("1") <= 1:
                return recv_data ^ (1 << i) ^ (1 << j), 2 + bin(diff).count("1"), True

    # 4. Durum: 3 veri hatası, 0 eşlik hatası
    for i in range(12):
        for j in range(i + 1, 12):
            for k in range(j + 1, 12):
                if s == (GOLAY_PARITY[i] ^ GOLAY_PARITY[j] ^ GOLAY_PARITY[k]):
                    return recv_data ^ (1 << i) ^ (1 << j) ^ (1 << k), 3, True

    return recv_data, 4, False


def block_interleave(data: bytes, rows: int, cols: int) -> bytes:
    assert len(data) == rows * cols
    out = bytearray(len(data))
    for r in range(rows):
        for c in range(cols):
            out[c * rows + r] = data[r * cols + c]
    return bytes(out)


def block_deinterleave(data: bytes, rows: int, cols: int) -> bytes:
    assert len(data) == rows * cols
    out = bytearray(len(data))
    for c in range(cols):
        for r in range(rows):
            out[r * cols + c] = data[c * rows + r]
    return bytes(out)


class KamoyCipher:
    DEFAULT_SBOX = [
        0x0C, 0x05, 0x06, 0x0B, 0x09, 0x00, 0x0A, 0x0D,
        0x03, 0x0E, 0x0F, 0x08, 0x04, 0x07, 0x01, 0x02
    ]

    def __init__(self, key: int = 0x1976197619761976, iv: int = 0x4D414355):
        key_high = (key >> 32) & 0xFFFFFFFF
        key_low = key & 0xFFFFFFFF
        self.lfsr1 = (key_high ^ iv ^ 0x5A5A5A5A) & 0xFFFFFFFF
        self.lfsr2 = (key_low ^ (~iv) ^ 0xA5A5A5A5) & 0xFFFFFFFF
        if self.lfsr1 == 0: self.lfsr1 = 0x19760001
        if self.lfsr2 == 0: self.lfsr2 = 0x20260001
        self.counter = 0
        self.sbox = list(self.DEFAULT_SBOX)
        for i in range(16):
            shift = (key_low >> (i * 2)) & 0x0F
            self.sbox[i] = self.DEFAULT_SBOX[(i ^ shift) & 0x0F]
        for _ in range(32):
            self.next_byte()

    def step(self):
        lsb1 = self.lfsr1 & 1
        self.lfsr1 >>= 1
        if lsb1:
            self.lfsr1 ^= 0x48000000

        lsb2 = self.lfsr2 & 1
        self.lfsr2 >>= 1
        if lsb2:
            self.lfsr2 ^= 0x80200003

        self.counter += 1

    def next_byte(self) -> int:
        self.step()
        n1 = ((self.lfsr1 ^ (self.lfsr1 >> 8) ^ (self.lfsr1 >> 16)) & 0x0F)
        n2 = ((self.lfsr2 ^ (self.lfsr2 >> 7) ^ (self.lfsr2 >> 19)) & 0x0F)
        s1 = self.sbox[n1]
        s2 = self.sbox[n2]
        cnt = (self.counter ^ (self.counter >> 8)) & 0xFF
        return ((s1 << 4) | s2) ^ cnt

    def process(self, data: bytes) -> bytes:
        out = bytearray(data)
        for i in range(len(out)):
            out[i] ^= self.next_byte()
        return bytes(out)

    @staticmethod
    def calculate_mac(key: int, data: bytes) -> int:
        h = 0x811C9DC5
        k0 = key & 0xFFFFFFFF
        k1 = (key >> 32) & 0xFFFFFFFF

        h = ((h ^ k0) * 0x01000193) & 0xFFFFFFFF
        for b in data:
            h = ((h ^ b) * 0x01000193) & 0xFFFFFFFF
            h = (((h << 13) | (h >> 19)) & 0xFFFFFFFF)

        h = ((h ^ k1) * 0x01000193) & 0xFFFFFFFF
        return h
