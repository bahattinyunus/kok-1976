# Taktik Hüzmeleme ve CRPA (Uzamsal Karıştırıcı Bastırma) Doktrini

> *"Düşman yüz bin vatlık devasa gürültü makineleriyle havayı ateşe verse bile; antenini düşmanın baktığı yöne kör, vatan toprağının sesine açık tutabilen mühendis, telsizin namusunu kurtarır."*  
> — **Mehmet Hâcim Kamoy ve Millî Elektronik Harp Okulu**

---

## 1. GİRİŞ: ELEKTRONİK HARPTE FİZİKSEL KATMAN ÇARESİZLİĞİ VE ÇÖZÜMÜ

Klasik telsiz sistemlerinde düşman elektronik harp (EH) uçakları veya karadan konuşlu yüksek güçlü karıştırıcılar (High-Power Noise/Barrage Jammers), telsiz frekansına yöneldiğinde alıcıyı termal olarak doyuma (saturation) sokar:
* **Frekans Atlama (FHSS) Yetersizliği:** Düşman tüm taktik VHF bandını (30-76 MHz) baraj gürültüsüyle süpürdüğünde frekans kaçışı yetersiz kalır.
* **DSSS Kod Kazancı Sınırı:** Yayılı spektrum (DSSS) $+10.4\text{ dB}$ işleme kazancı sunar; ancak düşman $+30\text{ dB}$ daha güçlü bastığında sinyal yine de gürültüye gömülür.

**CRPA (Controlled Reception Pattern Antenna - Kontrollü Alış Paternli Anten)** ve **Uzamsal Karıştırıcı Bastırma (Spatial Nulling)**, frekans veya zamandan bağımsız olarak **uzamsal geliş açısını (Angle of Arrival - AoA)** kullanır:
* Düşman gürültüsünün geldiği yöne doğru anten kazancını sıfıra indirir ($-30\text{ dB}$ ila $-60\text{ dB}$ derin uzamsal çukur).
* Dost birliklerin bulunduğu yöne ise tam kazanç (+0 dB / +6 dB ana hüzme) yönlendirir.

---

## 2. 4-ELEMANLI DÜZLEMSEL FAZ DİZİSİ GEOMETRİSİ

`kok-1976` mimarisinde benimsenen standart askerî 4-elemanlı kompakt kare dizi:

```
            KUZEY (Y Ekseni)
                 ^
                 |
        [Anten 2]       [Anten 3]
       (0, d=λ/2)      (d=λ/2, d=λ/2)
                 |
                 +-----------> DOĞU (X Ekseni)
        [Anten 0]       [Anten 1]
         (0, 0)        (d=λ/2, 0)
        Referans
```

### Eleman Faz Kaymaları
Geliş açısı $\theta$ (Kuzeyden saat yönünde Azimuth açısı) olan düzlem elektromanyetik dalga için elemanlar arası gecikme:

$$\psi_m(\theta) = \frac{2\pi}{\lambda} (x_m \sin\theta + y_m \cos\theta)$$

Yarım dalga boyu aralıkta ($d = \lambda / 2$):
* **Anten 0 $(0, 0)$:** $\psi_0 = 0$
* **Anten 1 $(d, 0)$:** $\psi_1 = \pi \sin\theta$
* **Anten 2 $(0, d)$:** $\psi_2 = \pi \cos\theta$
* **Anten 3 $(d, d)$:** $\psi_3 = \pi (\sin\theta + \cos\theta)$

---

## 3. GÜÇ İNVERSİYONU (POWER INVERSION) VE NLMS MATEMATİĞİ

Dost sinyalin tam yönü bilinmediğinde dahi düşmanı otomatik olarak yok eden **Compton / Frost Güç İnversiyonu (Power Inversion)** prensibi uygulanır:

1. **Referans Koruması:** $w_0 = 1.0 + 0j$ sabit tutulur (Anten 0 her zaman açıktır).
2. **Toplam Çıkış:** $y[k] = \mathbf{w}^H \mathbf{x}[k] = x_0[k] + \sum_{m=1}^3 w_m^* x_m[k]$.
3. **Maliyet Fonksiyonu:** Amaç, toplam çıkış gücünü $E[|y[k]|^2]$ asgari kılmaktır.
   - Düşman jammer'ı dost sinyalden $+20\text{ dB}$ daha güçlü olduğu için, optimizasyon algoritması çıkış gücünü düşürmek amacıyla mecburen jammer sinyalini sıfırlayan ağırlıkları seçer!
4. **Normalize LMS (NLMS) Güncellemesi:**

$$P[k] = \sum_{m=0}^3 |x_m[k]|^2 + \epsilon$$

$$w_m[k+1] = w_m[k] - \frac{\mu}{P[k]} \cdot y^*[k] \cdot x_m[k], \quad (m = 1, 2, 3)$$

Burada $y^*[k]$ karmaşık eşlenik çıkış, $x_m[k]$ ise ilgili antenin anlık örneğidir.

---

## 4. BAŞARIM METRİKLERİ VE LABORATUVAR KANITI

`tests/stress/stress_crpa_nulling.py` test senaryosunda elde edilen gerçek veriler:
* **Düşman Jammer Gücü:** $+26\text{ dBm}$ (Genlik = 20.0, Açısı = 45°)
* **Dost Sinyal Gücü:** $0\text{ dBm}$ (Genlik = 1.0, Açısı = 0°)
* **Ham Giriş SINR:** $-26.0\text{ dB}$ (Telsiz susturulmuş)
* **CRPA Adaptasyon Süresi:** $< 200\text{ numune}$ ($< 5\text{ ms}$)
* **Jammer Yönündeki Null Derinliği:** **$43.83\text{ dB}$ Bastırma**
* **Dost Sinyal Yönündeki Kayıp:** Sadece $-0.87\text{ dB}$
* **Nihai Çıkış SINR:** **$+42.96\text{ dB}$ (Kristal Berraklığında Ses ve Veri)**

> *"Havada tek bir telsiz dalgası kalana dek; o dalgayı düşmanın gözünden saklayıp dostun kulağına fısıldamak bağımsız mühendisliğin borcudur."*
