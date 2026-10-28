# Millî Elektronik Harp (EH) ve Taktik Muhabere Doktrini

> *"Cephede top ve tüfeğin namlusu hedefe dönmeden önce, elektromanyetik spektrumda görünmez bir savaş verilir. Frekansını kaybeden komutan, birliğini yönetemez; birliğini yönetemeyen ordu, savaşı kaybeder."*  
> — **KÖK-1976 Harp Notları**

---

## 1. ELEKTRONİK HARP KAVRAMI VE 3 TEMEL DİSİPLİN

Elektronik Harp (EH / Electronic Warfare), elektromanyetik spektrumun (telsiz, radar, lazer, elektro-optik) kontrolünü ele geçirmek, dost kuvvetlerin bu spektrumu güvenle kullanmasını temin etmek ve düşmanın spektral erişimini engellemek üzere yürütülen tüm harekâttır.

NATO ve TSK doktrininde EH üç ana sütuna ayrılır:

```
                                  ELEKTRONİK HARP (EH)
                                           |
         +---------------------------------+---------------------------------+
         |                                 |                                 |
         v                                 v                                 v
ELEKTRONİK DESTEK (ED / ESM)    ELEKTRONİK TAARRUZ (ET / ECM)    ELEKTRONİK KORUNMA (EK / ECCM)
Düşman frekansını dinleme,      Düşman telsiz ve radarlarını     Dost telsizleri karıştırmadan
yön kestirme (DF) ve teşhis     gürültü basarak felç etme        kaçırma (FHSS, DSSS, LPI/LPD)
```

---

## 2. DÜŞMAN KARIŞTIRMA MODELLERİ VE MATEMATİĞİ

Taktik sahada telsiz muhaberesine karşı uygulanan başlıca karıştırma (Jamming) teknikleri şunlardır:

### A. Baraj Karıştırması (Barrage Jamming)
Düşman vericisinin gücünü geniş bir frekans bandına yayarak o bandı bütünüyle beyaz gürültü denizine boğmasıdır.
- **Avantajı:** Telsizin tam olarak hangi kanalda olduğunu bilmeye gerek duymaz.
- **Zafiyeti:** Güç geniş spektruma dağıldığı için kanal başına düşen karıştırma gücü ($J_0$) düşüktür:

$$J_0 = \frac{P_j}{B_j \cdot L_j}$$

Burada $P_j$ karıştırıcı gücü, $B_j$ karıştırma bant genişliğidir.

### B. Nokta Karıştırması (Spot Jamming)
Düşman bütün enerjisini tek bir taktik frekansa odaklar.
- **Tehlikesi:** Çok yüksek Karıştırma-Sinyal Oranı ($J/S \gg 30\text{ dB}$) oluşturur; sabit frekanslı klasik telsizleri tamamen susturur.

### C. Frekans Taramalı Karıştırıcı (Sweep / Chirp Jamming)
Hızlı bir frekans sentezörü ile tüm taktik bandı periyodik olarak tarayan yüksek güçlü sinüs veya gürültü darbesidir.

### D. Reaktif Takipçi Karıştırıcı (Responsive / Follower Jammer)
Telsizin yayına başladığı kanalı milisaniyeler mertebesinde spektrum analizörüyle tespit eder ve anında o kanala karşı yayın başlatır.
- **KÖK-1976 Çözümü:** Bu karıştırıcıyı etkisiz kılmanın tek yolu, düşmanın algılama ve kilitlenme süresinden daha kısa sürede frekans değiştiren (**Dwell Time** kontrolü) hızlı atlamalı **FHSS** algoritmasıdır.

---

## 3. ELEKTRONİK KORUNMA TEKNOLOJİLERİ (ECCM)

### A. Frekans Atlama (FHSS - Frequency Hopping)
KÖK-1976 mimarisinde sinyal tek bir frekansta sabit kalmaz. Kamoy-76 kriptografik üreteci tarafından üretilen deterministik sözde-rastgele diziye göre saniyede onlarca kez frekans değiştirilir.

* **İşleme Kazancı (Processing Gain):**

$$G_p = 10 \log_{10} \left( \frac{B_{toplam}}{B_{kanal}} \right) = 10 \log_{10}(N_{kanal}) \quad [\text{dB}]$$

64 kanallı KÖK-1976 sisteminde:

$$G_p = 10 \log_{10}(64) \approx 18.06\text{ dB}$$

Bu kazanç, düşman karıştırıcısının sinyali bastırabilmesi için telsizin yaydığı güçten **en az 64 kat daha fazla güç** harcamasını zorunlu kılar.

### B. Karıştırma Marjı (Jamming Margin)
Bir taktik alıcının karıştırma ortamında çalışabilirlik eşiği:

$$M_j = G_p - \left( \frac{S}{N} \right)_{out} - L_{sys} \quad [\text{dB}]$$

Burada $(S/N)_{out}$ demodülasyon için gereken asgari SNR, $L_{sys}$ donanım kayıplarıdır. $M_j > 0$ olduğu sürece telsiz mesajı kayıpsız çözer.

### C. Doğrudan Dizili Yayılı Spektrum (DSSS)
Sinyalin her biti, çok daha yüksek hızlı bir çip dizisi (Chip Sequence / Pseudo-Noise Code) ile modüle edilir. Böylece sinyal güç yoğunluğu termal gürültü seviyesinin altına ($SNR < 0\text{ dB}$) indirilerek düşman dinleme cihazlarından saklanır (**LPI/LPD - Düşük Yakalanma Olasılığı**).

---

## 4. İLERİ HATA DÜZELTME (FEC) VE SERPİŞTİRME SİNERJİSİ

Elektronik harp ortamında karıştırma sinyalleri genellikle paket üzerinde peş peşe gelen bitleri tahrip eden **ardışık (burst) hatalara** yol açar.

KÖK-1976 şu çift katmanlı savunma kalkanını kurar:
1. **Matris Serpiştirme (Block Interleaver):** Ardışık gelen 12x8 bitlik matrisi satır-sütun yer değiştirmesiyle dağıtır. Böylece düşman jammer'ının tahrip ettiği 5 ardışık bit, de-interleave yapıldığında 5 farklı Golay kod sözcüğüne tek birer bit olarak dağılır.
2. **Genişletilmiş Golay(24,12) Süzgeci:** Her kod sözcüğünde tek bitlik hataları zahmetsizce onarır ve paket kurtarılır.

```
       DÜŞMAN KARISTIRMASI (5 Bitlik Ardışık Hasar)
                 [X][X][X][X][X]
                        |
                 [De-Interleaver]
                        |
      +--------+--------+--------+--------+--------+
      |        |        |        |        |        |
    Sözcük 1 Sözcük 2 Sözcük 3 Sözcük 4 Sözcük 5 Sözcük 6
     [X]..    .[X].    ..[X]    ...[X]   ....[X]  ......
    (1 Hata) (1 Hata) (1 Hata) (1 Hata) (1 Hata) (0 Hata)
        |        |        |        |        |        |
        v        v        v        v        v        v
  [Golay Onarımı: %100 BAŞARILI, SIFIR VERİ KAYBI!]
```

---

## 5. DOKTRİNİN TEMEL İLKELERİ

1. **Hiçbir Frekansa Âşık Olma:** Sürekli frekans değiştir, tahmin edilebilir kalıplardan kaçın.
2. **Spektral Sessizlik (Radio Silence):** İhtiyaç yoksa yayın yapma. Yayına başladığında en kısa sürede (burst transmission) mesajı bitir ve sessizliğe dön.
3. **Kendi Kripton, Kendi Frekansın:** Dış kaynaklı kripto anahtarları harp anında düşmanın eline geçer; anahtar akışı yerli donanım içinde üretilmelidir.
4. **Alıcı Duyarlılığı Verici Gücünden Üstündür:** Yüksek güçle basmak düşmana koordinat vermektir; düşük güçle konuşup gelişmiş algoritmayla duymak esastır.
