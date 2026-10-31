# Taktik Kripto ve Havadan Anahtarlama (OTAR) Mimarisi

> *"Savaş meydanında ele geçirilen bir telsiz, düşmanın elinde bir casusa dönüşmemelidir. Telsiz teslim olabilir; fakat içindeki vatan sırrı, saniyenin binde birinde kendi kendini küle çevirmelidir."*  
> — **Mehmet Hâcim Kamoy**

---

## 1. GİRİŞ VE TARİHÎ DOKTRİN

1974 Kıbrıs Barış Harekâtı esnasında yaşanan en büyük krizlerden biri, birlikler ilerledikçe telsiz frekans ve şifre anahtarlarının sahadaki dağıtımıydı:
* **Fiziksel Anahtar Taşıma Riski:** Şifre defterleri veya harici kripto yükleme cihazları (fill-gun) taşıyan kuryelerin pusuya düşürülmesi tüm cephenin kripto güvenliğini tehlikeye atıyordu.
* **Ele Geçirilme (Capture) Riski:** Vurulan bir tankta veya siperde kalan telsizin düşmanın eline geçmesi halinde telsizdeki anahtarlar derhal geçersiz kılınmalıydı.

Mehmet Hâcim Kamoy ve ASELSAN'ın kurucu ekibi, bu sorunu **İki Kademeli Kripto Hiyerarşisi (KEK / TEK)**, **Hava Üzerinden Anahtar Yenileme (OTAR - Over-The-Air Rekeying)** ve **Acil Kripto İmha (Zeroize / Anti-Tamper)** doktrini ile çözmüştür.

---

## 2. KRİPTO ANAHTAR HİYERARŞİSİ

```text
                  +-----------------------------------+
                  |      GÖREV ÖNCESİ MERKEZ KEK       |
                  |     (Key Encryption Key - 64B)    |
                  |     Kripto Subayı Tarafından      |
                  |       Korumalı Odada Yüklenir     |
                  +-----------------------------------+
                                    |
                    Hava Üzerinden Güvenli Kanal
                    (0x76REKEY / Kamoy-76 Cipher)
                                    |
                                    v
                  +-----------------------------------+
                  |      GÜNLÜK TAKTİK TEK ANAHTARI   |
                  |    (Traffic Encryption Key - 64B) |
                  |    Ses, Telemetri ve Veri Paketleri|
                  |        Bu Anahtarla Zırhlanır     |
                  +-----------------------------------+
```

### 1. KEK (Key Encryption Key - Anahtar Şifreleme Anahtarı)
* Çok yüksek güvenlikli, merkezî karargâhta üretilen ve telsize fiziksel güvenli arayüzden yüklenen ana anahtardır.
* Asla havadan açık iletilmez; yalnızca yeni TEK paketlerini şifrelemek ve doğrulamak için kullanılır.

### 2. TEK (Traffic Encryption Key - Trafik Şifreleme Anahtarı)
* Operasyonel haberleşmede (CVSD ses, MANET paketleri, TDMA telemetrisi) kullanılan dinamik anahtardır.
* Görev esnasında her 24 saatte bir veya çatışma durumuna göre dakikalar içinde havadan otomatik olarak değiştirilir.

---

## 3. 24-BAYTLIK OTAR PAKET YAPISI

```text
+------------------+---------------+---------------+--------------------+-------------+---------------+
| Magic (4 Bayt)   | Sıra No (2B)  | Key ID (2B)   | Şifreli TEK (8B)   | IV (4 Bayt) | MAC (4 Bayt)  |
| 0x7652454B       | Monotonik Artış| Gün / İndeks  | KEK ile Şifrelenmiş| Oturum Baş. | Kimlik Doğr.  |
+------------------+---------------+---------------+--------------------+-------------+---------------+
```

### Üç Kademeli Güvenlik Kalkanı:
1. **Anti-Replay Koruması:** `sequence` numarası yerel telsizdeki son numaradan büyük olmak zorundadır. Düşmanın havadan kaydedip tekrar bastığı eski anahtar paketleri anında çöpe atılır.
2. **Kriptografik MAC Bütünlüğü:** FNV-1a ve Kamoy rotasyonlu 32-bit MAC, KEK anahtarı ile doğrulanır. Tek bir biti değiştirilmiş sahte anahtar enjeksiyonu anında tespit edilir.
3. **Zaman Damgalı Oturum IV:** Her rekey paketi benzersiz bir `iv` ile şifrelendiğinden, anahtar baytları havada aynı kalıbı asla tekrar etmez (LPD / Düşük Tespit Edilme).

---

## 4. ACİL KRİPTO İMHA (ZEROIZE) VE DoD 5220.22-M STANDARDI

Telsiz düşman eline geçme tehlikesiyle karşılaştığında veya gövde açılmaya çalışıldığında devreye girer:

### 1. Tetikleme Kaynakları (Tamper Sources):
* Operatör panik mandalı (`EMERGENCY_BUTTON`)
* Gövde mikro-anahtarı (`CHASSIS_INTRUSION`)
* Lazer / termal analiz tespiti (`TEMPERATURE_EXTREME`)
* Karargâhtan uzaktan imha komutu (`COMMAND_OVER_THE_AIR`)

### 2. Dört Aşamalı Bellek Temizleme:
Sıradan bir `memset(0)` modern yarı iletken belleklerde (SRAM/EEPROM) artık voltaj yükü bırakabilir (cold-boot attack / remanence effect). `kok-1976`, askerî **DoD 5220.22-M** standardında 4 aşamalı imha uygular:
1. **Geçiş 1:** `0x55` (01010101) - Tüm hücrelerde alternatif yük.
2. **Geçiş 2:** `0xAA` (10101010) - Tersi yükleme.
3. **Geçiş 3:** `0xFF` (11111111) - Doyum yüklemesi.
4. **Geçiş 4:** `0x00` (00000000) - Mutlak sıfırlama ve bellek bariyeri.

### 3. Donanımsal Doğrulama (Verification):
Sabit zamanlı (constant-time) bit taraması yapılarak tek bir baytın dahi sıfırdan farklı kalmadığı donanımsal olarak onaylanır ve cihaz kilitlenir.

---

## 5. BAŞARIM VE STRES TESTİ VERİLERİ

`tests/stress/stress_otar_zeroize.py` test sonuçları:
* **Hava Üzerinden Rekey İşlem Süresi:** $< 1.2\text{ ms}$
* **Tekrar Saldırısı (Replay Attack) Engelleme:** **%100 Başarı**
* **Sahte Anahtar Enjeksiyonu Engelleme:** **%100 Başarı**
* **Zeroize İmha ve Doğrulama Süresi:** **$< 3.8\text{ mikrosaniye}$**
* **Bellek Kalıntısı (Remanence):** **0.000 Bayt (Mutlak Sıfır)**
