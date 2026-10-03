# 0006 — Solar lab flight domain

Owner: Codex coordinator after explicit transfer on 2026-10-03; the flight
subagent session ended at its usage limit. Partial files are preserved and
reviewed by Codex. Base e09696e7269565fff9668db5838003ba0c6f2749.
Assigned checkout: coordinator current checkout; branch codex/solar-flight.
Disjoint paths ONLY: Source/DaedalusSimulation/Public/DaedalusFlightModel.h,
Source/DaedalusSimulation/Private/DaedalusFlightModel.cpp,
Source/DaedalusSimulation/Private/Tests/DaedalusFlightTests.cpp (under Game/Daedalus),
Tasks/0006/PROGRESS.md and HANDOFF.md. Coordinator commits/pushes checkpoints.
No Unreal/editor/build/process ownership. Do not change old simulation API.

Implement value-only configurable fixed-step flight model for STO-inspired
assisted capital-ship handling: persistent throttle -0.25..1, acceleration/brake
and lateral drift, smoothed bounded yaw/pitch, limited pitch +/-80 degrees,
visual bank returning level, pause and large-coordinate swept body safety.
Public agreed API is specified by coordinator message. Reject invalid config,
input, time and state without poisoning simulation. No gravity/warp/combat.
Tests cover distinct acceleration/turn/inertia/brake/time partition/pause/body
sweep cases. Tests under Daedalus.Flight. Document exact units and approximation.
