# 0026 — Aurora-class: reviewed model and distinct playable flight

Owner: Codex coordinator. Base:62ec47d9a7a157ab5a845e7e21db5e9d2c4a1cd2.
Branch: codex/solar-flight; own A: checkout. Input: reviewed source commit
a193736 from claude/aurora-ancient-warship (Blender look-development asset).

User authorizes review, Internet research and full integration of Aurora as
a second controllable ship, after0025. Singleplayer flight lab only. Preserve
Daedalus, existing systems, player saves and previous verified package. No
combat, energy gameplay or new hyperspace mechanics. Codex owns isolated
background Blender/import/build processes; never touch a human application's
session or another agent's checkout.

Allowed: Art/Ships/Aurora/**; Tools ship preparation/import/validators/probe and
delivery launcher; Game/Daedalus/Content/Ships/Aurora/** and Data/Solar/**;
Solar runtime, HUD, ship presentation and flight configuration/tests; relevant
documentation and Tasks/0026 + INDEX. Source/license attribution retained.

Deliver: inspect actual saved scene, orientation/scale/geometry/materials;
reproducible game export with sharp tiled source textures and original paint;
ship catalog with independent flight tuning through the shared flight model;
safe lab ship selection, matching name/outline/camera and conservative collision
radius. Aurora is larger/slower turning but has technologically stronger drive.
Separate sourced story facts from game balancing assumptions; uncertain physical
dimensions are explicitly a working game scale.

Checks: model/material import validation, editor build, targeted domain tests
and one native3840x2160 package render/switch check. Most subjective flight/style
testing belongs to user. New separate package; update launcher after successful
checks, publish normal commit/push and honest handoff with limitations.
Latest user steering: P must open the pause/options menu (final game will use
ESC). Ship selection belongs there, rather than direct F1/F2 keys. Include exit,
settings and save/load positions; currently unavailable flight save features
must be clearly labelled. Test real mouse hit targets at native4K and preserve
pause across the ship swap.
