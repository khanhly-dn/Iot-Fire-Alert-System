# 🌡️ IoT Sensor Monitoring System

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif" />
  <img src="https://img.shields.io/badge/Sensor-DHT11-orange?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Protocol-WiFi%20%7C%20HTTP-green?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Display-OLED%20SSD1306-purple?style=for-the-badge" />
  <img src="https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge" />
</p>

<p align="center">
  Hệ thống giám sát <strong>nhiệt độ & độ ẩm</strong> thời gian thực sử dụng <strong>ESP32 + DHT11</strong>,<br/>
  tích hợp <strong>Web Dashboard</strong>, <strong>OLED</strong>, <strong>Buzzer</strong> và <strong>LED cảnh báo</strong>.
</p>

---

## 📌 Giới thiệu

Dự án **IoT Sensor Monitoring System** được phát triển nhằm xây dựng hệ thống **giám sát nhiệt độ và độ ẩm** theo thời gian thực.  
Hệ thống liên tục đọc dữ liệu từ cảm biến **DHT11**, phân tích mức độ cảnh báo và phản hồi ngay lập tức thông qua:

- 🌐 **Web Dashboard** hiện đại, truy cập từ bất kỳ thiết bị nào trong cùng mạng WiFi
- 📟 **Màn hình OLED** hiển thị nhiệt độ, độ ẩm và trạng thái trực tiếp
- 🔔 **Buzzer** cảnh báo âm thanh theo mức độ nguy hiểm
- 💡 **LED đỏ** báo hiệu trực quan khi có cảnh báo

---

## ⚙️ Chức năng chính

- **Đọc nhiệt độ & độ ẩm** liên tục mỗi 2 giây qua cảm biến DHT11 (GPIO4)
- **3 mức cảnh báo:** `SAFE` → `WARNING` → `DANGER`
- **Buzzer thông minh** – tần suất chuông thay đổi theo mức độ (`200ms` khi DANGER, `600ms` khi WARNING)
- **OLED hiển thị** – thông tin nhiệt độ, độ ẩm và trạng thái blink khi nguy hiểm
- **Web Dashboard** – giám sát realtime, test alert, im lặng buzzer, reset hệ thống
- **Activity Log** – lưu lịch sử 20 sự kiện gần nhất
- **Thống kê** – theo dõi Max Temp, Min Temp, Uptime, WiFi RSSI
- **REST API** – giao tiếp HTTP đơn giản, hỗ trợ CORS

---

## 🧩 Sơ đồ hoạt động

<p align="center">
  <img width="700" alt="Sơ đồ hoạt động" src="https://github.com/khanhly-dn/Iot-Fire-Alert-System/blob/main/SD.png?raw=true" />
</p>

```
Cảm biến DHT11 (GPIO4) → ESP32 đọc Temp & Humi mỗi 2s
           ↓
  Temp < 35°C & Humi < 90%   →  SAFE    → LED tắt, Buzzer tắt, OLED bình thường
  Temp ≥ 35°C hoặc Humi ≥ 90% →  WARNING → LED sáng, Buzzer chậm (600ms), OLED cảnh báo
  Temp ≥ 40°C hoặc Humi ≥ 100% → DANGER  → LED sáng, Buzzer nhanh (200ms), OLED nhấp nháy
           ↓
  Web Dashboard cập nhật mỗi 2 giây
```

---

## 🛠️ Phần cứng sử dụng

| Linh kiện | Chân kết nối | Mô tả |
|---|---|---|
| **ESP32** | – | Vi điều khiển chính, WiFi tích hợp |
| **Cảm biến DHT11** | GPIO4 | Đo nhiệt độ & độ ẩm |
| **LED đỏ** | GPIO2 | Báo hiệu khi có cảnh báo |
| **Buzzer** | GPIO5 | Cảnh báo âm thanh |
| **OLED SSD1306** | SDA/SCL (I²C, 0x3C) | Hiển thị trạng thái 128×64 |
| **Nguồn** | 5V USB | Cấp điện toàn bộ hệ thống |

<p align="center">
  <img width="600" alt="Linh kiện" src="https://github.com/khanhly-dn/Iot-Fire-Alert-System/blob/main/TB.png?raw=true" />
</p>

---

## 💻 Phần mềm & Công nghệ

- **Ngôn ngữ:** Arduino C++, HTML / CSS / JavaScript
- **Framework:** Arduino ESP32 Core
- **Thư viện:**
  - `WiFi.h` – kết nối mạng WiFi
  - `WebServer.h` – HTTP server nội bộ trên cổng 80
  - `DHT.h` – đọc dữ liệu cảm biến DHT11
  - `Adafruit_SSD1306` + `Adafruit_GFX` – điều khiển màn hình OLED
- **Giao tiếp:** HTTP REST API qua WiFi nội bộ
- **Giao diện:** Web Dashboard chạy độc lập (file `index.html` + `script.js`) kết nối thẳng tới ESP32

---

## 🌐 API Endpoints

| Endpoint | Method | Mô tả |
|---|---|---|
| `GET /status` | GET | Trả về JSON: temp, humi, level, alert, uptime, log... |
| `GET /info` | GET | Thông tin thiết bị: IP, SSID, RSSI |
| `GET /test` | GET | Kích hoạt test cảnh báo DANGER |
| `GET /silence` | GET | Tắt buzzer (giữ nguyên cảnh báo) |
| `GET /reset` | GET | Reset toàn bộ trạng thái hệ thống |

**Ví dụ response `/status`:**
```json
{
  "temp": 32.50,
  "humi": 75.00,
  "alert": false,
  "silenced": false,
  "level": "SAFE",
  "uptime": 120000,
  "maxTemp": 36.10,
  "minTemp": 28.40,
  "rssi": -62,
  "logs": [
    { "ts": 115000, "temp": 36.10, "humi": 88.00, "level": "WARNING" }
  ]
}
```

---

## 📊 Thông số kỹ thuật

| Thông số | Giá trị |
|---|---|
| Ngưỡng WARNING nhiệt độ | ≥ 35°C |
| Ngưỡng WARNING độ ẩm | ≥ 90% |
| Ngưỡng DANGER nhiệt độ | ≥ 40°C |
| Ngưỡng DANGER độ ẩm | ≥ 100% |
| Tần suất đọc cảm biến | 2000ms / lần |
| Buzzer DANGER interval | 200ms |
| Buzzer WARNING interval | 600ms |
| Dung lượng log sự kiện | 20 mục gần nhất |
| Cổng WebServer | 80 (HTTP) |
| Màn hình OLED | 128×64px, I²C 0x3C |
| Baud Serial Monitor | 115200 |

---

## 🚀 Hướng dẫn cài đặt

### 1. Cài đặt môi trường
```bash
# Cài Arduino IDE >= 2.0
# Thêm ESP32 board vào Board Manager với URL:
# https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

### 2. Cài thư viện (Arduino Library Manager)
```
DHT sensor library (Adafruit)
Adafruit SSD1306
Adafruit GFX Library
```

### 3. Cấu hình trong code
Mở file `IoT_Sensor_Monitoring_System.ino`, chỉnh các thông số sau:
```cpp
#define WIFI_SSID   "Tên_WiFi_của_bạn"
#define WIFI_PASS   "Mật_khẩu_WiFi"

#define TEMP_THRESHOLD  35.0   // Ngưỡng nhiệt độ cảnh báo (°C)
#define HUMI_THRESHOLD  90.0   // Ngưỡng độ ẩm cảnh báo (%)
```

### 4. Kết nối phần cứng
```
DHT11  DATA  →  GPIO4
LED    (+)   →  GPIO2  →  330Ω  →  GND
Buzzer (+)   →  GPIO5  →  GND
OLED   SDA   →  GPIO21
OLED   SCL   →  GPIO22
OLED   VCC   →  3.3V
OLED   GND   →  GND
```

### 5. Nạp code & chạy
```
1. Mở file IoT_Sensor_Monitoring_System.ino trong Arduino IDE
2. Chọn đúng board: ESP32 Dev Module
3. Chọn đúng cổng COM tương ứng
4. Nạp code qua cáp USB
5. Mở Serial Monitor (115200 baud) → xem địa chỉ IP được cấp
6. Mở file index.html trên trình duyệt
7. Nhập địa chỉ IP của ESP32 → nhấn "Kết Nối" → Dashboard hoạt động!
```

---

## 📷 Demo

| Web Dashboard |
|:---:|
| ![Demo](https://github.com/khanhly-dn/Iot-Fire-Alert-System/blob/main/DEMO.jpg?raw=true) |

---

## 🚀 Hướng phát triển

- [ ] Tích hợp **MQTT** để kết nối Home Assistant / Node-RED
- [ ] Thêm **Telegram Bot** gửi cảnh báo khẩn cấp tới điện thoại
- [ ] **OTA Update** – cập nhật firmware qua WiFi không cần cáp
- [ ] Hỗ trợ **nhiều cảm biến** trên một dashboard
- [ ] Thêm **biểu đồ lịch sử** nhiệt độ & độ ẩm theo thời gian thực
- [ ] Tích hợp **cảm biến MQ-2** để phát hiện khói / gas kết hợp
- [ ] Hỗ trợ **cảm biến DHT22** để tăng độ chính xác

---

## 👤 Thực hiện

**Lý Gia Khánh**  
Khoa Công nghệ Thông tin – Trường Đại học Đại Nam

---

<p align="center">
  Made with ❤️ using ESP32 · DHT11 · OLED · Arduino
</p>
