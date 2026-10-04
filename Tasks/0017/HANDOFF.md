# 0017 handoff

READY_FOR_REVIEW; no helper commit/push/build. Root coordinates the shared
codex/solar-flight checkout and owns integration, checks and publication.

Added only assigned files:

- Public/DaedalusNavigationModel.h
- Private/DaedalusNavigationModel.cpp
- Private/Tests/DaedalusNavigationTests.cpp
- Tasks/0017/PROGRESS.md and HANDOFF.md

Namespace Daedalus. Public structs and planner API match BRIEF. The catalog is
copied internally; later caller edits cannot mutate it. Each system requires
at least one body for its default body target. IDs follow existing foundation
ASCII letters/digits/dash/underscore/dot conventions (max128), names max1024.
System IDs are global, body IDs unique within their system. Parents remain in
the same system; self/multi-body cycles and missing references are rejected.
Linear iterative validation supports a10000-node parent chain without recursion.
Galaxy coordinate bound±1e9LY matches the existing foundation; local±1e15m.
Total100000body guard applies alongside10000systems/10000bodies per system.

Unknown-radius bodies must use RadiusMetres0 and bKnownRadiusfalse. Known-radius
bodies require finite positive radii; the planner never invents a radius and
never subtracts one from centre-to-centre distance. No collision, travel,
teleport, energy, simulation clock or position mutation through target selection.

Configure is atomic: invalid input preserves the whole previous plan. Successful
refresh intentionally resets target to first system/body and location to that
system's local zero (as specified in BRIEF). SetLocation changes only reported
location; SelectTarget changes only the canonical target pair. Query is const.

Same-system distances subtract local metres directly, never giant galaxy
metre positions. Cross-system difference is galaxy delta×9460730472580800m/LY
plus target-local minus ship-local. Direction uses the common galaxy basis,
normalized except for coincident points (zero vector). bValid concerns geometry;
speed/time failures make their independent ETA/required flags false with values0.
Positive finite inputs provide estimates; overflowing quotients remain unavailable.

Four automation groups added (execution pending root):

- Daedalus.Navigation.CatalogRollback — invalidIDs/duplicates/parents/cycles,
  NaN/bounds/known-vs-unknown radii, rollback, reused bodyIDs across systems,
  valid10000parent chain and exact100000/invalid100001total limits.
- LocalGalacticDistances — centimetres at100000LY galaxy coordinates,1LY,
  normalized3-4-5direction, smaller separations with local offsets, coincident.
- ETABoundaries — separate actual/planned/required estimates, zero/negative/
  NaN/Inf/tiny overflowing denominators, partial validity, zero-distance behavior.
- SelectionLocationCommands — target never moves location, invalid command
  rollback, explicit active-system change, immutable copied catalog, refresh
  reset semantics and nonmutating queries.

Source reviewed only. No executable checks were run by this helper; this handoff
does not claim compilation or automation success. Root should require all four
groups to pass and verify staged catalog/UI integration in the package.
