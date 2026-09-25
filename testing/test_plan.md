# Test Plan

**Environment:** Prototype unit, indoor, ~25°C ambient, 220V AC mains

---

## 1. Introduction

This Test Plan defines the strategy, scope, approach, and deliverables for testing the IoT-Integrated Intelligent Refrigerator System. It covers unit testing of individual subsystems, integration testing of the complete operational flow, performance measurement of key operations, and a 24-hour long-duration stability test.

---

## 2. Test Objectives

1. Verify each hardware subsystem functions as specified
2. Verify the integrated system behaves correctly under realistic usage
3. Measure performance against defined targets
4. Confirm system stability over extended continuous operation
5. Validate the system against approved functional requirements
6. Identify and document any defects found during testing

---

## 3. Scope

### In scope:
- RFID access control (card reading, decision logic, lock actuation)
- Environmental monitoring (temperature, humidity)
- Door-ajar detection and alert escalation
- Wi-Fi configuration portal
- Web dashboard (data rendering, manual unlock)
- Compressor current monitoring
- OLED display output
- System stability over 24 hours

### Out of scope:
- Long-term reliability beyond 24 hours
- Harsh environmental conditions (extreme temperature, high humidity)
- Cybersecurity penetration testing
- Mobile app testing (no mobile app in this version)
- Cloud connectivity (not implemented)

---

## 4. Test Approach

| Test Level | Description | Coverage |
|---|---|---|
| Unit Testing | Each subsystem tested in isolation | Hardware, sensors, actuators |
| Integration Testing | Full operational scenarios | End-to-end flows |
| Performance Testing | Quantitative measurements | Latency, accuracy, throughput |
| Regression Testing | Re-verification after fixes | Bug-related areas |
| Stability Testing | 24-hour continuous run | Overall system reliability |
| UAT | User-perspective validation | Final acceptance |

---

## 5. Test Environment

| Component | Specification |
|---|---|
| DUT | Assembled prototype (water dispenser with electronics) |
| Power | 220V AC, standard wall outlet |
| Ambient | Indoor, 23–27°C, normal humidity |
| Wi-Fi | 2.4 GHz home router, RSSI −55 to −65 dBm |
| Test Cards | 1 authorized MIFARE Classic, 3 unauthorized |
| Reference Instruments | Calibrated digital thermometer-hygrometer, plug-in power meter |
| Clients | Windows laptop (Chrome), Android phone, iPhone (Safari) |

---

## 6. Entry Criteria

- Prototype fully assembled and powered
- Firmware compiled and uploaded successfully
- Test environment set up with reference instruments
- Test cases documented and reviewed

## 7. Exit Criteria

- All High-priority test cases executed
- ≥ 95% pass rate on High-priority cases
- All critical and high-severity bugs resolved or documented
- 24-hour stability test passed
- UAT sign-off completed

---

## 8. Roles & Responsibilities

| Role | Person | Responsibility |
|---|---|---|
| Test Lead | Saad Sohail Ansari | Test plan, execution, reporting |
| Test Engineer | Fatima-tu-Zahra Qayyum | Unit + integration test execution |
| Test Engineer | Abdul Mueed Malik | Performance measurements, bug documentation |
| Supervisor | Dr. Yasir Aziz | Review, UAT approval |

---

## 9. Test Deliverables

1. Test Plan (this document)
2. Test Cases (`test_cases.md`)
3. Traceability Matrix (`traceability_matrix.md`)
4. Bug Reports (`bug_reports.md`)
5. Test Execution Log (see §10 below)
6. Performance Results (see `04_Results/`)
7. UAT Sign-off (see §11 below)

---

## 10. Test Execution Log

| Test ID | Description | Date | Result |
|---|---|---|---|
| UT-01 | RFID authorized card read | Day 1 | ✅ Pass |
| UT-02 | RFID unauthorized card read | Day 1 | ✅ Pass |
| UT-03 | Solenoid lock actuation | Day 1 | ✅ Pass |
| UT-04 | DHT22 accuracy check | Day 1 | ✅ Pass |
| UT-05 | IR door sensor response | Day 1 | ✅ Pass |
| IT-01 | First boot + Wi-Fi config | Day 2 | ✅ Pass |
| IT-02 | Normal operation display | Day 2 | ✅ Pass |
| IT-03 | Authorized RFID scan | Day 2 | ✅ Pass |
| IT-04 | Unauthorized RFID scan | Day 2 | ✅ Pass |
| IT-05 | Door open < 30s | Day 2 | ✅ Pass |
| IT-06 | Door open > 30s | Day 2 | ✅ Pass |
| IT-07 | Dashboard load | Day 2 | ✅ Pass |
| IT-08 | Manual unlock | Day 2 | ✅ Pass |
| PERF-01 | RFID latency (n=50) | Day 3 | ✅ Pass |
| PERF-02 | Dashboard latency (n=500) | Day 3 | ✅ Pass |
| PERF-03 | Alert timing (n=10) | Day 3 | ✅ Pass |
| STAB-01 | 24-hour continuous run | Day 4 | ✅ Pass |

---

## 11. User Acceptance Testing (UAT)

### UAT Objectives
Confirm the system meets real-world user needs and is acceptable for deployment.

### UAT Scenarios

| UAT ID | Scenario | Acceptance Criteria | Result |
|---|---|---|---|
| UAT-01 | First-time Wi-Fi setup | Non-technical user completes setup in < 3 minutes | ✅ Pass |
| UAT-02 | Authorized access | Card unlocks door within 1 second | ✅ Pass |
| UAT-03 | Denied access | Unauthorized card does not unlock door | ✅ Pass |
| UAT-04 | Forget to close door | Alert draws attention within 15 seconds of escalation | ✅ Pass |
| UAT-05 | Check fridge from another room | Dashboard loads and shows live data | ✅ Pass |
| UAT-06 | Recover forgotten card | Manual unlock via dashboard works | ✅ Pass |

### UAT Sign-off

| Role | Name | Signature | Date |
|---|---|---|---|
| User Representative | Saad Sohail Ansari | ✅ Approved | 2026-04-07 |
| Test Lead | Fatima-tu-Zahra Qayyum | ✅ Approved | 2026-04-07 |
| Supervisor | Dr. Yasir Aziz | ✅ Approved | 2026-04-10 |

---

## 12. Risks & Mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| Mains voltage fluctuation damages electronics | High | Certified SMPS with protection; tested at 220V ±5% |
| Wi-Fi interference in dense environments | Medium | Dual-mode (STA + AP) operation; local fallback |
| DHT22 NaN reads during heavy Wi-Fi activity | Low | NaN-rejection logic; retained last valid reading |
| Solenoid misalignment due to thermal cycling | Medium | Steel guide plate installed; 200-cycle verification |
| Compressor startup inrush misread | Low | Zero-current offset calibrated at startup |

---

## 13. Test Schedule

| Phase | Duration | Completed |
|---|---|---|
| Unit testing | 1 day | ✅ |
| Integration testing | 1 day | ✅ |
| Performance measurement | 1 day | ✅ |
| 24-hour stability test | 1 day | ✅ |
| UAT | 0.5 day | ✅ |
| Documentation | 0.5 day | ✅ |

---

