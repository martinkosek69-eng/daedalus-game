# 0019 — Dwarf surface sources and complete ring atlases
Owner Codex sourcehelper, base6c8db7b, codex/solar-flight. Root integratesbuilds/Git.
AllowedONLY Art/Space/SolarDetails/**, Tools/Prepare-SolarDetails.py,
Tasks/0019/PROGRESS.md,HANDOFF.md. Read shareddocs,0014handoff/source schemas.
No existing Art/SolarSystem/SolarCatalog/catalog/Game/Toolsroot edits/appsGUI.
1.FindprimaryNASA/JPL/ESA permissiblyusableequirectangularPluto/Charon real
NewHorizons maps (publicsource,citedlicence), prefer2k/4k, handleunobservedarea
honestly. Ifunavailable useexplicitownschematicicy dwarf maps (Pluto/Eris/Makemake/
Haumea/Ceres only) withloggedfictional/unmapped detail; don'tclaimmeasuredterrain.
2.Read Art/Space/SolarCatalog/rings.json (31records) andcreate RGBA radialatlas
foreachJupiter/Saturn/Uranus/Neptune,8192x32. JSONring-views.json perplanet
innerMetres/outerMetres/texture path, componentrecordscoverage. Broadedges and
narrowradius+widthrange cases separate; geometrymissingZeta remainsunknown.
Use sourceedges andwidthranges: chosen representative widths explicitlynoted.
Jupiter/Neptunefaint dusty, Saturn brighticewithgaps, Uranusdarknarrow. Satellite
photometricrealbrightnessisnotguaranteed: sourceopticaldepthkeptseparatefrom
moderateauthoredalphaforreadability. PreserveexistingINOVEsaturnsourcealbedo
whereuseful, mapits original74.5..140.22Mmradialrangecorrectly intofullatlas.
No arbitrarydenseopaqueoutercloud. Document usedphysicalsources/maps/source
licenses/uncertainvalues/readabilitychoices. Skipunknowngeometriesdon'tinvent.
3.Save repeatablePythonrecipe stdlib/Pillow/numpy availablethrough bundled
runtimeorBlenderpython. Sourcefiledecode/atlasmath/hashchecks/offlinereproduce.
No giant3DsystemGLBs; independentmaps only. Root mapsruntime.sourcePaths.
NoGit/build/Unreal/userBlenderGUI. BackgroundsavedBlenderallowedonlyifnecessary.
Deliverearlyringviews +mapfilenames forrootpipeline. Checked sourcefiles
andknownlimitationsHANDOFF READY_FOR_REVIEW. LatestClaudeplanetassets separate.

## Root-authorized continuation after source review
2026-10-04: worker additionally owns Tools/Prepare-SolarSystemMaterials.py and
Tools/Prepare-SolarContent.py for nullable expanded catalogs, all six available
systems, world/map materials, source texture paths, five irregular unit meshes
and dependency/cache validation. No application/build/Git calls. Root integrates.
