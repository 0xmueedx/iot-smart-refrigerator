# IoT-Integrated Intelligent Refrigerator System

Final Year Project — B.Sc. Computer Engineering, BZU Multan

## What It Does
A retrofitted water dispenser turned into a smart refrigerator with:
- RFID access control (MFRC522 + solenoid lock)
- Temperature/humidity monitoring (DHT22)
- Door-ajar alert (IR sensor, 30s delay → escalating alarm)
- Web dashboard over Wi-Fi (ESP32)
- Compressor current monitoring (ACS712)

## Key Results
- 100% RFID access accuracy (100 trials)
- 99.78% sensor reliability over 24hr continuous run
- 478ms mean unlock latency
- Stable operation, no crashes in 24hr test

## SDLC Documentation
| Artifact | Link |
|---|---|
| Requirements Specification | [SRS](requirements/srs.md) |
| Design Decisions | [Design](design/decisions.md) |
| Test Plan & Cases | [Testing](testing/test_plan.md) |
| Traceability Matrix | [Matrix](testing/traceability_matrix.md) |
| Bug Reports | [Bugs](testing/bug_reports.md) |
| Lessons Learned | [Reflections](reflections/lessons_learned.md) |
