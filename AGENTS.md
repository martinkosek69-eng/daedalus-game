# Shared technical workflow

Read README.md and Docs/ENVIRONMENT.md before environment work. These are
development instructions; creative game ideas remain editable.
Read Tasks/README.md before accepting or handing off a numbered task.
Read Docs/FOUNDATION.md, Docs/ARCHITECTURE.md, Docs/CODING_RULES.md and
Docs/CURRENT_STATE.md before gameplay work. Confirm singleplayer scope and
canonical data/state/presentation boundaries; do not mutate state through visuals.

- Codex coordinates the project and integrates reviewed results. Claude and
  other assistants take individual tasks; there are no permanent world/ship
  roles. The project must remain resumable without any particular assistant.
- Within the user's authorized objective, Codex chooses whether to delegate,
  the task size, dependencies and acceptance criteria. Codex remains responsible
  for the entire requested result, including subcontracted work and integration.
  Choose resumable tasks given the helper's available quota; Codex can finish
  them after an explicit ownership transfer. Direct Claude launching/messaging
  is not configured; the user currently relays the prepared launch prompt.
- A numbered task must specify its owner, base commit, work branch, allowed
  paths, deliverables and acceptance checks. Read its BRIEF.md before editing.
- Workers update only their task's PROGRESS.md, HANDOFF.md and assigned output
  paths. The coordinator owns task briefs and Tasks/INDEX.md. Save meaningful
  checkpoints with commits and normal pushes; local saves alone are not shared.
- Work branches are reviewable deliveries. Do not merge them into main, force
  push or delete another worker's branch. Report interrupted work honestly;
  READY_FOR_REVIEW is not ACCEPTED. A reassignment must be explicit.
- Publish source assets, required dependencies and concise implementation/check
  notes. Do not publish private chat exports, hidden reasoning, credentials,
  caches or raw machine diagnostics. GitHub does not automatically synchronize
  open applications or notify an idle agent; fetch when starting or resuming.

- Work in your assigned checkout and branch. Prefer worktrees on A: on this PC.
  Never switch another agent's branch or overwrite their uncommitted work.
- Before editor MCP calls, verify which .uproject the running editor opened.
  A loopback URL does not identify a checkout. Editor and edited files must refer
  to the same working copy.
- One agent owns a shared Unreal editor at a time. MCP calls must be sequential.
  Coordinate ownership explicitly. A branch does not isolate a live editor.
  Prefer one editor and one build at a time in the default local workflow.
- Close the applicable editor before a full C++ build. Do not terminate a user's
  existing application to resolve a conflict. Test-created processes may be closed.
- Blender tools use saved files in background processes. Save explicitly and
  inspect the export or render. Do not claim control of an unsaved GUI scene.
- Installation paths belong in ignored .local/toolchain.json or DAEDALUS_* env
  variables. The shared repository helper builds its own checkout.
- Do not edit the same binary .uasset, .umap or .blend concurrently. Assign an
  owner first; exchange finished assets through Git LFS and review.
- Use branches and reviewable changes. Hand off changed files, checks and any
  remaining limitations. Tool availability alone is not a successful test.
- Track source, reusable tooling, public technical docs and required assets.
  Keep credentials, machine settings, caches and personal chat exports outside
  the public repository. Preserve the old prototype backup.
- Run checks appropriate to changes. Test outputs belong in ignored directories.

Codex and Claude use different client configs and do not share chat memory.
Keep these instructions and the technical documentation as their common reference.
