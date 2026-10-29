# Taktik Radyo Yön Bulma (RDF) ve Elektronik Destek (ED) Kestirim Mimarisi

> *"Elektronik harpte düşmanın sesini kesmek zaferin yarısıdır; ancak düşmanın telsizini susturmadan önce mevziini santimetre hassasiyetinde bulup topçuya bildirmek, muharebenin mutlak sonucudur."*  
> — **ASELSAN Elektronik Harp ve Radar Araştırmaları Grubu (1982)**

---

## 1. GİRİŞ VE TARİHÎ ELEKTRONİK DESTEK (ED) DOKTRİNİ

Taktik sahada düşman birimleri ve karıştırıcıları (jamming istasyonları), telsiz mandalına her bastıklarında çevreye elektromanyetik enerji yayarlar. Bu enerji, sinyalin içeriği şifreli olsa dahi gizlenemeyen fiziksel bir izdir (fiziksel katman parmak izi).

1974 Kıbrıs Harekâtı ve sonrasında Türk Silahlı Kuvvetleri'nin en büyük ihtiyacı:
1. **Düşman Karıştırıcılarının Tespiti:** Taktik VHF bandımızı boğmaya çalışan düşman ECM araçlarının tam coğrafi mevziini bulup imha etmek.
2. **Düşman Birlik Hareketlerinin İzlenmesi:** Düşman telsizlerinin sinyal yaydığı koordinatları kerteriz açısıyla harita üzerine işlemek.
3. **Muharebe Arama Kurtarma (CSAR):** Düşman hatlarının ardına atlayan pilotlarımızın acil durum sinyallerini (Emergency Locator Beacon) dinleyip kurtarma timine koordinat vermek.

Bu doküman, `kok-1976` mimarisine entegre edilen **C++17 Freestanding Watson-Watt ve Adcock Faz İnterferometrisi Yön Bulucu (RDF)** motorunun matematiksel temellerini ve kerteriz üçgenlemesini (Triangulation) açıklar.

---

## 2. WATSON-WATT VE ADCOCK ANTEN DİZİSİ MİMARİSİ

```
                  KUZEY (000°)
                       |
                  [Anten Kuzey]
                       |
  BATI (270°) --- [Duyarga Sense] --- DOĞU (090°)
                       |
                  [Anten Güney]
                       |
                  GÜNEY (180°)
```

### 1. Dikey Çapraz Dipol (Adcock) Geometrisi
Klasik çerçeve antenler (loop antenna) yatay polarizasyonlu gökyüzü dalgalarından (night effect / skywave) etkilenip yanlış açı gösterir. **Adcock anten dizisi**, yalnızca dikey polarizasyonlu yer dalgalarını toplayarak gökyüzü hatasını sıfırlar:
* **Kuzey-Güney Çifti ($V_{NS}$):** $\theta$ yönünden gelen dalga için $V_{NS}(t) = A \cos(\theta) \cos(\omega t)$
* **Doğu-Batı Çifti ($V_{EW}$):** $\theta$ yönünden gelen dalga için $V_{EW}(t) = A \sin(\theta) \cos(\omega t)$
* **Duyarga (Sense) Anteni ($V_{Sense}$):** Merkezde yer alan dairesel/omnidirectional anten: $V_{Sense}(t) = A \cos(\omega t)$

### 2. Koherent Çarpım ve $180^\circ$ Belirsizliğinin (Ambiguity) Çözümü
Yalnızca $V_{NS}$ ve $V_{EW}$ oranına bakıldığında, $\theta$ ile $\theta + 180^\circ$ açıları aynı orana sahiptir (çift kutuplu sekiz şeklindeki ışıma diyagramı).

`kok-1976`, Adcock sinyallerini merkezî **Sense** anteni ile koherent zaman düzleminde skaler çarpar:

$$\langle V_{NS}, V_{Sense} \rangle = \frac{1}{N} \sum_{k=1}^N V_{NS}[k] \cdot V_{Sense}[k] = \frac{A^2}{2} \cos(\theta)$$

$$\langle V_{EW}, V_{Sense} \rangle = \frac{1}{N} \sum_{k=1}^N V_{EW}[k] \cdot V_{Sense}[k] = \frac{A^2}{2} \sin(\theta)$$

$$\theta = \text{atan2}\left(\langle V_{EW}, V_{Sense} \rangle, \ \langle V_{NS}, V_{Sense} \rangle \right)$$

Bu matematiksel model sayesinde $180^\circ$ ön-arka belirsizliği doğal olarak ve heuristik kurallara gerek kalmaksızın kesin çözülür.

---

## 3. İKİ İSTASYONLU HEDEF ÜÇGENLEMESİ (TRIANGULATION)

Tek bir istasyon yalnızca **kerteriz hattı (Bearing Line)** verir; hedefin ne kadar uzakta olduğunu bilemez. İki ayrı noktada konuşlu kestirme istasyonu ortak bir hedefi dinlediğinde:

```
    İstasyon 1 (Antalya)                   İstasyon 2 (Alanya)
        (X1, Y1)                               (X2, Y2)
           \                                      /
            \  Kerteriz θ1              Kerteriz / θ2
             \                                  /
              \                                /
               +------------ [HEDEF] ---------+
                           (Xt, Yt)
```

Matematiksel çözüm:
Doğruların parametrik denklemi:
$$\begin{aligned}
x &= x_1 + t_1 \sin(\theta_1) \\
y &= y_1 + t_1 \cos(\theta_1)
\end{aligned} \quad \text{ve} \quad
\begin{aligned}
x &= x_2 + t_2 \sin(\theta_2) \\
y &= y_2 + t_2 \cos(\theta_2)
\end{aligned}$$

İki denklem eşitlenerek $2 \times 2$ lineer sistem çözülür:
$$D = \sin(\theta_2 - \theta_1) = \cos(\theta_1)\sin(\theta_2) - \sin(\theta_1)\cos(\theta_2)$$

Eğer $|D| \ge 0.05$ ise kesişim tekildir:
$$t_1 = \frac{(y_2 - y_1)\sin(\theta_2) - (x_2 - x_1)\cos(\theta_2)}{D}$$

$$\mathbf{X}_{\text{hedef}} = x_1 + t_1 \sin(\theta_1), \quad \mathbf{Y}_{\text{hedef}} = y_1 + t_1 \cos(\theta_1)$$

---

## 4. BAŞARIM VE STRES TESTİ SONUÇLARI

`tests/stress/stress_direction_finding.py` senaryosunda Akdeniz'de 60 km mesafede konuşlu bir hedef kestirilmiştir:
* **İstasyon Taban Mesafesi (Baseline):** 50 km
* **Kerteriz Açı Hatası (+18 dB SNR):** **$< 0.4^\circ$**
* **Konumlandırma Hatası (Triangulation Error):** **$< 0.5 \ \text{km}$** (60 km mesafeden)
* **Gürültü Dayanımı (+3 dB SNR):** **$5.5^\circ$ açı hatası ile hedef çemberi başarıyla korundu.**
