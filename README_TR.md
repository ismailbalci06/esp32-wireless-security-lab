# ESP32 Wireless Security Lab 6.0

**ESP32 ile yerel ağ üzerinde çalışan, pasif Wi-Fi ve Bluetooth Low Energy (BLE) gözlem paneli.**

Proje, yakındaki 2.4 GHz Wi-Fi erişim noktalarını ve BLE reklam yayınlarını tarar. ESP32 kendi parola korumalı erişim noktasını ve web sunucusunu açar; panel Chrome üzerinden `http://192.168.4.1` adresinde görüntülenir. Son 40 tarama tarayıcının `localStorage` alanında saklanır; en güçlü Wi-Fi sinyali grafiği, son iki tarama arasındaki BLE gözlem farkları ve CSV dışa aktarma sunulur.

> **Güvenlik ve gizlilik:** Yalnızca kendi ekipmanınızda, özel laboratuvarınızda ve pasif gözlemin izinli olduğu ortamlarda kullanın. Proje taranan Wi-Fi ağlarına bağlanmaz, BLE cihazlarıyla eşleşmez ve BLE aktif tarama isteği göndermez. Yine de çevredeki ağ ve cihaz metaverileri görülebilir. Ham CSV dosyalarını ve ekran görüntülerini gözden geçirmeden herkese açık paylaşmayın.

## Özellikler

- ESP32 üzerinden pasif Wi-Fi taraması ve RSSI'ye göre sıralı liste.
- SSID, RSSI, kanal ve açık/korumalı temel sınıflandırması.
- 2.4 GHz kanallarındaki (1–13) gözlenen erişim noktası sayısı.
- `setActiveScan(false)` ile pasif BLE reklam taraması.
- Ham BLE adresi yerine kısa takma gözlem kimliği.
- BLE ve Wi-Fi taramalarını sırayla çalıştıran birleşik uç nokta.
- Wi-Fi, Bluetooth BLE, Kanallar ve Geçmiş sekmeleri.
- 30 saniyede otomatik tarama ve manuel tarama düğmesi.
- Son 40 tarama Chrome `localStorage` alanında.
- En güçlü Wi-Fi RSSI geçmiş grafiği ve son iki BLE taramasının karşılaştırması.
- Saklanan taramaların CSV dışa aktarımı.

## Donanım ve yazılım

| Bileşen | Test edilen yapılandırma |
|---|---|
| Kart | ESP32 geliştirme kartı; yükleme kaydında `ESP32-D0WD-V3` |
| Arduino kart seçimi | `ESP32 Dev Module` |
| Arduino IDE | 2.3.11 |
| ESP32 Arduino core | 3.3.12 |
| Partition Scheme | `Huge APP (3MB No OTA/1MB SPIFFS)` |
| Ek bileşen | Gerekli değil; USB kablosu yeterli |

## Kurulum

1. Arduino IDE 2.x'i kurun.
2. Boards Manager üzerinden **esp32 by Espressif Systems** paketini yükleyin. Test edilen sürüm 3.3.12'dir.
3. `ESP32_Wireless_Security_Lab_6_0.ino` dosyasını açın.
4. Kart olarak **ESP32 Dev Module** seçin.
5. Doğru USB seri portunu seçin.
6. **Tools → Partition Scheme → Huge APP (3MB No OTA/1MB SPIFFS)** seçin.
7. Kodun üst kısmındaki `AP_PASSWORD` değerini en az 8 karakterli, size özel bir parolayla değiştirin. Kişisel veya üretim parolanızı kullanmayın.
8. Önce **Verify**, ardından **Upload** yapın.
9. Serial Monitor'ü **115200 baud** ile açın. Başlangıç mesajları görünmüyorsa kartı resetleyin.
10. Bilgisayarınızı ESP32'nin Wi-Fi ağına bağlayıp Chrome'da `http://192.168.4.1` adresini açın.

Kaynak koddaki SSID varsayılanı `ESP32-Wireless-Lab` şeklindedir. ZIP içerisindeki parola yalnızca değiştirilmesi gereken örnek değerdir: `ChangeThis_AP_Password_2026!`.

## Teknik sınırlamalar

- RSSI, ESP32 anteninde ölçülen yaklaşık sinyal gücüdür; internet hızı veya girişim ölçümü değildir.
- Kanal grafiği kanalda görülen erişim noktalarını sayar; kanal doluluğunu, airtime'ı veya paraziti ölçmez.
- `Secured`, ağın tarama API'sine göre açık olmadığını belirtir; parola gücünü veya genel güvenliği ölçmez.
- BLE taraması yalnızca tarama penceresinde reklam yayınlayan ve menzil içinde bulunan cihazları görebilir.
- BLE gözlem kimliği, adresin 32-bit FNV-1a özetiyle üretilir. Bu kriptografik anonimleştirme değildir; çakışma olabilir ve adres rastgeleleştirme kimliği değiştirebilir.
- Bir cihazın sonraki taramada görünmemesi kapandığını kanıtlamaz.
- ESP32'nin ortak 2.4 GHz radyosu nedeniyle taramalar sıralı yapılır; tarama sırasında web istekleri kısa süre bekleyebilir.
- Geçmiş ESP32 flash belleğinde değil, `http://192.168.4.1` kaynağına ait tarayıcı `localStorage` alanındadır. Site verilerini temizlemek geçmişi siler.

## Test özeti

Kullanıcının ESP32 kartında derleme ve yükleme başarılı oldu. Yerel panelde Wi-Fi/BLE listeleri, kanal grafiği, geçmiş grafiği ve CSV dışa aktarma çalıştırıldı. Sayfa yenilemesinden sonra geçmişin korunması gözlendi. Ayrıntılar: [docs/TEST_RESULTS.md](docs/TEST_RESULTS.md).

## Lisans ve yayınlama

Henüz lisans seçilmedi. Başkalarının kodu yeniden kullanmasına izin vermeden önce uygun bir lisans ekleyin. README'deki yazar/GitHub bağlantısı alanlarını yayınlamadan önce doldurun.
