# Firmware — Build & Upload Guide

**Target Board:** ESP32-S3 WROOM-1 N16R8  
**Framework:** Arduino (C++)  
**Toolchain:** arduino-cli 1.4.1  
**Sketch Size:** ~900 lines

---

## 1. Overview

The firmware runs on the ESP32-S3 and handles:

- RFID card reading (MFRC522) and access decision logic
- DHT22 temperature and humidity sensing
- IR door-ajar detection with escalating alert
- ACS712 compressor current monitoring
- SSD1306 OLED display management
- Passive buzzer tone generation
- Dual-mode Wi-Fi (Station + Access Point)
- HTTP server for the local web dashboard
- SPIFFS credential storage
- NTP time synchronization

Everything is contained in a single `.ino` file for simplicity of 
deployment and review.

---

## 2. Hardware Requirements

| Component | Model | Purpose |
|---|---|---|
| Microcontroller | ESP32-S3 WROOM-1 N16R8 | Main processor |
| RFID Reader | MFRC522 | Access control |
| Temp/Humidity | DHT22 (AM2302) | Environmental monitoring |
| Current Sensor | ACS712 30A | Power monitoring |
| OLED Display | SSD1306 128×64 I²C | Local UI |
| IR Sensor | KY-032 or equivalent | Door detection |
| Buzzer | KY-006 passive piezo | Audio alerts |
| Relay | 1-channel, 5V | Lock control |
| Solenoid Lock | 12V electromagnetic | Door lock |
| Power Supply | 5V / 3A SMPS | System power |

---

## 3. Pin Assignments

| Peripheral | ESP32-S3 Pin |
|---|---|
| MFRC522 SS (SDA) | GPIO 10 |
| MFRC522 RST | GPIO 9 |
| MFRC522 SCK | GPIO 12 |
| MFRC522 MOSI | GPIO 11 |
| MFRC522 MISO | GPIO 13 |
| OLED SDA | GPIO 8 |
| OLED SCL | GPIO 18 |
| Relay Control | GPIO 2 |
| Buzzer | GPIO 17 |
| IR Sensor | GPIO 15 |
| DHT22 Data | GPIO 4 |
| ACS712 Analog | GPIO 14 |

---

## 4. Required Libraries

| Library | Version |
|---|---|
| MFRC522 | 1.4.10 |
| Adafruit GFX | 1.11.9 |
| Adafruit SSD1306 | 2.5.7 |
| DHT sensor library | 1.4.4 |

Built-in (no install): WiFi, SPIFFS, SPI, Wire, time.

---

## 5. Setup

### Install arduino-cli
```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

### Add ESP32 board support
```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls \
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32@2.0.14
```
### Install libraries
```bash
arduino-cli lib install "MFRC522@1.4.10"
arduino-cli lib install "Adafruit GFX Library@1.11.9"
arduino-cli lib install "Adafruit SSD1306@2.5.7"
arduino-cli lib install "DHT sensor library@1.4.4"
```

### Build & Upload
```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32s3 .

# Upload (replace /dev/ttyUSB0 with your port)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32s3 .

# Serial monitor
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

## 7. First-Time Configuration
1. Power on the device — OLED shows `WiFi Setup Mode`
2. On your phone/laptop, connect to Wi-Fi network `Fridge_Setup`
3. Open browser to `http://192.168.4.1`
4. Select your network, enter password, tap Save & Reboot
5. Device reboots and connects to your network

After setup, the dashboard is available at:
* `http://192.168.4.1` via soft AP `MiniFridge_Local`
* Or via the device's assigned IP on your home network

## 8. Key Configuration Constants
At the top of `fridge_main.ino`:
```cpp
#define DOOR_AJAR_THRESHOLD_MS  30000   // 30 seconds
#define LOCK_UNLOCK_DURATION_MS  3000   // 3 seconds
#define DHT_POLL_INTERVAL_MS     5000   // 5 seconds
#define WIFI_AP_SSID       "MiniFridge_Local"
#define CONFIG_AP_SSID     "Fridge_Setup"

// Authorized card UIDs (add new rows to authorize more cards)
const byte AUTHORIZED_UIDS[][4] = {
  {0xE4, 0xD8, 0xF3, 0x06},   // Admin card
};
const int AUTHORIZED_COUNT = 1;
```
To authorize a new card: read its UID from the serial monitor, add a
row, re-upload the firmware.

## 9. Serial Debug Output

The firmware logs to the serial console at **115200 baud**. Example output:

```text
[BOOT] IoT Smart Refrigerator v1.0
[WIFI] Connecting to Home_5G...
[WIFI] Connected. IP: 192.168.1.42
[WIFI] Soft AP: MiniFridge_Local
[NTP]  Time synced: 03:42 PM
[RFID] Card UID: E4 D8 F3 06 → ACCESS GRANTED
[RFID] Card UID: 1A 2B 3C 4D → ACCESS DENIED
[DHT22] Temp: 4.2°C  Hum: 52%
[IR]   Door OPEN (timer started)
[IR]   Door CLOSED (timer reset)
[ACS712] Current: 0.78 A  Power: 96 VA
```

## 10. Firmware Modes

The firmware operates in one of two high-level modes:

### Configuration Mode

Entered when SPIFFS contains no valid `wifi.txt` file.

- Broadcasts access point **`Fridge_Setup`**
- Serves the captive portal at `192.168.4.1`
- Accepts SSID + password, saves to SPIFFS, then reboots

### Normal Mode

Entered when valid credentials are found in SPIFFS.

- Connects to the stored network in **Station (STA)** mode
- Simultaneously broadcasts **`MiniFridge_Local`** as a soft Access Point
- Serves the dashboard at `192.168.4.1` (or the assigned IP on the home network)

### Main Loop (Normal Mode)

```text
1. Handle incoming HTTP requests
2. Poll DHT22 every 5 seconds
3. Check RFID reader for cards
4. Check IR sensor for door state
5. Update lock timer
6. Update OLED display state machine
7. Handle buzzer beeps
```

## 11. Known Behaviors

| Behavior | Description |
|---|---|
| **RFID watchdog** | MFRC522 is reinitialized every 10 seconds to recover from SPI communication glitches |
| **DHT22 NaN handling** | If a read fails (returns NaN), the previous valid reading is retained; no gap visible to the user |
| **OLED I²C contention** | The firmware skips OLED updates while serving a web client, preventing I²C bus corruption |
| **NTP fallback** | If NTP sync is unavailable (no Internet), the OLED shows `--- ---` in the time field |

