# Bug Reports
## IoT-Integrated Intelligent Refrigerator System

**Total Bugs Reported:** 5  
**Resolved:** 5  
**Open:** 0

## Bug Severity & Priority Legend
| Severity | Meaning |
|---|---|
| Critical | System unusable; blocks core functionality |
| High | Major function impaired |
| Medium | Function degraded; workaround exists |
| Low | Cosmetic or minor inconvenience |

| Priority | Meaning |
|---|---|
| High | Must fix before release |
| Medium | Should fix in current cycle |
| Low | Can defer to next version |

## BUG-001: Solenoid Lock Misalignment After Thermal Cycling
| Field | Value |
|---|---|
| **Bug ID** | BUG-001 |
| **Title** | Solenoid lock bolt misaligns with strike plate after 6+ hours of operation |
| **Severity** | Medium |
| **Priority** | Medium |
| **Status** | Closed — Fixed |
| **Reported by** | Abdul Mueed Malik |
| **Date** | Integration testing phase |

### Environment
ESP32-S3, 12V solenoid lock, 3 mm acrylic door, ambient 24°C

### Description
After several hours of continuous operation, the acrylic door warps 
slightly due to the temperature difference between the cold interior 
and warm exterior. This shifts the strike plate by approximately 1 mm, 
causing the solenoid bolt to scrape against the edge of the strike 
hole rather than entering cleanly.

### Steps to Reproduce
1. Power on the refrigerator
2. Allow it to operate for 6 or more hours
3. Present an authorized RFID card
4. Observe lock actuation

### Expected Result
Bolt engages strike plate cleanly with no friction

### Actual Result
Bolt scrapes against strike plate; occasional failure to lock fully

### Root Cause
Thermal expansion coefficient of acrylic differs from the cabinet frame, 
causing differential movement of approximately 1 mm at temperature 
extremes.

### Fix Applied
Added a small stainless-steel guide plate (funnel shape) around the 
strike hole to guide the bolt into position even with minor misalignment.

### Verification
Re-tested 200 unlock cycles after fix; all 200 successful. Re-tested 
after 12-hour continuous run; alignment remained within tolerance.

## BUG-002: DHT22 Returns NaN During Heavy Wi-Fi Activity
| Field | Value |
|---|---|
| **Bug ID** | BUG-002 |
| **Title** | DHT22 sensor returns NaN values during periods of high Wi-Fi transmission |
| **Severity** | Low |
| **Priority** | Low |
| **Status** | Closed — Mitigated |
| **Reported by** | Fatima-tu-Zahra Qayyum |
| **Date** | Long-duration testing |

### Environment
ESP32-S3, DHT22, 2.4 GHz Wi-Fi active, 24-hour continuous run

### Description
During the 24-hour continuous run, the DHT22 sensor occasionally 
returned NaN (Not a Number) values. Failed reads were clustered in 
short bursts (never more than 2 consecutive) and correlated with 
periods of intense Wi-Fi transmission.

### Steps to Reproduce
1. Run system continuously
2. Trigger frequent Wi-Fi traffic (e.g., repeated dashboard refreshes)
3. Observe serial log for NaN readings

### Expected Result
All sensor reads return valid temperature and humidity values

### Actual Result
38 out of 17,280 reads (0.22%) returned NaN; bursts coincided with 
Wi-Fi activity

### Root Cause
The DHT22 uses a single-wire protocol with strict timing requirements. 
When Wi-Fi transmission interrupts the bit-banged read sequence, 
the sensor returns a checksum error, which the library reports as NaN.

### Fix Applied
Added NaN-rejection logic: if a read returns NaN, the firmware retains 
the last valid reading and displays it. The next successful read 
overwrites it. No alert is triggered for missed reads.

### Verification
After fix, the OLED and dashboard never showed a gap during the 24-hour 
test. Serial log confirmed 99.78% success rate.

### Notes
This is a known limitation of the DHT22 on microcontrollers with 
concurrent Wi-Fi. The mitigation ensures user-invisible recovery.

---

## BUG-003: ACS712 Reads Apparent Power, Not True Power
| Field | Value |
|---|---|
| **Bug ID** | BUG-003 |
| **Title** | ACS712 current sensor overreports power for inductive compressor load |
| **Severity** | Medium |
| **Priority** | Low |
| **Status** | Closed — Documented as limitation |
| **Reported by** | Saad Sohail Ansari |
| **Date** | Energy monitoring testing |

### Environment
ESP32-S3, ACS712-30A, 220V compressor load, plug-in power meter as reference

### Description
When measuring the compressor's current, the ACS712 reads the current 
magnitude but does not account for the phase angle between voltage and 
current. As a result, the calculated power (V × I) overreports the true 
power consumed.

### Steps to Reproduce
1. Connect plug-in power meter to refrigerator
2. Compare its reading with the dashboard's power display
3. Observe discrepancy during compressor operation

### Expected Result
Dashboard power estimate matches true power within ±5%

### Actual Result
Plug-in meter: 82 W (0.75 A at 220 V, PF 0.5)
ACS712 estimate: 96 VA (0.80 A apparent)
Error: approximately +17% over true power

### Root Cause
The ACS712 is a current-only sensor. True power requires simultaneous 
voltage measurement and phase-angle calculation, which the sensor 
cannot provide alone.

### Fix Applied
No hardware fix applied in this version. The dashboard displays the 
value as "indicative power" and the limitation is documented in the 
system's non-functional requirements (NFR-013).

### Verification
Resistive load test (60 W bulb) achieved ±2.2% accuracy, confirming 
the sensor works correctly for non-inductive loads.

### Future Improvement
Replace with HLW8032 or ATM90E26 energy metering IC, which measures 
both voltage and current and computes true power directly.

## BUG-004: MFRC522 Anti-Collision Timeout
| Field | Value |
|---|---|
| **Bug ID** | BUG-004 |
| **Title** | MFRC522 fails to read card when card is moved during read |
| **Severity** | Low |
| **Priority** | Low |
| **Status** | Closed — Mitigated |
| **Reported by** | Abdul Mueed Malik |
| **Date** | Long-duration testing |

### Environment
ESP32-S3, MFRC522 reader, MIFARE Classic card

### Description
On a single occasion during 24-hour testing, the MFRC522 detected a 
card present but the anti-collision sequence returned a timeout error. 
The card was not read, and the lock did not actuate.

### Steps to Reproduce
1. Present card to reader while in motion (not stationary)
2. Observe anti-collision timeout error in serial log

### Expected Result
Card UID is read and access decision is made

### Actual Result
First attempt failed; second attempt (card held stationary) succeeded

### Root Cause
MIFARE anti-collision protocol requires the card to remain in the 
reader's field during the entire sequence. Movement during read causes 
partial byte reads and a timeout.

### Fix Applied
Watchdog mechanism reinitializes the MFRC522 reader every 10 seconds, 
clearing any stuck state. Users who present the card again immediately 
succeed.

### Verification
After fix, the single failure observed over 24 hours was transparently 
recovered. No further action needed.

### Notes
This is documented MFRC522 behavior and acceptable for a domestic 
appliance. A commercial setting with high card traffic might need 
a more robust reader.

## BUG-005: Compressor Start Relay Loose (Hardware Fault Detected via Current Monitoring)
| Field | Value |
|---|---|
| **Bug ID** | BUG-005 |
| **Title** | Abnormal inrush current spike detected due to loose start relay |
| **Severity** | High |
| **Priority** | High |
| **Status** | Closed — Hardware fixed |
| **Reported by** | Saad Sohail Ansari |
| **Date** | Early performance testing |

### Environment
Prototype unit, ACS712 current sensor, serial monitor

### Description
During early testing, one compressor start produced an inrush current 
spike of approximately 7.2 A, significantly higher than the normal 
4.8–5.0 A. The compressor also emitted an unusual rattling noise. This 
was detected via the ACS712 current monitoring feature — a real-world 
demonstration of the diagnostic value of current sensing.

### Steps to Reproduce
1. Observe compressor current profile on serial monitor
2. Start compressor normally (typical 4.8–5.0 A spike)
3. In one instance, a 7.2 A spike and rattling noise occurred

### Expected Result
Compressor start spike of 4.8–5.0 A for ~150 ms

### Actual Result
7.2 A spike, accompanied by unusual noise

### Root Cause
The start relay was loose in its socket, causing arcing and erratic 
current draw during compressor startup.

### Fix Applied
Physical inspection confirmed loose relay. Relay reseated and secured. 
Compressor start spikes returned to normal (4.8–5.0 A).

### Verification
Tested 50 subsequent compressor starts; all within normal range. No 
rattling noise.

### Significance
This bug demonstrates the practical diagnostic value of the current 
monitoring feature. Without ACS712 data, this hardware fault would 
likely have progressed to compressor failure before being noticed.

## Summary
| Bug ID | Title | Severity | Status |
|---|---|---|---|
| BUG-001 | Solenoid lock misalignment | Medium | ✅ Closed |
| BUG-002 | DHT22 NaN reads | Low | ✅ Closed |
| BUG-003 | ACS712 apparent vs true power | Medium | ✅ Closed (documented) |
| BUG-004 | MFRC522 anti-collision timeout | Low | ✅ Closed (mitigated) |
| BUG-005 | Compressor start relay loose | High | ✅ Closed |

**Total: 5 bugs, all closed. Zero open.**

### Key Takeaways

- **2 bugs found through functional testing** (BUG-001, BUG-004)
- **1 bug found through accuracy testing** (BUG-003)
- **1 bug found through long-duration testing** (BUG-002)
- **1 bug found through monitoring** (BUG-005) — a real-world demonstration 
  of why the current sensor matters

Every bug has a documented root cause, fix, and verification method — 
a complete QA cycle from discovery to closure.
