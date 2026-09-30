# Software Requirements Specification

## 1. Introduction
### 1.1 Purpose
This Software Requirements Specification (SRS) defines the functional and non-functional requirements for the IoT-Integrated Intelligent Refrigerator System — a retrofitted water dispenser augmented with RFID-based access control, environmental monitoring, door-ajar alerting, energy metering, and a local web dashboard.

This document serves as the authoritative reference for what the system shall do and how well it shall perform it. It is intended for developers, testers, and reviewers involved in the project.

### 1.2 Scope
The system encompasses the full development lifecycle:

- Mechanical adaptation of a salvaged water dispenser into a refrigerated locker
- Electronic integration of sensors, actuators, and display on a single ESP32-S3
- Firmware development in C++ using the Arduino framework
- Local web dashboard served directly from the microcontroller
- Unit, integration, and performance testing

**Out of scope:**
- Cloud-based remote access beyond the local network
- Dynamic RFID card enrollment through a user interface
- Mobile application development
- Battery backup or uninterruptible power supply
- Firmware control of the compressor (mechanical thermostat retained)

### 1.3 Definitions & Acronyms
| Term | Meaning |
|---|---|
| SRS | Software Requirements Specification |
| RFID | Radio Frequency Identification |
| UID | Unique Identifier (of an RFID card) |
| HF | High Frequency (13.56 MHz) |
| UAT | User Acceptance Testing |
| SDLC | Software Development Life Cycle |
| ESP32-S3 | Dual-core 240 MHz Wi-Fi microcontroller |
| DHT22 | Digital temperature & humidity sensor |
| ACS712 | Hall-effect linear current sensor |
| MFRC522 | 13.56 MHz RFID reader module |
| SSD1306 | OLED display controller (I²C) |
| SPIFFS | SPI Flash File System |
| IR | Infrared |
| AP | Access Point |
| STA | Station (Wi-Fi client mode) |
| NTP | Network Time Protocol |

### 1.4 References
- IEEE Std 830-1998 — Recommended Practice for Software Requirements Specifications
- DHT22 Datasheet (Aosong AM2302)
- MFRC522 Datasheet (NXP)
- ACS712 Datasheet (Allegro MicroSystems)
- ESP32-S3 Technical Reference Manual (Espressif)

### 1.5 Document Overview
Section 2 describes the product, its users, and its operating environment. Section 3 specifies the functional and non-functional requirements in detail. Section 4 covers external interface requirements.

## 2. Overall Description

### 2.1 Product Perspective
The IoT-Integrated Intelligent Refrigerator System is a standalone, retrofittable IoT appliance. It augments a conventional vapor-compression cooling unit with intelligence: electronic access control, continuous environmental monitoring, automated alerting, and remote visibility through a local web dashboard.

The system is designed as a single-controller architecture — all sensing, actuation, web serving, and display functions run on one ESP32-S3 microcontroller, eliminating the need for a secondary processor.

### 2.2 Product Functions
1. **RFID access control** — Only authorized cards unlock the door
2. **Temperature & humidity monitoring** — Continuous sensing via DHT22
3. **Door-ajar detection** — IR sensor with escalated alert
4. **Wi-Fi configuration portal** — Captive portal for network setup
5. **Local web dashboard** — Live data over Wi-Fi
6. **Compressor current monitoring** — ACS712 for power awareness
7. **On-device OLED display** — Local status at a glance

### 2.3 User Classes & Characteristics
| User | Description | Technical Skill |
|---|---|---|
| Owner | Primary user; scans RFID card, views dashboard | Non-technical |
| Administrator | Configures Wi-Fi network, manages card list | Basic |
| Technician | Reviews current data, diagnoses compressor issues | Technical |

### 2.4 Operating Environment
n- Indoor use, ambient temperature 15–35°C
- 220V AC mains supply (Pakistan Standard)
- Local Wi-Fi network (2.4 GHz, 802.11 b/g/n)
- Modern web browser (Chrome, Safari, Samsung Internet) for dashboard
- MIFARE Classic 13.56 MHz RFID cards

### 2.5 Design Constraints
- Single ESP32-S3 microcontroller — no secondary processor
- Total electronics cost ≤ USD 50 (excluding donor appliance)
- Dashboard accessible only on local network (no cloud dependency)
- Authorized RFID UIDs stored as compile-time constants in firmware
- Compressor control retained by the original mechanical thermostat
- No battery backup — system relies on mains power

### 2.6 Assumptions & Dependencies
- Donor water dispenser available in working condition
- Stable 220V AC supply
- Wi-Fi 2.4 GHz network available for dashboard access
- Users possess authorized MIFARE Classic RFID cards

## 3. Specific Requirements

### 3.1 Functional Requirements

See `functional_reqs.md` for the complete list (FR-001 through FR-014).

### 3.2 Non-Functional Requirements

See `non_functional_reqs.md` for the complete list (NFR-001 through NFR-022).

### 3.3 External Interface Requirements

**User Interfaces:**
- **OLED display (SSD1306, 128×64):** Displays time, temperature, humidity, access status, and door-ajar warnings
- **Web dashboard:** Renders live temperature, humidity, door state, power draw, and manual unlock button
- **Buzzer (KY-006):** Startup melody, access-granted tone, access-denied tone, door-ajar alert pattern

**Hardware Interfaces:**
| Peripheral | Interface | ESP32-S3 Pin |
|---|---|---|
| MFRC522 SS | SPI | GPIO 10 |
| MFRC522 RST | Digital | GPIO 9 |
| MFRC522 SCK | SPI | GPIO 12 |
| MFRC522 MOSI | SPI | GPIO 11 |
| MFRC522 MISO | SPI | GPIO 13 |
| OLED SDA | I²C | GPIO 8 |
| OLED SCL | I²C | GPIO 18 |
| Relay (lock) | Digital | GPIO 2 |
| Buzzer | PWM | GPIO 17 |
| IR sensor | Digital | GPIO 15 |
| DHT22 | One-wire | GPIO 4 |
| ACS712 | Analog (ADC1_CH4) | GPIO 14 |

**Software Interfaces:**
- Arduino framework (C++)
- MFRC522 library v1.4.10
- Adafruit GFX v1.11.9 + SSD1306 v2.5.7
- DHT sensor library v1.4.4
- Wi-Fi (dual-mode STA + AP)
- HTTP server on port 80
- SPIFFS for credential storage
- NTP for time synchronization

**Communication Interfaces:**
- Wi-Fi 802.11 b/g/n (2.4 GHz)
- SPI @ 1 MHz (RFID reader)
- I²C (OLED display)
- HTTP (dashboard)
