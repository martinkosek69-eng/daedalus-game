# Implementation rules

1. Read FOUNDATION, ARCHITECTURE and CURRENT_STATE before gameplay work. Work in
   assigned paths; preserve shared contracts and player data.
2. Keep dependencies one way: simulation does not import presentation/engine
   Actors. State mutations go through validated domain commands; UI reads state.
3. Parse complete data into temporary objects and validate before commit. Reject
   duplicates, unknown references, NaN/Inf and unsupported formats explicitly.
4. Use stable IDs and explicit units/frame names. Do not mix strategic ly,
   system metres and scene centimetres. Add tests for coordinate conversions.
5. Small named functions and readable formatting. No giant master gameplay file
   or class per ship that duplicates complete flight/energy/combat logic.
6. One canonical source for parameters. New behavior gets a documented command
   or interface plus a meaningful test; new content gets validated definitions
   and dependency/source evidence. No unvalidated hardcoded franchise balancing.
7. Tests verify boundaries and observable behavior, not just the implementation
   against itself. Restart persistence and packaged loads are release gates.
8. Checkpoint working increments. Record failures and actual verification.
   Do not label deferred functions or available tools as tested.
9. Avoid speculative networking, extra services, engine forks or large plugins.
   Adopt dependencies only with a demonstrated need, compatibility/restore tests
   and a recorded decision. Tool/engine upgrades happen in reviewable branches.

Singleplayer is an explicit scope decision. Multiple development agents are
independent of runtime multiplayer. AGENTS.md and Tasks/README remain the shared
ownership/branch/editor rules.
