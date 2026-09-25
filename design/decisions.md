# Design Decisions

This document records the key design decisions made during development, the alternatives considered, and the rationale for each choice.

---

## 1. System Architecture

### Decision: Single-controller architecture

**Chosen:** One ESP32-S3 microcontroller handles all sensing, actuation, web serving, and display.

**Alternatives considered:**
- ESP32 + Raspberry Pi (two-board setup + expensive)
- Arduino + ESP8266 (separate processing and communication)

**Rationale:**
- Reduces component cost to under USD 100
- Eliminates inter-processor communication overhead
- Simplifies firmware (single codebase)
- Lower power consumption and fewer failure points
- ESP32-S3's dual-core 240 MHz processor and 8 MB PSRAM are sufficient for concurrent sensor polling, web serving, and display updates

**Trade-off accepted:** No headroom for heavy computation (e.g., machine learning), but not required for this application.

---

## 2. Microcontroller Selection

### Decision: ESP32-S3 WROOM-1 N16R8

**Chosen:** ESP32-S3 with 16 MB flash, 8 MB PSRAM.

**Alternatives considered:**
- ESP32-WROOM-32 (older, less RAM/flash)
- ESP8266 (single-core, no Bluetooth)
- Raspberry Pi Pico W (no native Wi-Fi AP mode as mature)

**Rationale:**
- Dual-core 240 MHz — one core for Wi-Fi/TCP stack, one for application
- Built-in Wi-Fi (STA + AP dual mode)
- 8 MB PSRAM allows in-memory serving of dashboard HTML
- 16 MB flash accommodates SPIFFS for credentials
- Mature Arduino core with all required libraries
- Widely available and low cost (~USD 8)

---

## 3. RFID Reader & Access Control

### Decision: MFRC522 at 13.56 MHz HF

**Chosen:** MFRC522 reader with MIFARE Classic cards.

**Alternatives considered:**
- LF (125 kHz) reader — shorter range, less convenient
- UHF (860–960 MHz) reader — too long range, risk of unintentional reads

**Rationale:**
- HF read range (up to 10 cm) is ideal: long enough for convenience, short enough to require deliberate presentation
- MFRC522 is widely available, well-documented, well-supported
- SPI interface at 1 MHz for reliable communication
- Cost: under USD 3 per module

**Trade-off accepted:** MIFARE Classic encryption has known vulnerabilities, but the threat model (protecting groceries) does not warrant DESFire-level security.

### Decision: 12V solenoid lock (fail-secure)

**Chosen:** 12V electromagnetic solenoid bolt lock.

**Alternatives considered:**
- Motorized deadbolt — higher cost, more complex
- Magnetic lock — requires continuous power to remain locked (fail-safe)

**Rationale:**
- Fail-secure: remains locked when unpowered (power loss = secure)
- Draws ~400 mA for only 3 seconds per unlock
- Simple relay drive with flyback diode protection
- Cost: under USD 5

---

## 4. Environmental Sensing

### Decision: DHT22 (AM2302)

**Chosen:** DHT22 for combined temperature and humidity sensing.

**Alternatives considered:**
- DS18B20 — better temperature accuracy but no humidity
- SHT31 — higher accuracy but ~5× cost
- BME280 — excellent but overkill for refrigeration range

**Rationale:**
- Temperature accuracy ±0.5°C is sufficient for food safety
- Humidity measurement is required (affects food preservation)
- Digital one-wire interface saves GPIO pins
- Cost: ~USD 3

**Placement:** Mounted at the center of the rear wall of the chilled compartment, away from the evaporator plate (to avoid cold-spot bias) and away from the door (to avoid warm-air bias on opening).

### Decision: Infrared obstacle sensor for door-ajar

**Chosen:** Reflective IR sensor module.

**Alternatives considered:**
- Magnetic reed switch — binary, cannot detect "slightly ajar"
- Mechanical microswitch — subject to wear, precise alignment required

**Rationale:**
- Non-contact detection
- Can identify a door that is slightly ajar but not fully latched
- No mechanical wear from repeated door operations
- Cost: ~USD 1

**Trade-off accepted:** Consumes ~5–10 mA continuously for the IR emitter. Acceptable given mains power.

---

## 5. Energy Monitoring

### Decision: ACS712 30A Hall-effect current sensor

**Chosen:** ACS712-30A for compressor current measurement.

**Alternatives considered:**
- HLW8032 energy metering IC — accurate true power, but complex interface
- ATM90E26 — accurate, but requires mains isolation and more circuitry
- Current transformer (CT) — bulky, requires AC signal processing

**Rationale:**
- Galvanic isolation between mains and microcontroller
- Simple analog output proportional to current
- Cost: ~USD 2
- Adequate for indicative monitoring (compressor duty cycle, inrush spikes, trend analysis)

**Trade-off accepted:** Cannot measure true power for inductive loads (measured error ~8% for compressor). The dashboard reports indicative power, not utility-grade measurement.

**Signal conditioning:** 10 kΩ / 10 kΩ voltage divider on sensor output to bring 5V-centered signal within ESP32-S3's 3.3V ADC range.

---

## 6. User Interface

### Decision: SSD1306 OLED (0.96", 128×64, I²C)

**Chosen:** Small OLED on the device itself.

**Alternatives considered:**
- LCD 16×2 character display — less flexible, less crisp
- TFT color display — larger, more expensive, unnecessary for text/icon UI
- No display (dashboard only) — reduces local usability

**Rationale:**
- High contrast, readable in all lighting
- I²C interface uses only 2 GPIO pins
- Low power consumption
- Can render simple vector icons (snowflake, clock)
- Cost: ~USD 3

### Decision: Passive buzzer (KY-006)

**Chosen:** Passive piezo buzzer driven via PWM.

**Alternatives considered:**
- Active buzzer — fixed frequency, no melody capability
- Small speaker — requires amplifier, bulky

**Rationale:**
- Can produce different tones for different events (access granted, access denied, door-ajar alert)
- Small, low power
- Cost: ~USD 1

---

## 7. Firmware Architecture

### Decision: Arduino framework with arduino-cli + Make

**Chosen:** C++ using Arduino framework, compiled with arduino-cli.

**Alternatives considered:**
- ESP-IDF — more control but steeper learning curve
- MicroPython — slower execution, less library support
- PlatformIO — viable, but arduino-cli + Make fits the existing workflow

**Rationale:**
- Arduino framework provides mature libraries for all peripherals
- arduino-cli integrates with version control and command-line workflow
- Makefile defines targets for compile, upload, and monitor
- Single-loop architecture is sufficient for the task rate (5s sensor poll, ~500ms RFID poll)

### Decision: Dual-mode Wi-Fi (STA + AP simultaneously)

**Chosen:** ESP32-S3 operates as both a Wi-Fi station (connected to home network) and a soft access point (`MiniFridge_Local`) for the dashboard.

**Rationale:**
- Users can access the dashboard from any device connected to the home network (via STA mode)
- Users can also access it directly via the soft AP if the home network is unavailable
- Configuration portal uses AP-only mode before credentials are stored

### Decision: SPIFFS for credential storage

**Chosen:** Credentials stored in SPIFFS as `wifi.txt`.

**Alternatives considered:**
- Preferences library (NVS) — viable, but SPIFFS allows human-readable file
- Hard-coded credentials — not viable (no portability)

**Rationale:**
- Persists across reboots and firmware updates
- Simple file-based read/write
- Easy to wipe via dashboard "Reset Wi-Fi Settings" button

---

## 8. Web Dashboard

### Decision: Single-page, self-contained HTML/CSS/JS

**Chosen:** Dashboard served as a single HTML response with embedded CSS and JavaScript. No external CDN dependencies.

**Rationale:**
- Works without Internet access (local AP only)
- Single HTTP response minimizes load time
- Total size ~6.5 KB — fits comfortably in ESP32-S3 memory
- Uses `fetch()` to poll `/data` every 2 seconds

### Decision: Responsive card-based layout

**Chosen:** Flexbox-based layout with rounded cards, soft shadows, colour-coded status indicators.

**Rationale:**
- Renders correctly on mobile, tablet, and desktop
- Large touch targets for unlock button
- Colour coding (green = closed, red = open) is instantly readable

---

## 9. Safety & Compliance

### Decision: Physical separation of mains and low-voltage

**Implementation:**
- All 220V AC wiring confined to a dedicated junction box
- Low-voltage electronics physically separated
- Heat-shrink tubing on all mains connections
- Commercially certified SMPS with over-current protection

### Decision: No PII collection

**Implementation:**
- Only anonymous RFID UIDs are logged
- Wi-Fi credentials stored locally, never transmitted off-device
- No cloud connectivity in current version

---

## 10. Summary of Key Trade-offs

| Decision | Trade-off Accepted |
|---|---|
| Single-controller architecture | No headroom for heavy computation |
| ACS712 current sensor | ~8% error for inductive loads |
| MIFARE Classic RFID | Known encryption weaknesses (acceptable threat model) |
| Local-only dashboard | No remote access |
| Fixed UID list | No dynamic card enrollment |
| IR door sensor | Continuous small power draw (~5–10 mA) |

---
