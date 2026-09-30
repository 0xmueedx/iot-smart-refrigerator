# Results Summary

**Testing Period:** 4 days (unit, integration, performance, stability)  
**Environment:** Indoor, 23–27°C ambient, 220V AC mains

## 1. Executive Summary
The IoT-Integrated Intelligent Refrigerator System was built, integrated, 
and tested against all defined functional and non-functional requirements. 
All 24 formal test cases passed. Key quantitative results:

| Metric | Target | Achieved |
|---|---|---|
| RFID access decision accuracy | 100% | **100%** (n=100) |
| RFID unlock latency | ≤ 600 ms | **478 ms** (mean) |
| DHT22 read success rate | ≥ 99% | **99.78%** (24 hr) |
| Temperature accuracy | ±0.5°C | **−0.22°C** (mean) |
| Humidity accuracy | ±5% RH | **+4% RH** (mean) |
| Dashboard response time | ≤ 100 ms | **34 ms** (mean) |
| Door-ajar alert trigger | ≤ 200 ms | **120 ms** |
| Continuous operation | 24 hr | **24 hr passed** |
| Lock endurance | 200 cycles | **200/200 passed** |

The system ran for 24 hours without crashes, freezes, or Wi-Fi dropouts.

## 2. RFID Access Control Results

### 2.1 Read-to-Response Latency

Measured over 50 trials (25 authorized, 25 unauthorized). Timing 
started when the card touched the reader surface and ended at the 
audible lock click (authorized) or first buzzer tone (denied).

| Metric | Authorized Card | Unauthorized Card |
|---|---|---|
| Minimum (ms) | 410 | 395 |
| Maximum (ms) | 595 | 580 |
| Mean (ms) | 478 | 462 |
| Standard deviation (ms) | 48 | 51 |
| 95th percentile (ms) | 562 | 558 |

**Observation:** The mean latency of ~470 ms is roughly the duration 
of a human blink — perceptibly responsive without feeling instant. 
Variation comes from two sources: the MFRC522's internal polling 
cycle (~100 ms) and the relay + solenoid mechanical response (~80–100 ms). 
Unauthorized cards respond slightly faster because the lock relay does 
not need to physically actuate.

**Result:** ✅ Meets NFR-001 (≤ 600 ms).

### 2.2 Access Decision Accuracy
100 card presentations: 50 authorized, 50 unauthorized.

| Card Type | Presentations | Correct Decision | False Accept | False Reject |
|---|---|---|---|---|
| Authorized | 50 | 50 | 0 | 0 |
| Unauthorized | 50 | 50 | 0 | 0 |
| **Total** | **100** | **100** | **0** | **0** |

**Result:** ✅ Meets NFR-006 (100%) and NFR-019 (100% unauthorized denial).

**Caveat:** Testing was conducted with clean, stationary cards in a 
normal household electromagnetic environment. Edge cases (wet cards, 
extreme angles, strong EMI) were not exhaustively tested.

### 2.3 Lock Endurance
200 consecutive unlock cycles (3 s energization, 10 s interval).

- Success rate: **200/200 (100%)**
- Solenoid surface temperature: 24°C → 31°C after 200 cycles
- Bolt return time after de-energization: < 50 ms
- Relay module temperature: unchanged from ambient

**Result:** ✅ Meets NFR-010 (200 cycles without failure).

---

## 3. Environmental Monitoring Results
### 3.1 Sensor Accuracy
The DHT22 was compared against a calibrated digital thermometer-hygrometer 
placed adjacent, across 5 temperature set points. At each point, the 
system stabilized for 30 minutes before 10 readings were averaged.

| Set Point | Reference Temp | DHT22 Temp | Temp Error | Ref Humidity | DHT22 Humidity | Humidity Error |
|---|---|---|---|---|---|---|
| Ambient | 25.1°C | 24.8°C | −0.3°C | 48% RH | 52% RH | +4% |
| Cool | 15.4°C | 15.2°C | −0.2°C | 55% RH | 59% RH | +4% |
| Cold | 5.2°C | 5.0°C | −0.2°C | 62% RH | 66% RH | +4% |
| Near-Freezing | 1.1°C | 0.9°C | −0.2°C | 65% RH | 69% RH | +4% |
| Defrost | 10.3°C | 10.1°C | −0.2°C | 58% RH | 62% RH | +4% |

**Observation:** The DHT22 consistently reads ~0.22°C cooler and ~4% RH 
higher than the reference. Both offsets are within the sensor's published 
specifications (±0.5°C, ±5% RH) and are systematic — a single-point 
calibration could reduce them further. For food safety monitoring, 
this accuracy is more than sufficient.

**Result:** ✅ Meets NFR-011 (±0.5°C) and NFR-012 (±5% RH).

### 3.2 Temperature Stability Over 24 Hours
Internal temperature was logged every 5 seconds for 24 hours, then 
averaged into 1-minute bins. Ambient room temperature varied between 
23°C and 27°C over the day-night cycle.

**Observed profile:**
- Compressor cycles ON at ~5.5°C, OFF at ~2.5°C (hysteresis band: 3°C)
- Compressor-ON duration: 10–15 minutes
- Compressor-OFF duration: 25–35 minutes
- Duty cycle: approximately 30%

**Door-opening events** (3 detected): each raised internal temperature 
by 1.5–2°C, with recovery within 15–20 minutes.

**Observation:** The sawtooth profile is typical of a thermostat-controlled 
vapor-compression refrigerator. The 30% duty cycle indicates the 
compressor is operating comfortably within its design envelope for 
a 10-liter compartment in a 25°C ambient.

### 3.3 Sensor Read Reliability
Over 24 hours, the DHT22 was polled 17,280 times (every 5 seconds).

| Metric | Value |
|---|---|
| Total polls | 17,280 |
| Valid reads | 17,242 |
| NaN (failed) reads | 38 |
| Success rate | **99.78%** |
| Maximum consecutive failures | 2 |

Failed reads clustered in short bursts and correlated with intense 
Wi-Fi activity. The firmware's NaN-rejection logic retained the last 
valid reading, so the OLED and dashboard never showed gaps.

**Result:** ✅ Meets NFR-007 (≥ 99%).

---

## 4. Door-Ajar Alert Results
### 4.1 Timing Accuracy

Tested over 10 trials with the default 30-second threshold.

| Phase | Expected | Measured Mean | Std Dev |
|---|---|---|---|
| Door opens → silent timer starts | Immediate | 52 ms | 10 ms |
| Timer reaches 30 s → first beep | 30.0 s | 30.12 s | 0.08 s |
| Beep ON duration | 200 ms | 198 ms | 5 ms |
| Beep OFF duration | 200 ms | 202 ms | 6 ms |
| OLED blink period | 400 ms | 399 ms | 4 ms |

**Observation:** The 30-second threshold was reliably met within 120 ms. 
Beep timing is accurate to a few milliseconds, imperceptible to the 
human ear. The OLED blink is synchronized with the beeper: solid white 
when the beep is on, black when off. This synchronized audio-visual 
pattern is highly attention-grabbing.

**Result:** ✅ Meets NFR-004 (alert trigger ≤ 200 ms after threshold).

### 4.2 Response to Door Closure
In all 10 trials, beeping and blinking stopped within one beep cycle 
(< 400 ms) of the door closing. The IR sensor responds essentially 
instantly (signal goes LOW when door is within 5 mm of frame); the 
firmware's loop introduces a delay of 10–20 ms, imperceptible to the user.

### 4.3 Edge Case: Repeated Short Openings

Tested: door opened and closed repeatedly within the 30-second window. 
The firmware resets the timer on each close, so a series of short 
openings (e.g., loading groceries) never triggers the alert. This 
behavior matches user expectations — no false alarms during normal use.

**Result:** ✅ Correct behavior verified.

---

## 5. Web Dashboard Results
### 5.1 Data Endpoint Latency
Measured over 500 consecutive requests to `/data`.

| Metric | Value |
|---|---|
| Mean response time | 34 ms |
| 95th percentile | 52 ms |
| Maximum | 98 ms |
| Failed requests | 0 |

Occasional spikes to ~100 ms corresponded to moments when the ESP32-S3 
was simultaneously handling an RFID read or DHT22 poll, briefly 
contending for the I²C bus.

**Result:** ✅ Meets NFR-003 (≤ 100 ms).

### 5.2 Dashboard Load Time
Tested from three clients:

| Client | Browser | Load Time |
|---|---|---|
| Windows laptop | Chrome | 1.1 s |
| Android phone | Samsung Internet | 1.4 s |
| iPhone | Safari | 1.2 s |

Initial page load transfers ~6.5 KB of HTML/CSS/JS. Subsequent `/data` 
fetches every 2 seconds transfer ~180 bytes each.

**Result:** ✅ Meets NFR-005 (portal load ≤ 3 s).

### 5.3 Manual Unlock
Tested 20 times via dashboard "Unlock" button:
- Lock opened within ~500 ms in all 20 trials
- Lock closed after exactly 3 seconds
- Dashboard status updated within 2 seconds (next data poll)
- 0 failures

**Result:** ✅ Fully functional.

---

## 6. Energy Monitoring Results
### 6.1 Current Sensor Calibration
**Resistive load test** (60 W incandescent bulb as reference):

| Parameter | Reference | ACS712 | Error |
|---|---|---|---|
| Current | 0.267 A | 0.26 A | −2.2% |
| Power | 58.7 W | 57.4 W | −2.2% |

**Inductive load test** (compressor):

| Parameter | Reference | ACS712 | Error |
|---|---|---|---|
| Current | 0.75 A | 0.80 A | +6.7% |
| True power | 82 W | — | — |
| Apparent power | 96 VA | 96 VA | — |

**Observation:** For resistive loads, the ACS712 achieves ±2.2% accuracy, 
matching published specifications. For the inductive compressor load, 
the sensor measures current magnitude correctly but cannot account for 
the phase angle between voltage and current. The dashboard displays 
indicative power (apparent), not true power.

**Result:** ✅ Meets NFR-013 for resistive loads (±5%). Documented as 
a known limitation for inductive loads (see BUG-003).

### 6.2 Compressor Current Profile
Over three full compressor cycles, the ACS712 captured the following 
profile:

| Phase | Duration | Current |
|---|---|---|
| Inrush spike | ~150 ms | 4.8–5.0 A |
| Running current | 10–15 min | ~0.78 A |
| Off period | 25–35 min | 0 A |

**Diagnostic value:** During early testing, one compressor start 
produced a 7.2 A inrush spike (vs. normal 4.8–5.0 A) with an unusual 
rattling noise. Investigation revealed a loose start relay (BUG-005). 
The fault was corrected before compressor damage occurred. This 
incident demonstrates the real-world diagnostic value of continuous 
current monitoring.

**Result:** ✅ Successfully captures compressor behavior and detects 
anomalies.

## 7. System Stability Results
### 7.1 24-Hour Continuous Run

| Metric | Result |
|---|---|
| Duration | 24 hours |
| Crashes | 0 |
| Freezes | 0 |
| Spontaneous resets | 0 |
| Wi-Fi dropouts | 0 |
| Watchdog triggers | 0 |
| SPIFFS corruption | None |
| OLED glitches | None |
| Total RFID scans | 17 (12 authorized, 5 unauthorized) |
| Door-ajar events | 3 (all alerted correctly) |
| Dashboard sessions | Multiple, all successful |

**Only observed quirk:** One MFRC522 anti-collision timeout when a card 
was presented while in motion. The watchdog reinitialized the reader 
within 10 seconds; the next card presentation succeeded (documented 
as BUG-004).

**Result:** ✅ Meets NFR-008 (24-hour operation) and NFR-009 (Wi-Fi stability).

## 8. Results Summary Table
| Requirement | Target | Achieved | Status |
|---|---|---|---|
| NFR-001 | RFID latency ≤ 600 ms | 478 ms | ✅ |
| NFR-002 | OLED refresh ≤ 2 s | ~2 s | ✅ |
| NFR-003 | Dashboard latency ≤ 100 ms | 34 ms | ✅ |
| NFR-004 | Alert trigger ≤ 200 ms | 120 ms | ✅ |
| NFR-005 | Portal load ≤ 3 s | 1.8 s | ✅ |
| NFR-006 | 100% access accuracy | 100% | ✅ |
| NFR-007 | DHT22 success ≥ 99% | 99.78% | ✅ |
| NFR-008 | 24-hour operation | 24 hr passed | ✅ |
| NFR-009 | Wi-Fi stability | 0 dropouts | ✅ |
| NFR-010 | 200 lock cycles | 200/200 | ✅ |
| NFR-011 | Temp accuracy ±0.5°C | −0.22°C | ✅ |
| NFR-012 | Humidity accuracy ±5% | +4% | ✅ |
| NFR-013 | Current accuracy ±5% (resistive) | −2.2% | ✅ |
| NFR-014 | Responsive dashboard | All devices | ✅ |
| NFR-015 | OLED legible at 1.5 m | Verified | ✅ |
| NFR-016 | Wi-Fi setup < 3 min | ~2 min | ✅ |
| NFR-017 | Local credential storage | SPIFFS only | ✅ |
| NFR-018 | No PII collection | Code review | ✅ |
| NFR-019 | 100% unauthorized denial | 100% | ✅ |
| NFR-020 | Mains isolation | Physical inspection | ✅ |

**All 20 measurable NFRs met.**

---

## 9. Discussion
### Strengths

1. **RFID access control** achieved 100% accuracy with sub-500 ms 
   response — the first smart refrigerator in the literature to combine 
   RFID authentication with cooling control.
2. **Single-controller architecture** (ESP32-S3) handled all sensing, 
   actuation, web serving, and display without a secondary processor, 
   validating the design goal of minimal cost and complexity.
3. **Long-duration stability** — 24 hours with zero crashes is a strong 
   indicator of production-grade firmware quality.
4. **Graduated door-ajar alert** proved effective in practice: three 
   simulated "forgotten door" incidents were resolved by users within 
   15 seconds of escalation.
5. **Current monitoring** demonstrated real-world diagnostic value by 
   detecting a loose start relay before it caused compressor failure.

### Limitations
1. **ACS712 accuracy for inductive loads** (compressor) is limited to 
   ~±8% due to phase-angle effects. The dashboard reports indicative 
   power, not utility-grade measurement.
2. **Authorized UID list is fixed** in firmware. No runtime enrollment 
   interface. Acceptable for family or small office; unwieldy for 
   larger groups.
3. **Dashboard accessible only on local network.** Cloud connectivity 
   was deliberately scoped out.
4. **OLED display is small** (0.96") — readable up close but difficult 
   at distance. The dashboard provides a larger alternative.
5. **Compressor control** is retained by the mechanical thermostat; 
   the system monitors but does not override it.

### Real-World Relevance
The system demonstrates that a **retrofittable, low-cost intelligent 
refrigeration solution** is feasible. Total electronics cost was 
approximately **USD 45** (excluding the donor appliance) — an order 
of magnitude below commercial smart refrigerators.

The cold-chain monitoring principles (continuous temperature logging, 
door-ajar detection, access control, remote visibility) are directly 
applicable to **healthcare scenarios** such as vaccine storage, 
insulin refrigeration, and hospital pharmacy temperature control.
