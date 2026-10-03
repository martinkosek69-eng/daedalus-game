# Handoff — task 0002

Status: READY_FOR_REVIEW; coordinator acceptance recorded in Tasks/INDEX.md.
Delivery branch: codex/game-foundation. Main merge is not part of this task.

Implemented ordinary Core/Json simulation, validated versioned catalog and stable
instance state, indexed local-system updates, fixed clock, flight, beam damage,
body collision/obstruction, safe arrivals, transport and independent ship state.
Unreal subsystem/pawn/HUD/scene adapters provide a small playable diagnostic.
Two rotating, verified JSON save generations reject incompatible overwrite and
restore transactionally. Engine-native SaveGame parsing asserted on corrupt
input, so it was replaced; no UObject payload is read from save files.

Editable original Blender source, GLB and imported mesh/material/map use LFS.
Tools build, export/import/check dimensions and material color, run named tests,
package, verify separate-process restart and actual rendered bindings, and check
all LFS source objects through a fresh remote fetch. The source-node color and
existing Interchange material overrides are explicitly reconciled. UE 5.8's
material setter returns false despite applying; the tool verifies actual values.

Checks: editor/game builds and Windows package; 11/11 C++ automation groups;
packaged write/read in separate processes; 20 revisits (21 -> 21 live Actors);
rendered movement/fire/travel/transport/return/F5/F9/pause/dynamic-spawn checks;
visual inspection of both captured views; Blender source reload/GLB export;
120m -> 12000cm engine measurement and saved BaseColorFactor; repeat prep keeps
map hash; five LFS objects roundtrip with SHA256; matching-project editor MCP
PIE start/query/stop; Windows PowerShell launcher compatibility.

Repeat with Tools/Invoke-Foundation.ps1 and Test-SourceDelivery.ps1 as documented
in Docs/TESTING.md. Logs, reports, builds, screenshots and test/player saves are
ignored on A, not published. Tool paths remain ignored local configuration.
Original prototype backup is preserved. Read Docs/CURRENT_STATE.md for material
limits and continuation instructions. No actual Claude-client test was claimed.

Found and corrected: unsafe native corruption deserialize, test array aliasing,
spawn argument lifetimes across map growth, missing cooked model dependencies,
invisible post-start ships, engine F5/F9 debug shortcut collision, immovable test
light, exporter node color/material reimport, incomplete-test acceptance, save
body overlap, unreported backlog, and repeated map replacement risk.

Future content is implemented against these tested contracts. Performance and
compatibility of future full battles, detailed planets or new schemas must be
measured; they are not established by this foundation delivery.

A separate Claude introductory report was retrieved from its own setup branch and reviewed in Docs/AgentChecks; source claims are distinguished from coordinator-observed Git delivery. The client needs a new-session live MCP check; numbered task 0001 was not started.
