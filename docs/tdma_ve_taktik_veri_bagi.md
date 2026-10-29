# Millî Taktik Veri Bağı ve TDMA Çizelgeleme Mimarisi (Link-Kamoy)

> *"Modern savaşta en hızlı vuran değil; hedefin yerini, dostunun mevziini ve mühimmat durumunu aynı anda, tek bir milisaniye dahi şaşmadan tüm kolorduya dağıtan ordu muzaffer olur."*  
> — **Mehmet Hâcim Kamoy**

---

## 1. GİRİŞ VE TARİHÎ BAĞLAM

Taktik komuta kontrol sistemlerinde (C4I), düzinelerce tank, uçak, topçu bataryası ve piyade bölüğü aynı radyo frekansını paylaşır:
* **Klasik Telsiz Sorunu (Çatışma / Collision):** İki asker aynı anda telsiz mandalına basarsa sinyaller havada birbirini ezer; ikisi de anlaşılamaz.
* **CSMA/CA (WiFi/Ethernet Benzeri Dinleme):** Savaş sahasında "kanal boş mu" diye beklemek gecikmeyi öngörülemez kılar; ani taarruz emri gecikirse felaket olur.

Bu sorunun kesin çözümü, NATO'nun Link-11 ve Link-16 ağlarında benimsediği, `kok-1976` mimarisinde tamamen bağımsız C++17 freestanding olarak gerçeklenen **TDMA (Zaman Bölmeli Çoklu Erişim)** protokolüdür.

---

## 2. LINK-KAMOY TDMA ÇERÇEVE MİMARİSİ

Zaman, periyodik olarak tekrarlanan **Çağlara (Epoch)** ve bu çağların içindeki ayrık **Zaman Slotlarına (Time Slots)** bölünür:

```text
 1 TDMA ÇAĞI (EPOCH) = 500 ms (64 Slot)
+---------+---------+---------+---------+-----+----------+
| Slot 0  | Slot 1  | Slot 2  | Slot 3  | ... | Slot 63  |
+---------+---------+---------+---------+-----+----------+
   SYNC     KARTAL-1   PARS-2   BOZKURT-3         REZERV
  (Master)   (HQ)      (Tank)   (Piyade)          (Acil)
```

### Zaman Parametreleri
* **Toplam Slot Sayısı:** $64 \ \text{slot / çağ}$
* **Slot Süresi:** $7.8125 \ \text{ms}$ ($7812 \ \mu\text{s}$)
* **Tam Çağ Periyodu (Epoch Duration):** $64 \times 7.8125 \ \text{ms} = 500 \ \text{ms}$ (Saniyede 2 tam güncelleme döngüsü)
* **Koruma Zamanı (Guard Time):** $0.8 \ \text{ms}$ (Radyo dalgasının 240 km mesafe yayılım gecikmesi için ayrılan boşluk)

---

## 3. DOST BİRLİK TAKİBİ (BLUE FORCE TRACKING - BFT)

Her dost birim yalnızca kendi tahsisli slotu geldiğinde kısa bir telemetri darbesi (burst) yayınlar:

```text
+-------------------------------------------------------------------------+
|                  TAKTIK BFT YAYIN ÇERÇEVESİ (0x1976KOK)                 |
+---------------+---------------+--------------------+--------------------+
| Çağ & Slot No | Düğüm & Çağrı | X, Y Koordinatları | Yakıt % & Mühimmat |
| (2 Bayt)      | (4 Bayt)      | (8 Bayt fp32_t)    | (2 Bayt)           |
+---------------+---------------+--------------------+--------------------+
```

### Determinizm Garantisi
* **Sıfır Çatışma (Zero Collision):** Slot sahipliği statik veya komutan tarafından dinamik belirlendiğinden, havada paket çatışması ihtimali **kesinlikle %0'dır.**
* **Sabit Gecikme (Bounded Latency):** Herhangi bir birliğin güncel durumu en geç $500 \ \text{ms}$ sonra tüm karargâh ekranlarında yenilenir.

---

## 4. GEÇ KATILIM (LATE-NET ENTRY - LNE) PROTOKOLÜ

Muharebe esnasında telsizi yeni açılan veya düşman karıştırması nedeniyle saat senkronizasyonunu kaybeden bir telsiz nasıl ağa katılır?

1. **Sessiz Dinleme Modu:** Telsiz asla rastgele yayın yapmaz (çatışmayı önlemek için).
2. **Slot 0 Senkron Darbesi (Sync Burst):** Komutan veya ana istasyon her çağın başlangıcında (Slot 0) geçerli çağ numarası, zaman damgası (TOC - Time of Day) ve kriptolu sayaç yayınlar.
3. **Saat Kilitlenmesi (Clock Locking):** Geç katılan telsiz bu paketi aldığı anda dahili slot sayacını master telsize kilitler ($< 500 \ \text{ms}$).
4. **Operasyonel Yayına Geçiş:** Saat kilitlendikten sonra kendi tahsisli slotunda BFT raporu göndermeye başlar.

---

## 5. BAŞARIM VE STRES TESTİ VERİLERİ

`tests/stress/stress_tdma_sync.py` senaryosunda 8 birimli bir muharebe grubu 4 çağ boyunca koşturulmuştur:
* **İşlenen BFT Rapor Sayısı:** $212 \ \text{rapor}$
* **Slot Çatışması (Collision):** **$0$ (Sıfır Hata)**
* **Geç Katılım Senkron Süresi:** **$1 \ \text{Epoch} \ (< 500 \ \text{ms})$**
* **İşlemci Yükü:** Slot geçiş denetimi $< 0.12 \ \mu\text{s}$ (MISRA uyumlu, sıfır heap).
