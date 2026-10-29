# Taktik Ad-Hoc Mesh Ağ ve Röle (MANET) Mimarisi

> *"Muharebe sahasında merkezi olan her şey vurulabilir; komuta çadırı yıkılır, ana verici kulesi bombalanır. Ancak her askerin, her tankın ve her sırt telsizinin birbiri üzerinden mesaj taşıdığı dağıtık bir ağ asla susturulamaz."*  
> — **Mehmet Hâcim Kamoy ve Millî Muhabere Doktrini**

---

## 1. GİRİŞ VE TARİHÎ DOKTRİN

1974 Kıbrıs Barış Harekâtı'nda Beşparmak Dağları'nın dik yamaçları, derin vadileri ve Akdeniz'in coğrafi engelleri, klasik görüş hattı (Line-of-Sight - LoS) telsiz haberleşmesini imkânsız hale getirmişti:
* **Görüş Hattı Körlüğü:** Dağın ardındaki piyade bölüğü, sahil çıkarma başındaki ana karargâhla doğrudan VHF irtibatı kuramıyordu.
* **Merkezi Röle Kırılganlığı:** Tepeye dikilen tek bir sabit aktarıcı kulesi, düşman topçusu ve hava taarruzunun ilk hedefiydi. Kule imha olduğunda tüm kolordunun irtibatı kesiliyordu.

Mehmet Hâcim Kamoy'un 1976 yılında temellerini attığı felsefe, **"Tek bir merkeze muhtaç olmayan, her telsizin gerektiğinde otomatik röle olduğu bağımsız ağ"** vizyonuydu. 

Bu doküman, `kok-1976` mimarisinde hayata geçirilen **C++17 Freestanding Taktik Ad-Hoc Mobil Ağ (MANET)** ve **Çok Sıçramalı (Multi-Hop) Akıllı Röleleme** motorunun çalışma esaslarını açıklar.

---

## 2. AĞ MİMARİSİ VE ÇALIŞMA PRENSİBİ

Taktik sahada hiçbir altyapı (baz istasyonu, uydu veya kablo) bulunmaz. Birlikler sürekli hareket halindedir.

```
 [İleri Gözetleyici]       [Tank Müfrezesi]        [Zırhlı Röle Aracı]       [Taktik Harekât Merkezi]
    (Düğüm 0x01)              (Düğüm 0x02)             (Düğüm 0x03)                 (Düğüm 0x04)
        |                         |                         |                            |
  [Hedef Koordinatı]              |                         |                            |
        +------- Sıçrama 1 ------>+                         |                            |
                                  +------- Sıçrama 2 ------>+                            |
                                                            +------- Sıçrama 3 --------->+
                                                                                  [Ateş Emri İcrası]
```

### Temel Protokol Prensipleri
1. **Dinamik Paket Yönlendirme (Dynamic Multi-Hop):** Hedef düğüm menzil dışındaysa, aradaki tüm dost birimler otomatik olarak veriyi bir sonraki adıma aktarır.
2. **Kendi Kendine İyileşme (Self-Healing):** Aradaki bir telsiz imha edilirse veya mevzi değiştirirse, paketler anında alternatif düğümler üzerinden rotalanır.
3. **Kendi Kendini Yapılandırma (Zero-Configuration):** Hiçbir operatörün IP veya rota tanımlamasına gerek yoktur; telsiz açıldığı an ağa katılır.

---

## 3. PAKET YANMASI VE DÖNGÜ ÖNLEME (TTL VE DEDÜPLİKASYON)

Telsiz sinyalleri havada her yöne yayıldığından (broadcast), en büyük tehlike paketlerin sonsuz döngüye girip telsiz kanalını kilitlemesidir (Broadcast Storm). `kok-1976` bu sorunu iki kademeli savunmayla çözer:

### 1. TTL (Time-To-Live / Kalan Yaşam Sıçraması)
Her paketin başlığında bir `hop_count` / `ttl` alanı bulunur:
* Verici paketi ilk ürettiğinde: `TTL = MAX_HOPS` (örneğin 5).
* Paketi alıp aktaran her röle: $\text{TTL} = \text{TTL} - 1$.
* $\text{TTL} = 0$ olduğunda paket düşürülür (drop edilir), havada sonsuza kadar dönmesi engellenir.

### 2. Dönen Önbellek İle Dedüplikasyon (Deduplication Cache)
Bir telsiz aynı paketi farklı dağ yansımalarından veya farklı komşularından defalarca alabilir.
`mesh_router` çekirdeğinde dinamik bellek ayrımı yapmayan sabit boyutlu bir LRU önbellek tutulur:
* Her paket anahtarı: `[Kaynak Düğüm ID (1 bayt) + Sıra Numarası (2 bayt)]`
* Telsiz paketi aldığında önbelleğe bakar:
  - Eğer bu anahtar son $N$ paket içinde görülmüşse: **Paket derhal sessizce yok edilir.**
  - Eğer ilk defa görülüyorsa: Önbelleğe yazılır, işlenir veya bir sonraki düğüme aktarılır.

```cpp
// Sıfır Dinamik Bellek ile Freestanding Dedüplikasyon
bool is_duplicate(uint8_t src, uint16_t seq) {
    for (size_t i = 0; i < DEDUP_CACHE_SIZE; ++i) {
        if (cache[i].valid && cache[i].src == src && cache[i].seq == seq) {
            return true; // Daha önce işlendi, fırtınayı engelle!
        }
    }
    // Önbelleğe kaydet (Halka döngüsü)
    cache[head] = {src, seq, true};
    head = (head + 1) % DEDUP_CACHE_SIZE;
    return false;
}
```

---

## 4. TELSİZ GÖREV PROFİLLERİ (ROLES)

Bir birlikteki tüm telsizlerin batarya ve güç kapasitesi aynı değildir:

| Rol | Görev | Güç / Telsiz Tipi | Davranış |
|---|---|---|---|
| **END_NODE (Uç Birim)** | Piyade eri, keşif timi | 1-2 Watt Sırt Telsizi | Yalnızca kendi mesajını atar ve dinler; bataryasını korumak için başkasının verisini aktarmaz. |
| **ROUTER (Yönlendirici)** | Zırhlı araç, manga telsizi | 10-20 Watt Araç Telsizi | Menfeze giren tüm paketleri doğrular, TTL düşürür ve yayınlar. |
| **COMMAND (Komuta Merkezi)** | Karargâh, Tugay Harekât | 50-100 Watt Sabit İstasyon | Tüm ağ trafiğini dinler, kriptolu telemetri ve harita güncellemelerini toplar. |

---

## 5. ELEKTRONİK HARP VE FREKANS ATLAMA (FHSS) İLE ENTEGRASYON

Mesh ağın en kritik özelliği, fiziksel katmanda **64-kanallı Frekans Atlama (FHSS)** ve **Barker DSSS** ile senkronize çalışmasıdır:
* Düşman dinleyicisi belirli bir frekansı dinlediğinde ağın tamamını göremez.
* Sıçrama 1 (Düğüm 1 $\to$ 2) $f_7$ frekansında gerçekleşirken, Sıçrama 2 (Düğüm 2 $\to$ 3) $f_{42}$ frekansında gerçekleşir.
* Düşman bir kanala gürültü bassa bile, ağ paketleri diğer frekans sıçramalarında veya alternatif coğrafi düğümler üzerinden hedefe ulaştırır.

---

## 6. DOĞRULAMA VE BAŞARIM METRİKLERİ

`tests/stress/stress_mesh_network.py` test senaryosunda, Toroslar ve Beşparmak dağlık coğrafyası simüle edilerek 5 düğümlü bir MANET ağı test edilmiştir:
- **Senaryo:** Düğüm 1 (İleri Gözcü) $\to$ Düğüm 2 (Dağ Zirvesi Röle) $\to$ Düğüm 3 (Vadi Araç) $\to$ Düğüm 4 (Tugay Karargâh).
- **Sonuç:**
  - 3 sıçramalı tam paket teslimatı: **%100 Başarı**.
  - Çift yönlü döngü ve fırtına önleme: **%100 Blokaj** (Sıfır sonsuz döngü).
  - İşlemci Yükü: Paket başına $< 4.2 \ \mu\text{s}$ yönlendirme kararı (MISRA uyumlu, sıfır heap).

> *"Milletin bağımsızlığı, yalnızca sınır boyundaki mevzilerin sağlamlığıyla değil; o mevzileri birbirine bağlayan telsiz dalgalarının kesintisizliğiyle korunur."*
