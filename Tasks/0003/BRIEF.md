# 0003 — Simulation module and tests

- Owner: Codex subagent foundation_simulation.
- Base: checkpoint of task 0002 brief (see work branch history).
- Work branch: codex/foundation-simulation, separate checkout under .local/agent-simulation.
- Review target: codex/game-foundation.
- Allowed paths: Game/Daedalus/Source/DaedalusSimulation/**,
  Tasks/0003/PROGRESS.md, Tasks/0003/HANDOFF.md.
- Deliver: C++ module depending only on Core/Json, catalog and domain state
  independent of scene Actors; versioned validated JSON load/export, fixed-step
  time, local metre coordinates with system IDs, ships/energy/shield/weapon damage,
  travel, transporter location state, meaningful WITH_DEV_AUTOMATION_TESTS tests.
- Acceptance: code review and root-built Unreal automation tests; invalid catalog
  and saves rejected transactionally; stable IDs, revisit and save roundtrip;
  no GUI/engine build started by worker (root owns all builds/editors).
- Do not edit root module/config/docs or use installed app processes. Commit
  local work on assigned branch and hand off commit; coordinator publishes and
  integrates. Read AGENTS, README, ENVIRONMENT, Tasks/README before work.
