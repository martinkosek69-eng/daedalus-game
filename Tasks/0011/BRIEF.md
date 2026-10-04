# 0011 — Responsive drift-free flight

Owner: Codex helper solar_handling. Base c56ff1d, branch codex/solar-flight,
shared checkout; root alone commits and builds. Target integration by0010.
Allowed ONLY DaedalusSimulation/DaedalusFlightModel .h/.cpp and its tests,
Content/Data/Solar/flight.json, Tasks/0011/PROGRESS.md and HANDOFF.md.
No editor, Blender, build or git mutation. Root owns solar visual adapter.

Pure singleplayer value simulation. User requires maximum speed reached within
three seconds in both profiles, no sideways drift at any turn/fast motion,
smooth natural banking at limits/return and gentle pitch-limit settling.
Keep partition-invariant fixed step, finite validation, transactional init,
pause and swept collision safeguards. Add meaningful tests of new requested
properties at both speeds, release/reversal/bounds and native brake behavior.
No permanent helper roles, persistence schema change or visuals writing state.
