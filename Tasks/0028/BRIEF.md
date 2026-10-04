# 0028 — Daedalus hyperspace entry animation

Owner/coordinator: Codex. Branch: codex/solar-flight.
Base: ef356d35e60bcf8720feb8df8c88b0231794387e.
Singleplayer, presentation-only animation delivery; no inter-system travel rules.

## Objective

Inspect the user's three original-series video excerpts frame by frame. Create
a reusable Daedalus entry effect and a playable/replayable presentation preview:
green luminous window formation, ship acceleration and crossing, collapse behind
the ship, and an interior hyperspace view informed by the third clip. Calibrate
colour, shape and timing against these exact references. Aurora is deferred.

## Allowed paths / ownership

Codex owns Tasks/0028, coordinator Tasks/INDEX.md and Docs/APP_OWNERSHIP.md,
Docs/HYPERSPACE_PRESENTATION.md and relevant current-state/control documentation;
Tools hyperspace preparation/preview helpers; Art/Effects/Hyperspace source shaders
and parameters; Game/Daedalus/Content/Hyperspace generated materials and
Content/Data/Hyperspace presentation timeline; Game/Daedalus/Source/Daedalus
presentation preview adapter and minimal launch hooks/config for cooking it.
Read existing ship assets; do not rewrite ship binaries or shared planet assets.
Raw supplied videos, extracted frames, diagnostics and preview renders stay
ignored under .local/hyper on A:. Do not publish series clips as game assets.

## Deliverables / acceptance

- Concise frame/timing/colour analysis, source hashes and honest visual limits.
- Original reproducible runtime effect with adjustable presentation parameters,
  current reviewed Daedalus model, deterministic replay and standalone launcher.
- Native 3840x2160 output checked during formation, crossing, collapse and transit.
- Verified build/package, effect shader compile and preview lifecycle; scene-only
  timeline never mutates saved or flight domain state. Normal lab remains usable.
- Reviewable commits/push with handoff and concise evidence. Most subjective
  reference likeness remains user review, not an assertion of pixel identity.
