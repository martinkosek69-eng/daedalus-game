# 0009 — Restore the original web flight presentation

Owner: Codex coordinator. Base: 6150226f4bbd0d8b011ae2727fcf954196e0874f.
Branch: codex/solar-flight. Review target: codex/game-foundation.
Allowed: Tools, Art/Ships/Daedalus/WebReference, Game/Daedalus solar
presentation/content/config, public Docs and coordinator Tasks.
Root owns Unreal, background Blender and source imports. Helper
web_controls_match owns only Content/Data/Solar/flight.json; no editor/build.

User rejected the pale, simplified-looking current ship. Restore the original
web hull and its procedural panel recipe, lighting and flight feel. Disable
camera motion blur/temporal smearing, use a compact flight HUD, preserve Earth,
singleplayer domain separation and existing foundation. Do not modify original
backup or overwrite Claude's source asset. Save the comparison source separately.
Retain a separate new package so the user's running prior version remains safe.

Check background source reopen/export, original geometry dimensions, material
compile, C++ build and packaged flight/camera probe. Inspect actual screenshots.
The user will judge the new artistic/control tuning interactively.
