"""Planet-quality source maps from public raw data (Blender background, OIIO + numpy).

blender -b --factory-startup --python Tools/Prepare-PlanetSources.py -- <repo_root> [step ...]

Steps: earth-day earth-relief earth-clouds earth-night spheres defaults (default: Earth maps).
Raw inputs are fetched by Tools/Fetch-PlanetSources.ps1 into ignored .local/planet-raw
and checked against the SHA-256 list there. Outputs are the tracked, editable sources in
Art/Space/PlanetQuality/<Body>/ that the Unreal recipe imports. Nothing here invents
geography: every Earth map is a resampled observation (NASA BMNG/Black Marble, NOAA ETOPO).
"""
import json
import sys
import time
from pathlib import Path

import numpy as np
import OpenImageIO as oiio

ROOT = Path(sys.argv[sys.argv.index('--') + 1]).resolve()
STEPS = sys.argv[sys.argv.index('--') + 2:] or ['earth-day', 'earth-relief', 'earth-clouds', 'earth-night']
RAW = ROOT / '.local/planet-raw'
WORK = ROOT / '.local/planet-work'
EARTH = ROOT / 'Art/Space/PlanetQuality/Earth'
W16, H16 = 16384, 8192
WORK.mkdir(parents=True, exist_ok=True)
EARTH.mkdir(parents=True, exist_ok=True)

# Land relief encoding shared with the Unreal material (Prepare-PlanetMaterials.py):
# 0 = water; land = LAND_MIN + (255-LAND_MIN)*sqrt(h/H_MAX) for heights 0..H_MAX metres.
LAND_MIN, H_MAX = 16, 8848.0


def log(*args):
    print('PLANET_SOURCES', *args, flush=True)


def read(path):
    buf = oiio.ImageBuf(str(path))
    assert buf.read(force=True), (path, buf.geterror())
    return buf


def resize(buf, width, height, filt='lanczos3'):
    spec = buf.spec()
    out = oiio.ImageBufAlgo.resize(buf, filtername=filt, roi=oiio.ROI(0, width, 0, height, 0, 1, 0, spec.nchannels))
    assert not out.has_error, out.geterror()
    return out


def write(array, path, quality=None):
    """uint8 HxW or HxWxC -> file. JPEG uses 4:4:4 at the given quality; PNG max compression."""
    if array.ndim == 2:
        array = array[..., None]
    array = np.ascontiguousarray(array, np.uint8)
    channels = array.shape[2]
    spec = oiio.ImageSpec(array.shape[1], array.shape[0], channels, oiio.UINT8)
    if quality:
        spec.attribute('Compression', f'jpeg:{quality}')
        spec.attribute('jpeg:subsampling', '4:4:4')
    else:
        spec.attribute('png:compressionLevel', 9)
    out = oiio.ImageOutput.create(str(path))
    assert out and out.open(str(path), spec), oiio.geterror()
    assert out.write_image(array), out.geterror()
    out.close()
    log('wrote', path.relative_to(ROOT).as_posix(), array.shape, f'{path.stat().st_size / 1e6:.1f} MB')


def cached(name, build):
    path = WORK / (name + '.npy')
    if path.exists():
        return np.load(path, mmap_mode='r')
    array = build()
    np.save(path, array)
    return array


def bmng_day():
    """NASA Blue Marble Next Generation (Oct 2004, no baked topography/bathymetry shading),
    eight 21600^2 tiles = 86400x43200 at 15 arcsec, resampled to 16384x8192 (Lanczos-3)."""
    out = np.zeros((H16, W16, 3), np.uint8)
    for column, letter in enumerate('ABCD'):
        for row in (1, 2):
            t = time.time()
            tile = read(RAW / f'world.200410.3x21600x21600.{letter}{row}.png')
            small = resize(tile, W16 // 4, H16 // 2).get_pixels(oiio.UINT8)
            out[(row - 1) * (H16 // 2):row * (H16 // 2), column * (W16 // 4):(column + 1) * (W16 // 4)] = small[..., :3]
            log('day tile', letter + str(row), round(time.time() - t, 1), 's')
    return out


def etopo_heights():
    """NOAA ETOPO 2022 60 arcsec surface elevation (metres), resampled to 16384x8192."""
    source = read(RAW / 'ETOPO_2022_v1_60s_N90W180_surface.tif')
    spec = source.spec()
    log('etopo', spec.width, spec.height, spec.nchannels, spec.format)
    # Pixel-registered 21600x10800 or grid-registered 21601x10801 (drop duplicated seam/pole rows).
    if spec.width == 21601:
        source = oiio.ImageBufAlgo.cut(source, oiio.ROI(0, 21600, 0, 10800 if spec.height == 10801 else spec.height, 0, 1, 0, 1))
    return resize(source, W16, H16, 'catmull-rom').get_pixels(oiio.FLOAT)[..., 0].astype(np.float32)


def water_mask(day, height):
    """Water = BMNG open-water colour (dark, blue-dominant; pixel aligned with the albedo) or
    ETOPO sea floor. Colour finds inland lakes; ETOPO keeps dark coastal sea consistent."""
    rgb = day.astype(np.int16)
    r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
    blue_water = (b > r + 12) & (b >= g) & (r + g + b < 200)
    sea_floor = height < -10
    mask = blue_water | sea_floor
    # Remove isolated single-texel speckle (dark shadows/forest) without touching coasts.
    count = np.zeros(mask.shape, np.int8)
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            count += np.roll(np.roll(mask, dy, 0), dx, 1)
    return (mask & (count >= 4)) | (~mask & (count >= 8))


def step_day():
    day = cached('earth_day_16k', bmng_day)
    write(day, EARTH / 'earth_day_16k.jpg', quality=95)


def step_relief():
    day = cached('earth_day_16k', bmng_day)
    height = cached('earth_height_16k', etopo_heights)
    water = water_mask(day, height)
    land = np.clip(np.asarray(height), 0, H_MAX)
    code = np.where(water, 0, LAND_MIN + np.round((255 - LAND_MIN) * np.sqrt(land / H_MAX))).astype(np.uint8)
    stats = dict(water_fraction=float(water.mean()), height_min=float(np.min(height)), height_max=float(np.max(height)),
                 encoding=dict(water=0, land_min=LAND_MIN, height_max_metres=H_MAX, curve='sqrt'))
    (WORK / 'earth_relief_stats.json').write_text(json.dumps(stats, indent=2))
    log('relief', stats)
    write(code, EARTH / 'earth_relief_16k.png')


GIBS_DATE = '2023-07-29'            # northern summer daylight snapshot (SNPP + NOAA-20 glint fill)
GIBS_SOUTH_DATE = '2023-01-29'      # southern summer snapshot for the Antarctic polar night
GIBS_LAYERS = {'snpp': 'VIIRS_SNPP_CorrectedReflectance_TrueColor', 'noaa20': 'VIIRS_NOAA20_CorrectedReflectance_TrueColor'}


_S = np.arange(256, dtype=np.float32) / 255
LINEAR = np.where(_S <= .04045, _S / 12.92, ((_S + .055) / 1.055) ** 2.4).astype(np.float32)


def luminance(srgb8):
    """Linear-light luminance via a 256-entry table (keeps 16K temporaries at one float plane)."""
    y = LINEAR[srgb8[..., 0]] * np.float32(.2126)
    y += LINEAR[srgb8[..., 1]] * np.float32(.7152)
    y += LINEAR[srgb8[..., 2]] * np.float32(.0722)
    return y


def box_blur(a, radius, passes=3):
    """Separable wrap-around (longitude) / clamped (latitude) box blur; 3 passes ~ Gaussian."""
    for _ in range(passes):
        for axis in (0, 1):
            pad = np.pad(a, [(radius, radius) if i == axis else (0, 0) for i in range(2)], mode='wrap' if axis == 1 else 'edge')
            c = np.cumsum(pad, axis, dtype=np.float64)
            c = np.concatenate([np.zeros_like(c.take([0], axis)), c], axis)
            n = a.shape[axis]
            a = ((c.take(np.arange(2 * radius + 1, n + 2 * radius + 1), axis) - c.take(np.arange(0, n), axis)) / (2 * radius + 1)).astype(np.float32)
    return a


def gibs_mosaic(name, date=GIBS_DATE, rows=range(20)):
    """NASA GIBS WMTS EPSG:4326 level 5 (512 px tiles, 40x20) = 20480x10240 daily true colour,
    resampled to 16384x8192; rows not fetched stay black (no data). Fetched by Fetch-PlanetSources.ps1."""
    folder = RAW / f'gibs-viirs-{name}-{date}'
    big = np.zeros((10240, 20480, 3), np.uint8)
    for r in rows:
        for c in range(40):
            big[r * 512:(r + 1) * 512, c * 512:(c + 1) * 512] = read(folder / f'5_{r}_{c}.jpg').get_pixels(oiio.UINT8)[..., :3]
    buf = oiio.ImageBuf(oiio.ImageSpec(20480, 10240, 3, oiio.UINT8))
    buf.set_pixels(oiio.ROI(), big)
    del big
    return resize(buf, W16, H16).get_pixels(oiio.UINT8)


def opacity(observed, surface):
    """Invert I = (1-a)*S + a*C for cloud opacity a (linear luminance)."""
    a = (observed - surface) / np.maximum(.88 - surface, .12)
    a *= 1 - np.clip((surface - .45) / .25, 0, 1)          # ice sheets/snow: indistinguishable, keep clear
    return np.clip((a - .04) / .96, 0, 1)                   # Rayleigh/haze floor over dark ocean


def small(a, f=8):
    return a.reshape(a.shape[0] // f, f, a.shape[1] // f, f).mean((1, 3), dtype=np.float32)


def large(a, f=8):
    """Bilinear upsample by f (longitude wraps)."""
    h, w = a.shape
    y = np.clip((np.arange(h * f) + .5) / f - .5, 0, h - 1)
    x = (np.arange(w * f) + .5) / f - .5
    y0 = np.floor(y).astype(int); y1 = np.minimum(y0 + 1, h - 1); fy = (y - y0)[:, None].astype(np.float32)
    x0 = np.floor(x).astype(int); fx = (x - x0)[None, :].astype(np.float32); x0 %= w; x1 = (x0 + 1) % w
    top = a[y0][:, x0] * (1 - fx) + a[y0][:, x1] * fx
    bottom = a[y1][:, x0] * (1 - fx) + a[y1][:, x1] * fx
    return top * (1 - fy) + bottom * fy


# VIIRS (Suomi NPP / NOAA-20): sun-synchronous, 98.74 deg, 101.44 min, daytime passes ascending.
# GIBS daily mosaics keep the pixel nearest nadir, so neighbouring passes (~100 min of cloud motion
# apart) meet midway between their ground tracks: seams run parallel to the track. NOAA-20 flies
# the same orbit half a revolution ahead, so its seams lie between SNPP's.
ORBIT_INCLINATION, ORBIT_MINUTES, SWATH_HALF_KM, EARTH_KM = 98.74, 101.44, 1520.0, 6371.0
ORBIT_STEP_DEG = 360 * ORBIT_MINUTES / 1440


def track_shape(side=0):
    """Latitude -> longitude offset (deg, relative to its equator crossing) of the ground track
    (side 0) or of a line SWATH_HALF_KM to either side of it (side -1/+1)."""
    i = np.radians(ORBIT_INCLINATION)
    u = np.radians(np.linspace(-89.5, 89.5, 7161))
    turn = 2 * np.pi * (u / (2 * np.pi) * ORBIT_MINUTES) / 1440      # Earth turning under the orbit plane
    x, y, z = np.cos(u), np.sin(u) * np.cos(i), np.sin(u) * np.sin(i)
    r = np.stack([x * np.cos(turn) + y * np.sin(turn), -x * np.sin(turn) + y * np.cos(turn), z], 1)
    c = np.cross(r, np.gradient(r, axis=0))
    c /= np.linalg.norm(c, axis=1, keepdims=True)
    h = SWATH_HALF_KM / EARTH_KM
    e = np.cos(h * abs(side)) * r + np.sin(h * abs(side)) * np.sign(side) * c
    lat = np.degrees(np.arcsin(np.clip(e[:, 2], -1, 1)))
    lon = np.degrees(np.unwrap(np.arctan2(e[:, 1], e[:, 0])))
    keep = np.concatenate([[True], np.diff(lat) > 0])
    keep &= np.maximum.accumulate(np.where(keep, lat, -90)) == lat
    lat, lon = lat[keep], lon[keep]
    return lat, lon - np.interp(0, lat, lon)


def seam_offsets(y_s, y_n, valid, side=0, lat_range=(15, 72)):
    """Equator-crossing longitudes of SNPP mosaic seams: edges present in SNPP but not in NOAA-20,
    accumulated along the predicted track-parallel curve and folded by the orbit step."""
    f = 4
    a, b = small(y_s, f), small(y_n, f)
    ok = small(valid.astype(np.float32), f) > .99
    def grad(img):
        gx = np.roll(img, -1, 1) - np.roll(img, 1, 1)
        gy = np.zeros_like(img); gy[1:-1] = img[2:] - img[:-2]
        return np.sqrt(gx * gx + gy * gy)
    evidence = np.clip(grad(a) - grad(b), -.05, .05) * ok
    h, w = evidence.shape
    lat_rows = 90 - (np.arange(h) + .5) * 180 / h
    edge_lat, edge_lon = track_shape(side)
    total = np.zeros(w)
    rows = np.where((np.abs(lat_rows) >= lat_range[0]) & (np.abs(lat_rows) <= lat_range[1]) &
                    (lat_rows >= edge_lat.min()) & (lat_rows <= edge_lat.max()))[0]
    for row in rows:
        shift = np.interp(lat_rows[row], edge_lat, edge_lon) / 360 * w
        total += np.roll(evidence[row], -int(round(shift)))
    step = ORBIT_STEP_DEG / 360 * w
    phases = np.arange(0, step, .25)
    folded = np.array([np.interp((p + np.arange(int(w // step) + 1) * step) % w, np.arange(w), total, period=w).sum() for p in phases])
    best = phases[folded.argmax()]
    score = float((folded.max() - folded.mean()) / max(folded.std(), 1e-9))
    offsets = [(best + k * step) / w * 360 - 180 for k in range(int(w // step) + 1) if best + k * step < w]
    return offsets, score, (edge_lat, edge_lon)


def stitch_seams(y_s, y_n, valid_s, valid_n, curves, half_band_km=260, feather=4.0):
    """Replace a band around each seam with NOAA-20, switching along least-difference paths
    (dynamic programming, as in panorama stitching) on either side of the seam."""
    h, w = y_s.shape
    use = np.zeros((h, w), np.float32)
    lat_rows = 90 - (np.arange(h) + .5) * 180 / h
    px_per_deg = w / 360
    for offset, (edge_lat, edge_lon) in curves:
        rows = np.where((lat_rows > edge_lat.min() + .5) & (lat_rows < edge_lat.max() - .5) & (np.abs(lat_rows) < 80))[0]
        centre = ((offset + np.interp(lat_rows[rows], edge_lat, edge_lon) + 180) * px_per_deg)
        half = np.maximum((half_band_km / (111.2 * np.maximum(np.cos(np.radians(lat_rows[rows])), .2))) * px_per_deg, 8).astype(int)
        span = int(half.max())
        j = np.arange(-span, span + 1)
        cols = (np.round(centre)[:, None].astype(int) + j[None, :]) % w
        cost = np.abs(y_s[rows[:, None], cols] - y_n[rows[:, None], cols])
        bad = ~(valid_s[rows[:, None], cols] & valid_n[rows[:, None], cols])
        cost = np.where(bad, 1.0, cost)
        outside = np.abs(j)[None, :] > half[:, None]
        paths = []
        for lo, hi in ((-span, -max(4, span // 6)), (max(4, span // 6), span)):
            allowed = (j >= lo) & (j <= hi)
            c = np.where(allowed[None, :] & ~outside, cost, 1e3)
            acc = c[0].copy(); back = np.zeros(c.shape, np.int8)
            for r in range(1, len(rows)):
                left = np.concatenate([[np.inf], acc[:-1]]); right = np.concatenate([acc[1:], [np.inf]])
                choice = np.argmin(np.stack([left, acc, right]), 0)
                acc = np.stack([left, acc, right])[choice, np.arange(len(acc))] + c[r]
                back[r] = choice - 1
            path = np.zeros(len(rows), int); path[-1] = int(np.argmin(acc))
            for r in range(len(rows) - 1, 0, -1):
                path[r - 1] = path[r] + back[r, path[r]]
            paths.append(path)
        k = np.arange(len(j), dtype=np.float32)[None, :]
        inside = np.clip((k - paths[0][:, None]) / feather + .5, 0, 1) * np.clip((paths[1][:, None] - k) / feather + .5, 0, 1)
        np.maximum.at(use, (rows[:, None].repeat(len(j), 1), cols), inside.astype(np.float32))
    return use * valid_n


def step_debug_seams():
    snpp = cached('viirs_snpp_16k', lambda: gibs_mosaic('snpp'))
    noaa = cached('viirs_noaa20_16k', lambda: gibs_mosaic('noaa20'))
    y_s, y_n = luminance(snpp), luminance(noaa)
    valid = (snpp.max(2) > 3) & (noaa.max(2) > 3)
    preview = np.ascontiguousarray(snpp[::8, ::8].copy())
    for side in (0,):
        offsets, score, (edge_lat, edge_lon) = seam_offsets(y_s, y_n, valid, side)
        log('seam side', side, 'score', round(score, 2), 'offsets', [round(o, 2) for o in offsets])
        for offset in offsets:
            for lat in np.arange(max(edge_lat.min(), -80), min(edge_lat.max(), 80), .1):
                lon = (offset + np.interp(lat, edge_lat, edge_lon) + 180) % 360 - 180
                y = int((90 - lat) / 180 * preview.shape[0]); x = int((lon + 180) / 360 * preview.shape[1]) % preview.shape[1]
                preview[min(y, preview.shape[0] - 1), x] = (255, 0, 0) if side > 0 else (0, 255, 0)
    write(preview, WORK / 'debug_seams.png')


def step_clouds():
    """Cloud opacity from a real daylight snapshot instead of the streaky MODIS 2001 composite.

    VIIRS true colour I is modelled as I = (1-a)*S + a*C over the cloud-free BMNG surface S with
    cloud reflectance C. Mosaic seams between SNPP passes and sun glint (smooth bright ocean bands
    along each SNPP swath) are replaced by
    the same-day NOAA-20 pass, whose swaths and glint lie between SNPP's. The Antarctic polar night
    comes from a southern-summer snapshot. Values are opacity, stored linear (not sRGB)."""
    day = cached('earth_day_16k', bmng_day)
    height = cached('earth_height_16k', etopo_heights)
    water = water_mask(day, height)
    snpp = cached('viirs_snpp_16k', lambda: gibs_mosaic('snpp'))
    noaa = cached('viirs_noaa20_16k', lambda: gibs_mosaic('noaa20'))
    y_s, y_n = luminance(snpp), luminance(noaa)
    valid_s = snpp.max(2) > 3
    valid_n = noaa.max(2) > 3
    # Mosaic seams: switch to NOAA-20 between least-difference paths around each detected seam.
    offsets, seam_score, shape = seam_offsets(y_s, y_n, valid_s & valid_n)
    use_seam = stitch_seams(y_s, y_n, valid_s, valid_n, [(o, shape) for o in offsets]) if seam_score > 2.5 else 0
    log('seams', len(offsets), 'score', round(seam_score, 2))
    y_s = y_s * (1 - use_seam) + y_n * use_seam
    del use_seam
    # Glint: SNPP systematically brighter than NOAA-20 over open water at ~100 km scale. Moving
    # clouds average out at that scale; glint does not. Work at 1/8 resolution.
    water_small = small(water.astype(np.float32))
    both = small((valid_s & valid_n).astype(np.float32))
    diff = small(np.where(valid_s & valid_n & water, y_s - y_n, 0).astype(np.float32))
    wsum = box_blur(np.where(both > .99, water_small, 0), 6)
    diff = box_blur(diff, 6) / np.maximum(wsum, 1e-3)
    glint = np.clip((diff - .012) / .025, 0, 1) * np.clip(wsum * 4, 0, 1)
    use_noaa = np.clip(large(glint), 0, 1) * water * valid_n
    use_noaa = np.where(valid_s, use_noaa, valid_n.astype(np.float32))
    observed = y_s * (1 - use_noaa) + y_n * use_noaa
    valid = valid_s | valid_n
    del snpp, noaa, y_s, y_n
    surface = luminance(np.asarray(day))
    a = opacity(observed, surface)
    # Residual sun glint: smooth, low-contrast, semi-transparent veils over open water (glint seen
    # by both passes). Real clouds are textured at the ~30 km scale; dense smooth decks are kept.
    mean = box_blur(a, 6, 2)
    sigma = np.sqrt(np.maximum(box_blur(a * a, 6, 2) - mean * mean, 0))
    veil = water * (1 - np.clip((sigma - .025) / .045, 0, 1)) * (1 - np.clip((mean - .35) / .2, 0, 1))
    veil = np.maximum(veil, np.clip(box_blur(veil, 8, 2) * 2, 0, 1) * water * (1 - np.clip((mean - .35) / .2, 0, 1)))  # band edges
    a *= 1 - veil
    del mean, sigma, veil
    # Antarctic polar night on the July date: south of 52 deg S cross-fade (8 deg) to the real
    # southern-summer snapshot; any remaining no-data feathers to clear sky over ~150 km.
    south = cached('viirs_snpp_south_16k', lambda: gibs_mosaic('snpp', GIBS_SOUTH_DATE, range(15, 20)))
    valid_south = south.max(2) > 3
    lat = 90 - (np.arange(H16) + .5) * 180 / H16
    band = np.clip((-52 - lat) / 8, 0, 1).astype(np.float32)[:, None]
    w = np.where(valid, band, 1) * valid_south
    a = a * (1 - w) + opacity(luminance(south), surface) * w
    valid = valid | valid_south
    del south
    fade = np.clip(large(box_blur(small((~valid).astype(np.float32)), 8)) * 2, 0, 1)
    fade = np.maximum(fade, (~valid).astype(np.float32))
    a *= 1 - fade
    # Daily mosaics meet at the pole with a time step (straight seam). Above 80 deg latitude blend
    # each row towards its zonal mean, fully at 86 deg.
    cap = np.clip((np.abs(lat) - 80) / 6, 0, 1).astype(np.float32)[:, None]
    a = a * (1 - cap) + a.mean(1, keepdims=True) * cap
    clouds = np.round(a * 255).astype(np.uint8)
    stats = dict(date=GIBS_DATE, south_date=GIBS_SOUTH_DATE, seams=len(offsets), seam_score=seam_score, glint_replaced_fraction=float((use_noaa > .5).mean()), polar_night_fraction=float((fade > .5).mean()),
                 mean_opacity=float(a.mean()))
    (WORK / 'earth_clouds_stats.json').write_text(json.dumps(stats, indent=2))
    log('clouds', stats)
    write(clouds, EARTH / 'earth_clouds_16k.jpg', quality=95)


def step_night():
    """NASA Black Marble 2016 (VIIRS DNB, 3 km, 13500x6750) resampled to 8192x4096."""
    buf = read(RAW / 'BlackMarble_2016_3km.jpg')
    night = resize(buf, 8192, 4096).get_pixels(oiio.UINT8)[..., :3].astype(np.float32)
    # Black Marble shows a dim blue land/ocean/ice base under the lights. Only artificial light
    # should glow: subtract a smooth local background (min filter ~150 km, then blur) per channel.
    background = night.copy()
    for axis in (0, 1):
        for _ in range(25):
            background = np.minimum(background, np.minimum(np.roll(background, 1, axis), np.roll(background, -1, axis)))
    background = np.stack([box_blur(background[..., c], 8, 2) for c in range(3)], 2)
    lights = np.clip(night - background * 1.3 - 6, 0, 255)
    # Residual moonlit snow/ice texture is blue-dominant; artificial lights are neutral to warm.
    lights *= np.clip(((lights[..., 0] + lights[..., 1]) * .5 - lights[..., 2]) / 8 + 1, 0, 1)[..., None]
    write(np.round(lights).astype(np.uint8), EARTH / 'earth_night_8k.jpg', quality=95)


def step_spheres():
    """Finer smooth unit spheres (same convention as Art/Space/SolarSphere.glb: radius 1 m, pole on
    Blender Z). The runtime picks one by projected pixel radius so the limb stays sub-pixel."""
    import bpy
    meshes = ROOT / 'Art/Space/PlanetQuality/Meshes'
    meshes.mkdir(parents=True, exist_ok=True)
    for segments in (256, 512):
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=segments // 2, radius=1)
        sphere = bpy.context.object
        sphere.name = f'SM_PlanetSphere_{segments}'
        for face in sphere.data.polygons:
            face.use_smooth = True
        path = meshes / f'PlanetSphere_{segments}.glb'
        bpy.ops.export_scene.gltf(filepath=str(path), export_format='GLB', use_selection=True,
                                  export_materials='NONE')
        log('wrote', path.relative_to(ROOT).as_posix(), len(sphere.data.vertices), 'vertices', f'{path.stat().st_size / 1e6:.1f} MB')


def step_defaults():
    """Tiny neutral textures for optional material inputs (no night lights, no clouds, flat land)."""
    folder = ROOT / 'Art/Space/PlanetQuality/Defaults'
    folder.mkdir(parents=True, exist_ok=True)
    write(np.zeros((4, 4, 3), np.uint8), folder / 'pq_black.png')
    write(np.zeros((4, 4), np.uint8), folder / 'pq_clear.png')
    write(np.full((4, 4), LAND_MIN, np.uint8), folder / 'pq_flat_land.png')


for step in STEPS:
    t0 = time.time()
    {'earth-day': step_day, 'earth-relief': step_relief, 'earth-clouds': step_clouds, 'earth-night': step_night,
     'spheres': step_spheres, 'defaults': step_defaults,
     'debug-seams': step_debug_seams,
     'cache-etopo': lambda: cached('earth_height_16k', etopo_heights),
     'cache-viirs': lambda: [cached('viirs_' + n + '_16k', lambda n=n: gibs_mosaic(n)) for n in GIBS_LAYERS]}[step]()
    log('step', step, 'done', round(time.time() - t0, 1), 's')
print('PLANET_SOURCES_PASS', STEPS)
