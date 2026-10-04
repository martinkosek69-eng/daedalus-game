# 0021 delivery

Coordinator: Codex. Branch codex/solar-flight, base4114e5c, main unmerged.
Technical acceptance; user visual/performance/handling evaluation pending.

User's supplied screenshot showed duplicated stars and broad halos from M_Sky.
That historic photographic sphere is now invisible; only8920catalogue-based,
analytic point-like star instances render against black. Source position/colour
conversion is retained; brightness curve now matches original web
celestial-effects.mjs. Bloom is disabled, no nebula/photo/zodiacal layer renders.
See Docs/POINT_STAR_SKY.md for NASA sources and exposure/artistic limitations.
The galaxy map remains separate and unchanged. Source art is preserved.

Quality-first defaults: texture streaming disabled, pool ceiling removed,
highest mesh LOD forced. Full mips load without selective distance reduction.
Old selective policy remains dormant for a future explicit performance setting.
Hidden/back-facing surface culling is retained because it cannot reduce visible
detail. Higher GPU memory use is intentional at user's request.

Flight: canonical JSON increases base angular rate9→10.8 and acceleration24→28.8.
LowSpeedTurnMultiplier1.45 and HighSpeedBankFraction0.6 are validated domain
parameters. Actual speed/current drive max is smoothly blended: slow/stopped
up to15.66°/s and36°bank; full drive10.8°/s and21.6°bank. Faster keyboard response,
easier slow U-turns, smaller fast banks, gentle limits and zero slip retained.
No drive speed, throttle keys or collision/persistence contracts changed.

Checks (ignored local outputs):
- Editor build: .local/solar/pointsky-build-console.log PASS.
-19/19 groups: .local/solar/tests-6b74f067ef054ddfb7fed6808a68711e.
  Existing flight group tests actual slow/fast rates/bank ordering and no slip,
  with validation checks; previous reduced stationary-turn expectation updated.
- Package: .local/solar/pointsky-package-console.log PASS.
- Single native1650frame input/render run:
  .local/solar/sharpnative-e8997d2ea56f4516aa859071e65135c9 PASS2560x1440.
  Full mips, drive inputs, smooth downshift/braking and map exclusion pass.
  Actual moving-orbit image inspected: black field, separate tiny sharp points,
  no photographic structures or broad halos. ForceLOD0/BloomQuality0 logged.
  No expanded full-universe, LFS or source-art repeat audits; no art changes.

Known minor runtime diagnostic: per-frame r.TextureStreaming lookup emits a
console performance advisory after500calls. It does not fail the run or change
render quality; cache that lookup during the next rendering code iteration.
No performance/FPS promise. Native display currently2560x1440, not a4K check.
User does most further testing per explicit usage preference. Launch the same
Tools/SPUSTIT_LET_DAEDALA.cmd and current Build-SolarSystem package on A:.
