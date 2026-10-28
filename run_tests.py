#!/usr/bin/env python3
"""
KÖK 1976 - Ana Test ve Stres Koşturucu (Master Test Runner)
Mehmet Hâcim Kamoy Mühendislik Doktrini Doğrulama Bataryası
"""

import sys
import os
import subprocess
import time

BANNER = r"""
                       _____________________________
                      |                             |
                      |          KÖK 1976           |
                      |   Savunma & AR-GE Doktrini  |
                      |_____________________________|
                                     |
         +---------------------------+---------------------------+
         |                           |                           |
         v                           v                           v
   1964: KIBRIS                1974: AMBARGO               1976: MACUNKÖY
  Gizli Hat & Dalga          Milli İrade & Vakıf        Kamoy Vizyonu & AR-GE
 (Teorik Sınırları Yıkmak)    (Halkın Alyansları)         (Mühendislik Okulu)
"""

TEST_SUITES = [
    ("Birim Testler (CRC, FEC, Ciphers, Framing)", ["python", "-s", "tests/test_core.py"]),
    ("Düşük SNR & AWGN Gürültü Stresi (Golay vs Uncoded)", ["python", "-s", "tests/stress/stress_noise.py"]),
    ("Elektronik Harp & Karıştırma Stresi (FHSS vs Jammer)", ["python", "-s", "tests/stress/stress_jamming.py"]),
    ("İleri Düzey EH, DSSS & Anti-Spoofing Savunması", ["python", "-s", "tests/stress/stress_ew_complex.py"]),
    ("Kısıtlı Kaynak & Statik Bellek Stresi (O(1) Tahsis)", ["python", "-s", "tests/stress/stress_memory.py"]),
    ("1964 Kıbrıs Troposferik Ufuk Ötesi Yayılımı", ["python", "-s", "tests/stress/stress_cyprus_1964.py"]),
]


def main():
    # Windows konsolunda UTF-8 çıktıyı garantiye al
    if sys.platform == "win32":
        try:
            sys.stdout.reconfigure(encoding="utf-8")
        except Exception:
            pass

    print(BANNER)
    print("=" * 76)
    print("  KÖK 1976 DOKTRİN DOĞRULAMA VE MİLLÎ MÜHENDİSLİK TESTLERİ BAŞLATIYOR")
    print("=" * 76)

    overall_start = time.perf_counter()
    results = []

    for name, cmd in TEST_SUITES:
        print(f"\n[+] KOŞULUYOR: {name}...")
        t0 = time.perf_counter()
        proc = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
        dt = time.perf_counter() - t0

        if proc.stdout:
            print(proc.stdout.strip())
        if proc.returncode != 0:
            print(f"[HATA ÇIKTISI]:\n{proc.stderr.strip()}")
            results.append((name, "BAŞARISIZ", dt))
        else:
            results.append((name, "BAŞARILI", dt))

    total_time = time.perf_counter() - overall_start

    print("\n" + "=" * 76)
    print(f"{'TEST BATARYASI ÖZET TABLOSU':^76}")
    print("=" * 76)
    print(f"{'Modül / Test Adı':<55} | {'Durum':<10} | {'Süre':<6}")
    print("-" * 76)

    all_ok = True
    for name, status, dt in results:
        status_str = f"\033[92m{status}\033[0m" if status == "BAŞARILI" else f"\033[91m{status}\033[0m"
        print(f"{name:<55} | {status:<10} | {dt:5.2f}s")
        if status != "BAŞARILI":
            all_ok = False

    print("-" * 76)
    print(f"Toplam Test Süresi: {total_time:.2f} saniye")

    if all_ok:
        print("\n[TÜM TESTLER BAŞARIYLA TAMAMLANDI]")
        print("\"Bilselerdi uçmaya kalkmazlardı; bilmedikleri için uçtular...\"")
        print("=" * 76 + "\n")
        return 0
    else:
        print("\n[BİR VEYA DAHA FAZLA TEST BAŞARISIZ OLDU]")
        print("=" * 76 + "\n")
        return 1


if __name__ == '__main__':
    sys.exit(main())
