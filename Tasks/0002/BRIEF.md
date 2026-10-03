# 0002 — Verified single-player game foundation

- Owner / coordinator: Codex (root).
- Base commit: e097331d24e234e3997cd1fccba333521e4ad57d.
- Work branch: codex/game-foundation. Review target: main; no merge authorized by this brief.
- Allowed paths: Game/, Data/, Art/, Docs/, Tools/, README.md, AGENTS.md,
  .gitignore, .gitattributes, Tasks/0002/, Tasks/0003/, Tasks/INDEX.md.
- Objective: implement and verify a reusable single-player foundation, not final
  game content. Capital ships normally stay in space; transport visits authored
  locations, with shuttle/landing gameplay an extension point.
- Deliver: canonical versioned catalog, independent simulation module, local
  coordinates/time/ships/weapons/travel/location state, transactional persistence,
  minimal playable presentation, original reusable asset source, repeatable
  validation/build/tests/packaging tools, architecture and extension instructions.
- Acceptance: build editor and game; meaningful C++ tests; packaged catalog load;
  save/restart/load and invalid-state rejection; revisit preserves state; bounded
  local presentation; source asset Blender roundtrip and Git LFS roundtrip;
  inspect actual game output; publish normal checkpoint commits/review branch.
- External limitation: actual Claude-client checks remain a separate deferred
  user-started activity (0001 is not started here).

Simulation implementation may be subcontracted in 0003, with explicit ownership
and its own checkout. Root remains responsible for integration and review.
