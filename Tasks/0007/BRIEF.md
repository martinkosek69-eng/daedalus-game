# 0007 — Recover original prototype Daedalus asset

Owner: Codex coordinator after explicit transfer on 2026-10-03; the asset
subagent session ended at its usage limit. Partial source/script preserved.
Base e09696e7269565fff9668db5838003ba0c6f2749.
Assigned checkout: coordinator current checkout; branch codex/solar-flight.
Allowed paths ONLY: Art/Ships/Daedalus/**, Tools/Prepare-DaedalusSource.py,
Tasks/0007/PROGRESS.md and HANDOFF.md; Art/Space/SolarSphere.glb and .blend;
disposable outputs .local/solar-art/**.
Coordinator commits/pushes. Exclusive Blender BACKGROUND ownership for this task.
No GUI interaction, Unreal editor, import commandlet or builds.

Read original prototype files without modifying backup at
A:/GPT-CODEX/BC-BACKUPS/BC-2026-10-02-BASELINE/BC/dist.
Recover actual assets/daedalus.glb, source metadata, and study livery.mjs.
Create editable Blender source and GLB ready for Unreal with +X forward, +Z up,
600 m length. Preserve mesh silhouette/details; avoid redesign. Approximate or
bake original runtime hull paint if feasible; explicitly record differences.
Consolidate into one mesh with material slots for reliable Unreal import.
Include correct Astrofossil source credit/CC BY-NC 4.0 notice and modification
record, file hashes, dimensional/material counts; no unrelated private data.
Use Blender executable from .local/toolchain.json and A: temporary storage.
Reopen saved .blend, inspect exports, render preview to ignored outputs.
Do not assume GLB -Z axis after Blender import; verify coordinates and front.
