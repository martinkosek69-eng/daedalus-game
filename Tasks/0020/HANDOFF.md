# 0020 delivery

Coordinator integrated helper domain changes. Technical checks passed; user
visual/play acceptance remains pending. Branch codex/solar-flight, main unmerged.

Rendering: enable bAllowHighDPIInGameMode before window creation; apply native
100% settings after game user settings. Disable FXAA, temporal history, motion
blur, DOF and fringe. Keep 16x anisotropy and modest0.25 sharpening. Reduce bloom
and star billboard footprint. Pin full ship/sky mips independent of camera zoom;
promote only four angularly largest nearby discs above0.004rad, release prior
body/ring pins on scene changes and all owned pins on EndPlay. This addresses
missing streaming build data of runtime-scaled celestial meshes. Player data and
source art are unchanged. Very small edges can alias; subpixel detail is finite.

Controls: R starts/stops selected ordinary mode; Shift+R starts full impulse at
100% throttle or returns to ordinary impulse.1/2 change harbour/impulse in flight.
Full impulse250000000m/s matches web helm.mjs250000km/s; acceleration100000000m/s2
meets user's prior <=3s request instead of original web charging delay. Max speed
is strictly below light. No hyperspace. Downshift preserves pose/velocity and
retains prior stronger braking until the new limit is reached. Input remains
blocked in map/pause. Collision remains conservative swept spheres. ETA updated.

Checks (ignored local reports):
- Editor build: .local/solar/sharp-build-console.log, success.
-19/19 groups: .local/solar/tests-48a9d91febec4039969f3382a497d150.
  Existing flight groups extended for transitions/rollback/pause/partition,
  drift/reverse/braking and full-sublight swept contact.
- Windows package: .local/solar/sharp-package-console.log, success.
- Native input/render: .local/solar/sharpnative-01502fef5239407aa8e22642ad310d97.
  PASS1650frames; actual2560x1440, highDPIenabled, no scaling or blur. Windows
  currently reports2560x1440 primary and1920x1080 secondary. This is not a4K test.
  Compared near/far with/without FXAA, captured moving orbit and full impulse.
  Ship2K12/12mips near/far; Earth and sky8K14/14mips during ordinary flight.
  Physical R and Shift+R controller inputs, full speed under3s, smooth exit,
  no drift, map input exclusion, live harbour selection and braking pass.

Per user's latest instruction, no broad repeated six-system suite or extra
art/source audits. Unchanged LFS assets retain previous0016remote proof. Native
appearance, monitor-specific4K, performance and handling feel are for user test.
Run Tools/SPUSTIT_LET_DAEDALA.cmd; same current Build-SolarSystem package.
