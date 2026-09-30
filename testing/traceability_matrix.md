# Requirements Traceability Matrix (RTM)

This matrix maps every requirement to the test case(s) that verify it. 
It ensures complete test coverage and provides audit-ready evidence that 
each requirement has been validated.

## 1. Functional Requirements → Test Cases
| Req ID | Requirement Summary | Test Cases | Status |
|---|---|---|---|
| FR-001 | Cooling system adaptation | Verified by physical inspection + IT-02 | ✅ Covered |
| FR-002 | RFID access control (hardware) | UT-01, IT-03 | ✅ Covered |
| FR-003 | Authorized/unauthorized decision | UT-01, UT-02, IT-03, IT-04 | ✅ Covered |
| FR-004 | 3-second lock actuation | UT-03, IT-03, IT-08 | ✅ Covered |
| FR-005 | Temperature & humidity monitoring | UT-04, IT-02, PERF-04 | ✅ Covered |
| FR-006 | OLED display of readings | IT-02 | ✅ Covered |
| FR-007 | Door-ajar detection | UT-05, IT-05, IT-06 | ✅ Covered |
| FR-008 | Escalating door-ajar alert | IT-06, PERF-03, REG-02 | ✅ Covered |
| FR-009 | Local web dashboard | IT-07, PERF-02 | ✅ Covered |
| FR-010 | Compressor current measurement | Verified in §04_Results (calibration) | ✅ Covered |
| FR-011 | Wi-Fi configuration portal | IT-01, IT-09 | ✅ Covered |
| FR-012 | Credential persistence in SPIFFS | IT-01, IT-09, REG-03 | ✅ Covered |
| FR-013 | Manual unlock via dashboard | IT-08 | ✅ Covered |
| FR-014 | NTP time display | IT-02 | ✅ Covered |

## 2. Non-Functional Requirements → Test Cases
| Req ID | Requirement Summary | Test Cases | Status |
|---|---|---|---|
| NFR-001 | RFID unlock latency ≤ 600 ms | PERF-01 | ✅ Covered |
| NFR-002 | OLED refresh ≤ 2 s | IT-02 | ✅ Covered |
| NFR-003 | Dashboard latency ≤ 100 ms | PERF-02 | ✅ Covered |
| NFR-004 | Alert trigger ≤ 200 ms after threshold | PERF-03 | ✅ Covered |
| NFR-005 | Config portal load ≤ 3 s | IT-01 | ✅ Covered |
| NFR-006 | 100% access decision accuracy | UT-01, UT-02, IT-03, IT-04 | ✅ Covered |
| NFR-007 | DHT22 success ≥ 99% | PERF-04 | ✅ Covered |
| NFR-008 | 24-hour continuous operation | STAB-01 | ✅ Covered |
| NFR-009 | Wi-Fi stability over 24 hr | STAB-01 | ✅ Covered |
| NFR-010 | 200 lock cycles without failure | PERF-05 | ✅ Covered |
| NFR-011 | Temp accuracy ±0.5°C | UT-04 | ✅ Covered |
| NFR-012 | Humidity accuracy ±5% RH | UT-04 | ✅ Covered |
| NFR-013 | Current accuracy ±5% (resistive) | §04_Results calibration | ✅ Covered |
| NFR-014 | Dashboard responsive on all devices | IT-07 | ✅ Covered |
| NFR-015 | OLED legible at 1.5 m | IT-02 | ✅ Covered |
| NFR-016 | Wi-Fi setup < 3 min by non-tech user | UAT-01 | ✅ Covered |
| NFR-017 | Credentials stored locally only | IT-01, IT-09 | ✅ Covered |
| NFR-018 | No PII collected | Verified by code review | ✅ Covered |
| NFR-019 | 100% unauthorized denial | UT-02, IT-04 | ✅ Covered |
| NFR-020 | Mains/low-voltage isolation | Physical inspection + safety review | ✅ Covered |
| NFR-021 | Modular firmware | Verified by code structure review | ✅ Covered |
| NFR-022 | UID list modifiable without hardware change | Design review | ✅ Covered |
| NFR-023 | Portable to other cooling units | Design verification | ✅ Covered |
| NFR-024 | Dashboard on any modern browser | IT-07 (Chrome, Safari, Samsung) | ✅ Covered |

## 3. Test Case → Requirement Reverse Mapping
| Test ID | Requirements Verified |
|---|---|
| UT-01 | FR-002, FR-003, NFR-006 |
| UT-02 | FR-003, NFR-006, NFR-019 |
| UT-03 | FR-004 |
| UT-04 | FR-005, NFR-011, NFR-012 |
| UT-05 | FR-007 |
| IT-01 | FR-011, FR-012, NFR-005, NFR-017 |
| IT-02 | FR-005, FR-006, FR-014, NFR-002, NFR-015 |
| IT-03 | FR-002, FR-003, FR-004, NFR-006 |
| IT-04 | FR-003, NFR-006, NFR-019 |
| IT-05 | FR-007, FR-014 |
| IT-06 | FR-008, NFR-004 |
| IT-07 | FR-009, NFR-014, NFR-024 |
| IT-08 | FR-004, FR-013 |
| IT-09 | FR-011, FR-012, NFR-017 |
| IT-10 | NFR-008 |
| PERF-01 | NFR-001 |
| PERF-02 | NFR-003 |
| PERF-03 | NFR-004 |
| PERF-04 | NFR-007 |
| PERF-05 | NFR-010 |
| REG-01 | FR-002 |
| REG-02 | FR-008 |
| REG-03 | FR-012 |
| STAB-01 | NFR-008, NFR-009 |

## 4. Coverage Summary
| Category | Total Requirements | Fully Covered | Partial | Not Covered |
|---|---|---|---|---|
| Functional (FR) | 14 | 14 | 0 | 0 |
| Non-Functional (NFR) | 24 | 24 | 0 | 0 |
| **Total** | **38** | **38** | **0** | **0** |

**Coverage: 100%**

## 5. Coverage Gaps & Notes
- **FR-001** (cooling system adaptation) is verified by physical 
  inspection rather than a formal test case, since it is a mechanical 
  adaptation rather than a software behavior.
- **FR-010** (current measurement) is verified through calibration 
  results documented separately in `04_Results/acs712_calibration.md`.
- **NFR-018** (no PII) is verified through code review, not runtime testing.
- **NFR-020** (mains isolation) is verified by physical safety inspection.

No requirement is left unverified. Where runtime testing is not 
applicable, alternative verification methods (inspection, review) are 
documented.
