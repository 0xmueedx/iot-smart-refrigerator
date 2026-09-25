# Non-Functional Requirements

Non-functional requirements define quality attributes — how well the system performs its functions. Each has a measurable target and a recorded measurement from system testing.

---

## 1. Performance Requirements

| ID | Requirement | Target | Measured | Status |
|---|---|---|---|---|
| NFR-001 | RFID card read-to-unlock latency shall not exceed 600 ms | ≤ 600 ms | 478 ms (mean, n=50) | ✅ Pass |
| NFR-002 | OLED screen refresh interval shall not exceed 2 seconds | ≤ 2 s | ~2 s | ✅ Pass |
| NFR-003 | Dashboard `/data` endpoint response time shall not exceed 100 ms | ≤ 100 ms | 34 ms (mean, n=500) | ✅ Pass |
| NFR-004 | Door-ajar alert shall trigger within 200 ms of the 30s threshold | ≤ 200 ms | 120 ms | ✅ Pass |
| NFR-005 | Wi-Fi setup portal shall load within 3 seconds | ≤ 3 s | 1.8 s | ✅ Pass |

---

## 2. Reliability Requirements

| ID | Requirement | Target | Measured | Status |
|---|---|---|---|---|
| NFR-006 | RFID access decision accuracy shall be 100% | 100% | 100% (n=100) | ✅ Pass |
| NFR-007 | DHT22 sensor read success rate shall exceed 99% | ≥ 99% | 99.78% (24 hr) | ✅ Pass |
| NFR-008 | System shall operate continuously for 24 hours without crash or reset | 24 hr | 24 hr passed | ✅ Pass |
| NFR-009 | Wi-Fi connection shall remain stable with no dropouts over 24 hours | 0 dropouts | 0 dropouts | ✅ Pass |
| NFR-010 | Solenoid lock shall complete 200 consecutive cycles without failure | 200 cycles | 200/200 passed | ✅ Pass |

---

## 3. Accuracy Requirements

| ID | Requirement | Target | Measured | Status |
|---|---|---|---|---|
| NFR-011 | DHT22 temperature accuracy vs reference shall be within ±0.5°C | ±0.5°C | −0.22°C (mean) | ✅ Pass |
| NFR-012 | DHT22 humidity accuracy vs reference shall be within ±5% RH | ±5% RH | +4% RH (mean) | ✅ Pass |
| NFR-013 | ACS712 current accuracy (resistive load) shall be within ±5% | ±5% | −2.2% | ✅ Pass |

---

## 4. Usability Requirements

| ID | Requirement | Target | Status |
|---|---|---|---|
| NFR-014 | Dashboard shall render correctly on mobile, tablet, and desktop browsers | Responsive CSS | ✅ Verified |
| NFR-015 | OLED display shall be legible at 1.5 m viewing distance in indoor lighting | Legible at 1.5 m | ✅ Verified |
| NFR-016 | Wi-Fi configuration shall be completable by a non-technical user in under 3 minutes | ≤ 3 min | ~2 min | ✅ Verified |

---

## 5. Security & Privacy Requirements

| ID | Requirement | Implementation |
|---|---|---|
| NFR-017 | Wi-Fi credentials shall be stored locally only and never transmitted off-device | SPIFFS storage; no external transmission |
| NFR-018 | No personally identifiable information shall be collected or logged | Only anonymous RFID UIDs are logged |
| NFR-019 | Unauthorized RFID cards shall not unlock the door under any condition | 100% denial observed (n=50) |
| NFR-020 | Mains-voltage wiring shall be physically isolated from low-voltage circuitry | Separate junction box |

---

## 6. Maintainability Requirements

| ID | Requirement | Implementation |
|---|---|---|
| NFR-021 | Firmware shall be modular, with clear separation of concerns | Header-based modules: RFID, sensors, web, display |
| NFR-022 | Authorized RFID UID list shall be modifiable without hardware changes | Compile-time constant array |

---

## 7. Portability Requirements

| ID | Requirement | Target | Status |
|---|---|---|---|
| NFR-023 | System shall be adaptable to any small vapor-compression cooling unit | Demonstrated on water dispenser | ✅ Verified |
| NFR-024 | Dashboard shall function on any modern browser without external dependencies | No CDN; self-contained HTML/CSS/JS | ✅ Verified |

---

## 8. Known Limitations

The following limitations are acknowledged and accepted in this version:

- **ACS712 accuracy degrades for inductive loads** — measured error increases from ~2% (resistive) to ~8% (compressor) due to phase shift. Energy figures are indicative, not precise.
- **Dashboard accessible only on local network** — no cloud connectivity; remote access not supported.
- **Authorized UID list is fixed in firmware** — no runtime card management interface.
- **OLED display is small** (0.96 inch) — may be difficult to read for users with visual impairment; dashboard provides larger alternative.
- **Compressor control is not overridden** — mechanical thermostat remains in sole control of cycling.

---
