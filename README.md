# Daedalus

Unreal C++ foundation for a pure single-player space game. Creative content
remains editable. Capital ships normally stay in space; authored locations are
visited by transport, with shuttles a future extension.
The previous Three.js game is a separate reference and backup.

Start with [the game foundation](Docs/FOUNDATION.md),
[architecture](Docs/ARCHITECTURE.md), [implementation rules](Docs/CODING_RULES.md),
[current state](Docs/CURRENT_STATE.md) and [checks](Docs/TESTING.md).

Read [the environment guide](Docs/ENVIRONMENT.md) and
[the audit of 3 October 2026](Docs/ENVIRONMENT_AUDIT_2026-10-03.md).
The subsequent completed storage migration is documented in
[storage on A](Docs/STORAGE_ON_A.md).
Numbered assignments, checkpoints and cross-agent handoffs use
[the task workflow](Tasks/README.md) and [the task index](Tasks/INDEX.md).

The project uses Unreal, Blender, Visual Studio with C++, Git and Git LFS, and
Node 24. Validated tool versions are in the public audit summary; each machine
keeps installation paths and settings in its ignored local configuration.

From a checkout on A: on this PC:

1. Run `git lfs install` and `git lfs pull` for that user's Git setup.
2. Make Node 24 available on the client process PATH. The helper has no npm
   dependencies. Restart clients after changing their PATH.
3. Create `.local`, copy `Tools/toolchain.example.json` to `.local/toolchain.json`,
   and adjust the installation paths and optional environment overrides.
   Create the configured temporary/cache directories. `.local` is ignored by Git.
4. Start the AI client in the repository root. Claude Code uses `.mcp.json`.
   Codex uses `.codex/config.toml`. Both directly start the same Node helper.
5. Build before opening the editor. Open `Game/Daedalus/Daedalus.uproject` from
   the same checkout. MCP is available while this editor is running.

No Claude session has yet been tested on this machine. Client-side trust and
permissions, Git authentication, and Node PATH must be verified in each client.
MCP compatibility is not proof that all tools work in an untested client.

An editor fallback for any agent with PowerShell is `./Tools/Unreal-Mcp.ps1`.
It reads tools using a fresh MCP session. Consult schemas before making calls.

`Tools/Test-AppsMcp.mjs` builds, saves a disposable Blender scene, exports it and
renders it. Close Unreal, run from the root with
`node Tools/Test-AppsMcp.mjs Tools/apps-mcp.mjs`; outputs are under `.local`.
