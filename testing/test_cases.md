# Test Cases

---

## Legend

| Symbol | Meaning |
|---|---|
| ✅ | Pass |
| ❌ | Fail |
| ⚠️ | Pass with observation |
| — | Not run |

| Severity | Meaning |
|---|---|
| Critical | System unusable without fix |
| High | Major function impaired |
| Medium | Function degraded |
| Low | Cosmetic or minor |

---

## 1. Unit Test Cases

### UT-01: RFID Reader — Authorized Card

| Field | Value |
|---|---|
| Test ID | UT-01 |
| Requirement | FR-002, FR-003 |
| Subsystem | MFRC522 RFID reader |
| Preconditions | System powered, authorized card available |
| Steps | 1. Present authorized MIFARE Classic card to reader<br>2. Observe serial output |
| Expected Result | Card UID is read correctly; serial log shows "ACCESS GRANTED" |
| Actual Result | UID E4:D8:F3:06 read; "ACCESS GRANTED" logged |
| Status | ✅ Pass |
| Severity | Critical |

### UT-02: RFID Reader — Unauthorized Card

| Field | Value |
|---|---|
| Test ID | UT-02 |
| Requirement | FR-003 |
| Subsystem | MFRC522 RFID reader |
| Preconditions | System powered, unauthorized card available |
| Steps | 1. Present unauthorized card to reader<br>2. Observe serial output |
| Expected Result | UID read; serial log shows "ACCESS DENIED"; lock remains closed |
| Actual Result | UID read; "ACCESS DENIED" logged; lock remained locked |
| Status | ✅ Pass |
| Severity | Critical |

### UT-03: Solenoid Lock Actuation

| Field | Value |
|---|---|
| Test ID | UT-03 |
| Requirement | FR-004 |
| Subsystem | 12V solenoid lock + relay |
| Preconditions | Test sketch uploaded toggling relay every 10 s |
| Steps | 1. Trigger relay via test sketch<br>2. Observe lock bolt retract<br>3. Verify de-energization after 3 s |
| Expected Result | Bolt retracts fully; current draw ~380 mA at 12V; bolt returns after 3 s |
| Actual Result | Retracts fully; 380 mA measured; returns within 50 ms |
| Status | ✅ Pass |
| Severity | High |

### UT-04: DHT22 Accuracy

| Field | Value |
|---|---|
| Test ID | UT-04 |
| Requirement | FR-005, NFR-011, NFR-012 |
| Subsystem | DHT22 sensor |
| Preconditions | Calibrated reference thermometer-hygrometer placed adjacent |
| Steps | 1. Stabilize at 5 test points: 25°C, 15°C, 5°C, 1°C, 10°C<br>2. At each, wait 30 min, take 10 readings at 1-min intervals<br>3. Compare averages |
| Expected Result | Temp error ≤ ±0.5°C; humidity error ≤ ±5% RH |
| Actual Result | Temp error mean −0.22°C; humidity error mean +4% RH |
| Status | ✅ Pass |
| Severity | High |

### UT-05: IR Door Sensor

| Field | Value |
|---|---|
| Test ID | UT-05 |
| Requirement | FR-007 |
| Subsystem | IR obstacle sensor |
| Preconditions | Door closed, sensor mounted and adjusted |
| Steps | 1. With door closed, read GPIO 15<br>2. Slowly open door<br>3. Record threshold at which signal changes |
| Expected Result | Signal LOW when closed; HIGH when door >5 mm from frame |
| Actual Result | Signal HIGH at approximately 5 mm gap; LOW when fully closed |
| Status | ✅ Pass |
| Severity | High |

---

## 2. Integration Test Cases

### IT-01: First Boot + Wi-Fi Configuration

| Field | Value |
|---|---|
| Test ID | IT-01 |
| Requirement | FR-011, FR-012 |
| Preconditions | Fresh firmware, no stored credentials |
| Steps | 1. Power on device<br>2. Connect phone to `Fridge_Setup` AP<br>3. Open browser to 192.168.4.1<br>4. Select home network, enter password<br>5. Save & reboot |
| Expected Result | AP starts; config page loads; device connects to home network after reboot |
| Actual Result | AP detected in 3–5 s; page loaded in 1.8 s; connected successfully |
| Status | ✅ Pass |
| Severity | Critical |

### IT-02: Normal Operation Display

| Field | Value |
|---|---|
| Test ID | IT-02 |
| Requirement | FR-005, FR-006, FR-014 |
| Preconditions | System connected to Wi-Fi |
| Steps | 1. Observe OLED for 2 minutes |
| Expected Result | Time displayed; temp/humidity update every 5 s; "SCAN CARD" blinks every 1.2 s |
| Actual Result | All elements correct; NTP time synced within 5 s of boot |
| Status | ✅ Pass |
| Severity | High |

### IT-03: Authorized RFID Scan

| Field | Value |
|---|---|
| Test ID | IT-03 |
| Requirement | FR-002, FR-003, FR-004 |
| Preconditions | System idle, ready screen |
| Steps | 1. Present authorized card<br>2. Observe OLED, lock, buzzer |
| Expected Result | OLED: "ACCESS GRANTED"; lock opens for 3 s; happy two-tone beep; returns to ready |
| Actual Result | All behaviors correct; lock opened and closed within 3 s |
| Status | ✅ Pass |
| Severity | Critical |

### IT-04: Unauthorized RFID Scan

| Field | Value |
|---|---|
| Test ID | IT-04 |
| Requirement | FR-003 |
| Preconditions | System idle |
| Steps | 1. Present unauthorized card |
| Expected Result | OLED: "ACCESS DENIED"; lock stays closed; grumpy descending tone |
| Actual Result | All behaviors correct; door remained locked |
| Status | ✅ Pass |
| Severity | Critical |

### IT-05: Door Open < 30 Seconds

| Field | Value |
|---|---|
| Test ID | IT-05 |
| Requirement | FR-007, FR-008, FR-014 |
| Preconditions | System in normal mode |
| Steps | 1. Open door<br>2. Keep open for 20 s<br>3. Close door |
| Expected Result | No alert; normal display resumes immediately after close |
| Actual Result | No alert triggered; timer reset on close |
| Status | ✅ Pass |
| Severity | High |

### IT-06: Door Open > 30 Seconds

| Field | Value |
|---|---|
| Test ID | IT-06 |
| Requirement | FR-008 |
| Preconditions | System in normal mode |
| Steps | 1. Open door<br>2. Keep open for 35 s<br>3. Observe alert<br>4. Close door |
| Expected Result | Silent 30 s; then intermittent beep + blinking OLED; stops within 400 ms of close |
| Actual Result | Alert triggered at 30.1 s; beeping and blinking as designed; stopped in <200 ms |
| Status | ✅ Pass |
| Severity | High |

### IT-07: Web Dashboard Load

| Field | Value |
|---|---|
| Test ID | IT-07 |
| Requirement | FR-009 |
| Preconditions | Laptop connected to `MiniFridge_Local` AP |
| Steps | 1. Open browser to 192.168.4.1<br>2. Observe dashboard rendering |
| Expected Result | Page loads within 2 s; live data displayed; energy gauge visible |
| Actual Result | Loaded in 1.2 s; data refreshed every 2 s |
| Status | ✅ Pass |
| Severity | High |

### IT-08: Manual Unlock from Dashboard

| Field | Value |
|---|---|
| Test ID | IT-08 |
| Requirement | FR-013 |
| Preconditions | Dashboard open |
| Steps | 1. Click "Unlock" button<br>2. Observe lock + dashboard status |
| Expected Result | Lock opens for 3 s; status updates within 2 s |
| Actual Result | Lock opened within ~500 ms; status updated correctly |
| Status | ✅ Pass |
| Severity | Medium |

### IT-09: Wi-Fi Reset from Dashboard

| Field | Value |
|---|---|
| Test ID | IT-09 |
| Requirement | FR-011, FR-012 |
| Preconditions | Dashboard open |
| Steps | 1. Click "Reset Wi-Fi Settings"<br>2. Observe device reboot into config mode |
| Expected Result | Credentials wiped from SPIFFS; device returns to config mode |
| Actual Result | Credentials removed; AP `Fridge_Setup` broadcast within 3 s |
| Status | ✅ Pass |
| Severity | Medium |

### IT-10: RFID Watchdog Recovery

| Field | Value |
|---|---|
| Test ID | IT-10 |
| Requirement | NFR-008 |
| Preconditions | System operating |
| Steps | 1. Simulate RFID bus hang (rapid card presentation)<br>2. Wait for watchdog reinit<br>3. Present card again |
| Expected Result | Reader recovers within 10 s; next read succeeds |
| Actual Result | Recovery in ~8 s; next read successful |
| Status | ✅ Pass |
| Severity | Medium |

---

## 3. Performance Test Cases

### PERF-01: RFID Read-to-Unlock Latency

| Field | Value |
|---|---|
| Test ID | PERF-01 |
| Requirement | NFR-001 |
| Preconditions | Stopwatch ready; 50 trials |
| Steps | 1. Present card<br>2. Time until audible lock click |
| Expected Result | ≤ 600 ms |
| Actual Result | Mean 478 ms; 95th percentile 562 ms; max 595 ms |
| Status | ✅ Pass |
| Severity | High |

### PERF-02: Dashboard Endpoint Latency

| Field | Value |
|---|---|
| Test ID | PERF-02 |
| Requirement | NFR-003 |
| Preconditions | 500 consecutive requests to `/data` |
| Steps | 1. Measure request-to-complete-response time |
| Expected Result | ≤ 100 ms mean |
| Actual Result | Mean 34 ms; 95th percentile 52 ms; max 98 ms; 0 failures |
| Status | ✅ Pass |
| Severity | Medium |

### PERF-03: Door-Ajar Alert Timing

| Field | Value |
|---|---|
| Test ID | PERF-03 |
| Requirement | NFR-004 |
| Preconditions | 10 trials |
| Steps | 1. Open door<br>2. Time until first beep |
| Expected Result | 30.0 s ± 200 ms |
| Actual Result | Mean 30.12 s; std dev 0.08 s |
| Status | ✅ Pass |
| Severity | High |

### PERF-04: DHT22 Read Success Rate

| Field | Value |
|---|---|
| Test ID | PERF-04 |
| Requirement | NFR-007 |
| Preconditions | 24-hour run, polling every 5 s |
| Steps | 1. Count total polls and valid reads |
| Expected Result | ≥ 99% success |
| Actual Result | 17,242 / 17,280 = 99.78% |
| Status | ✅ Pass |
| Severity | High |

### PERF-05: Solenoid Endurance

| Field | Value |
|---|---|
| Test ID | PERF-05 |
| Requirement | NFR-010 |
| Preconditions | Automated test sketch, 200 cycles |
| Steps | 1. Trigger relay every 10 s (3 s on)<br>2. Count successful retract/extend cycles |
| Expected Result | 200/200 successful |
| Actual Result | 200/200 successful; solenoid temp rose from 24°C to 31°C |
| Status | ✅ Pass |
| Severity | Medium |

---

## 4. Regression Test Cases

### REG-01: RFID After Dashboard Fix

| Field | Value |
|---|---|
| Test ID | REG-01 |
| Requirement | FR-002 |
| Preconditions | After dashboard latency fix |
| Steps | 1. Present authorized card<br>2. Confirm unlock still works |
| Expected Result | Unlock latency unchanged (≤ 600 ms) |
| Actual Result | 482 ms (within tolerance) |
| Status | ✅ Pass |
| Severity | High |

### REG-02: Door Alert After OLED Fix

| Field | Value |
|---|---|
| Test ID | REG-02 |
| Requirement | FR-008 |
| Preconditions | After OLED I²C bus contention fix |
| Steps | 1. Open door > 30 s<br>2. Confirm alert sequence intact |
| Expected Result | Beep + blink trigger at 30 s; stop on close |
| Actual Result | All behaviors correct |
| Status | ✅ Pass |
| Severity | High |

### REG-03: Wi-Fi Reconnect After Credential Fix

| Field | Value |
|---|---|
| Test ID | REG-03 |
| Requirement | FR-012 |
| Preconditions | After SPIFFS write fix |
| Steps | 1. Reset Wi-Fi<br>2. Reconfigure<br>3. Reboot |
| Expected Result | Credentials persist across reboots |
| Actual Result | Reconnected automatically after reboot |
| Status | ✅ Pass |
| Severity | Medium |

---

## 5. Stability Test

### STAB-01: 24-Hour Continuous Run

| Field | Value |
|---|---|
| Test ID | STAB-01 |
| Requirement | NFR-008, NFR-009 |
| Preconditions | Full system, home environment, 12 authorized + 5 unauthorized scans over run |
| Steps | 1. Power on<br>2. Run continuously for 24 hours<br>3. Log serial output |
| Expected Result | No crashes, freezes, or Wi-Fi dropouts |
| Actual Result | No crashes; no Wi-Fi dropouts; RFID reader recovered from 1 timeout via watchdog |
| Status | ✅ Pass |
| Severity | Critical |

---

## Test Summary

| Category | Total | Pass | Fail |
|---|---|---|---|
| Unit | 5 | 5 | 0 |
| Integration | 10 | 10 | 0 |
| Performance | 5 | 5 | 0 |
| Regression | 3 | 3 | 0 |
| Stability | 1 | 1 | 0 |
| **Total** | **24** | **24** | **0** |

**Pass Rate: 100%**

*Note: 8 additional exploratory tests were executed but not documented as 
formal cases. All produced expected results.*

---

