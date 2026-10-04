# 0025 — Společná hratelná verze: planety, HUD a nejnovější Daedalus

- Owner: Codex coordinator. Base:70b2d5bda7911e8fce9ed414e475f7dcecccc782.
- Branch/checkout: codex/solar-flight, own A: worktree.
- User authorizes completed0023 integration,0024 HUD/minimap into the current
  playable version, and newest0008 model/animations. Main Git branch is not
  merged; current game delivery is the coordinator branch and normal launcher.
- Inputs: planet headaa9072c (explicit editor/build release), ship head7c35635.
- Codex now owns one build/editor and imported ship assets in its own checkout.
  Never touch Claude's checkout or human apps. Preserve player saves/old package.

Allowed: reviewed deliveries of0023/0008; Tools/Prepare-SolarContent.py,
Prepare-DaedalusMaterials.py, own new ship preparation/validation helpers;
Tools/Prepare-DaedalusDetail.py and Validate-DaedalusDelivery.mjs metadata
inventory corrections and current multi-piece delivery checks;
Art/Ships/Daedalus/**, Game/Daedalus/Content/Ships/Daedalus/**, relevant generated
Solar ship materials/effects and Content/Data/Solar/ship-details.json;
SolarFlightGameMode.cpp/.h, new Solar/Ship*.h/.cpp, SolarHUD*, probe files and
targeted tests for presentation; launcher delivery selection and task/docs.
No new combat, shuttles, physics changes or state mutation through visuals.

Deliver: current model with latest materials, lights/blinking beacons, engine
effects and split hangar doors preserved as separate animated pieces; no
obsolete added turret barrels. Source metadata drives animations. Preserve
600m scale and handedness. All-source/import dependencies and provenance remain
versioned. Native4K HUD with functional read-only minimap, other systems deferred.

Acceptance: fetch/LFS SHA verification, proper scale/material/animation bindings,
editor build and relevant radar/presentation tests, content validators,
separate Windows package and targeted native4K rendered check. Most subjective
testing belongs to user; avoid unnecessary repeats. Update launcher only after
verified package. Publish normal checkpoint/push and truthful handoff/checks.
