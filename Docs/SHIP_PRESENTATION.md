# Latest Daedalus presentation — integration0025

Source delivery: task0008 branch through7c35635. Preserve exact600m hull,
222330 triangles,5 source PBR slots, original licensing/attribution and the
source colour/UV/normal maps. Material variants isolate hull vs add-ons when
their exported vertex-colour recipes differ. Do not restore obsolete extra
turret barrels; the original domes and guns are already in the hull.

`Tools/Prepare-ShipPresentation.py` imports add-ons and four split hangar doors,
two light meshes and separate mast beacons. Full recipe saves source hashes,
asset paths, animation extras and ten source point lights to
Content/Data/Solar/ship-details.json version2. Source Blender(x,y,z) becomes
UE(x,-y,z)*100; glTF(x,y,z) becomes UE(x,z,y)*100. Interchange bakes mesh
transforms; opening offsets must never apply the node placement twice.
Light positions resolve the complete node hierarchy before conversion.

`Solar/ShipPresentation.cpp` only updates derived presentation:

- Six engine glows retain their baked positions. Throttle changes their
  emissive/point-light power; the combined core/plume meshes are not scaled.
- Mast beacons use source5s period,1/3s on,emission8/0. Flight clock controls
  them, so pause/map freezes their phase.
- H opens/closes all four hangar door halves over two seconds with easing.
  Upper/lower halves slide±5.6m on localZ. Pause/map stops travel and blocks H.
  This is a model inspection preview, not a shuttle/hangar gameplay system.
- Four bay lights keep source colour/position/radius but use0.01 visual
  intensity scale to suit the lab's fixed exposure/2.8-lux sun. Blender's much
  brighter scene illumination otherwise overexposed the bay fronts. Source
  candela remains unchanged in the manifest. Six engine lights retain their
  source power, scaled only by the existing engine visual level.

No combat, firing, missiles, energy allocation or rotating turret domes are
implemented. Claude supplied no separate moving turret domes; adding rotation
would require extracting the existing dome geometry. Original .blend preview
keyframes are not silently treated as Unreal animations.

Reproduce: full `Invoke-SolarFlight.ps1 -Mode Assets`, read-only
Validate-IntegratedContent.py commandlet, compile/test/package, then
`Tools/Invoke-IntegrationProbe.ps1`. The probe uses isolated player data,
native3840x2160 captures, actual H/R/map/pause input and component/material
checks. Most aesthetic/flying evaluation remains with the user.
