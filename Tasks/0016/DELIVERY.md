# 0016 publication and recovery checks

Runtime/source checkpoint: `e3ecef6024912acbd60bf5e2df21d4823d3f4d89`, published
on `codex/solar-flight`. Main remains unmerged. Reviewed delivery is draft PR3:
https://github.com/martinkosek69-eng/daedalus-game/pull/3

Normal push uploaded444 changed LFS objects (163MB). Then
`Tools/Test-SourceDelivery.ps1 -Branch codex/solar-flight` forced a fresh
independent remote LFS store, fetched all696 tracked source/assets and compared
SHA256 identities, local source hashes and remote object sizes. PASS; evidence
is ignored `.local/solar/final-delivery.log`. Subsequent documentation commit
changes no binary source/assets, so the verified objects remain identical.

Actual final package/input evidence and known limitations are in HANDOFF.md.
Read-only imported source graph check matched210 world/map bindings.
Repeated Assets preparation returned `SOLAR_CONTENT_PASS cached verified recipe`
without reimport or scene replacement; evidence `.local/solar/assets.log` and
`cached-final-assets-console.log`. User play launcher uses the same checked
Build-SolarSystem package on A:, native monitor resolution.

Five Claude systems are accepted in task0015/REVIEW.md. Its periodic monitor
is PAUSED after completion, and the last checked worker source remains61bde4f.
No worker checkout, active Blender document or user application was overwritten.
Credentials, machine-specific toolchain/cache/player data and raw logs stay ignored.
