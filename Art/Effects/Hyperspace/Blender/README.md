# Green rupture — Blender source (0030)

Daedalus_GreenRupture.blend is an editable offline animation scene, authored
in Blender 5.2.2. The effect uses 28 independently folded mesh membranes with
84 keyed deformation shapes, tapered spatial fracture curves and a bounded
emissive volume with animated procedural structure. Opening and collapse are
keyframed; no external simulation cache or downloaded effect assets are needed.

Native 3840x2160, 24 fps, frames 1-108 (4.5 seconds), EEVEE, 48 render samples.
Motion blur and depth of field are disabled. The compositor adds limited glow
around emitted light; volume softness is intentional light structure, not an
upscaled background image. Source appearance still requires user review.

Daedalus GLBs are read-only dependencies from Art/Ships/Daedalus, imported as
context and packed into the scene. Their geometry and source textures are not
modified. Scene-only materials clip the bow-first crossing at the rotated window
plane through X=2, using its actual normal. Local lights
are rescaled for the miniature rehearsal: one scene unit represents 200 m.
This does not define or modify authoritative flight state or propulsion.

Regenerate: Tools/Invoke-BlenderHyperspace.ps1 -Mode Create.
Reload dependency check: same helper -Mode Audit.
Render saved scene: -Mode Render; optional -Frames '20,44,61,90' or '1:108'.
Render resumes only complete PNGs under frames-final and rejects a changed
scene hash. After editing the scene, preserve old frames separately before
rerendering to avoid mixing revisions. Encode: -Mode Video.
All outputs stay under ignored .local/hyper/blender0030 on A:.

Existing original-series reference clips are private local comparison inputs,
not shipped textures or published footage. Reference observations and sources:
Tasks/0029/REFERENCE_REVIEW.md and Tasks/0028/REFERENCES.json. New effect geometry,
animation and material nodes are original project work. Existing ship provenance
remains in Art/Ships/Daedalus/SOURCE.md and REFERENCES.md.

This is not an Unreal-ready import: Blender volume/compositor nodes do not
transfer as ordinary glTF materials. After visual approval, preserve the timing
and geometry and explicitly adapt/bake the effect for Unreal, with another
native 4K in-game review. No normal game launcher or game materials are replaced.
