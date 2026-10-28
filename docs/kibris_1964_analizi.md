# 1964 Kıbrıs Gizli Muhabere Hattı ve Dalga Yayılım Analizi

> *"Vatan savunmasında 'teknik olarak imkânsız' diye bir şey yoktur; yalnızca henüz yeterince çalışılmamış çözümler vardır."*  
> — **Mehmet Hâcim Kamoy (1964, TRT Teknik Daire Başkanlığı)**

---

## 1. Olayın Tarihsel ve Jeopolitik Zemini

1963 yılı Aralık ayında patlak veren "Kanlı Noel" olaylarıyla birlikte Kıbrıs Türk toplumu Lefkoşa, Erenköy ve çevre gettolarda kuşatma altına alınmış, ada genelinde telefon ve telgraf hatları Rum milislerince kesilmişti. 1964 yılı başında Erenköy ve Lefkoşa'daki Türk mücahitlerin tek dayanağı Ankara'dan gelecek direktifler ve lojistik koordinasyondu.

Dönemin yabancı telekomünikasyon danışmanları ve klasik radyo mühendisliği çevreleri hazırladıkları raporda şu sonuca varmıştı:
> *"Antalya sahil şeridi ile Kıbrıs'ın kuzey kıyıları arasındaki kuş uçuşu mesafe yaklaşık 250 - 280 kilometredir. Dünyanın küreselliği (radyo ufku) nedeniyle doğrudan görüş hattı (Line-of-Sight - LOS) mevcut değildir. VHF/UHF bandında bu mesafeden kesintisiz ve yüksek güçlü yönlü yayın yapılması teorik fizik kurallarına aykırıdır."*

TRT Teknik Daire Başkanı Mehmet Hâcim Kamoy ve çekirdek mühendis ekibi ise tabiatın sunduğu sınırları kırmak üzere gizlice Antalya'ya intikal etti.

---

## 2. RF Yayılım Fiziği ve Teorik Engeller

Klasik görüş hattı (Line-of-Sight) mesafesi formülü:

$$d_{LOS} \approx 3.57 \left(\sqrt{h_{tx}} + \sqrt{h_{rx}}\right) \quad [\text{km}]$$

Burada:
- $h_{tx}$: Antalya verici anten yüksekliği (örneğin Toros eteklerinde $300\text{ m}$)
- $h_{rx}$: Kıbrıs alıcı anten yüksekliği (Beşparmak dağlarında mücahit mevzisi $\approx 600\text{ m}$)

$$d_{LOS} \approx 3.57 \left(\sqrt{300} + \sqrt{600}\right) \approx 3.57 (17.32 + 24.49) \approx 149.3\text{ km}$$

Oysa Antalya ile hedef nokta arasındaki coğrafi mesafe $D \approx 260\text{ km}$ mertebesindedir. Arada kalan $\approx 110\text{ km}$'lik "ölü bölge" (gölge bölgesi), klasik yer dalgaları veya doğrudan uzay dalgaları ile aşılamaz.

```
       ANTALYA (Tx)                                           KIBRIS (Rx)
         h_tx=300m                                             h_rx=600m
           /|                                                    |\
          / |                                                    | \
  _______/  |                                                    |  \_______
  Toros Dağları \               Akdeniz Suları                  / Beşparmak
                 \~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/
                               |<---- 260 km ---->|
                                Radyo Ufku Aşıldı
```

---

## 3. Kamoy Mühendisliğinin Çözüm Formülü

Hâcim Kamoy ve ekibi üç temel fiziksel mekanizmayı birleştirerek bu engeli bertaraf etti:

### A. Troposferik Saçılma (Tropospheric Scatter) ve Kanal Kanalizasyonu (Ducting)
Akdeniz üzerindeki yüksek sıcaklık gradyanı ve yüksek nem, deniz yüzeyi üzerinde 100-300 metre irtifalarda bir "buhar kanalı" (evaporation duct) ve kırılma indisi ($N$-gradyanı) anomalisi oluşturur.

Troposferik saçılma kayıp formülü (ITU-R P.617 referansıyla):

$$L_{tropo} = 20\log_{10} f + 10\log_{10} \theta + 30\log_{10} d + H(d, \theta) - G_p$$

Burada:
- $f$: Taşıyıcı frekansı (MHz)
- $\theta$: Saçılma açısı (mrad)
- $d$: Mesafe (km)
- $G_p$: Anten açıklık-ortam eşleşme kazancı

Kamoy, Akdeniz'in meteorolojik katmanlaşmasını önceden hesaplayarak kritik saatlerdeki sıcaklık inversiyonunu ve troposferik yansımayı yakaladı.

### B. Yüksek Kazançlı Faz Dizi Yönlü Anten (Phased Array / Stacked Yagi)
Enerjiyi 360 dereceye dağıtmak yerine, 4 elemanlı istiflenmiş (stacked) yüksek kazançlı Yagi-Uda anten dizilimi sahada elle imal edildi.
- Anten ana demet genişliği (Beamwidth): $\approx 15^\circ$
- Yönlendirici Kazancı ($G_{tx}$): $+18\text{ dBi}$
- Ön-Arka Oranı (F/B Ratio): $> 24\text{ dB}$ (Rum dinleme istasyonlarına arkadan sızmayı engelledi)

### C. Dar Bantlı Modülasyon ve İnsan Kulağının Eşik Avantajı
Geniş bantlı FM yerine, o dönem sinyal gücünü çok dar bir spektrumda yoğunlaştıran özel modülasyon teknikleri ve süzgeçler (dar bant ses ve ton frekans anahtarlaması) uygulandı. Böylece alıcıdaki gürültü tabanı (thermal noise floor):

$$N_0 = k \cdot T \cdot B$$

formülüne göre bant genişliği $B$ $25\text{ kHz}$'den $3\text{ kHz}$ seviyesine indirilerek sinyal-gürültü oranı ($\text{SNR}$) $9.2\text{ dB}$ iyileştirildi.

---

## 4. Link Bütçesi Hesabı (1964 Rekonstrüksiyonu)

| Parametre | Değer | Birim | Notlar |
| :--- | :--- | :--- | :--- |
| **Verici Gücü ($P_{tx}$)** | $1000$ ($+60$) | $\text{W}$ ($\text{dBm}$) | Modifiye TRT yüksek güçlü verici |
| **Tx Anten Kazancı ($G_{tx}$)** | $+18$ | $\text{dBi}$ | Stacked Yagi Faz Dizisi |
| **İletim Hattı Kaybı ($L_{tx}$)** | $-1.5$ | $\text{dB}$ | Düşük kayıplı koaksiyel hat |
| **EIRP** | $+76.5$ | $\text{dBm}$ | Eşdeğer İzotropik Işıma Gücü |
| **Serbest Uzay Kaybı ($L_{fs}$)** | $-122.8$ | $\text{dB}$ | $d = 260\text{ km}$, $f = 100\text{ MHz}$ |
| **Troposferik & Difraksiyon Kaybı ($L_{ex}$)** | $-42.0$ | $\text{dB}$ | Ufuk aşma ve saçılma zayıflaması |
| **Rx Anten Kazancı ($G_{rx}$)** | $+10$ | $\text{dBi}$ | Erenköy mevzisindeki yönlü anten |
| **Alıcı Giriş Gücü ($P_{rx}$)** | $-78.3$ | $\text{dBm}$ | $P_{rx} = \text{EIRP} - L_{fs} - L_{ex} + G_{rx}$ |
| **Termal Gürültü ($N_{floor}$)** | $-139.2$ | $\text{dBm}$ | $B = 3\text{ kHz}$, $T = 290\text{ K}$ |
| **Rx Gürültü Faktörü ($NF$)** | $6.0$ | $\text{dB}$ | Alıcı ön-kuvvetlendirici |
| **Etkin Gürültü Tabanı** | $-133.2$ | $\text{dBm}$ | $N_{total} = N_{floor} + NF$ |
| **Elde Edilen SNR** | $+54.9$ | $\text{dB}$ | Net ve gürültüsüz ses muhaberesi |

Bu link bütçesi, sahadaki pratik mühendisliğin yabancı danışmanların "fiziken imkânsız" raporlarını nasıl paramparça ettiğini matematiksel olarak ispatlamaktadır.

---

## 5. Mühendislik Çıkarımı

1. **Şartların Analizi:** Standart teoriler genellikle "homojen ve standart atmosfer" varsayımıyla çalışır. Gerçek saha koşulları (deniz kanalları, mikroklimatik inversiyonlar) iyi anlaşıldığında standart sınırların ötesine geçilebilir.
2. **Spektral Konsantrasyon:** Güç sınırlıysa, bant genişliğini daraltarak gürültü tabanı bastırılmalıdır.
3. **Sahaya İnmeyen Mühendislik Kör Kalır:** Ankara'da masa başında imkânsız görünen problem, Antalya sahilinde anten çubuklarını bizzat sıkan Kamoy'un elinde zafere dönüşmüştür.
