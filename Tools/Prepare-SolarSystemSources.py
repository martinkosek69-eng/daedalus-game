"""Recover the protected web solar catalog and make owned schematic rock assets.

Run with Blender --background --python this_file -- --prototype <web dist>.
No open GUI file is touched. Provider maps are copied unchanged if missing;
existing reviewed high-resolution copies are preserved. This recipe writes the
single canonical runtime catalog, source manifest, and its assigned Art folder.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import shutil
import struct
import sys

import bpy
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / "Art/Space/SolarSystem"
TEXTURES = ART / "Textures"
CATALOG = ROOT / "Game/Daedalus/Content/Data/Solar/system.json"
SUN = np.array([105781669000.0, -105781669000.0, 0.0])


def field(line, name, default=None):
    found = re.search(r"\b" + re.escape(name) + r":('([^']*)'|\[[^\]]*\]|[^,}]+)", line)
    if not found:
        return default
    value = found.group(1)
    if value.startswith("'"):
        return found.group(2)
    if value.startswith("["):
        return json.loads(value)
    if value == "true":
        return True
    return float(value)


def recover_rows(prototype):
    text = (prototype / "data.mjs").read_text(encoding="utf-8")
    planets = []
    asteroids = []
    mode = None
    for line in text.splitlines():
        if line.startswith("export const planets="):
            mode = planets
        elif line.startswith("export const asteroids="):
            mode = asteroids
        elif line.startswith("];"):
            mode = None
        if mode is not None and "{id:" in line:
            mode.append({name: field(line, name) for name in
                         ("id", "name", "radius", "au", "texture", "rotation",
                          "tilt", "phase", "atmosphere", "axes", "equatorial")})
    moons = []
    for match in re.finditer(r"\['([^']*)','([^']*)','([^']*)',([\d.]+),([\d.]+),'([^']*)',(-?[\d.]+)\]", text):
        ident, name, parent, radius, orbit, texture, rotation = match.groups()
        moons.append(dict(id=ident, name=name, parent=parent, radius=float(radius),
                          orbit=float(orbit), texture=texture, rotation=float(rotation),
                          phase=1.2 + len(moons) * 2.4, tilt=0))
    assert (len(planets), len(moons), len(asteroids)) == (9, 24, 4)
    return planets, moons, asteroids


def write_catalog(planets, moons, asteroids):
    # Web +Y was up and +Z the second orbital-plane axis. This is a fixed
    # illustrative epoch, not a scientific ephemeris or changing orbital state.
    au_metres = float(np.linalg.norm(SUN))
    angle = math.atan2(-SUN[1], -SUN[0]) - .7
    c, s = math.cos(angle), math.sin(angle)

    def transform(raw):
        x, vertical, orbital_y = raw
        return SUN + np.array([c * x - s * orbital_y,
                               s * x + c * orbital_y, vertical])

    raw_positions = {}
    output = []
    equatorial_km = dict(mercury=2440.53, venus=6051.8, earth=6378.1366,
                         mars=3396.19, jupiter=71492, saturn=60268,
                         uranus=25559, neptune=24764)
    for body in planets + moons + asteroids:
        ident = body["id"]
        phase = body.get("phase") or 0
        parent = body.get("parent")
        if parent:
            orbit = body["orbit"] * 1000
            raw = raw_positions[parent] + np.array([
                math.cos(phase) * orbit,
                math.sin(phase) * orbit * math.sin(.12),
                math.sin(phase) * orbit * math.cos(.12)])
        else:
            orbit = body["au"] * au_metres
            raw = np.array([math.cos(phase) * orbit, 0, math.sin(phase) * orbit])
        raw_positions[ident] = raw
        position = transform(raw)
        if ident == "earth":
            position = np.zeros(3)  # Preserve exact existing lab reference.
        texture = body.get("texture") or "rock_schematic.png"
        if ident in ("hyperion", "phoebe", "nereid", "proteus"):
            texture = ident + "_schematic.png"
        kind = "star" if ident == "sun" else "moon" if parent else \
               "asteroid" if body in asteroids else "planet"
        row = dict(id="sol." + ident, name=body["name"], kind=kind,
                   parentId="sol." + parent if parent else "" if ident == "sun" else "sol.sun",
                   radiusMetres=round(body["radius"] * 1000, 6),
                   positionMetres=[round(float(v), 6) for v in position],
                   texture=texture, rotationHours=body["rotation"] * 24,
                   tiltDegrees=body.get("tilt") or 0,
                   surfaceQuality="schematic" if "schematic" in texture else
                   "representative-cloud-map" if ident in ("venus", "jupiter", "saturn", "uranus", "neptune", "titan") else
                   "spacecraft-derived-map")
        if body.get("atmosphere"):
            row["atmosphereColor"] = body["atmosphere"]
        if ident == "earth":
            row.update(cloudTexture="earth_clouds.jpg", nightTexture="earth_nightmap.jpg")
        eq = body.get("equatorial") or equatorial_km.get(ident, body["radius"])
        if body.get("axes"):
            # Original Eros X/Z horizontal axes, Y vertical -> engine X/Y/Z.
            axes = body["axes"]
            row["shapeScale"] = [axes[0] / body["radius"], axes[2] / body["radius"], axes[1] / body["radius"]]
        elif ident in ("phobos", "deimos", "hyperion", "phoebe", "nereid", "proteus", "vesta", "bennu"):
            # Generic irregular rock mesh; volume/radius are nominal approximations.
            row["shapeScale"] = [1., 1., 1.]
        elif eq != body["radius"]:
            row["shapeScale"] = [eq / body["radius"], eq / body["radius"], (body["radius"] / eq) ** 2]
        if ident == "saturn":
            row.update(ringInnerMetres=74500000., ringOuterMetres=140220000.,
                       ringTexture="saturn_ring_alpha.png")
        if ident == "uranus":
            row.update(ringInnerMetres=38000000., ringOuterMetres=51149000.,
                       ringTexture="uranus_ring_schematic.png")
        output.append(row)
    document = dict(version=1, id="sol", layout="protected-web-reference-static-epoch",
                    bodies=output, asteroidBelt=dict(parentId="sol.sun",
                        innerMetres=2.1 * au_metres, outerMetres=3.3 * au_metres,
                        count=600, seed=3040012))
    CATALOG.parent.mkdir(parents=True, exist_ok=True)
    CATALOG.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return document


def procedural_surface(name, seed):
    """Own periodic rock color, evaluated on a sphere to avoid longitude seams."""
    width, height = 2048, 1024
    latitude = np.linspace(-math.pi / 2, math.pi / 2, height, dtype=np.float32)[:, None]
    longitude = np.linspace(-math.pi, math.pi, width, endpoint=False, dtype=np.float32)[None, :]
    x = np.cos(latitude) * np.cos(longitude)
    y = np.cos(latitude) * np.sin(longitude)
    z = np.sin(latitude) + np.zeros_like(x)
    rng = np.random.default_rng(seed)
    noise = np.zeros_like(x)
    for frequency, amplitude in ((3, .18), (7, .12), (19, .07), (49, .03), (113, .016), (257, .009)):
        axis = rng.normal(size=3)
        axis /= np.linalg.norm(axis)
        phase = rng.uniform(-math.pi, math.pi)
        noise += amplitude * np.sin(frequency * (axis[0] * x + axis[1] * y + axis[2] * z) + phase)
    for _ in range(85):
        axis = rng.normal(size=3)
        axis /= np.linalg.norm(axis)
        radius = rng.uniform(.018, .14)
        distance = np.sqrt(np.maximum(0, 2 - 2 * (axis[0] * x + axis[1] * y + axis[2] * z)))
        bowl = np.exp(-((distance / radius) ** 4))
        rim = np.exp(-((distance / radius - 1.1) / .15) ** 2)
        noise += .09 * rim - .14 * bowl
    rgb = np.clip((.42 + noise[..., None]) * np.array([1., .955, .9]), .06, .8)
    pixels = np.ones((height, width, 4), dtype=np.float32)
    pixels[..., :3] = rgb
    # Image pixels are scene-linear; the PNG writer handles display encoding.
    image = bpy.data.images.new(name, width=width, height=height, alpha=True)
    image.pixels.foreach_set(pixels.ravel())
    image.filepath_raw = str(TEXTURES / (name + ".png"))
    image.file_format = "PNG"
    image.save()
    bpy.data.images.remove(image)


def procedural_ring():
    width, height = 4096, 32
    radial = np.linspace(38000000., 51149000., width)
    bands = ((41837000., 60000., .32), (42234000., 55000., .3),
             (42571000., 45000., .25), (44718000., 70000., .32),
             (45661000., 60000., .32), (47176000., 65000., .35),
             (47627000., 90000., .4), (50024000., 80000., .35),
             (51110000., 100000., .75))
    alpha = np.zeros(width)
    for centre, sigma, opacity in bands:
        alpha += opacity * np.exp(-((radial - centre) / sigma) ** 2)
    pixels = np.zeros((height, width, 4), dtype=np.float32)
    pixels[..., :3] = [.46, .48, .5]
    pixels[..., 3] = np.clip(alpha, 0, 1)
    image = bpy.data.images.new("uranus_ring_schematic", width=width, height=height, alpha=True)
    image.pixels.foreach_set(pixels.ravel())
    image.filepath_raw = str(TEXTURES / "uranus_ring_schematic.png")
    image.file_format = "PNG"
    image.save()
    bpy.data.images.remove(image)


def asteroid_geometry():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=5, radius=1)
    obj = bpy.context.object
    obj.name = "SolarRock"
    for vertex in obj.data.vertices:
        n = vertex.co.normalized()
        amount = (1 + .075 * math.sin(n.x * 7 + n.y * 3) * math.cos(n.z * 8)
                  + .032 * math.sin(n.x * 23 + n.z * 19)
                  + .02 * math.sin(n.y * 39 + n.z * 31))
        vertex.co *= amount
    max_radius = max(v.co.length for v in obj.data.vertices)
    for vertex in obj.data.vertices:
        vertex.co /= max_radius
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    # Give this own generic shape sphere UVs, not per-face random patches.
    uv_layer = obj.data.uv_layers.new(name="UVMap")
    for polygon in obj.data.polygons:
        values = []
        for loop_index in polygon.loop_indices:
            normal = obj.data.vertices[obj.data.loops[loop_index].vertex_index].co.normalized()
            values.append([loop_index, math.atan2(normal.y, normal.x) / (2 * math.pi) + .5,
                           math.asin(normal.z) / math.pi + .5])
        crosses_seam = max(v[1] for v in values) - min(v[1] for v in values) > .5
        for index, u, v in values:
            uv_layer.data[index].uv = (u + (1 if crosses_seam and u < .5 else 0), v)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1.
    bpy.ops.wm.save_as_mainfile(filepath=str(ART / "SolarRock.blend"))
    bpy.ops.export_scene.gltf(filepath=str(ART / "SolarRock.glb"), export_format="GLB",
                              use_selection=True, export_materials="NONE")
    # Explicit reopen verifies the editable file, independent GLB checks below.
    bpy.ops.wm.open_mainfile(filepath=str(ART / "SolarRock.blend"))
    assert len(bpy.data.objects["SolarRock"].data.polygons) == 5120


def image_size(path):
    image = bpy.data.images.load(str(path), check_existing=False)
    result = list(image.size)
    assert min(result) > 0
    bpy.data.images.remove(image)
    return result


def verify_and_manifest(document, prototype):
    ids = {b["id"] for b in document["bodies"]}
    assert len(ids) == 37
    for body in document["bodies"]:
        assert body["radiusMetres"] > 0
        assert all(math.isfinite(v) for v in body["positionMetres"])
        assert not body["parentId"] or body["parentId"] in ids
        for key in ("texture", "ringTexture", "cloudTexture", "nightTexture"):
            if key in body:
                assert (TEXTURES / body[key]).is_file() or (ROOT / "Art/Space/Textures" / body[key]).is_file()
    manifest = dict(version=1, generatedWith="Blender " + bpy.app.version_string,
                    attributionDate="2026-10-04", files=[])
    for path in sorted(TEXTURES.iterdir()):
        if path.suffix not in (".png", ".jpg"):
            continue
        own = "schematic" in path.name
        original = prototype / "assets" / path.name
        size = image_size(path)
        if own:
            source = "Project-authored deterministic procedural illustration"
            licence = "Project-owned; no third-party imagery"
        elif path.name.startswith("jpl-"):
            source = "https://maps.jpl.nasa.gov/tmaps/pix/" + path.name[4:]
            licence = "JPL Image Use Policy; Courtesy NASA/JPL-Caltech; additional USGS credit"
        else:
            # Some URLs labelled8k currently contain4k pixel payloads. Preserve
            # actual retrieved URL separately from the measured dimensions.
            prefix = {"mercury.jpg": "8k", "mars.jpg": "8k", "jupiter.jpg": "8k",
                      "saturn.jpg": "8k", "moon.jpg": "8k", "sun.jpg": "8k",
                      "stars_milky_way.jpg": "8k", "venus_atmosphere.jpg": "4k"}.get(path.name, "2k")
            source = "https://www.solarsystemscope.com/textures/download/" + prefix + "_" + path.name
            licence = "CC BY 4.0; Solar System Scope / INOVE"
        payload = path.read_bytes()
        manifest["files"].append(dict(file="Textures/" + path.name,
            width=size[0], height=size[1], bytes=len(payload), sha256=hashlib.sha256(payload).hexdigest(),
            source=source, license=licence,
            unchangedFromWeb=original.exists() and payload == original.read_bytes()))
    payload = (ART / "SolarRock.glb").read_bytes()
    magic, version, total = struct.unpack_from("<III", payload)
    assert magic == 0x46546c67 and version == 2 and total == len(payload)
    json_size = struct.unpack_from("<I", payload, 12)[0]
    gltf = json.loads(payload[20:20 + json_size])
    assert len(gltf["meshes"]) == 1 and not gltf.get("images")
    triangles = sum(gltf["accessors"][p["indices"]]["count"] // 3 for p in gltf["meshes"][0]["primitives"])
    assert triangles == 5120
    manifest["rock"] = dict(file="SolarRock.glb", editableFile="SolarRock.blend", radiusMetres=1., triangles=triangles,
                             sha256=hashlib.sha256(payload).hexdigest(),
                             editableSha256=hashlib.sha256((ART / "SolarRock.blend").read_bytes()).hexdigest(),
                             license="Project-owned original generic geometry")
    manifest["sharedTextures"] = []
    for filename in ("earth_daymap.jpg", "earth_clouds.jpg", "earth_nightmap.jpg"):
        path = ROOT / "Art/Space/Textures" / filename
        manifest["sharedTextures"].append(dict(file="../Textures/" + filename,
            width=image_size(path)[0], height=image_size(path)[1],
            sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
            source="https://www.solarsystemscope.com/textures/download/8k_" + filename,
            license="CC BY 4.0; Solar System Scope / INOVE"))
    (ART / "sources.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print("SOLAR SOURCES PASS: 37 stable bodies, " + str(len(manifest["files"])) +
          " decoded texture files, 5120-triangle saved/reopened own rock")


def main():
    bpy.context.preferences.filepaths.file_preview_type = "NONE"
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--prototype", required=True, type=Path)
    parser.add_argument("--verify-only", action="store_true")
    options = parser.parse_args(arguments)
    prototype = options.prototype.resolve()
    if options.verify_only:
        bpy.ops.wm.open_mainfile(filepath=str(ART / "SolarRock.blend"))
        assert len(bpy.data.objects["SolarRock"].data.polygons) == 5120
        document = json.loads(CATALOG.read_text(encoding="utf-8"))
        verify_and_manifest(document, prototype)
        return
    planets, moons, asteroids = recover_rows(prototype)
    TEXTURES.mkdir(parents=True, exist_ok=True)
    for body in planets + moons:
        name = body.get("texture")
        if name and not (TEXTURES / name).exists() and not (ROOT / "Art/Space/Textures" / name).exists():
            shutil.copyfile(prototype / "assets" / name, TEXTURES / name)
    for name in ("saturn_ring_alpha.png", "stars_milky_way.jpg"):
        if not (TEXTURES / name).exists():
            shutil.copyfile(prototype / "assets" / name, TEXTURES / name)
    document = write_catalog(planets, moons, asteroids)
    for index, name in enumerate(("rock", "hyperion", "phoebe", "nereid", "proteus")):
        procedural_surface(name + "_schematic", 3040012 + index)
    procedural_ring()
    asteroid_geometry()
    verify_and_manifest(document, prototype)


if __name__ == "__main__":
    main()
