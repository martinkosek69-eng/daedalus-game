# 0025 integration checkpoint

Status: READY_FOR_REVIEW; coordinator technically accepted. Owner: Codex, codex/solar-flight.

- Reviewed/fetched/cherry-picked0023 through aa9072c, latest0008 through
  7c35635. LFS sources downloaded; Claude explicitly released editor/build.
- Approved0024 HUD compiles; full automation suite20/20 passed, including
  Daedalus.HUD.RadarCoordinates. No new combat/energy/flight rules.
- Full content recipe succeeds. Read-only checks:105 planet bindings,
  10 cloud layers,3 relief maps,5 night maps,4 master materials; latest ship
  SHA,600m hull,5 material slots,split doors and10 source lights validated.
- Updated multi-piece ship import: hull plus add-ons,2 light meshes,beacon,
  4 door halves,6 engine outlets; no obsolete duplicate turret barrels.
  Hull/add-on material recipes are isolated where source vertex colours differ.
- Runtime uses source blink5s/on1/3s, source engine and bay point lights,
  H toggles doors with smooth two-second travel; pause/map freeze/block H.
  These are presentation previews, not gameplay hangar/shuttle systems.
- Repaired incomplete texture hash/file inventory in latest0008 metadata;
  standalone PNG bytes match embedded GLB images. Source validator passes.
  Blender generator now resolves relative texture paths when writing inventory.
- Editor build passed. Separate Build-Integrated25 package running.
  Initial render found clipped labels/overexposed bay lights; both fixed.
- Final editor/package builds and native3840x2160 packaged probe pass.
  Four door halves,10 lights,beacon material phases,engine power,H/R/map/pause,
  real HUD/planet/galaxy renders checked. No probe errors. Actual screenshots
  inspected; labels fit and hangar structure is visible.
- Launcher now selects Build-Integrated25; prior Build-SolarSystem and same
  PlayerData retained. No test processes remain. Final publication follows
  this checkpoint; remote commit is read from the branch, never inferred.
