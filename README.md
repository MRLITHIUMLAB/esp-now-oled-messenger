# 📡 ESP32 → ESP8266 Text Messenger via ESP-NOW + OLED Display

![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP8266-blue)
![Protocol](https://img.shields.io/badge/Protocol-ESP--NOW-green)
![Display](https://img.shields.io/badge/Display-SSD1306%200.96%22%20OLED-yellow)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%28Arduino%29-orange)
![License](https://img.shields.io/badge/License-MIT-brightgreen)

A beginner-friendly, **router-free wireless text messenger** using Espressif's built-in **ESP-NOW protocol**.  
The ESP32 sends text messages directly to the ESP8266, which displays them live on a **0.96" OLED screen**.

---

## 📋 Table of Contents

- [Features](#-features)
- [How It Works](#-how-it-works)
- [Hardware Required](#-hardware-required)
- [Wiring Diagram](#-wiring-diagram)
- [Project Structure](#-project-structure)
- [Libraries Required](#-libraries-required)
- [Setup & Flashing Guide](#-setup--flashing-guide)
- [Code Explanation](#-code-explanation)
  - [ESP32 Sender](#esp32-sender-explained)
  - [ESP8266 Receiver](#esp8266-receiver-explained)
- [Serial Monitor Output](#-serial-monitor-output)
- [Customisation](#-customisation)
- [Troubleshooting](#-troubleshooting)
- [Extending the Project](#-extending-the-project)
- [License](#-license)

---

## ✨ Features

- ✅ **Zero Wi-Fi router needed** — pure peer-to-peer via ESP-NOW
- ✅ **Sub-millisecond latency** — ESP-NOW is faster than MQTT or HTTP
- ✅ **OLED live display** — messages render instantly on the 0.96" screen
- ✅ **Auto word-wrap** — long messages split cleanly across OLED lines
- ✅ **MAC address shown at boot** — no need for Serial Monitor to configure
- ✅ **Delivery confirmation** — ESP32 prints SUCCESS/FAIL for every packet
- ✅ **Packet counter** — track total received messages on the OLED
- ✅ **Cyclic message demo** — 7 rotating demo messages out of the box

---

## 🔧 How It Works

```
┌─────────────────────┐         ESP-NOW (2.4GHz)        ┌──────────────────────────┐
│       ESP32         │  ──────────────────────────────► │       ESP8266            │
│                     │       Direct Peer-to-Peer         │                          │
│  Sends text packet  │       No router required          │  Receives & displays on  │
│  every 3 seconds    │       Range: ~200m (open air)     │  0.96" OLED screen       │
└─────────────────────┘                                   └──────────────────────────┘
```

**ESP-NOW** is a proprietary Espressif wireless protocol that works at the MAC layer — it doesn't need an IP address, DHCP, or internet access. Devices communicate directly using their **MAC addresses** as identifiers, with latency under 1 ms.

### Communication Flow

```
ESP32                                    ESP8266
  │                                         │
  │  1. Build MessagePacket struct          │
  │     { text[64], packetID }              │
  │                                         │
  │  2. esp_now_send(receiverMAC, data) ──► │  3. onDataReceived() callback fires
  │                                         │     Copy data into latestPacket
  │  4. onDataSent() callback fires         │
  │     Print SUCCESS / FAIL               │  5. Main loop detects newMessageFlag
  │                                         │     Display message on OLED
  │                                         │     Print to Serial Monitor
```

---

## 🛒 Hardware Required

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32 Dev Board | 1 | Any variant (ESP32-WROOM, NodeMCU-32, etc.) |
| ESP8266 Dev Board | 1 | NodeMCU v1/v2, Wemos D1 Mini, etc. |
| 0.96" I²C OLED Display | 1 | SSD1306 controller, 128×64 px, I²C interface |
| Jumper Wires | 4 | For OLED to ESP8266 |
| USB Cables | 2 | One per board for flashing + power |

---

## 🔌 Wiring Diagram

### ESP8266 → OLED (SSD1306)

```
ESP8266 Pin          OLED Pin
───────────          ────────
3.3V         ──────► VCC
GND          ──────► GND
D2 (GPIO 4)  ──────► SDA
D1 (GPIO 5)  ──────► SCL
```

> ⚠️ **Use 3.3V only!** The SSD1306 OLED is NOT 5V tolerant.

> The ESP32 does **not** connect to the OLED in this project — it only transmits wirelessly.

### Visual Wiring

```
    ESP8266                  OLED SSD1306
  ┌─────────┐               ┌──────────┐
  │    3.3V ├──────────────►│ VCC      │
  │     GND ├──────────────►│ GND      │
  │ D2/SDA  ├──────────────►│ SDA      │
  │ D1/SCL  ├──────────────►│ SCL      │
  └─────────┘               └──────────┘
```

---

## 📁 Project Structure

```
esp-now-oled-messenger/
│
├── ESP32_Sender/
│   └── ESP32_Sender.cpp        # Sender sketch — flash onto ESP32
│
├── ESP8266_Receiver/
│   └── ESP8266_Receiver.cpp    # Receiver sketch — flash onto ESP8266
│
├── README.md                   # This file
├── .gitignore                  # Arduino/IDE build artifact exclusions
└── LICENSE                     # MIT License
```

---

## 📦 Libraries Required

Install these via **Arduino IDE → Sketch → Include Library → Manage Libraries**:

| Library | Where to Find | Used In |
|---------|---------------|---------|
| `Adafruit SSD1306` | Library Manager → search "Adafruit SSD1306" | ESP8266 Receiver |
| `Adafruit GFX Library` | Auto-installed as dependency of SSD1306 | ESP8266 Receiver |
| `ESP8266 Core` | Board Manager → search "esp8266" → install "ESP8266 by ESP8266 Community" | ESP8266 Receiver |
| `ESP32 Core` | Board Manager → search "esp32" → install "esp32 by Espressif Systems" | ESP32 Sender |

> `esp_now.h` (ESP32) and `espnow.h` (ESP8266) are **built into their respective cores** — no separate install needed.

---

## 🚀 Setup & Flashing Guide

### Step 1 — Flash the ESP8266 Receiver

1. Open `ESP8266_Receiver/ESP8266_Receiver.cpp` in Arduino IDE
2. Select board: **Tools → Board → NodeMCU 1.0 (ESP-12E Module)** (or your variant)
3. Select the correct COM port
4. Click **Upload**
5. Open **Serial Monitor** (baud: **115200**)
6. Note the printed MAC address, e.g.:
   ```
   Receiver MAC: EC:FA:BC:CB:47:96
   ```
   The OLED will also show this MAC on startup.

### Step 2 — Configure the ESP32 Sender

Open `ESP32_Sender/ESP32_Sender.cpp` and find line ~20:

```cpp
// BEFORE (placeholder)
uint8_t receiverMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

// AFTER (your actual MAC — example)
uint8_t receiverMAC[] = {0xEC, 0xFA, 0xBC, 0xCB, 0x47, 0x96};
```

**MAC conversion rule:** Replace each `:` with `, 0x` and prepend `0x` to the first byte.

| MAC String | Array Format |
|------------|-------------|
| `EC:FA:BC:CB:47:96` | `{0xEC, 0xFA, 0xBC, 0xCB, 0x47, 0x96}` |

### Step 3 — Flash the ESP32 Sender

1. Select board: **Tools → Board → ESP32 Dev Module**
2. Select the correct COM port
3. Click **Upload**

### Step 4 — Test!

Power both boards. The ESP32 will send a new message every **3 seconds** and you'll see it appear live on the OLED. 🎉

---

## 💡 Code Explanation

### ESP32 Sender — Explained

#### 1. Payload Structure

```cpp
typedef struct MessagePacket {
    char text[64];   // The message — up to 63 characters
    int  packetID;   // Auto-incrementing ID for tracking drops
} MessagePacket;
```

Both boards define **the exact same struct**. ESP-NOW sends raw bytes, so the structure must be identical on sender and receiver — otherwise the data will be misinterpreted.

#### 2. Wi-Fi Mode Setup

```cpp
WiFi.mode(WIFI_STA);   // Station mode — required by ESP-NOW
WiFi.disconnect();      // Don't connect to any AP
```

ESP-NOW requires Wi-Fi to be active in **station mode**, but you don't need to connect to a router. The radio is used purely for the ESP-NOW MAC-layer communication.

#### 3. Registering the Peer

```cpp
esp_now_peer_info_t peerInfo = {};
memcpy(peerInfo.peer_addr, receiverMAC, 6);
peerInfo.channel  = 0;      // 0 = auto-detect channel
peerInfo.encrypt  = false;  // No AES encryption (keep it simple)
esp_now_add_peer(&peerInfo);
```

The ESP32 must register every device it wants to talk to as a **peer** before sending. You can register up to **20 peers** simultaneously.

#### 4. Sending Data

```cpp
esp_now_send(receiverMAC,         // Destination MAC
             (uint8_t *)&outgoing, // Raw bytes of the struct
             sizeof(outgoing));    // Number of bytes to send
```

`esp_now_send()` is **non-blocking** — it returns immediately. The result (success/fail) comes via the callback.

#### 5. Delivery Callback

```cpp
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS ✓" : "FAILED ✗");
}
```

This fires after every send attempt and tells you whether the receiving device acknowledged the packet at the MAC layer.

---

### ESP8266 Receiver — Explained

#### 1. OLED Initialisation

```cpp
Wire.begin(4, 5);  // SDA = GPIO4 (D2), SCL = GPIO5 (D1)
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
```

`0x3C` is the default I²C address for most SSD1306 OLEDs. If your display stays blank, try `0x3D`.

#### 2. Receive Callback

```cpp
void onDataReceived(uint8_t *mac, uint8_t *data, uint8_t len) {
    if (len == sizeof(MessagePacket)) {
        memcpy(&latestPacket, data, sizeof(MessagePacket));
        newMessageFlag = true;  // signal main loop
        totalReceived++;
    }
}
```

This runs in **interrupt context** — keep it short. We copy data and set a flag; the actual OLED update happens in `loop()` to avoid crashes.

#### 3. Word Wrap

```cpp
void wrapText(const char* src, int width, char* l1, char* l2, char* l3) {
    strncpy(l1, src, width); l1[width] = '\0';
    // ... splits into 3 lines of `width` characters each
}
```

At `TextSize 1`, the SSD1306 fits **~21 characters per line**. This function slices a long string into 3 display lines automatically.

#### 4. OLED Display Layout

```
┌────────────────────────┐
│ ESP-NOW RECEIVER       │  ← Inverted header bar
│                        │
│ Hello from ESP32!      │  ← Message line 1
│                        │  ← Message line 2 (if wrapped)
│                        │  ← Message line 3 (if wrapped)
│                   Pkt:7│  ← Packet counter (bottom-right)
└────────────────────────┘
```

---

## 📟 Serial Monitor Output

**ESP32 Sender** (115200 baud):
```
=== ESP32 ESP-NOW Sender ===
Sender MAC: 30:AE:A4:12:34:56
ESP-NOW initialised. Sending messages every 3 s...

[TX #1] "Hello from ESP32!"
[ESP-NOW] Delivery status: SUCCESS ✓
[TX #2] "ESP-NOW is fast!"
[ESP-NOW] Delivery status: SUCCESS ✓
```

**ESP8266 Receiver** (115200 baud):
```
=== ESP8266 ESP-NOW Receiver ===
Receiver MAC: EC:FA:BC:CB:47:96
Waiting for messages...

[RX #1] "Hello from ESP32!"
[RX #2] "ESP-NOW is fast!"
```

---

## 🎨 Customisation

### Change the Messages (ESP32)

Edit the `messages[]` array in `ESP32_Sender.cpp`:

```cpp
const char* messages[] = {
    "Your custom message 1",
    "Sensor value: 42",
    "Door opened!",
    // Add as many as you like
};
```

### Change the Send Interval

```cpp
delay(3000);   // Change 3000 to any millisecond value
```

### Change OLED I²C Address

```cpp
#define OLED_I2C_ADDR  0x3C   // Try 0x3D if screen is blank
```

### Enable Encryption

```cpp
// In ESP32 — set key and enable encryption on peer
peerInfo.encrypt = true;
memcpy(peerInfo.lmk, "MySecretKey12345", 16);  // 16-byte key
```

---

## 🔍 Troubleshooting

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| OLED blank at startup | Wrong I²C address | Change `0x3C` to `0x3D` |
| OLED blank at startup | Wrong wiring | Double-check SDA/SCL pins |
| "Failed to add peer" on ESP32 | Wrong MAC in code | Re-check MAC from Serial Monitor |
| Delivery status always FAILED | Boards too far apart | Move closer; check antennas |
| Delivery status always FAILED | Wrong MAC format | Use `0x` hex format, not string |
| esp_now_init() failed | Wi-Fi not in STA mode | Ensure `WiFi.mode(WIFI_STA)` runs first |
| Garbled text on OLED | Struct mismatch | Make sure both structs are identical |

---

## 🚀 Extending the Project

Here are ideas to build on top of this project:

- **📊 Sensor Data** — Attach a DHT11/DHT22 to ESP32 and send live temperature/humidity to the OLED
- **↔️ Two-Way Chat** — ESP8266 sends ACK messages back to ESP32 (both act as peer + slave)
- **📻 Multi-Sender** — One ESP8266 receiver can handle up to **20 ESP32 senders** simultaneously
- **🔐 Encryption** — Enable AES-128 encryption with a 16-byte Local Master Key (LMK)
- **🔔 Alert System** — Trigger a buzzer on ESP8266 when specific keywords arrive
- **📡 Long Range** — Use external antenna variants (ESP32-WROOM with U.FL connector) for ~1 km range
- **💾 Message Log** — Store last N messages in ESP8266's SPIFFS/LittleFS file system

---

## 📄 License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) for details.  
Feel free to use, modify, and distribute for personal or commercial projects.

---

## 🙌 Acknowledgements

- [Espressif ESP-NOW Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306)
- [Arduino ESP8266 Community Core](https://github.com/esp8266/Arduino)

---

> Made with ❤️ using ESP-NOW — no routers were harmed in the making of this project.
