# Taktik Ses Kodlama (Vocoder) ve 150 Hz Susturma Mimarisi

> *"Muharebe meydanında bir askerin telsiz mandalına bastığında duyduğu ses, sadece bir ses dalgası değil; komutanın emri, siperin koordinatı ve vatan savunmasının nefesidir."*  
> — **ASELSAN 1980 Taktik Ses Mühendisliği Notları**

---

## 1. GİRİŞ: SAVAŞ ALANINDA SESİN ÖNEMİ

Askerî telsiz muhaberesinde ses iletimi, sivil telekomünikasyondan tamamen farklı gereksinimlere tabidir:
* Sivil şebekelerde paket düştüğünde yeniden iletim istenir veya ses kesilir (drop call).
* Muharebe alanında ise **gecikme (latency) sıfıra yakın olmalı**, mermi ve patlama sesleri arasında telsiz mandalına basıldığı an ses karşı mevzide duyulmalıdır.
* En önemlisi: Şiddetli düşman karıştırması veya zayıf sinyal nedeniyle **%5 ila %10 bit hatası olsa dahi**, ses tamamen kesilmemeli; askerî emir anlaşılabilir kalmalıdır.

Bu doküman, ASELSAN 4600/9600 serisi telsizlerde ve NATO standartlarında benimsenen **16 kbps CVSD ses kodlayıcısının** ve **150 Hz Tonlu Susturma (Tone Squelch)** mimarisinin matematiksel ve elektronik temellerini açıklamaktadır.

---

## 2. CVSD (CONTINUOUSLY VARIABLE SLOPE DELTA MODULATION) MİMARİSİ

### Neden Klasik PCM veya MP3 Değil?
Geleneksel 16-bit PCM ses kodlamasında, en yüksek ağırlıklı bit (MSB) bozulursa kulakları sağır eden devasa bir patlama sesi oluşur. MP3, AAC gibi karmaşık sıkıştırma algoritmaları ise tek bir bayt hatasında tüm çerçeveyi (frame) kaybeder.

**CVSD (Sürekli Değişken Eğimli Delta Modülasyonu)**, her ses örneğini **yalnızca 1 bit** ile temsil eder:
- Eğer analog ses sinyali mevcut tahmin değerinden yüksekse: `1`
- Eğer analog ses sinyali mevcut tahmin değerinden alçaksa: `0`

```
               GİRİŞ SESİ (PCM)
                     |
                     v
             +-------+-------+
             | Karşılaştırıcı| <-------+
             +-------+-------+         |
                     |                 |
                Çıkış Biti (0/1)       |
                     |                 |
        +------------+------------+    |
        |                         |    |
        v                         v    |
   [Hece Süzgeci]          [İntegratör]+
   (Eğim Aşımı Denetimi)   (Tahmini Sinyal)
        |                         ^
        +---- Adım Büyüklüğü (Delta)
```

### Hece Süzgeci ve Eğim Aşımı (Slope Overload) Koruması
Klasik delta modülasyonu hızlı yükselen ses dalgalarını takip edemez (eğim aşımı). CVSD bu sorunu kaydırma kaydedicisi ile çözer:
- Peş peşe gelen son 3 veya 4 bit aynıysa (`111` veya `000`), sinyalin çok hızlı yükseldiği anlaşılır ve adım büyüklüğü ($\Delta$) katlanarak artırılır.
- Sinyal durağanlaştığında (`1010`), adım büyüklüğü yavaşça asgari seviyeye ($\Delta_{min}$) sönümlendirilir.

### Bit Hatalarına Karşı Doğal Zırh
CVSD akışında tek bir bitin düşman gürültüsüyle `1` yerine `0` olması, seste yalnızca mikrosaniyelik hafif bir pıtırtı üretir; sesin melodisi ve sözcüklerin tınısı bozulmaz. Bu sebeple **STANAG 4209** ve **MIL-STD-188-113** taktik ses standardı olarak kabul edilmiştir.

---

## 3. 150 HZ ASKERÎ TON SUSTURMA (TONE SQUELCH / CTCSS)

### Muharebe Alanında "Gürültü Patlaması" Tehlikesi
Frekans Modülasyonlu (FM) telsizlerde alıcıda sinyal yokken hoparlörden yüksek güçlü beyaz gürültü (hışırtı) fışkırır. Kulaklıkla nöbet tutan askerin dikkatini dağıtmamak ve düşmanın gürültü basarak askerin kulağını sağır etmesini önlemek için susturma devresi (squelch) şarttır.

### ASELSAN 4600'ün 150 Hz Pilot Çözümü
ASELSAN 4600 telsizi konuşma sesinin altına insanın duyamayacağı kadar pes bir **150 Hz sub-audible pilot ton** ekler.
Alıcı, hoparlörü açmak için iki şart arar:
1. **Gürültü Sessizleşmesi (Noise Quieting):** Yüksek frekanslı (>6 kHz) FM gürültüsü bastırılmış olmalıdır.
2. **150 Hz Ton Tespiti:** Sesin içinde 150 Hz ton bulunmalıdır.

Düşman frekansa gürültü bastığında 150 Hz ton bulunmadığı için telsiz hoparlörü anında kapatır; asker gürültüye maruz kalmaz.

### Goertzel Algoritması ile Sayısal Ton Tespiti
KÖK-1976 mimarisinde 150 Hz tonun varlığı, tüm FFT spektrumunu hesaplamaya gerek bırakmayan son derece verimli **Goertzel Algoritması** ile doğrulanır:

$$s[n] = x[n] + 2\cos(\omega_0) \cdot s[n-1] - s[n-2]$$

$$P = s[N-1]^2 + s[N-2]^2 - 2\cos(\omega_0) \cdot s[N-1] \cdot s[N-2]$$

Burada $\omega_0 = 2\pi \cdot 150 / F_s$ olup, Q14 formatında sabit noktalı matematik ile mikrodenetleyicilerde mikrosaniyeler içinde işletilir.
