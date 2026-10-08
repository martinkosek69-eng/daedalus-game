# 0017 progress

Status: READY_FOR_REVIEW. Owner: Codex helper solar_handling. Base81fd5d1,
shared codex/solar-flight checkout. Root owns builds, tests and publication.

Added pure singleplayer value navigation catalog/planner and four requested
automation groups. Configure validates/copies before any replacement; invalid
catalogs/commands preserve prior catalog, target and local location. Catalog
limits:10000systems,10000bodies/system,100000total; local coordinates±1e15m,
common galaxy coordinates±1e9LY. Body-parent graphs validate iteratively in
linear time. Unknown radii remain explicit0/false, never invented measurements.

Local same-system subtraction preserves centimetres in distant galaxy sectors.
Cross-system estimates subtract galaxy positions before converting light-years,
then add local metre offsets. Query only reads state; ETA/required-speed values
have independent validity flags and safe zeros for invalid/overflowing inputs.

Source inspection completed. No apps/build/git/test execution by helper. Next:
root integrates the header/API into0016 and builds/runs all four new groups.
