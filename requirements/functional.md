# Functional Requirements

Requirements are numbered FR-001 through FR-014. Each requirement is testable, has an assigned priority, and maps to at least one test case in the traceability matrix.

---

## Priority Definitions

| Priority | Meaning |
|---|---|
| **High** | Essential — system does not function without it |
| **Medium** | Important for usability — system is degraded without it |
| **Low** | Nice-to-have — improves experience but not critical |

---

## Functional Requirements

| ID | Requirement | Priority |
|---|---|---|
| FR-001 | The system shall repurpose the lower compartment of a water dispenser as a refrigerated storage unit using its existing 220V vapor-compression cooling system. | High |
| FR-002 | The system shall provide RFID-based access control using the MFRC522 reader and a 12V electromagnetic solenoid lock. | High |
| FR-003 | The system shall distinguish authorized from unauthorized RFID cards and grant or deny access accordingly. | High |
| FR-004 | The system shall energize the solenoid lock for exactly 3 seconds upon a valid card scan. | High |
| FR-005 | The system shall monitor internal temperature and humidity using the DHT22 sensor. | High |
| FR-006 | The system shall display current temperature and humidity on the OLED screen. | High |
| FR-007 | The system shall detect a door-ajar condition via an IR sensor. | High |
| FR-008 | The system shall trigger an escalating audible-visual alert after a 30-second silent door-ajar threshold. | High |
| FR-009 | The system shall serve a local web dashboard over Wi-Fi displaying real-time temperature, humidity, door status, and power consumption. | Medium |
| FR-010 | The system shall measure compressor current via the ACS712 sensor and estimate instantaneous power. | Medium |
| FR-011 | The system shall provide a Wi-Fi configuration portal (captive portal) when no credentials are stored. | Medium |
| FR-012 | The system shall persist Wi-Fi credentials in SPIFFS across reboots. | Medium |
| FR-013 | The system shall allow manual unlock via a dashboard button for 3 seconds. | Low |
| FR-014 | The system shall display the current time via NTP when connected to a network with Internet access. | Low |

---

## Requirement Details

### FR-001: Cooling System Adaptation
The lower compartment of a commercial water dispenser, including its hermetic 220V reciprocating compressor, wire-and-tube condenser, capillary tube, and roll-bond evaporator, shall be retained and repurposed as a general-purpose refrigerated locker.

### FR-002: RFID Access Control
An MFRC522 reader operating at 13.56 MHz shall read MIFARE Classic cards presented within 10 cm. On a valid UID match, a relay shall energize a 12V solenoid lock, retracting the bolt.

### FR-003: Access Decision
The firmware shall compare the scanned card UID against a stored list of authorized UIDs. Authorized cards shall unlock the door; unauthorized cards shall be denied with visual and audible feedback.

### FR-004: Lock Timing
The solenoid lock shall remain energized for exactly 3 seconds per authorized scan, after which it shall return to the locked position.

### FR-005: Environmental Monitoring
A DHT22 sensor shall measure internal temperature (−40 to 80°C range) and relative humidity (0–100% RH) at 5-second intervals.

### FR-006: OLED Display
An SSD1306 128×64 OLED shall display time, temperature, humidity, and a "SCAN CARD" prompt in normal operation, and dedicated access/door messages during events.

### FR-007: Door-Ajar Detection
An infrared obstacle sensor mounted on the door frame shall detect when the door is more than 5 mm from the closed position.

### FR-008: Door-Ajar Alert
After a 30-second silent window, the system shall emit an intermittent beep (200 ms on / 200 ms off at 3.1 kHz) and blink the OLED warning message until the door is closed.

### FR-009: Web Dashboard
The ESP32-S3 shall host a single-page web application on port 80, serving live sensor data via JSON at `/data` and updating every 2 seconds.

### FR-010: Current Monitoring
An ACS712 30A Hall-effect sensor shall measure the compressor's live wire current and estimate instantaneous power for dashboard display.

### FR-011: Configuration Portal
On first boot (no stored credentials), the device shall start an access point named `Fridge_Setup` and serve a captive portal for Wi-Fi setup.

### FR-012: Credential Persistence
Wi-Fi SSID and password shall be stored in SPIFFS and survive reboots.

### FR-013: Manual Unlock
The dashboard shall include an "Unlock" button that triggers a 3-second unlock cycle via HTTP POST to `/unlock`.

### FR-014: Time Synchronization
When connected to a network with Internet access, the system shall synchronize time via NTP and display it in 12-hour format on the OLED.

---
