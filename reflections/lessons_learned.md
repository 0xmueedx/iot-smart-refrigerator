# Lessons Learned

## Technical
- **Plan for sensor failure.** DHT22 failed 0.22% of reads. Required NaN-rejection logic.
- **Mechanical parts drift.** Solenoid misaligned after thermal cycling. Fixed with a guide plate.
- **Know what your sensor measures.** ACS712 reads current, not true power. ~±8% error on inductive loads.
- **Timing constraints matter.** DHT22 one-wire protocol breaks during Wi-Fi activity.

## Process
- **Write requirements first.** Building before documenting left gaps in success criteria.
- **Traceability is a thinking tool.** Forced us to ask "how would we know this works?"
- **Test earlier.** Unit tests should come before integration.
- **Bugs must be reproducible.** Exact steps > vague descriptions.

## SDLC Insight
- **The SDLC loops, not flows.** Design ↔ test ↔ fix is iterative.
- **Quality is built in.** Best fix was a mechanical design change, not a code patch.
- **Documentation is the deliverable.** A system without docs is a black box.

## Reflection
The best moment was when the traceability matrix proved every requirement had a test. That's when this became an engineering deliverable, not a student assignment.

This system monitors food. In another context, it could monitor medicine. Reliability is not optional.
