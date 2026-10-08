# Flight domain handoff

Status: READY_FOR_REVIEW. Owner: Codex flight helper. Base: 9f9b34f.
Shared branch: codex/solar-flight. Coordinator owns compilation and publication.

Changed only DaedalusFlightModel.h/.cpp, DaedalusFlightTests.cpp and this handoff.

`FFlightModel::SetConfig(config, error)` applies validated tuning to an initialized
model without changing any state field, input, pause or pending time. It rejects
incompatible current attitude/rate limits and a radius that would overlap a body.
Invalid commands preserve configuration, state and transition braking authority.

Upgrading accelerates normally. Downgrading preserves current velocity exactly,
then gradually slows toward the new throttle target. Previous braking/coasting
authority survives repeated downgrades until speed is inside the new maximum;
ordinary drive deceleration then resumes. Swept contacts remain authoritative.
Maximum supported configuration speed is strictly below 299792458 m/s.

Added transition, rollback, retained-input/pause/backlog, frame partition,
repeated downgrade, no-slip and full-sublight swept contact cases within the
existing four test groups. Canonical profile loading now requires three entries;
the third is expected to be the coordinator's 250000000 m/s full impulse profile.

Checks performed here: source review and git diff --check passed. No Unreal,
build or test process launched, as assigned. Coordinator must compile and run
the existing flight automation groups before acceptance. No commit/push by helper.
