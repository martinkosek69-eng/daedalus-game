# 0021 — Dark point-star sky, full detail and responsive turning

Owner: Codex coordinator. Base4114e5c, branch codex/solar-flight.
Singleplayer; canonical tuning/simulation owns flight, visuals remain disposable.
Read shared instructions and this brief before editing.

User approves ship sharpness, rejects blurred and busy photographed star layer.
Remove the photographic sky/nebula layer; use dark sky with separate catalog
point-like stars, understated colours/brightness and no bloom halos. Use NASA
human-view references, not long-exposure photograph appearance.
Default to full texture and mesh detail; disable performance-driven reduction
for now as user requested. Retain invisible-surface culling (no quality loss).
Increase angular response20%; low-speed turns should be easier with more bank,
high-speed turns slower and shallower. Preserve no-slip, smooth limits and drives.

Allowed: Solar presentation, flight model/tests, flight.json, Config, solar
launch/check tooling, Docs, Tasks/0021 and INDEX. Preserve source art/prototype,
player data and Claude checkout. No new binary art is required.
Checks: editor build, existing flight groups, Windows package and one short
packaged sky/input sanity check; user does most visual/performance testing.
Publish normal checkpoints on work branch; never merge main.
