# 0020 — Sharp flight view and sublight controls

Owner: Codex coordinator. Base: 9f9b34f. Branch: codex/solar-flight.
Scope: singleplayer flight laboratory; keep canonical simulation separate from presentation.

User reports global softness, especially ship when zoomed out. Compare the
preserved web renderer and controls. Correct display resolution/DPI, unwanted
postprocessing and texture detail in ordinary flight, not only inspection mode.
Add normal impulse and Shift+R full sublight impulse, smoothly usable in flight.
Do not implement hyperspace or modify the preserved prototype/Claude checkout.

Allowed: Game/Daedalus Solar presentation, simulation flight model/tests,
Config, Content/Data/Solar/flight.json, owned derived Solar/ship assets if needed,
Tools solar preparation/validation/launching, Docs and Tasks/0020 plus INDEX.

Acceptance: compile, domain transition/rollback/drift/braking checks, actual
packaged input tests for R and Shift+R, near/far/orbit and planet/sky rendered
comparison at native resolution; check effective rendering settings and texture
residency in ordinary flight. Preserve player data and report visual limitations.
Publish reviewed source and required assets on the work branch, never main.

Parallel helper scope: only DaedalusFlightModel.h/.cpp, DaedalusFlightTests.cpp
and Tasks/0020/FLIGHT_HANDOFF.md; coordinator owns all Unreal/app/build access.
