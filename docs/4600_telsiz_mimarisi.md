# ASELSAN 4600 Serisi Taktik Telsiz Mimarisi ve Tasarım Felsefesi

> *"Bir cihazın askerî kıymeti, en son teknolojiye sahip olmasından değil; çamurun, yağmurun ve düşman karıştırmasının içinde bile çalışmayı sürdürmesinden gelir."*  
> — **ASELSAN 1979 Saha Notları**

---

## 1. Giriş ve Tarihsel Anlam

1979 yılında tasarımı tamamlanan ve 1980 başında Türk Silahlı Kuvvetleri envanterine giren **ASELSAN 4600 Serisi VHF/FM Taktik Telsiz Ailesi**, Türkiye Cumhuriyeti'nin kendi imkânlarıyla tasarlayıp seri ürettiği ilk millî taktik haberleşme sistemidir.

Yabancı menşeili PRC-25 ve PRC-77 telsizlerinin ambargo sebebiyle yedek parçasız kaldığı bir dönemde ortaya konan 4600 Serisi, sadece bir telsiz değil; Türk ordusunun haberleşmede bağımsızlığını ilan ettiği dönüm noktasıdır.

```
       +-----------------------------------------------------------+
       |             ASELSAN 4600 BLOK DİYAGRAMI (1979)             |
       +-----------------------------------------------------------+
                                     ANTEN
                                       |
                                    [T/R RÖLE]
                                   /          \
                     (ALICI ALTI) /            \ (VERİCİ HATTI)
                                 v              ^
                          [Ön Süzgeç (RF)]   [Güç Kuvv. (PA)]
                                 |              ^
                          [LNA (Düşük Gürültü)] |
                                 |         [Sürücü / Tampon]
                            [1. Karıştırıcı]    ^
                                 ^              |
                                 |        [Faz Modülatörü]
                     [Sentezör / PLL / VCO]     ^
                                 |              |
                          [1. IF (21.4 MHz)] [Mikrofon Kuvv.]
                                 |              |
                          [2. Karıştırıcı]   [Susturma (Squelch)]
                                 |              |
                          [2. IF (455 kHz)]  [Kripto / Ton Modülü]
                                 |
                         [FM Ayrıştırıcı (Discriminator)]
                                 |
                        [Ses Kuvvetlendirici]
                                 |
                              HOPARLÖR
```

---

## 2. Sistem Özellikleri ve RF Parametreleri

| Parametre | Değer / Özellik | Açıklama |
| :--- | :--- | :--- |
| **Frekans Bandı** | $30.000\text{ MHz} - 75.975\text{ MHz}$ | VHF Taktik Askerî Band |
| **Kanal Aralığı** | $25\text{ kHz}$ / $50\text{ kHz}$ | 920 kullanılabilir kanal |
| **Modülasyon Türü** | $16\text{K}0\text{F}3\text{E}$ (FM - Frekans Modülasyonu) | Azami sapma $\pm 5.0\text{ kHz}$ |
| **Verici Gücü ($P_{tx}$)** | $1.5\text{ W}$ (Düşük) / $4.0\text{ W}$ (Yüksek) | Sırt konfigürasyonu (PRC) |
| **Araç Konfigürasyonu (VRC)** | $25\text{ W} - 50\text{ W}$ | Harici RF Güç Yükselticisi ile |
| **Alıcı Hassasiyeti** | $< 0.4\ \mu\text{V}$ ($10\text{ dB SINAD}$ için) | Çift dönüşümlü süperheterodin |
| **1. Ara Frekans (IF)** | $21.4\text{ MHz}$ | Kristal kafes süzgeçli yüksek seçicilik |
| **2. Ara Frekans (IF)** | $455\text{ kHz}$ | Seramik filtreleme ve ayrıştırma |
| **Susturma (Squelch)** | $150\text{ Hz}$ Ton Susturma & Gürültü Susturması | Yanlış alarmları önleyen çift kademe |
| **Çalışma Sıcaklığı** | $-40^\circ\text{C}$ ila $+65^\circ\text{C}$ | MIL-STD-810C standardı |

---

## 3. Kamoy Doktrininin Donanım Tasarımındaki İzleri

### A. Modüler Tak-Çıkar Kart Mimarisi (Line-Replaceable Units)
4600 serisi telsizin iç mimarisi tek bir ana karta lehimlenmiş monolitik bir yapı değildi. Cihaz:
1. **RF Güç ve Ön-Kat Modülü**,
2. **Frekans Sentezörü ve PLL Modülü**,
3. **IF / Audio ve Susturma Kartı**,
4. **DC-DC Güç Dönüştürücü Kartı**,
5. **Kripto / Güvenlik Ara Yüz Yuvası**

olmak üzere bağımsız ekranlanmış alüminyum kutucuklar halinde tasarlandı. Bu mimari sayesinde sahada arızalanan telsiz saatlerce lehimle uğraşılmadan 5 dakika içinde sadece arızalı modülü değiştirilerek harbe hazır hale getiriliyordu.

### B. "Direnç ve Kondansatör Millîleştirmesi"
Kamoy, 4600'ün her bileşeninin Türkiye'de üretilebilir olmasını şart koştu. Telsizin bobinleri, kristal tutucuları, alüminyum döküm kasası, su geçirmez contaları ve şasi vidaları dahi yerli sanayi atölyeleri eğitilerek imal ettirildi.

### C. Gürültü ve Karıştırma Dayanıklılığı (Anti-Jamming)
4600 serisinde kullanılan 150 Hz tonlu susturma devresi (tone squelch), cephe hattında düşmanın gürültü basarak telsizi kilitlemesini önleyen kritik bir emniyet katmanıydı. Sadece 150 Hz alt-ses pilot sinyalini alan alıcı hoparlörü açıyor; düşmanın bastığı düz gürültü bataryayı ve askerin dikkatini tüketmiyordu.

---

## 4. 4600'den `kok-1976` Sayısal Modeline Geçiş

ASELSAN 4600'ün analog dünyadaki prensipleri, bu depoda sayısal yazılım tabanlı telsiz (SDR - Software Defined Radio) mimarisiyle yeniden hayat bulmaktadır:

1. **Donanım PLL ve Mikser $\longrightarrow$ CORDIC Algoritması ve Sayısal Osilatör (NCO):**  
   Frekans kaydırma ve sentezleme, harici çip olmadan C++ CORDIC çekirdeğiyle yapılmaktadır.
2. **Seramik IF Filtre $\longrightarrow$ Kök Yükseltilmiş Kosinüs (RRC) ve Sayısal Süzgeç:**  
   Bant sınırlama sıfır heap kullanan sonlu dürtü yanıtlı (FIR) algoritmalarla çözülmektedir.
3. **150 Hz Ton Susturma $\longrightarrow$ 0x1976KOK Taktik Senkronizasyon Başlığı:**  
   Her telemetri paketi donanımsal koruyucu eşleme ile gürültü denizinden çekilip çıkarılmaktadır.
4. **Analog Modüler Kartlar $\longrightarrow$ Sıfır Bağımlılıklı C++ Başlıkları:**  
   Modüller arasındaki katı yalıtım, yazılımda sıfır dış bağımlılık prensibiyle sürdürülmektedir.
