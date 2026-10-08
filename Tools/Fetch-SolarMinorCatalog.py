"""Fetch bounded public JPL catalogs, or reproduce normalized data offline.

Standard-library Python only. Network requests are sequential and cached as
public scientific data snapshots. Never edits the canonical game catalog/assets.
"""
import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
from html.parser import HTMLParser
import json
import math
from pathlib import Path
import re
import time
from urllib.parse import urlencode
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "Art/Space/SolarCatalog"
AU = 149597870700.0
OBLIQUITY = math.radians(23.439291111)
CURATED = ["1", "2", "4", "10", "16", "243", "433", "65803", "101955", "162173", "25143",
           "486958", "134340", "136199", "136108", "136472", "50000", "90377",
           "90482", "225088", "1P", "2P", "9P", "19P", "21P", "67P", "81P", "103P", "109P"]
DWARFS = {"1", "134340", "136199", "136108", "136472"}
LEGACY = {"301": "moon", "401": "phobos", "402": "deimos", "501": "io",
          "502": "europa", "503": "ganymede", "504": "callisto", "601": "mimas",
          "602": "enceladus", "603": "tethys", "604": "dione", "605": "rhea",
          "606": "titan", "607": "hyperion", "608": "iapetus", "609": "phoebe",
          "701": "ariel", "702": "umbriel", "703": "titania", "704": "oberon",
          "705": "miranda", "801": "triton", "802": "nereid", "808": "proteus"}
MINOR_LEGACY = {"1": "ceres", "4": "vesta", "433": "eros", "101955": "bennu",
                "134340": "pluto", "136199": "eris", "136108": "haumea", "136472": "makemake"}


class TableParser(HTMLParser):
    def __init__(self, wanted):
        super().__init__(convert_charrefs=True)
        self.wanted = wanted
        self.active = False
        self.in_body = False
        self.cell = None
        self.row = None
        self.rows = []

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag == "table" and (attrs.get("id") == self.wanted or self.wanted in attrs.get("class", "").split()):
            self.active = True
        if not self.active:
            return
        if tag == "tbody":
            self.in_body = True
        elif tag == "tr" and self.in_body:
            self.row = []
        elif tag == "td" and self.row is not None:
            # HTML permits omitted </td>; current JPL physical table uses it.
            if self.cell is not None:
                self.row.append(" ".join("".join(self.cell).split()))
            self.cell = []
        elif tag == "br" and self.cell is not None:
            self.cell.append(" ")

    def handle_data(self, data):
        if self.cell is not None:
            self.cell.append(data)

    def handle_endtag(self, tag):
        if not self.active:
            return
        if tag == "td" and self.cell is not None:
            self.row.append(" ".join("".join(self.cell).split()))
            self.cell = None
        elif tag == "tr" and self.row is not None:
            if self.row:
                self.rows.append(self.row)
            self.row = None
        elif tag == "tbody":
            self.in_body = False
        elif tag == "table":
            self.active = False


def number(raw):
    if raw is None or str(raw).strip() in ("", "n/a", "-", "--"):
        return None
    try:
        value = float(str(raw).replace(",", ""))
        return value if math.isfinite(value) else None
    except ValueError:
        return None


def epoch_jd(epoch):
    # JPL table's civil-day fractional format, e.g. 2000-01-01.5 TDB.
    match = re.fullmatch(r"(\d{4})-(\d{2})-(\d{2})(\.\d+)?", epoch)
    assert match, epoch
    year, month, day, fraction = match.groups()
    date = datetime(int(year), int(month), int(day), tzinfo=timezone.utc)
    return date.timestamp() / 86400 + 2440587.5 + float(fraction or 0)


def cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def normalized(vector):
    length = math.hypot(*vector)
    assert length > 0
    return [x / length for x in vector]


def kepler(orbit):
    """Bound ellipse at source epoch; relative, right-handed ecliptic J2000."""
    a, e = orbit.get("semiMajorAxisMetres"), orbit.get("eccentricity")
    keys = ("meanAnomalyDegrees", "argumentPeriapsisDegrees", "inclinationDegrees", "ascendingNodeDegrees")
    if not a or e is None or not 0 <= e < 1 or any(orbit.get(k) is None for k in keys):
        return None
    mean, argument, inclination, node = [math.radians(orbit[k]) for k in keys]
    mean = (mean + math.pi) % (2 * math.pi) - math.pi
    anomaly = mean
    for _ in range(60):
        change = (anomaly - e * math.sin(anomaly) - mean) / (1 - e * math.cos(anomaly))
        anomaly -= change
        if abs(change) < 1e-13:
            break
    assert abs(anomaly - e * math.sin(anomaly) - mean) < 1e-10
    x = a * (math.cos(anomaly) - e)
    y = a * math.sqrt(1 - e * e) * math.sin(anomaly)
    x, y = x * math.cos(argument) - y * math.sin(argument), x * math.sin(argument) + y * math.cos(argument)
    v = [x * math.cos(node) - y * math.cos(inclination) * math.sin(node),
         x * math.sin(node) + y * math.cos(inclination) * math.cos(node), y * math.sin(inclination)]
    if orbit.get("frame", "ecliptic").lower() != "ecliptic":
        ra, dec = orbit.get("poleRightAscensionDegrees"), orbit.get("poleDeclinationDegrees")
        if ra is None or dec is None:
            return None
        ra, dec = math.radians(ra), math.radians(dec)
        pole = [math.cos(dec) * math.cos(ra), math.cos(dec) * math.sin(ra), math.sin(dec)]
        # JPL longitude zero is the reference-plane node on the ICRF equator.
        axis_x = normalized(cross([0., 0., 1.], pole))
        axis_y = cross(pole, axis_x)
        eq = [v[0] * axis_x[i] + v[1] * axis_y[i] + v[2] * pole[i] for i in range(3)]
        v = [eq[0], math.cos(OBLIQUITY) * eq[1] + math.sin(OBLIQUITY) * eq[2],
             -math.sin(OBLIQUITY) * eq[1] + math.cos(OBLIQUITY) * eq[2]]
    assert a * (1 - e) - 1 < math.hypot(*v) < a * (1 + e) + 1
    return v


def fetch(url):
    request = Request(url, headers={"User-Agent": "Daedalus-source-catalog/1.0 (bounded educational prototype)"})
    with urlopen(request, timeout=60) as response:
        return response.read()


def table_snapshot(name, url, table_id, offline):
    path = OUT / (name + "-table.json")
    if offline:
        return json.loads(path.read_text(encoding="utf-8"))
    payload = fetch(url)
    parser = TableParser(table_id)
    parser.feed(payload.decode("utf-8-sig"))
    assert parser.rows
    snapshot = dict(sourceUrl=url, retrievedUtc=datetime.now(timezone.utc).isoformat(),
                    sourcePayloadSha256=hashlib.sha256(payload).hexdigest(), rows=parser.rows)
    path.write_text(json.dumps(snapshot, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return snapshot


def satellites(physical, orbital, poles):
    physical_by_code = {}
    for row in physical["rows"]:
        assert len(row) == 12, row
        physical_by_code[row[2]] = row
    result = []
    for row in orbital["rows"]:
        assert len(row) == 20, row
        ident, parent, name, code, ephemeris, frame, epoch = row[:7]
        a, e, argument, anomaly, inc, node, period, apsis_period, node_period, ra, dec, tilt = map(number, row[7:19])
        assert a and a > 0 and e is not None and 0 <= e < 1
        orbit = dict(sourceUrl=orbital["sourceUrl"], ephemeris=ephemeris, frame=frame,
                     epochTdb=epoch, epochJdTdb=epoch_jd(epoch), semiMajorAxisMetres=a * 1000,
                     eccentricity=e, argumentPeriapsisDegrees=argument, meanAnomalyDegrees=anomaly,
                     inclinationDegrees=inc, ascendingNodeDegrees=node,
                     periodSeconds=period * 86400 if period else None,
                     apsisPrecessionYears=apsis_period, nodePrecessionYears=node_period,
                     poleRightAscensionDegrees=ra, poleDeclinationDegrees=dec,
                     laplaceTiltDegrees=tilt, reference=row[19], quality="published-mean-elements")
        if frame.lower() == "equatorial" and ra is None and dec is None:
            model = poles["planets"].get(parent)
            if model:
                centuries = (orbit["epochJdTdb"] - 2451545.) / 36525
                orbit["poleRightAscensionDegrees"] = sum(value * centuries ** i for i, value in enumerate(model["rightAscensionCoefficientsDegrees"]))
                orbit["poleDeclinationDegrees"] = sum(value * centuries ** i for i, value in enumerate(model["declinationCoefficientsDegrees"]))
                orbit["poleSourceUrl"] = poles["sourceUrl"]
        phys = physical_by_code.get(code)
        radius = number(phys[6]) if phys else None
        sigma = number(phys[7]) if phys else None
        shape = poles.get("bodyEllipsoids", {}).get(code)
        radius_status = "published-volume-equivalent-radius" if radius is not None else "unknown"
        if radius is None and shape:
            radius = math.prod(shape["semiaxesMetres"]) ** (1 / 3) / 1000
            radius_status = "published-NAIF-ellipsoid-derived-radius"
        relative = kepler(orbit)
        body = dict(id="sol." + LEGACY.get(code, "moon.jpl_" + code), name=name,
                    kind="moon", parentId="sol." + parent.lower(), jplSatelliteCode=code,
                    radiusMetres=radius * 1000 if radius is not None else None,
                    radiusSigmaMetres=sigma * 1000 if sigma is not None else None,
                    radiusStatus=radius_status, shapeProvenance=shape,
                    positionMetres=None, relativePositionMetres=relative,
                    positionFrame="right-handed-ecliptic-J2000-parent-centred",
                    positionEpochJdTdb=orbit["epochJdTdb"],
                    positionStatus="mean-element-illustrative-epoch" if relative else "unknown",
                    texture=None, surfaceQuality="unmapped-in-project", rotationHours=None,
                    orbit=orbit, physicalReference=phys,
                    uncertainty="Mean elements are approximate; no ephemeris accuracy bound is claimed.")
        result.append(body)
    # Puck currently has two published solutions, both retained; choose newest
    # as one definition, rather than silently creating a second physical moon.
    grouped = {}
    for body in result:
        grouped.setdefault(body["id"], []).append(body)
    selected = []
    for alternatives in grouped.values():
        alternatives.sort(key=lambda b: b["positionEpochJdTdb"], reverse=True)
        chosen = alternatives[0]
        if len(alternatives) > 1:
            chosen["alternativeOrbitSolutions"] = [b["orbit"] for b in alternatives[1:]]
        selected.append(chosen)
    result = selected
    assert len({b["id"] for b in result}) == len(result)
    orbit_codes = {b["jplSatelliteCode"] for b in result}
    missing = set(physical_by_code) - orbit_codes
    assert not missing, "Physical-only rows need explicit handling: " + str(missing)
    return result


def planet_poles(offline):
    path = OUT / "planet-poles.json"
    if offline:
        return json.loads(path.read_text(encoding="utf-8"))
    url = "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/pck/pck00011.tpc"
    payload = fetch(url)
    text = payload.decode("utf-8")
    # Only active kernel data, never values quoted in historical comments.
    data = "\n".join(re.findall(r"\\begindata(.*?)(?=\\begintext|\Z)", text, re.S))
    planets = {}
    for name, code in (("Earth", 399), ("Mars", 499), ("Jupiter", 599), ("Saturn", 699),
                       ("Uranus", 799), ("Neptune", 899), ("Pluto", 999)):
        coefficients = {}
        for label, field in (("RA", "rightAscensionCoefficientsDegrees"), ("DEC", "declinationCoefficientsDegrees")):
            match = re.search(r"BODY" + str(code) + "_POLE_" + label + r"\s*=\s*\(([^)]*)\)", data)
            assert match, name
            coefficients[field] = [float(x.replace("D", "E")) for x in match.group(1).split()]
        planets[name] = coefficients
    shapes = {}
    for code, raw in re.findall(r"BODY(\d+)_RADII\s*=\s*\(([^)]*)\)", data):
        axes = [float(x.replace("D", "E")) * 1000 for x in raw.split()]
        if len(axes) == 3 and all(x > 0 for x in axes):
            shapes[code] = dict(semiaxesMetres=axes, sourceUrl=url,
                               uncertainty="Published reference ellipsoid; no uncertainty supplied in kernel. May be historical approximate shape.")
    snapshot = dict(sourceUrl=url, retrievedUtc=datetime.now(timezone.utc).isoformat(),
                    sourcePayloadSha256=hashlib.sha256(payload).hexdigest(), planets=planets,
                    bodyEllipsoids=shapes,
                    note="Linear/polynomial pole terms only; periodic nutation terms excluded for illustrative source-frame conversion.")
    path.write_text(json.dumps(snapshot, indent=2) + "\n", encoding="utf-8")
    return snapshot


def minor_snapshots(offline):
    path = OUT / "small-body-api.json"
    cached = json.loads(path.read_text(encoding="utf-8")) if path.exists() else {"records": []}
    by_id = {r["query"]: r for r in cached["records"]}
    if not offline:
        for query in CURATED:
            if query in by_id and by_id[query].get("satelliteDataRequested"):
                continue
            url = "https://ssd-api.jpl.nasa.gov/sbdb.api?" + urlencode({"sstr": query, "phys-par": "true", "full-prec": "true", "sat": "true"})
            payload = json.loads(fetch(url))
            assert "object" in payload and "orbit" in payload, payload
            by_id[query] = dict(query=query, sourceUrl=url, satelliteDataRequested=True,
                               retrievedUtc=datetime.now(timezone.utc).isoformat(), response=payload)
            cached["records"] = list(by_id.values())
            path.write_text(json.dumps(cached, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
            print("Fetched SBDB", query, flush=True)
            time.sleep(.35)  # Requests remain strictly sequential per API policy.
    assert all(query in by_id for query in CURATED)
    return [by_id[query] for query in CURATED]


def minors(records, planets, poles):
    result = []
    for record in records:
        query, raw = record["query"], record["response"]
        physical = {p["name"]: p for p in raw.get("phys_par", [])}
        elements = {p["name"]: p for p in raw["orbit"]["elements"]}
        diameter = number(physical.get("diameter", {}).get("value"))
        sigma = number(physical.get("diameter", {}).get("sigma"))
        orbit = dict(frame="ecliptic", epochJdTdb=number(raw["orbit"]["epoch"]),
                     semiMajorAxisMetres=number(elements.get("a", {}).get("value")),
                     eccentricity=number(elements["e"]["value"]),
                     argumentPeriapsisDegrees=number(elements["w"]["value"]),
                     meanAnomalyDegrees=number(elements.get("ma", {}).get("value")),
                     inclinationDegrees=number(elements["i"]["value"]),
                     ascendingNodeDegrees=number(elements["om"]["value"]),
                     elementDetails=raw["orbit"]["elements"],
                     orbitId=raw["orbit"].get("orbit_id"), covarianceNotRequested=True)
        if orbit["semiMajorAxisMetres"] is not None:
            orbit["semiMajorAxisMetres"] *= AU
        vector = kepler(orbit)
        rotation = number(physical.get("rot_per", {}).get("value"))
        dwarf_name = MINOR_LEGACY.get(query, "").title()
        dwarf = next((r for r in planets["rows"] if r and r[0] == dwarf_name), None) if query in DWARFS else None
        if dwarf and diameter is None:
            match = re.match(r"([\d.]+)", dwarf[2])
            assert match, dwarf
            diameter = float(match.group(1)) * 2
            match = re.search(r"±\s*([\d.]+)", dwarf[2])
            sigma = float(match.group(1)) * 2 if match else None
            physical["diameter"] = dict(value=str(diameter), sigma=str(sigma) if sigma else None,
                sourceUrl=planets["sourceUrl"], reference=dwarf[2],
                derivation="twice published mean radius; retain source model/date limitations")
            if rotation is None:
                match = re.match(r"(-?[\d.]+)", dwarf[5])
                rotation = float(match.group(1)) * 24 if match else None
        naif_shape_code = "999" if query == "134340" else str(2000000 + int(query)) if query.isdigit() else str(raw["object"]["spkid"])
        shape = poles.get("bodyEllipsoids", {}).get(naif_shape_code)
        row = dict(id="sol." + MINOR_LEGACY.get(query, "smallbody.sp_k" + str(raw["object"]["spkid"])),
                   name=raw["object"]["fullname"], parentId="sol.sun",
                   kind="comet" if raw["object"]["kind"] == "cn" or query.endswith("P") else
                        "dwarf-planet" if query in DWARFS else "minor-body",
                   jplSmallBodyId=str(raw["object"]["spkid"]), designation=raw["object"]["des"],
                   radiusMetres=diameter * 500 if diameter is not None else None,
                   radiusSigmaMetres=sigma * 500 if sigma is not None else None,
                   radiusStatus="published-diameter-derived-spherical-radius" if diameter else "unknown",
                   radiusProvenance=physical.get("diameter"), physicalParameters=raw.get("phys_par", []),
                   shapeProvenance=shape,
                   positionMetres=vector, positionFrame="right-handed-ecliptic-J2000-heliocentric",
                   positionEpochJdTdb=orbit["epochJdTdb"],
                   positionStatus="osculating-two-body-source-epoch" if vector else "unknown",
                   rotationHours=rotation, texture=None, surfaceQuality="unmapped-in-project",
                   sourceUrl=record["sourceUrl"], orbit=orbit,
                   uncertainty="Diameter may be modeled/estimated; preserve reference and sigma. No spherical shape or current-date ephemeris accuracy implied.")
        result.append(row)
    assert len({r["id"] for r in result}) == len(result)
    return result


def smallbody_satellites(records, parent_rows):
    parents = {r["jplSmallBodyId"]: r["id"] for r in parent_rows}
    result = []
    for record in records:
        parent_raw = record["response"]["object"]
        parent_id = parents[str(parent_raw["spkid"])]
        # Pluto's five moons already have stronger planetary table definitions.
        # Preserve the API records in the public snapshot, never duplicate bodies.
        if parent_id == "sol.pluto":
            continue
        for sat in record["response"].get("satellites", record["response"].get("sat", [])):
            name = sat.get("iau_name") or sat["fullname"]
            stable = sat.get("prov_des") or sat["fullname"]
            stable = re.sub(r"[^a-z0-9]+", "_", stable.lower()).strip("_")
            source_orbit = sat.get("orbit", {}).get(str(sat.get("oid")), {})
            elements = {p["name"]: p for p in source_orbit.get("elements", [])}
            def element(key):
                return number(elements.get(key, {}).get("value"))
            axis = element("a")
            if axis is not None:
                unit = elements["a"].get("units")
                assert unit in ("m", "km"), unit
                axis *= 1000 if unit == "km" else 1
            orbit = dict(frame="ecliptic" if source_orbit.get("frame") == "EC" else "equatorial",
                         epochJdTdb=number(source_orbit.get("epoch")),
                         semiMajorAxisMetres=axis, eccentricity=element("e"),
                         meanAnomalyDegrees=element("ma"), inclinationDegrees=element("i"),
                         argumentPeriapsisDegrees=element("w"), ascendingNodeDegrees=element("om"),
                         elementDetails=source_orbit.get("elements", []), sourceRecord=source_orbit)
            relative = kepler(orbit) if source_orbit else None
            result.append(dict(id=parent_id + ".moon." + stable, parentId=parent_id, name=name,
                               kind="moon", radiusMetres=None, radiusStatus="unknown-in-selected-API",
                               relativePositionMetres=relative, positionMetres=None,
                               positionEpochJdTdb=orbit["epochJdTdb"],
                               positionFrame="right-handed-ecliptic-J2000-parent-centred",
                               positionStatus="osculating-epoch-illustration" if relative else "unknown-or-insufficient-elements",
                               orbit=orbit, sourceRecord=sat, sourceUrl=record["sourceUrl"],
                               texture=None, surfaceQuality="unmapped-in-project", rotationHours=None))
    assert len({r["id"] for r in result}) == len(result)
    return result


def supplemental_sizes(bodies):
    """Explicitly cited scientific measurements, preserving replaced alternatives."""
    source = json.loads((OUT / "supplemental-measurements.json").read_text(encoding="utf-8"))
    for measurement in source["measurements"]:
        body = next((b for b in bodies if b["id"] == measurement["bodyId"]), None)
        assert body is not None, measurement["bodyId"]
        if body["radiusMetres"] is not None:
            body.setdefault("radiusAlternatives", []).append(dict(radiusMetres=body["radiusMetres"],
                radiusSigmaMetres=body.get("radiusSigmaMetres"), provenance=body.get("radiusProvenance")))
        body["radiusMetres"] = measurement["radiusMetres"]
        body["radiusSigmaMetres"] = measurement["radiusSigmaMetres"]
        body["radiusStatus"] = "published-supplemental-measurement"
        body["radiusProvenance"] = measurement


def save_json(name, value):
    (OUT / name).write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def environment_catalog():
    """Cited numeric summaries + explicitly authored sparse sampling envelopes."""
    science = "https://science.nasa.gov/"
    fact = "https://nssdc.gsfc.nasa.gov/planetary/factsheet/"
    rings = []
    def broad(parent, name, inner, outer, depth, source, thickness=None):
        rings.append(dict(id=parent + ".ring." + name.lower().replace(" ", "_"), parentId=parent,
            name=name, innerRadiusMetres=inner * 1000, outerRadiusMetres=outer * 1000,
            opticalDepthRange=depth, thicknessMetres=thickness, plane="parent-equatorial",
            sourceUrl=source, numericStatus="approximate-published-summary", texture=None))
    for name, inner, outer, depth, thickness in (
        ("Halo", 89400, 123000, [3e-6, 3e-6], 10000000),
        ("Main", 123000, 128940, [5e-6, 5e-6], None),
        ("Amalthea", 128940, 181350, [1e-7, 1e-7], 2600000),
        ("Thebe", 181350, 221900, [1e-7, 1e-7], 8800000),
        ("Thebe Extension", 221900, 280000, [1e-7, 1e-7], 8800000)):
        broad("sol.jupiter", name, inner, outer, depth, fact + "jupringfact.html", thickness)
    rings[1]["thicknessUpperLimitMetres"] = 100000
    for name, inner, outer, depth, thickness in (
        ("D", 66900, 74510, [1e-5, 1e-5], None),
        ("C", 74658, 91975, [.05, .35], 5),
        ("B", 91975, 117507, [.4, 2.5], None),
        ("Cassini Division", 117507, 122340, [0, .1], None),
        ("A", 122340, 136780, [.4, 1], None),
        ("G", 166000, 173000, [1e-6, 1e-6], 100000),
        ("E", 180000, 480000, [1e-6, 1e-6], 10000000)):
        broad("sol.saturn", name, inner, outer, depth, fact + "satringfact.html", thickness)
    rings.append(dict(id="sol.saturn.ring.f", parentId="sol.saturn", name="F",
        characteristicRadiusMetres=139826000, radiusDefinition="inner edge for narrow ring in cited fact sheet",
        widthMetres=None, widthUpperLimitMetres=1000000, eccentricity=.0026,
        opticalDepthRange=[.1, .1], plane="parent-equatorial", sourceUrl=fact + "satringfact.html",
        numericStatus="approximate-published-summary", texture=None))
    for name, radius, width_min, width_max, eccentricity in (
        ("6", 41837, 1.5, 1.5, .0010), ("5", 42234, 2, 2, .0019),
        ("4", 42571, 2, 2, .0011), ("Alpha", 44718, 4, 10, .0008),
        ("Beta", 45661, 5, 11, .0004), ("Eta", 47176, 1.6, 1.6, None),
        ("Gamma", 47627, 1, 4, .0011), ("Delta", 48300, 3, 7, .0004),
        ("Lambda", 50024, 2, 2, 0), ("Epsilon", 51149, 20, 96, .0079),
        ("Nu", 67300, 3800, 3800, 0), ("Mu", 97700, 17000, 17000, 0)):
        rings.append(dict(id="sol.uranus.ring." + name.lower(), parentId="sol.uranus", name=name,
            characteristicRadiusMetres=radius * 1000, radiusDefinition="summary-table reference radius",
            widthRangeMetres=[width_min * 1000, width_max * 1000], eccentricity=eccentricity,
            plane="parent-equatorial", sourceUrl=fact + "uranringfact.html",
            numericStatus="approximate-published-summary", texture=None))
    rings.append(dict(id="sol.uranus.ring.zeta", parentId="sol.uranus", name="Zeta",
        characteristicRadiusMetres=None, widthRangeMetres=None, plane="parent-equatorial",
        sourceUrl=science + "uranus/facts/", numericStatus="known-ring-numeric-geometry-unavailable-in-selected-source", texture=None))
    for name, radius, width in (("Galle", 41900, 15), ("Leverrier", 53200, 15),
                              ("Lassell", 55400, None), ("Arago", 57600, None), ("Adams", 62930, None)):
        rings.append(dict(id="sol.neptune.ring." + name.lower(), parentId="sol.neptune", name=name,
            characteristicRadiusMetres=radius * 1000, radiusDefinition="NASA summary-table reference radius",
            widthMetres=width * 1000 if width else None,
            widthUpperLimitMetres=50000 if name == "Adams" else None,
            plane="parent-equatorial", sourceUrl=science + "neptune/neptune-facts/",
            numericStatus="approximate-published-summary", texture=None))
    save_json("rings.json", dict(version=1, reviewedDate="2026-10-04", rings=rings,
        notes=["All radii measured from planet centre, not surface altitude. Plane orientation requires parent pole.",
               "Summary factsheets are historical models; not complete ringlets, azimuthal arcs, temporal clumps or phase functions.",
               "Adams arcs Liberte/Egalite/Fraternite/Courage are known; no measured epoch longitude/extent selected here.",
               "Saturn Cassini division is low optical depth, not an entirely empty vacuum. F-ring exact edges are not invented.",
               "Jupiter/Uranus/Neptune dust/dark rings must not inherit Saturn's bright ice shader."]))
    def region(ident, name, geometry, source, bounds=None, envelope=None, note=""):
        return dict(id="sol.environment." + ident, name=name, parentId="sol.sun", geometry=geometry,
            sourceUrls=source if isinstance(source, list) else [source], approximatePhysicalBoundsAu=bounds,
            authoredSamplingEnvelopeAu=envelope, physicalNumberDensity=None, catalogBodyCount=None,
            sampledObjectsAreAuthoritative=False, seed=int(hashlib.sha256(ident.encode()).hexdigest()[:8], 16),
            visualPolicy="sparse-local-samples-and-map-overlay; no opaque global volume", uncertainty=note)
    regions = [
        region("main_belt", "Main asteroid belt", "broad-orbital-disk",
            science + "solar-system/asteroids/", envelope=[2.1, 3.3],
            note="NASA source locates main belt between Mars and Jupiter. 2.1-3.3 AU is an authored sampling envelope, not measured sharp boundaries or density."),
        region("jupiter_trojans", "Jupiter Trojan populations", "two-resonant-orbital-lobes",
            [science + "mission/lucy/science/", "https://www.nasa.gov/solar-system/how-the-sun-affects-asteroids-in-our-neighborhood/"],
            note="L4/L5 approximately 60 degrees ahead/behind Jupiter. No measured lobe width or density selected; use current Jupiter orbital phase."),
        region("kuiper_belt", "Kuiper belt core", "thick-orbital-disk",
            science + "solar-system/kuiper-belt/facts/", bounds=[30, 50],
            note="Approximate core, not empty outside its boundaries. Cold and dynamically hot populations have different inclinations/eccentricities."),
        region("scattered_disk", "Scattered trans-Neptunian population", "eccentric-inclined-orbits",
            science + "solar-system/kuiper-belt/facts/", bounds=[None, 1000],
            note="Rough maximum reach quoted by NASA; not a uniformly filled shell or hard edge."),
        region("inner_oort", "Inner Oort population", "inferred-outer-population",
            science + "solar-system/oort-cloud/facts/", bounds=[2000, 5000],
            note="2000-5000 AU expresses uncertainty in where inner boundary may begin, not the thickness of a measured narrow shell. No direct cloud image."),
        region("outer_oort", "Outer Oort population", "inferred-spherical-population",
            science + "solar-system/oort-cloud/facts/", bounds=[10000, 100000],
            note="10000-100000 AU expresses uncertainty in outer extent, not a measured hard shell. Extremely distant sparse icy bodies; not visible fog."),
        region("zodiacal_dust", "Interplanetary zodiacal dust", "diffuse-sunlight-scattering-disk",
            ["https://www.nasa.gov/missions/serendipitous-juno-spacecraft-detections-shatter-ideas-about-origin-of-zodiacal-light/",
             "https://pds.nasa.gov/data/uly-j-gas-5-sky-maps-v1.0/uly_5001/document/dust/dustinst.htm"],
            note="Faint diffuse illumination from tiny grains. Selected source gives no absolute density or sharp radial edges. Do not render universal dense glowing particles.")]
    regions[1]["resonantLongitudeOffsetsDegrees"] = [-60, 60]
    regions[4]["approximatePhysicalBoundsAu"] = None
    regions[4]["innerBoundaryUncertaintyRangeAu"] = [2000, 5000]
    regions[5]["approximatePhysicalBoundsAu"] = None
    regions[5]["outerBoundaryUncertaintyRangeAu"] = [10000, 100000]
    regions[6]["lightProbedGrainSizeMetres"] = [10e-6, .001]
    save_json("distributions.json", dict(version=1, astronomicalUnitMetres=AU, reviewedDate="2026-10-04",
        regions=regions, cometAppearance=dict(sourceUrl=science + "solar-system/comets/",
            nucleus="solid ice-rock-dust body", coma="activity-dependent sunlight-scattering gas/dust around nucleus",
            tail="points generally away from Sun; do not orient by velocity alone", activityThresholdAu=None),
        pilotMotionAid=dict(kind="optional-authored-local-particles", measuredAstronomicalDust=False,
                           mutatesCanonicalBodies=False, collide=False),
        note="Sampling counts/radii/LOD/brightness are engine presentation choices, never measured population density or undiscovered persistent celestial bodies."))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--offline", action="store_true")
    options = parser.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    physical = table_snapshot("satellite-physical", "https://ssd.jpl.nasa.gov/sats/phys_par/", "sat_phys_par", options.offline)
    orbital = table_snapshot("satellite-orbital", "https://ssd.jpl.nasa.gov/sats/elem/", "sat_elem", options.offline)
    planets = table_snapshot("planet-physical", "https://ssd.jpl.nasa.gov/planets/phys_par.html", "planet-phys-par", options.offline)
    poles = planet_poles(options.offline)
    moon_rows = satellites(physical, orbital, poles)
    print("JPL satellites:", len(moon_rows), "orbital rows;", len(physical["rows"]), "physical rows", flush=True)
    (OUT / "satellites.json").write_text(json.dumps(dict(version=1, sources=[physical["sourceUrl"], orbital["sourceUrl"]],
        retrievedUtc=orbital["retrievedUtc"], counts=dict(total=len(moon_rows), physicalRadiusKnown=sum(b["radiusMetres"] is not None for b in moon_rows),
        byParent=dict(Counter(b["parentId"] for b in moon_rows))), bodies=moon_rows), ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    snapshots = minor_snapshots(options.offline)
    minor_rows = minors(snapshots, planets, poles)
    small_moons = smallbody_satellites(snapshots, minor_rows)
    supplemental_sizes(minor_rows + small_moons)
    save_json("minor-bodies.json", dict(version=1, bodies=minor_rows))
    save_json("minor-body-satellites.json", dict(version=1,
        note="Five duplicate Pluto moons excluded: use satellites.json. No unknown sizes or phases invented.", bodies=small_moons))
    environment_catalog()
    print("SOLAR MINOR CATALOG PASS:", len(moon_rows), "planetary satellites;", len(minor_rows), "curated minor bodies;", len(small_moons), "small-body satellites")


if __name__ == "__main__":
    main()
