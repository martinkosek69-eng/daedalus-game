# Original web presentation comparison

Generated from the unchanged `../Original/daedalus.glb`, Astrofossil,
Thingiverse 2256025, recorded CC BY-NC 4.0. Same noncommercial prototype scope
and attribution as ../SOURCE.md. No original hull vertex is redesigned.

`Tools/Prepare-WebDaedalus.py` transforms the original 222330-triangle hull
to +X forward/+Z up, 600 metres, then reproduces the original web runtime
fittings: 14 closed hatch sets, 36 vent slats and 30 small windows.
Total 223458 triangles. These fittings are the web prototype's interpretation,
not claims about the film model. Claude's separate source remains preserved.

The original web livery recipe is evaluated per pixel in
`Tools/Prepare-SolarContent.py`: staggered 27×43 normalized plate grid,
antialiased dark seams, panel variation, recessed underside, dark spine and
engine metal. Colors are not approximated by interpolation across hull
vertices. Object-local coordinates keep the paint fixed to the moving ship.
No image textures or external dependencies. Metadata records source/export
hashes and saved Blender reopen. The source scene is fully editable.

Unreal solar presentation uses this comparison mesh. Existing six engine
effects remain separate and aligned. Bright hangar paint from the photograph
revision is not used. Actual moving weapons remain future work.
