# Repeatable foundation checks

Configure ignored .local/toolchain.json (see ENVIRONMENT). Run from repository
root in PowerShell. Tools/Invoke-Foundation.ps1 derives its own checkout.

1. `./Tools/Invoke-Foundation.ps1 Build` with all Unreal editors closed.
2. `./Tools/Invoke-Foundation.ps1 Assets` creates the original source/export,
   reopens Blender source, checks dimensions, imports owned assets and saves map.
3. `./Tools/Invoke-Foundation.ps1 Test` runs Daedalus.Foundation automation.
   Inspect exported index.json; exit zero without completed tests is insufficient.
4. `./Tools/Invoke-Foundation.ps1 Package` builds/cooks/stages a Windows game.
5. `./Tools/Invoke-Foundation.ps1 Smoke` starts TWO independent packaged processes.
   First loads packaged catalog, fires/moves/travels/transports and saves; second
   restores and checks identity/damage/time/location/revisit and active visuals.
6. `./Tools/Invoke-Foundation.ps1 Play` for actual visual/input inspection.

All outputs are ignored .local/foundation, test saves unique. Runtime user/save
paths come from this checkout on A: via launcher, not machine paths in Git.
Never overwrite player's saves for a test. Foundation commands are diagnostics:
WASD/QE translate; Space beam fire; T instant test travel; B transport/return;
F5 save; F9 load; P domain pause. These are not approved final controls.

Test corrupt/newer/invalid catalogs and saves, no partial commit, fixed-step
partition/pause/backlog, shield spill/energy/cooldown/range/obstruction, stable
travel/transport state, dynamic instances/content additions, coordinates, large
catalog queries, and recoverable save generations. Do not confuse a large catalog
with a measured large battle or fully detailed planet.

Source asset delivery gate: push LFS objects on review branch, fetch object into
a fresh independent LFS store and compare SHA256. Current Claude-client checks
remain separate user-started work. Main merge requires reviewed delivery, not
automatic acceptance because tools or folders exist.
