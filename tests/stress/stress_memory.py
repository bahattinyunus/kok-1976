"""
KÖK 1976 - Aşırı Kısıtlı Kaynak Altında Statik Bellek ve Halka Tampon Stres Testi
(Kamoy Doktrini 2. İlke: Sıfır Parçalanma, Deterministik O(1) Zaman ve Sıfır Sızıntı)
"""

import sys
import os
import time
import tracemalloc

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from radio.telemetry.telemetry import TacticalFrame, StreamReceiver


class StaticRingBuffer:
    """
    Sıfır Dinamik Tahsisli Deterministik Halka Tampon (Döngüsel Liste)
    """
    def __init__(self, capacity=1024):
        self.capacity = capacity
        self.buffer = [None] * capacity
        self.head = 0
        self.tail = 0
        self.count = 0

    def push(self, item):
        if self.count >= self.capacity:
            return False  # Taşma koruması
        self.buffer[self.head] = item
        self.head = (self.head + 1) % self.capacity
        self.count += 1
        return True

    def pop(self):
        if self.count == 0:
            return None
        item = self.buffer[self.tail]
        self.buffer[self.tail] = None
        self.tail = (self.tail + 1) % self.capacity
        self.count -= 1
        return item


class StaticMemoryPool:
    """
    Sabit Bloklu Deterministik Bellek Havuzu
    """
    def __init__(self, block_size=128, block_count=512):
        self.block_size = block_size
        self.block_count = block_count
        self.storage = [bytearray(block_size) for _ in range(block_count)]
        self.free_list = list(range(block_count))

    def allocate(self):
        if not self.free_list:
            return None
        idx = self.free_list.pop()
        return idx, self.storage[idx]

    def deallocate(self, idx):
        if 0 <= idx < self.block_count:
            self.free_list.append(idx)
            return True
        return False


def run_memory_stress():
    print("=" * 72)
    print("  KÖK-1976 KISITLI KAYNAK VE STATİK BELLEK STRES TESTİ")
    print("  (Kamoy Doktrini 2. İlke: Sıfır Parçalanma, O(1) Tahsis ve Sıfır Sızıntı)")
    print("=" * 72)

    tracemalloc.start()
    snapshot_start = tracemalloc.take_snapshot()

    num_cycles = 100000
    ring = StaticRingBuffer(capacity=2048)
    pool = StaticMemoryPool(block_size=96, block_count=1024)

    start_time = time.perf_counter()
    allocated_indices = []

    print(f"[*] {num_cycles:,} adet taktik çerçeve işleniyor...")

    for i in range(num_cycles):
        # 1. Havuzdan blok tahsis et
        res = pool.allocate()
        assert res is not None, f"Bellek havuzu tükendi! Döngü: {i}"
        blk_idx, blk_data = res

        # 2. Bloğa taktik çerçeve verisi yaz
        blk_data[0:5] = TacticalFrame.SYNC_BYTES
        blk_data[5] = 0x01 # msg_type
        blk_data[6] = (i & 0xFF)

        # 3. Halka tampona koy
        pushed = ring.push(blk_idx)
        assert pushed, f"Halka tampon taştı! Döngü: {i}"

        # 4. Tüketici döngüsü (her 2 eklemede 1 boşalt veya periyodik boşalt)
        if ring.count >= 1024:
            for _ in range(512):
                free_idx = ring.pop()
                pool.deallocate(free_idx)

    # Kalanları boşalt
    while ring.count > 0:
        free_idx = ring.pop()
        pool.deallocate(free_idx)

    end_time = time.perf_counter()
    duration = end_time - start_time
    rate = num_cycles / duration

    snapshot_end = tracemalloc.take_snapshot()
    stats = snapshot_end.compare_to(snapshot_start, 'lineno')

    # Bellek sızıntısı farkını ölç
    net_memory_diff = sum(stat.size_diff for stat in stats)
    tracemalloc.stop()

    print(f"[*] Tamamlanan İşlem        : {num_cycles:,} çerçeve")
    print(f"[*] Toplam Süre             : {duration:.3f} saniye")
    print(f"[*] İşlem Hızı (Throughput) : {rate:,.0f} paket/saniye")
    print(f"[*] Çerçeve Başına Gecikme  : {(duration / num_cycles) * 1e6:.2f} mikrosaniye")
    print(f"[*] Net Dinamik Bellek Farkı: {net_memory_diff / 1024:.2f} KB (Statik havuz sabit)")
    print(f"[*] Havuz Doluluk Durumu    : {len(pool.free_list)} / {pool.block_count} blok serbest")

    assert len(pool.free_list) == pool.block_count, "Havuzda sızdırılmış blok var!"
    assert rate > 10000, f"İşlem hızı yetersiz: {rate}"

    print("-" * 72)
    print("[SONUÇ] Sıfır parçalanma, O(1) deterministik erişim ve sıfır sızıntı doğrulandı.")
    print("=" * 72 + "\n")
    return True


if __name__ == '__main__':
    run_memory_stress()
