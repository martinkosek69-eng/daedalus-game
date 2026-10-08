"""Task 0019: preserve real partial mosaics and generate radial ring views.

Pillow/numpy asset recipe. Reads source catalogs/assets; writes SolarDetails only.
--offline reproduces all deliverables from saved public source images.
"""
import argparse
import hashlib
import json
from pathlib import Path
from urllib.request import Request, urlopen
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "Art/Space/SolarDetails"
TEXTURES = OUT / "Textures"
SOURCES = OUT / "Sources"
WIDTH, HEIGHT = 8192, 32
MAPS = [
    dict(id="sol.pluto", name="pluto", code="PIA20658",
         url="https://d2pn8kiwq2w21t.cloudfront.net/original_images/jpegPIA20658.jpg",
         page="https://www.jpl.nasa.gov/images/pia20658-pluto-a-global-perspective/"),
    dict(id="sol.moon.jpl_901", name="charon", code="PIA19866",
         url="https://d2pn8kiwq2w21t.cloudfront.net/original_images/jpegPIA19866.jpg",
         page="https://www.jpl.nasa.gov/images/pia19866-global-map-of-plutos-moon-charon/")]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save_json(name, payload):
    (OUT / name).write_text(json.dumps(payload, ensure_ascii=False, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def real_maps(offline):
    records = []
    for source in MAPS:
        path = SOURCES / (source["code"] + ".jpg")
        if not path.exists():
            if offline:
                raise FileNotFoundError(path)
            with urlopen(Request(source["url"], headers={"User-Agent": "Daedalus-source-assets/1.0"}), timeout=60) as response:
                payload = response.read()
            assert payload[:2] == b"\xff\xd8", "Expected JPEG, not a redirect HTML page"
            path.write_bytes(payload)
        image = Image.open(path).convert("RGB")
        assert image.width == 2 * image.height, (source["code"], image.size)
        # Black source no-data remains black. No inpainting or invented texture.
        gray = np.array(image.convert("L"))
        mask = Image.fromarray(np.where(gray > 2, 255, 0).astype(np.uint8))
        result = image.resize((4096, 2048), Image.Resampling.LANCZOS)
        name = source["name"] + "_lorri_4k.jpg"
        result.save(TEXTURES / name, quality=96, subsampling=0)
        mask_name = source["name"] + "_observation_mask.png"
        mask.resize((4096, 2048), Image.Resampling.NEAREST).save(TEXTURES / mask_name)
        records.append(dict(bodyId=source["id"], texture="Textures/" + name,
            observationMask="Textures/" + mask_name, sourceUrl=source["page"], downloadUrl=source["url"],
            sourceFile="Sources/" + path.name, sourceSha256=digest(path), sourceSize=list(image.size),
            outputSize=[4096, 2048], surfaceQuality="real-partial-panchromatic-New-Horizons-mosaic",
            projection="simple cylindrical; north at top; Charon zero longitude at centre, Pluto encounter hemisphere at centre; no authoritative runtime spin phase implied",
            credit="NASA/Johns Hopkins University Applied Physics Laboratory/Southwest Research Institute",
            licence="JPL Image Use Policy; image-specific credit retained; no endorsement",
            maskPolicy="black/near-black no-data threshold <=2 in decoded source luminance; conservative visual coverage indicator, not a calibrated scientific confidence map",
            note="Observed far hemisphere is genuinely low resolution. Pluto's 2400-wide source is resampled upward for a uniform 4k pipeline; no added observed detail. Black unmapped southern area is not a dark geological feature; no photographic color or relief reconstructed."))
        print(source["code"], "source", image.size, "to", result.size, flush=True)
    return records


def schematic_maps():
    records = []
    # Original seam-safe spherical albedo variation, explicitly NOT observations.
    latitude = np.linspace(np.pi / 2, -np.pi / 2, 1024)[:, None]
    longitude = np.linspace(-np.pi, np.pi, 2048, endpoint=False)[None, :]
    x = np.cos(latitude) * np.cos(longitude)
    y = np.cos(latitude) * np.sin(longitude)
    z = np.sin(latitude)
    for name, color, phase in (("eris", [190, 188, 181], 1), ("makemake", [156, 125, 109], 2),
                               ("haumea", [180, 182, 177], 3), ("ceres", [106, 104, 99], 4)):
        value = 1 + .075 * np.sin(8 * x + 3 * y + phase) * np.cos(9 * z - y)
        value += .025 * np.sin(24 * x - 11 * y + phase) * np.cos(15 * z)
        pixels = np.clip(value[:, :, None] * np.array(color)[None, None, :], 0, 255).astype(np.uint8)
        filename = name + "_schematic_albedo.jpg"
        Image.fromarray(pixels).save(TEXTURES / filename, quality=95, subsampling=0)
        records.append(dict(bodyId="sol." + name, texture="Textures/" + filename,
            sourceUrl=None, credit="Daedalus project original procedural schematic",
            licence="Project-owned original generated map", outputSize=[2048, 1024],
            surfaceQuality="explicit-fictional-unmapped-schematic", observedTerrain=False,
            note="Smooth authored albedo placeholder, not a photographed map, measured crater distribution or reconstructed terrain. Ceres has Dawn observations, but none is imported by this recipe."))
    return records


def ring_geometry(source):
    """Display radial bounds, retaining every source record and unknowns."""
    item = dict(sourceRecord=source, componentId=source["id"])
    if source.get("innerRadiusMetres") is not None:
        item.update(innerMetres=source["innerRadiusMetres"], outerMetres=source["outerRadiusMetres"],
                    geometryPolicy="published broad inner/outer radial edges")
        return item
    radius = source.get("characteristicRadiusMetres")
    if radius is None:
        item.update(rendered=False, reason="published numeric geometry unavailable; no invented radial band")
        return item
    width = source.get("widthMetres")
    policy = "published summary reference radius +/- half width"
    if source.get("widthRangeMetres"):
        width = sum(source["widthRangeMetres"]) / 2
        policy = "arithmetic representative of published width range; circular radial average ignores eccentricity/azimuthal width variation"
    if source["parentId"] == "sol.neptune":
        # Dedicated NSSDCA table gives useful widths omitted/different in NASA
        # Science's summary. Keep both sources and explicitly select display.
        widths = {"Galle": 2000000, "Leverrier": 50000, "Lassell": 4000000,
                  "Arago": 50000, "Adams": 15000}
        width = widths[source["name"]]
        item["widthSupplement"] = dict(sourceUrl="https://nssdc.gsfc.nasa.gov/planetary/factsheet/nepringfact.html",
            reference="NASA NSSDCA ring fact sheet, updated 2015-10-14",
            measuredWidthMetres=width if source["name"] in ("Galle", "Lassell", "Adams") else None,
            widthUpperLimitMetres=100000 if source["name"] in ("Leverrier", "Arago") else None,
            note="NASA Science original source retained. Galle width differs; Lassell/Arago geometry is absent there. Selected widths from dedicated historical table, reference radius kept from assigned catalog. Upper-limit widths choose authored 50 km representative, not a measurement.")
        policy = "assigned reference radius +/- selected historical width/2; mixed-source approximate circular display model"
    if width is None and source.get("widthUpperLimitMetres") is not None:
        width = min(100000, source["widthUpperLimitMetres"] / 2)
        policy = "authored representative below published upper limit; not a measured width"
    if width is None:
        item.update(rendered=False, reason="width unknown; no invented radial band")
        return item
    if "inner edge" in source.get("radiusDefinition", ""):
        inner, outer = radius, radius + width
        policy += "; published reference radius is inner edge"
    else:
        inner, outer = radius - width / 2, radius + width / 2
    item.update(innerMetres=inner, outerMetres=outer, selectedWidthMetres=width, geometryPolicy=policy)
    return item


def ring_atlases():
    path = ROOT / "Art/Space/SolarCatalog/rings.json"
    original = json.loads(path.read_text(encoding="utf-8"))["rings"]
    views = []
    inove = ROOT / "Art/Space/SolarSystem/Textures/saturn_ring_alpha.png"
    albedo = np.array(Image.open(inove).convert("RGBA"))[62].astype(float) / 255
    for planet in ("jupiter", "saturn", "uranus", "neptune"):
        components = [ring_geometry(r) for r in original if r["parentId"] == "sol." + planet]
        shown = [r for r in components if "innerMetres" in r]
        inner, outer = min(c["innerMetres"] for c in shown), max(c["outerMetres"] for c in shown)
        # Exact per-pixel covered length preserves subpixel narrow rings rather
        # than widening their physical bounds to make them visible.
        edges = np.linspace(inner, outer, WIDTH + 1)
        centres = (edges[:-1] + edges[1:]) / 2
        step = (outer - inner) / WIDTH
        premultiplied = np.zeros((WIDTH, 3)); alpha = np.zeros(WIDTH)
        for c in shown:
            source, name = c["sourceRecord"], c["sourceRecord"]["name"]
            coverage = np.maximum(0, np.minimum(edges[1:], c["outerMetres"]) - np.maximum(edges[:-1], c["innerMetres"])) / step
            assert abs(np.sum(coverage) * step - (c["outerMetres"] - c["innerMetres"])) < .01
            base_color = [145, 140, 131]
            if planet == "saturn":
                amount = {"D": .055, "C": .23, "B": .78, "Cassini Division": .035,
                          "A": .61, "F": .35, "G": .04, "E": .025}[name]
                base_color = [186, 174, 150]
            elif planet == "jupiter":
                amount = {"Halo": .075, "Main": .24, "Amalthea": .065, "Thebe": .045, "Thebe Extension": .025}[name]
            elif planet == "uranus":
                amount = .05 if name in ("Mu", "Nu") else .48
                base_color = [98, 102, 102] if name not in ("Mu", "Nu") else [109, 131, 158] if name == "Mu" else [145, 108, 92]
            else:
                amount = {"Galle": .045, "Leverrier": .24, "Lassell": .055, "Arago": .16, "Adams": .28}[name]
                base_color = [120, 126, 130]
            colors = np.tile(np.array(base_color) / 255, (WIDTH, 1))
            local = amount * coverage
            if planet == "saturn" and name in ("C", "B", "Cassini Division", "A", "F"):
                fraction = (centres - 74500000) / (140220000 - 74500000)
                valid = (fraction >= 0) & (fraction <= 1)
                indices = np.arange(albedo.shape[0])
                for channel in range(3):
                    colors[valid, channel] = np.interp(fraction[valid] * (len(indices) - 1), indices, albedo[:, channel])
                source_alpha = np.interp(np.clip(fraction, 0, 1) * (len(indices) - 1), indices, albedo[:, 3])
                # Preserve artist source gaps; optical depth remains source data.
                local[valid] *= source_alpha[valid]
            premultiplied += colors * local[:, None]
            alpha += local
            c.update(rendered=True, radialCoveragePixels=float(np.sum(coverage)), authoredPeakAlpha=amount,
                     appearancePolicy="original display alpha/RGB for readable rings, not calibrated optical-depth-to-brightness transfer")
        colors = np.divide(premultiplied, alpha[:, None], out=np.zeros_like(premultiplied), where=alpha[:, None] > 0)
        pixels = np.round(np.column_stack((colors, np.clip(alpha, 0, 1))) * 255).astype(np.uint8)
        filename = planet + "_rings_rgba.png"
        Image.fromarray(np.tile(pixels[None, :, :], (HEIGHT, 1, 1))).save(TEXTURES / filename)
        views.append(dict(parentId="sol." + planet, innerMetres=inner, outerMetres=outer,
            texture="Textures/" + filename, widthPixels=WIDTH, heightPixels=HEIGHT,
            radialUv="U=(planet-centred radius-innerMetres)/(outerMetres-innerMetres); V=0.5",
            textureAddress="clamp", rgbColorSpace="sRGB", alphaColorSpace="linear", alphaPremultiplied=False,
            plane="parent-equatorial", components=components,
            sourceCatalog="Art/Space/SolarCatalog/rings.json", sourceCatalogSha256=digest(path),
            note="Axisymmetric radial display. Does not reconstruct eccentric azimuthal ring edges, Neptune arcs or physical vertical halo thickness. Subpixel widths integrated by exact pixel coverage."))
        print(planet, len(shown), "/", len(components), "rings", inner, outer, flush=True)
    save_json("ring-views.json", dict(version=1, views=views,
        inoveSaturnSource=dict(path="Art/Space/SolarSystem/Textures/saturn_ring_alpha.png", sha256=digest(inove),
            originalRadialRangeMetres=[74500000, 140220000], credit="Solar System Scope / INOVE",
            licence="CC BY 4.0", adaptation="reprojected radial RGB/alpha sample into full Saturn physical-radius view")))
    return views


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--offline", action="store_true")
    parser.add_argument("--maps-only", action="store_true")
    options = parser.parse_args()
    TEXTURES.mkdir(parents=True, exist_ok=True)
    SOURCES.mkdir(parents=True, exist_ok=True)
    records = real_maps(options.offline)
    if not options.maps_only:
        records += schematic_maps()
        views = ring_atlases()
    save_json("surface-views.json", dict(version=1, sourceReviewDate="2026-10-04", surfaces=records))
    print("SOLAR DETAILS MAP PASS", len(records))


if __name__ == "__main__":
    main()
