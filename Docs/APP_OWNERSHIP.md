# Shared application ownership — task0022 proposal phase

Claude Code is assigned one Unreal editor and any necessary build of the
unchanged baseline for **read-only diagnosis** in task0022 upon user launch.
Codex will not concurrently use editor MCP/imports/compilation/package jobs
until Claude reports release or there is an explicit reassignment.

Project: Game/Daedalus/Daedalus.uproject in Claude's own new A: checkout of
codex/claude-0022-planet-quality. Usual MCP port8000; verify actual .uproject
before each call, never identify a checkout only by loopback URL.
No implementation, asset import, save/rewrite of tracked content or source
changes are authorized in this phase. Generated local build/cache diagnostics
are ignored and remain on A:. Human-owned processes and unsaved sessions are
excluded: never terminate or repurpose them. Blender GUI remains user-owned;
saved background read-only inspection is permitted.

Codex may read GitHub/text files and prepare its independent comparison.
Claude records application ownership in its PROGRESS and confirms release in
HANDOFF after publishing the proposal. Implementation ownership will be assigned
separately after the user chooses a solution. User instructions take precedence.
