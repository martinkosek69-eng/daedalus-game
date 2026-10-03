# 0009 — Web presentation comparison build

Coordinator: Codex. Branch codex/solar-flight. Singleplayer only.
Technical changes ready; the user will judge appearance/flight feel in play.

The active ship is now SM_WebDaedalus, generated from the unchanged original
web GLB, not the photograph recolor. Same 600 m hull, 222330 original triangles,
plus the web's runtime fittings (223458 total). A separate saved/reopened
WebDaedalus.blend/GLB and SHA256 metadata preserve the photo source alongside it.
Unreal independently verifies 60000 cm. Nanite is disabled on the comparison
mesh to retain the complete original geometry. The original web livery shader
is translated to per-pixel object-local plate variation and derivative-smoothed
seams, including the recess/spine/engine shading. Earlier coarse interpolated
paint is not used on this comparison hull. No new hull design was invented.

Softer 2.8 key, cool .45 fill and modest blue ambient surface response replace
the harsh 4.5/.8 illumination. Bright photograph hangars are not used. Six
separate engine effects remain aligned and respond to throttle. The separate
window component is hidden because the web hull already includes the windows.
HUD is smaller and centered below the ship; distance-field speed text is sharp.

Web-derived presets: harbour150m/s, impulse250km/s, initial impulse. Throttle20%
therefore targets50km/s. Acceleration/braking restored to web values. Turn9°/s,
pitch±60°, bank22°. Domain simulation, safeguards and lateral assist preserved;
this is a tuning comparison, not an exact port of every web/STO physical formula.
W lowers the bow, S raises it as in the web. Camera52°FOV/1500m, manual orbit
direct and independent of ship heading until Home, follow easing2.2, level roll.
Mouse sensitivity normalized to1 with FOV scaling/mouse smoothing disabled;
this fixes the first probe's overly small orbit. MotionBlurAmount/Quality0 and
FXAA remove frame-history ghosting. FXAA can show more fine edge aliasing.

Checks:
- Background Blender reopen, original geometry assertion and 600m dimensions.
- Independent Node GLB triangle/dependency/hash checks pass; original Claude
  delivery validator still passes unchanged (58 mounts, six outlets).
- Final Windows C++ editor/game build and IoStore package pass.
- All15 automation groups pass: tests-e1813394b9464f7ea8cc0018fc2eae06.
- Final930-frame packaged controller/render probe passes all checks:
  visual-6a689104f4044a2498d1434a795202b8. Earth and front-quarter comparison
  images inspected. No temporal AA or motion blur is enabled. This fixed-time
  functional probe is not a measured frame-rate benchmark.
- SolarFlight.umap remains SHA256
  5e6a29e499857b7ced170ded955457b04214b3a39d73e6dc4c0dad8899b8fb37.

All raw logs/screenshots are ignored local files. The existing launcher
Tools/SPUSTIT_LET_DAEDALA.cmd selects .local/solar/Build-WebReference. The user's
already-running .local/solar/Build remains untouched. Close the old window and
launch again to test the new version. BuildName parameter supports separate
archive selection; it accepts a folder token, not an arbitrary path.

Main stays unmerged; deliver through existing PR3. The late Claude checkpoint
d5846a1 stays on its worker branch and is not part of this comparison.

Final foundation restart passed via WindowsPowerShell5.1:
foundation-restart-76381027e16a4d52bec1be0ce7e95abe.
