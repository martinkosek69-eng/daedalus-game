"""Five original fictional star systems for flight testing (task 0015).

Run from the repository root (background Blender, saved files only):
  blender --background --factory-startup --python Tools/Prepare-FiveSystems.py
Optional environment:
  FIVE_SYSTEMS_ONLY=fic.asterion[,fic.velara]  build a subset (iteration)
  FIVE_SYSTEMS_FAST=1                           half-resolution textures (iteration only, never delivered)

For every system directory Art/Space/Systems/<Name>/ the script writes
system.json (data contract of Tasks/0015/BRIEF.md), SOURCES.md, an editable
<Name>.blend with a clearly schematic, animated preview scene, Models/*.glb
(1 m unit sphere, unit irregular bodies and belt rock variants), Textures/*
and preview.png. All maps and meshes are procedural and original (value-noise
fBm on the sphere, craters, bands, storms, ice cracks, lava, clouds, rings);
no third-party images or models. Physical sizes and positions live only in the
JSON, in metres; every exported mesh is normalised to a 1 m maximum radius.
The script validates parents, metres, overlaps, Hill stability, Kepler periods,
texture decoding, GLB bounds and the reopened .blend, then prints
FIVE_SYSTEMS_PASS with a summary.
"""
import bpy, bmesh, json, math, hashlib, os, sys
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix

ROOT = Path(__file__).resolve().parent.parent
OUTROOT = ROOT / 'Art/Space/Systems'
ONLY = set(filter(None, os.environ.get('FIVE_SYSTEMS_ONLY', '').split(',')))
FAST = os.environ.get('FIVE_SYSTEMS_FAST') == '1'

AU, R_E, R_J, R_SUN = 1.495978707e11, 6.371e6, 6.9911e7, 6.957e8
M_SUN, M_E, M_J, G = 1.98847e30, 5.9722e24, 1.89813e27, 6.6743e-11
KM = 1000.0


# ============================================================== noise on the sphere
class Noise:
    """3D value noise (smooth trilinear, hashed lattice). Deterministic per seed."""
    def __init__(self, seed):
        self.tab = np.random.default_rng(seed).random(1 << 16).astype(np.float32)
    def __call__(self, x, y, z):
        xi, yi, zi = np.floor(x), np.floor(y), np.floor(z)
        xf, yf, zf = (x - xi).astype(np.float32), (y - yi).astype(np.float32), (z - zi).astype(np.float32)
        xi, yi, zi = xi.astype(np.int64), yi.astype(np.int64), zi.astype(np.int64)
        u, v, w = xf * xf * (3 - 2 * xf), yf * yf * (3 - 2 * yf), zf * zf * (3 - 2 * zf)
        t = self.tab
        def h(a, b, c): return t[((a * 73856093) ^ (b * 19349663) ^ (c * 83492791)) & 0xFFFF]
        x0 = h(xi, yi, zi) * (1 - u) + h(xi + 1, yi, zi) * u
        x1 = h(xi, yi + 1, zi) * (1 - u) + h(xi + 1, yi + 1, zi) * u
        x2 = h(xi, yi, zi + 1) * (1 - u) + h(xi + 1, yi, zi + 1) * u
        x3 = h(xi, yi + 1, zi + 1) * (1 - u) + h(xi + 1, yi + 1, zi + 1) * u
        return (x0 * (1 - v) + x1 * v) * (1 - w) + (x2 * (1 - v) + x3 * v) * w


def sphere_grid(W, H):
    """Equirectangular directions; row 0 = south pole (Blender pixel order), u=0 at longitude -180 deg."""
    lon = (np.arange(W, dtype=np.float32) + 0.5) / W * 2 * np.pi - np.pi
    lat = (np.arange(H, dtype=np.float32) + 0.5) / H * np.pi - np.pi / 2
    LON, LAT = np.meshgrid(lon, lat)
    return np.cos(LAT) * np.cos(LON), np.cos(LAT) * np.sin(LON), np.sin(LAT), LAT, LON


def fbm(nz, X, Y, Z, octaves=6, freq=2.0, gain=0.5, lac=2.03, ridged=False, stretch=(1.0, 1.0, 1.0), warp=None):
    sx, sy, sz = stretch
    if warp is not None:
        X, Y, Z = X + warp[0], Y + warp[1], Z + warp[2]
    total = np.zeros(X.shape, np.float32); amp, norm, f = 1.0, 0.0, freq
    for o in range(octaves):
        n = nz(X * f * sx + 17.1 * o, Y * f * sy - 9.3 * o, Z * f * sz + 4.7 * o) * 2 - 1
        if ridged: n = 1 - np.abs(n); n = n * n
        total += amp * n; norm += amp; amp *= gain; f *= lac
    return total / norm


def up2(a):
    """2x bilinear upsample with horizontal wrap (equirectangular)."""
    H, W = a.shape[:2]
    r = np.roll(a, -1, 1); d = np.concatenate([a[1:], a[-1:]], 0); dr = np.roll(d, -1, 1)
    out = np.empty((2 * H, 2 * W) + a.shape[2:], a.dtype)
    out[0::2, 0::2] = a; out[0::2, 1::2] = (a + r) / 2; out[1::2, 0::2] = (a + d) / 2; out[1::2, 1::2] = (a + r + d + dr) / 4
    return out


def field(W, H, fn):
    """Evaluate fn(X,Y,Z,LAT,LON) at <= 2048 px wide and upsample to W (big maps stay affordable)."""
    w, h = W, H
    while w > 2048: w //= 2; h //= 2
    out = fn(*sphere_grid(w, h))
    while out.shape[1] < W: out = up2(out)
    return out


def ramp(t, stops):
    """Colour ramp: stops = [(t, '#rrggbb'), ...] -> sRGB float (…,3)."""
    ts = np.array([s[0] for s in stops], np.float32)
    cs = np.array([[int(s[1][i:i + 2], 16) / 255 for i in (1, 3, 5)] for s in stops], np.float32)
    t = np.clip(t, ts[0], ts[-1])
    return np.stack([np.interp(t, ts, cs[:, k]) for k in range(3)], -1).astype(np.float32)


def hexrgb(h): return np.array([int(h[i:i + 2], 16) / 255 for i in (1, 3, 5)], np.float32)


def smooth(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0, 1); return t * t * (3 - 2 * t)


def craters(W, H, rng, count, rmin, rmax, depth=0.5, fresh=0.2):
    """Bowl + raised rim height field and bright ejecta halos of fresh craters (angular radii in radians)."""
    X, Y, Z, LAT, LON = sphere_grid(W, H)
    hgt = np.zeros((H, W), np.float32); bright = np.zeros((H, W), np.float32)
    lat_rows = (np.arange(H) + 0.5) / H * np.pi - np.pi / 2
    for _ in range(count):
        z = rng.uniform(-1, 1); a = rng.uniform(0, 2 * np.pi); s = math.sqrt(1 - z * z)
        c = (s * math.cos(a), s * math.sin(a), z)
        r = rmin * (rmax / rmin) ** (rng.random() ** 2.2)
        isfresh = rng.random() < fresh
        reach = r * (3.5 if isfresh else 2.2)
        rows = np.nonzero(np.abs(lat_rows - math.asin(z)) < reach)[0]
        if len(rows) == 0: continue
        d = np.arccos(np.clip(X[rows] * c[0] + Y[rows] * c[1] + Z[rows] * c[2], -1, 1)) / r
        bowl = np.where(d < 1, (d * d - 1) * depth, 0) + np.exp(-((d - 1.0) / 0.18) ** 2) * depth * 0.35 * (d < 2.2)
        hgt[rows] += bowl.astype(np.float32)
        if isfresh:
            bright[rows] += (np.exp(-np.maximum(d - 0.9, 0) / 0.9) * (d < 3.5) * 0.6).astype(np.float32)
    return hgt, np.clip(bright, 0, 1)


def hillshade(h, LAT, k=0.18):
    """Subtle baked relief accent (light from the north-west) so terrain reads at a distance."""
    dx = (np.roll(h, -1, 1) - np.roll(h, 1, 1)) / np.maximum(np.cos(LAT), 0.15)
    dy = np.concatenate([h[1:] - h[:-1], h[-1:] - h[-2:-1]], 0) * 2
    s = -(dx * 0.7 - dy * 0.7) * h.shape[1] / 64.0
    return np.clip(1 + k * np.tanh(s), 0.6, 1.4)


def add_detail(col, nz, W, H, amount=0.06, freq=48.0):
    """Full-resolution fine variation after upsampling (keeps 4K maps crisp)."""
    if amount <= 0: return col
    X, Y, Z, LAT, LON = sphere_grid(W, H)
    d = fbm(nz, X, Y, Z, 2, freq, 0.5)
    return np.clip(col * (1 + amount * d)[..., None], 0, 1)


def paint_star(sp, W, H):
    nz = Noise(sp['seed'])
    def f(X, Y, Z, LAT, LON):
        gran = fbm(nz, X, Y, Z, 4, 60.0, 0.55)
        cells = fbm(nz, X, Y, Z, 5, 14.0, 0.5)
        big = fbm(nz, X, Y, Z, 3, 2.5)
        act = smooth(0.55, 0.75, fbm(nz, X * 1.1, Y * 1.1, Z * 1.1, 4, 3.0) * 0.5 + 0.5) * smooth(0.75, 0.2, np.abs(LAT))
        spots = act * smooth(0.62, 0.8, fbm(nz, X, Y, Z, 4, 18.0) * 0.5 + 0.5)
        fac = act * smooth(0.45, 0.65, fbm(nz, Y, Z, X, 4, 10.0) * 0.5 + 0.5) * (1 - spots)
        t = 0.62 + 0.14 * gran + 0.12 * cells + 0.06 * big + 0.12 * fac - 0.75 * spots
        return ramp(t, sp['palette'])
    return add_detail(field(W, H, f), nz, W, H, 0.04, 90.0) if W > 2048 else f(*sphere_grid(W, H))


def paint_rocky(sp, W, H, rng):
    """Rocky body: warped continents, ridged ranges, craters with fresh ejecta, maria, caps, relief accent."""
    nz = Noise(sp['seed'])
    def base(X, Y, Z, LAT, LON):
        warp = (fbm(nz, X, Y, Z, 3, 1.5) * 0.35, fbm(nz, Y, Z, X, 3, 1.5) * 0.35, fbm(nz, Z, X, Y, 3, 1.5) * 0.35)
        cont = fbm(nz, X, Y, Z, 8, sp.get('freq', 1.6), 0.52, warp=warp)
        ridge = fbm(nz, X, Y, Z, 6, 4.0, 0.5, ridged=True, warp=warp)
        detail = fbm(nz, X, Y, Z, 6, 12.0, 0.55)
        tone = fbm(nz, Y, Z, X, 4, 3.0)
        return np.stack([cont, ridge, detail, tone, LAT, np.cos(LON) * np.cos(LAT)], -1)
    b = field(W, H, base)
    cont, ridge, detail, tone, LAT, FACE = (b[..., i] for i in range(6))
    hgt = cont + 0.28 * (ridge - 0.5) * sp.get('mountains', 1.0) + 0.22 * detail
    bright = np.zeros_like(hgt)
    if sp.get('craters'):
        cr, br = craters(min(W, 2048), min(H, 1024), rng, sp['craters'], sp.get('crmin', 0.005), sp.get('crmax', 0.16), sp.get('crdepth', 0.5))
        while cr.shape[1] < W: cr, br = up2(cr), up2(br)
        hgt = hgt + cr; bright = br * sp.get('rays', 1.0)
    t = (hgt - np.percentile(hgt, 1)) / (np.percentile(hgt, 99) - np.percentile(hgt, 1) + 1e-6)
    col = ramp(np.clip(t, 0, 1), sp['palette'])
    col *= (0.9 + 0.2 * (tone * 0.5 + 0.5))[..., None]
    if sp.get('maria'):   # dark lowland basins
        col = col * (1 - 0.38 * smooth(0.5, 0.36, t)[..., None])
    col = np.clip(col + 0.35 * bright[..., None] * (1 - col), 0, 1)
    if sp.get('caps'):
        edge = sp['caps'] + 0.06 * detail + 0.04 * tone
        cap = smooth(edge, edge + 0.05, np.abs(LAT))[..., None]
        col = col * (1 - cap) + hexrgb(sp.get('capcol', '#f2f4f7')) * (0.92 + 0.08 * detail[..., None]) * cap
    if sp.get('twotone'):  # one dark leading hemisphere (Iapetus-like), centred on longitude 0
        col = col * (1 - sp['twotone'] * smooth(-0.15, 0.25, FACE + 0.15 * detail)[..., None])
    if sp.get('spot'):     # one dark reddish spot (Haumea-like)
        m = smooth(0.82, 0.92, FACE)[..., None]; col = col * (1 - m) + hexrgb(sp['spot']) * m
    col *= hillshade(hgt, LAT, sp.get('relief', 0.16))[..., None]
    return add_detail(np.clip(col, 0, 1), nz, W, H, 0.05)


def paint_gas(sp, W, H):
    """Banded giant: several band frequencies with sharp edges, zonal jets, festoons, white ovals and storms."""
    nz = Noise(sp['seed']); rng = np.random.default_rng(sp['seed'] + 3)
    nb = sp.get('bands', 9.0)
    ovals = [(rng.uniform(-1.0, 1.0), rng.uniform(-np.pi, np.pi), rng.uniform(0.015, 0.04)) for _ in range(sp.get('ovals', 10))]
    def f(X, Y, Z, LAT, LON):
        turb = fbm(nz, X, Y, Z, 6, 3.0, 0.55, stretch=(1, 1, 6.0))
        swirl = fbm(nz, X, Y, Z, 6, 9.0, 0.55, stretch=(1, 1, 3.0))
        jets = fbm(nz, X, Y, Z, 5, 22.0, 0.5, stretch=(1, 1, 10.0))
        lat_w = LAT + 0.05 * turb * sp.get('turb', 1.6) + 0.012 * swirl
        phase = lat_w * nb
        bands = 0.5 * np.sin(phase + 0.7 * np.sin(phase * 0.37 + 1.0)) + 0.22 * np.sin(phase * 2.6 + 1.3)
        edge = np.abs(np.cos(phase + 0.7 * np.sin(phase * 0.37 + 1.0)))
        t = 0.5 + 0.62 * bands + 0.05 * jets + 0.07 * swirl * edge
        col = ramp(t, sp['palette'])
        for (lat0, lon0, rr) in ovals:   # small white ovals
            dlon = np.angle(np.exp(1j * (LON - lon0)))
            e = (dlon / (rr * 1.8)) ** 2 + ((LAT - lat0) / rr) ** 2
            m = smooth(1.0, 0.2, e)[..., None] * 0.45
            col = col * (1 - m) + hexrgb(sp.get('ovalcol', '#efe4cf')) * m
        for (lat0, lon0, rx, ry, colh, s) in sp.get('storms', []):
            dlon = np.angle(np.exp(1j * (LON - lon0)))
            e = (dlon / rx) ** 2 + ((LAT - lat0) / ry) ** 2
            spiral = 0.5 + 0.5 * np.sin(10 * np.sqrt(e) - 3 * np.arctan2(LAT - lat0, dlon) + swirl * 2)
            m = smooth(1.0, 0.0, e)[..., None] * s
            ring_ = (smooth(1.25, 1.0, e) * smooth(0.8, 1.0, e))[..., None] * 0.35 * s
            col = col * (1 - m - ring_) + (hexrgb(colh) * (0.8 + 0.2 * spiral[..., None])) * m + hexrgb('#f6eee0') * ring_
        if sp.get('polar'):
            p = smooth(1.0, 1.35, np.abs(LAT) + 0.05 * turb)[..., None]
            col = col * (1 - p) + hexrgb(sp['polar']) * (0.9 + 0.2 * (swirl[..., None] * 0.5 + 0.5)) * p
        return np.clip(col, 0, 1)
    return add_detail(field(W, H, f), nz, W, H, 0.035, 60.0)


def paint_ocean(sp, W, H):
    """Global ocean with island arcs, shallow shelves, polar ice."""
    nz = Noise(sp['seed'])
    def f(X, Y, Z, LAT, LON):
        land = fbm(nz, X, Y, Z, 8, 1.8, 0.55, warp=(fbm(nz, Y, Z, X, 3, 2.0) * 0.4, 0, 0))
        arcs = fbm(nz, X, Y, Z, 6, 4.0, ridged=True)
        hgt = land + 0.35 * arcs - sp.get('sea', 0.38)
        deep = ramp(np.clip(hgt + 0.6, 0, 1), sp['ocean'])
        shelf = smooth(-0.12, 0.0, hgt)[..., None]
        col = deep * (1 - shelf) + hexrgb(sp['shallow']) * shelf
        isl = smooth(0.0, 0.02, hgt)[..., None]
        col = col * (1 - isl) + ramp(np.clip(hgt * 4, 0, 1), sp['land']) * isl
        col *= (0.94 + 0.06 * fbm(nz, X, Y, Z, 4, 10.0)[..., None])
        ice = smooth(sp.get('ice', 1.2), sp.get('ice', 1.2) + 0.08, np.abs(LAT) + 0.05 * land)[..., None]
        return np.clip(col * (1 - ice) + hexrgb('#eef4fa') * ice, 0, 1)
    return add_detail(field(W, H, f), nz, W, H, 0.03)


def paint_ice(sp, W, H, rng):
    """Icy body: bright ice with contrasting crack networks (lineae), mottled terrains, stripes / eyeball ocean."""
    nz = Noise(sp['seed'])
    def f(X, Y, Z, LAT, LON):
        warp = (fbm(nz, X, Y, Z, 3, 2.0) * 0.25, fbm(nz, Y, Z, X, 3, 2.0) * 0.25, 0)
        crack = fbm(nz, X, Y, Z, 6, sp.get('cfreq', 5.0), 0.55, ridged=True, warp=warp)
        crack2 = fbm(nz, Y, Z, X, 5, sp.get('cfreq', 5.0) * 2.2, 0.5, ridged=True)
        base = fbm(nz, X, Y, Z, 6, 2.2, 0.5)
        mottle = fbm(nz, X, Y, Z, 5, 7.0, 0.55)
        t = 0.6 + 0.25 * base + 0.08 * mottle - 0.6 * smooth(0.72, 0.95, crack) - 0.25 * smooth(0.8, 0.97, crack2)
        col = ramp(t, sp['palette'])
        if sp.get('stripes'):   # geyser fissures near the south pole
            sz = smooth(-0.9, -1.3, LAT)
            fis = smooth(0.12, 0.04, np.abs(np.sin(LON * 2.0 + LAT * 9.0 + base * 2)))
            m = (fis * sz)[..., None]; col = col * (1 - m) + hexrgb(sp['stripes']) * m
        if sp.get('eyeball'):   # liquid sub-stellar ocean at longitude lon0
            lon0, r0 = sp['eyeball']
            d = np.arccos(np.clip(np.cos(LAT) * np.cos(LON - lon0), -1, 1)) + 0.12 * base
            ocean = smooth(r0, r0 - 0.15, d)[..., None]
            water = ramp(np.clip(d / r0, 0, 1), [(0, '#0b2c55'), (0.7, '#13467a'), (1.0, '#3f7aa6')])
            col = col * (1 - ocean) + water * ocean
            shore = (smooth(r0 - 0.15, r0 - 0.05, d) * smooth(r0 + 0.05, r0 - 0.05, d))[..., None]
            col = col * (1 - 0.5 * shore) + hexrgb('#8c8a80') * 0.5 * shore
        return np.concatenate([np.clip(col, 0, 1), base[..., None], LAT[..., None]], -1)
    out = field(W, H, f); col, base, LAT = out[..., :3], out[..., 3], out[..., 4]
    if sp.get('craters'):
        cr, br = craters(min(W, 2048), min(H, 1024), rng, sp['craters'], 0.005, 0.1, 0.4)
        while cr.shape[1] < W: cr, br = up2(cr), up2(br)
        col = np.clip(col * hillshade(base * 0.3 + cr, LAT, 0.2)[..., None] + 0.2 * br[..., None] * (1 - col), 0, 1)
    return add_detail(col, nz, W, H, 0.04)


def paint_lava(sp, W, H):
    """Volcanic body (Io-like): sulphur plains in yellow/orange/white, black flows, glowing lava. Returns (albedo, night)."""
    nz = Noise(sp['seed'])
    def f(X, Y, Z, LAT, LON):
        base = fbm(nz, X, Y, Z, 7, 2.0, 0.55)
        rivers = fbm(nz, X, Y, Z, 6, 3.5, 0.55, ridged=True, warp=(base * 0.4, base * 0.3, 0))
        sul = fbm(nz, Y, Z, X, 6, 2.5, 0.55)
        frost = smooth(0.35, 0.6, fbm(nz, Z, X, Y, 5, 4.0) * 0.5 + 0.5)
        col = ramp(base * 0.5 + 0.5, sp['palette'])
        sulc = ramp(sul * 0.5 + 0.5, [(0, '#8a5a1e'), (0.4, hex_of(sp.get('sulphur', '#c9b24a'))), (0.75, '#e8d070'), (1, '#f4ecd0')])
        sm = smooth(-0.2, 0.25, sul)[..., None]
        col = col * (1 - 0.75 * sm) + sulc * 0.75 * sm
        col = col * (1 - 0.35 * frost[..., None]) + hexrgb('#f2eee2') * 0.35 * frost[..., None]
        flows = smooth(sp.get('lava', 0.78) - 0.1, sp.get('lava', 0.78), rivers)
        lava = smooth(sp.get('lava', 0.78), 0.97, rivers)
        pits = smooth(0.8, 0.94, fbm(nz, X * 2, Y * 2, Z * 2, 4, 6.0) * 0.5 + 0.5)
        glow = np.clip(lava + pits, 0, 1)
        col = col * (1 - 0.8 * flows[..., None]) + hexrgb('#1a0c06') * 0.8 * flows[..., None]
        col = col * (1 - 0.6 * glow[..., None]) + hexrgb('#5a1a08') * 0.6 * glow[..., None]
        night = ramp(glow, [(0, '#000000'), (0.25, '#2a0600'), (0.6, '#ff4a0c'), (1.0, '#ffd884')])
        return np.concatenate([col, night], -1)
    out = field(W, H, f)
    return add_detail(np.clip(out[..., :3], 0, 1), nz, W, H, 0.05), np.clip(out[..., 3:], 0, 1)


def hex_of(h): return h


def paint_terra(sp, W, H):
    """Living world that is NOT an Earth recolour: crimson/violet vegetation, ochre deserts, teal seas."""
    nz = Noise(sp['seed'])
    def f(X, Y, Z, LAT, LON):
        warp = (fbm(nz, Y, Z, X, 4, 1.5) * 0.5, fbm(nz, Z, X, Y, 4, 1.5) * 0.5, 0)
        cont = fbm(nz, X, Y, Z, 8, 1.4, 0.55, warp=warp) - sp.get('sea', 0.05)
        moist = fbm(nz, Y, Z, X, 6, 2.5) - np.abs(LAT) * 0.25
        mtn = fbm(nz, X, Y, Z, 6, 5.0, ridged=True)
        sea = ramp(np.clip(cont + 0.7, 0, 1), sp['sea_pal'])
        landc = ramp(np.clip(moist * 0.9 + 0.5, 0, 1), sp['land_pal'])
        landc = landc * (1 - 0.4 * smooth(0.6, 0.9, mtn)[..., None]) + hexrgb('#9a8f86') * 0.4 * smooth(0.6, 0.9, mtn)[..., None]
        land = smooth(0.0, 0.025, cont)[..., None]
        col = sea * (1 - land) + landc * land
        ice = smooth(1.15, 1.25, np.abs(LAT) + 0.08 * moist)[..., None]
        col = col * (1 - ice) + hexrgb('#eef2f6') * ice
        col = col * (0.9 + 0.2 * (fbm(nz, X, Y, Z, 5, 14.0) * 0.5 + 0.5))[..., None]
        relief = np.where(cont > 0, cont + 0.3 * mtn, 0)
        return np.concatenate([np.clip(col, 0, 1), relief[..., None], LAT[..., None]], -1)
    out = field(W, H, f)
    col = out[..., :3] * hillshade(out[..., 3], out[..., 4], 0.14)[..., None]
    return add_detail(np.clip(col, 0, 1), nz, W, H, 0.05)


def paint_clouds(sp, W, H):
    """Cloud layer RGBA (white, alpha = coverage). Optional giant cyclone(s) and banding."""
    nz = Noise(sp['seed'] + 101)
    def f(X, Y, Z, LAT, LON):
        lat_w = LAT; lon_w = LON
        for (lat0, lon0, rad, turns) in sp.get('cyclones', []):   # spiral warp around a storm centre
            dlon = np.angle(np.exp(1j * (LON - lon0))); dl = LAT - lat0
            r = np.sqrt((dlon * np.cos(lat0)) ** 2 + dl ** 2)
            ang = turns * np.exp(-(r / rad) ** 2)
            lon_w = lon0 + dlon * np.cos(ang) - dl * np.sin(ang); lat_w = lat0 + dlon * np.sin(ang) + dl * np.cos(ang)
        Xw, Yw, Zw = np.cos(lat_w) * np.cos(lon_w), np.cos(lat_w) * np.sin(lon_w), np.sin(lat_w)
        c = fbm(nz, Xw, Yw, Zw, 7, sp.get('cfreq', 3.0), 0.55, stretch=(1, 1, sp.get('zonal', 2.0)))
        bands = 0.15 * np.sin(LAT * sp.get('cbands', 6.0))
        cov = smooth(sp.get('cover', 0.05), sp.get('cover', 0.05) + 0.35, c + bands)
        for (lat0, lon0, rad, turns) in sp.get('cyclones', []):
            dlon = np.angle(np.exp(1j * (LON - lon0))); r = np.sqrt((dlon * np.cos(lat0)) ** 2 + (LAT - lat0) ** 2)
            eye = smooth(rad * 0.08, rad * 0.16, r); wall = smooth(rad * 1.3, rad * 0.2, r)
            cov = np.clip(np.maximum(cov, wall * 0.95) * eye, 0, 1)
        tint = hexrgb(sp.get('ctint', '#ffffff'))
        rgb = np.ones(cov.shape + (3,), np.float32) * tint * (0.85 + 0.15 * cov[..., None])
        return np.concatenate([rgb, cov[..., None]], -1)
    return np.clip(field(W, H, f), 0, 1)


def paint_ring(sp, n=2048):
    """Radial ring profile RGBA (x: inner -> outer), 64 rows of identical data."""
    rng = np.random.default_rng(sp['seed'])
    x = np.linspace(0, 1, n, dtype=np.float32)
    dens = np.zeros(n, np.float32)
    for (c, w, a) in sp['bands']: dens += a * np.exp(-((x - c) / w) ** 2)
    fine = np.convolve(rng.random(n + 32).astype(np.float32), np.ones(9) / 9, 'same')[16:-16]
    dens *= 0.75 + 0.5 * fine
    for (c, w) in sp.get('gaps', []): dens *= 1 - np.exp(-((x - c) / w) ** 8)
    dens *= smooth(0.0, 0.03, x) * smooth(1.0, 0.96, x)
    alpha = np.clip(dens, 0, 1) * sp.get('opacity', 0.9)
    col = ramp(x + 0.08 * (fine - 0.5), sp['palette'])
    img = np.concatenate([col, alpha[:, None]], -1)
    return np.repeat(img[None], 64, 0)


# ============================================================== system definitions
# Units: R metres, mass kg, a metres from the parent, angles degrees, rot hours ('locked' = orbital period).
# 'paint' selects the painter; 'res' 4096 for the headline worlds. All values are fictional but physically
# plausible (mass-radius, insolation, Hill spheres, Roche limits); see SOURCES.md of each system.
def P(slug, name, kind, parent, R, mass, a, ang, inc, rot, tilt, paint, res=2048, **kw):
    d = dict(slug=slug, name=name, kind=kind, parent=parent, R=R, mass=mass, a=a, ang=ang, inc=inc, rot=rot, tilt=tilt, paint=paint, res=res)
    d.update(kw); return d

ROCK_GREY = [(0, '#2b2723'), (0.4, '#5d554c'), (0.7, '#8a8074'), (1, '#b8ad9f')]
ICE_GREY = [(0, '#6e6a66'), (0.5, '#a9a7a2'), (1, '#dedcd6')]

SYSTEMS = [
 dict(id='fic.asterion', name='Asterion', dir='Asterion', pos=[25970, 40, 20], color='#fff1df', seed=1100,
      star=P('sun', 'Asterion', 'star', None, 1.03 * R_SUN, 1.02 * M_SUN, 0, 0, 0, 609.6, 6.0,
             dict(type='star', seed=1101, palette=[(0, '#b8552a'), (0.35, '#f2a54a'), (0.6, '#ffe2a0'), (0.85, '#fff6dc'), (1, '#ffffff')])),
      feature='A true double planet: Tessaly and Tessa Minor orbit a barycentre that lies outside both bodies. '
              'The ringed giant Varunis keeps a tidally heated volcanic moon, Pyra, whose lava glows on the night side. '
              'The main belt hosts Kestrel, an irregular battered protoplanet.',
      bodies=[
        P('ignis', 'Ignis', 'planet', 'sun', 0.42 * R_E, 0.07 * M_E, 0.32 * AU, 25, 2.1, 1407.0, 0.2,
          dict(type='rocky', seed=1110, palette=ROCK_GREY, craters=260, maria=True, streaks=0.15)),
        P('calyx', 'Calyx', 'planet', 'sun', 0.95 * R_E, 0.82 * M_E, 0.68 * AU, 140, 1.2, -2802.0, 2.0,
          dict(type='rocky', seed=1120, palette=[(0, '#4a3a2a'), (0.5, '#8f7452'), (1, '#c9ae82')], craters=20),
          clouds=dict(seed=1121, cover=-0.45, ctint='#f0d9a8', zonal=4.0, cbands=10.0), atmos='#e9c98f'),
        P('tessaly', 'Tessaly', 'planet', 'sun', 7136 * KM, 1.45 * M_E, 1.08 * AU, 230, 0.4, 'locked_pair', 11.0,
          dict(type='rocky', seed=1130, palette=[(0, '#3b1d14'), (0.35, '#7a3a22'), (0.6, '#b8643a'), (0.8, '#d89b66'), (1, '#efd2a8')],
               craters=60, caps=1.22, capcol='#f4f1ec'), res=4096, atmos='#d9a07a',
          clouds=dict(seed=1131, cover=0.42, ctint='#e8c8a8', zonal=2.5)),
        P('tessa_minor', 'Tessa Minor', 'moon', 'tessaly', 3900 * KM, 0.16 * M_E, 1.15e8, 40, 0, 'locked', 11.0,
          dict(type='rocky', seed=1140, palette=[(0, '#3e434a'), (0.5, '#7d858c'), (1, '#c7ccd1')], craters=180, streaks=0.2), res=4096),
        P('kestrel', 'Kestrel', 'asteroid', 'sun', 265 * KM, 2.4e20, 2.72 * AU, 300, 3.0, 7.6, 30.0,
          dict(type='rocky', seed=1150, palette=[(0, '#2d2925'), (0.5, '#5a524a'), (1, '#958a7c')], craters=120), mesh=dict(seed=1151, elong=(1.0, 0.68, 0.55), rough=0.3)),
        P('varunis', 'Varunis', 'planet', 'sun', 1.05 * R_J, 1.3 * M_J, 4.8 * AU, 20, 1.3, 9.6, 26.5,
          dict(type='gas', seed=1160, palette=[(0, '#6e4a32'), (0.3, '#b98a5e'), (0.55, '#e9d2a8'), (0.75, '#f6ead2'), (1, '#a8693f')],
               bands=11.0, storms=[(-0.38, 1.2, 0.25, 0.12, '#b0532c', 0.9)], polar='#8b7a6a'), res=4096,
          ring=dict(inner=1.38, outer=2.25, seed=1161, palette=[(0, '#8c7a66'), (0.5, '#d6c4a6'), (1, '#a99683')],
                    bands=[(0.15, 0.12, 0.5), (0.45, 0.2, 0.9), (0.78, 0.12, 0.7)], gaps=[(0.62, 0.025)])),
        P('pyra', 'Pyra', 'moon', 'varunis', 1840 * KM, 8.9e22, 4.4e8, 70, 0, 'locked', 0.0,
          dict(type='lava', seed=1170, palette=[(0, '#1f1712'), (0.5, '#4a3a26'), (1, '#a08a52')], sulphur='#d8c24a')),
        P('glacia', 'Glacia', 'moon', 'varunis', 1560 * KM, 4.8e22, 7.0e8, 160, 0, 'locked', 0.0,
          dict(type='ice', seed=1180, palette=[(0, '#8a7f74'), (0.5, '#d7dce0'), (1, '#f6f8fa')], cfreq=6.0, craters=40)),
        P('umbra', 'Umbra', 'moon', 'varunis', 2410 * KM, 1.48e23, 1.12e9, 250, 0, 'locked', 0.0,
          dict(type='rocky', seed=1190, palette=[(0, '#1e1c1b'), (0.5, '#4a4540'), (1, '#9b938a')], craters=300, streaks=0.25)),
        P('halcyon', 'Halcyon', 'planet', 'sun', 3.6 * R_E, 14 * M_E, 11.6 * AU, 115, 1.8, 15.2, 41.0,
          dict(type='gas', seed=1200, palette=[(0, '#4f8a9e'), (0.5, '#8cc7d4'), (1, '#c9eef2')], bands=5.0, turb=0.8)),
        P('skerry', 'Skerry', 'moon', 'halcyon', 540 * KM, 6.0e20, 2.4e8, 30, 0, 'locked', 0.0,
          dict(type='rocky', seed=1210, palette=ICE_GREY, craters=150)),
      ],
      belts=[dict(slug='main_belt', inner=2.2 * AU, outer=3.2 * AU, kind='asteroid', seed=1301)]),

 dict(id='fic.velara', name='Velara', dir='Velara', pos=[25400, 300, 100], color='#ffd2a1', seed=2100,
      star=P('sun', 'Velara', 'star', None, 0.79 * R_SUN, 0.78 * M_SUN, 0, 0, 0, 792.0, 4.0,
             dict(type='star', seed=2101, palette=[(0, '#8f2f12'), (0.35, '#e2702c'), (0.65, '#ffb46a'), (0.9, '#ffe2bd'), (1, '#fff3e2')])),
      feature='Thalassa is a global ocean world crowned by a permanent superstorm with a clear eye. '
              'Corvan is a rare ringed rocky super-Earth: an icy debris ring well inside its Roche limit. '
              'The golden giant Aurelion carries broad gapped rings and a cracked-ice ocean moon, Brine.',
      bodies=[
        P('cinder', 'Cinder', 'planet', 'sun', 0.6 * R_E, 0.22 * M_E, 0.17 * AU, 60, 1.5, 'locked', 0.5,
          dict(type='lava', seed=2110, palette=[(0, '#140f0d'), (0.5, '#3a2a22'), (1, '#7a5d48')], sulphur='#8c5a2a', lava=0.74)),
        P('thalassa', 'Thalassa', 'planet', 'sun', 1.35 * R_E, 2.2 * M_E, 0.64 * AU, 170, 0.6, 19.5, 18.0,
          dict(type='ocean', seed=2120, ocean=[(0, '#06152e'), (0.5, '#0b2f63'), (1, '#1f5c8f')], shallow='#2f8ea3',
               land=[(0, '#5c4a3a'), (0.5, '#7f8a55'), (1, '#c9c0a8')], sea=0.5, ice=1.3), res=4096, atmos='#8ccaff',
          clouds=dict(seed=2121, cover=0.0, cyclones=[(0.32, 0.9, 0.22, 4.5)], cfreq=3.2, zonal=2.2)),
        P('coral', 'Coral', 'moon', 'thalassa', 950 * KM, 1.2e22, 3.1e8, 220, 0, 'locked', 0.0,
          dict(type='rocky', seed=2130, palette=[(0, '#4a3f38'), (0.5, '#8b7a6c'), (1, '#c9b8a6')], craters=150)),
        P('nimbus', 'Nimbus', 'planet', 'sun', 2.15 * R_E, 5.8 * M_E, 1.12 * AU, 285, 1.1, 14.2, 9.0,
          dict(type='gas', seed=2140, palette=[(0, '#7d7396'), (0.5, '#c9c2df'), (1, '#f2eefa')], bands=7.0, turb=1.2), res=4096,
          atmos='#d8d0f2', clouds=dict(seed=2141, cover=-0.1, ctint='#f7f2ff', zonal=5.0, cbands=9.0)),
        P('corvan', 'Corvan', 'planet', 'sun', 1.72 * R_E, 4.1 * M_E, 1.85 * AU, 35, 0.9, 21.7, 23.0,
          dict(type='rocky', seed=2150, palette=[(0, '#2d2f33'), (0.4, '#5f5b55'), (0.7, '#9a8a74'), (1, '#d9d4cc')], caps=1.0, craters=40),
          res=4096, ring=dict(inner=1.6, outer=2.7, seed=2151, palette=[(0, '#9fa6ad'), (0.5, '#e4e7ea'), (1, '#b9bec4')],
                               bands=[(0.25, 0.15, 0.8), (0.6, 0.18, 0.9), (0.88, 0.06, 0.5)], gaps=[(0.45, 0.03)])),
        P('shard', 'Shard', 'moon', 'corvan', 320 * KM, 4.0e20, 6.2e7, 120, 0, 'locked', 0.0,
          dict(type='ice', seed=2160, palette=ICE_GREY, craters=60)),
        P('aurelion', 'Aurelion', 'planet', 'sun', 0.93 * R_J, 0.62 * M_J, 5.4 * AU, 205, 1.4, 10.4, 14.0,
          dict(type='gas', seed=2170, palette=[(0, '#6a4a1e'), (0.3, '#b58a3c'), (0.55, '#e8c87a'), (0.8, '#f6e6b8'), (1, '#9a6a2a')],
               bands=13.0, storms=[(0.45, -2.0, 0.12, 0.05, '#f4ecd4', 0.8)], polar='#7a6a52'), res=4096,
          ring=dict(inner=1.24, outer=2.35, seed=2171, palette=[(0, '#7d6a52'), (0.4, '#cdb48a'), (0.7, '#e9dcc0'), (1, '#9b8a72')],
                    bands=[(0.12, 0.08, 0.4), (0.4, 0.16, 1.0), (0.72, 0.14, 0.9), (0.93, 0.05, 0.5)], gaps=[(0.58, 0.03), (0.85, 0.012)])),
        P('mira', 'Mira', 'moon', 'aurelion', 1120 * KM, 1.6e22, 3.0e8, 300, 0, 'locked', 0.0,
          dict(type='ice', seed=2180, palette=[(0, '#9aa3ab'), (0.5, '#dfe5ea'), (1, '#fbfdff')], craters=60)),
        P('opaline', 'Opaline', 'moon', 'aurelion', 760 * KM, 5.0e21, 4.6e8, 30, 0, 'locked', 0.0,
          dict(type='ice', seed=2190, palette=[(0, '#8f7f86'), (0.5, '#d9cdd2'), (1, '#f6eff2')], craters=90)),
        P('brine', 'Brine', 'moon', 'aurelion', 1930 * KM, 1.0e23, 8.4e8, 110, 0, 'locked', 0.0,
          dict(type='ice', seed=2200, palette=[(0, '#7a4f32'), (0.4, '#c4a68a'), (0.7, '#e8dccd'), (1, '#f7f3ee')], cfreq=6.5), res=2048),
      ],
      belts=[dict(slug='main_belt', inner=2.5 * AU, outer=3.4 * AU, kind='asteroid', seed=2301)]),

 dict(id='fic.nivara', name='Nivara', dir='Nivara', pos=[7000, -7000, -120], color='#ff9d63', seed=3100,
      star=P('sun', 'Nivara', 'star', None, 0.21 * R_SUN, 0.18 * M_SUN, 0, 0, 0, 96.0, 3.0,
             dict(type='star', seed=3101, palette=[(0, '#5a1006'), (0.4, '#c23a14'), (0.7, '#ff7a3a'), (0.92, '#ffb07a'), (1, '#ffd6b0')])),
      feature='A compact resonant chain of tidally locked worlds around a red dwarf. Iris is an "eyeball" world: '
              'a liquid ocean under the sub-stellar point surrounded by global ice (texture longitude 0 faces the star). '
              'Ember is a tidally heated lava world whose night side glows; Gelid keeps three small moons.',
      bodies=[
        P('ember', 'Ember', 'planet', 'sun', 0.82 * R_E, 0.6 * M_E, 0.011 * AU, 15, 0.2, 'locked', 0.0,
          dict(type='lava', seed=3110, palette=[(0, '#1a110c'), (0.5, '#3e2a1c'), (1, '#80603e')], sulphur='#b8862e', lava=0.7), res=4096),
        P('ashfall', 'Ashfall', 'planet', 'sun', 0.96 * R_E, 0.9 * M_E, 0.0175 * AU, 95, 0.3, 'locked', 0.0,
          dict(type='rocky', seed=3120, palette=[(0, '#1a1816'), (0.5, '#3c3631'), (0.8, '#6b6255'), (1, '#c8b878')], craters=30, streaks=0.12)),
        P('iris', 'Iris', 'planet', 'sun', 1.07 * R_E, 1.25 * M_E, 0.029 * AU, 185, 0.1, 'locked', 0.0,
          dict(type='ice', seed=3130, palette=[(0, '#8a96a3'), (0.5, '#d3dde6'), (1, '#f4f8fb')], eyeball=(0.0, 0.95), cfreq=4.0),
          res=4096, atmos='#a8cfff', clouds=dict(seed=3131, cover=0.3, zonal=1.5)),
        P('mote', 'Mote', 'moon', 'iris', 160 * KM, 3.0e19, 2.6e7, 80, 0, 'locked', 0.0,
          dict(type='rocky', seed=3140, palette=ROCK_GREY, craters=80), mesh=dict(seed=3141, elong=(1.0, 0.72, 0.6), rough=0.35)),
        P('rime', 'Rime', 'planet', 'sun', 0.92 * R_E, 0.7 * M_E, 0.046 * AU, 275, 0.4, 'locked', 0.0,
          dict(type='ice', seed=3150, palette=[(0, '#3e5a78'), (0.45, '#9fbcd6'), (0.75, '#dbe8f3'), (1, '#f7fbff')], cfreq=4.5)),
        P('glint', 'Glint', 'moon', 'rime', 190 * KM, 5.0e19, 3.2e7, 200, 0, 'locked', 0.0,
          dict(type='ice', seed=3160, palette=ICE_GREY, craters=70), mesh=dict(seed=3161, elong=(1.0, 0.8, 0.66), rough=0.3)),
        P('gelid', 'Gelid', 'planet', 'sun', 2.4 * R_E, 7.5 * M_E, 0.2 * AU, 330, 0.8, 31.0, 12.0,
          dict(type='gas', seed=3170, palette=[(0, '#3a3b55'), (0.5, '#6f6f99'), (1, '#a9a6cf')], bands=6.0, turb=1.0)),
        P('frost', 'Frost', 'moon', 'gelid', 610 * KM, 8.0e20, 1.7e8, 20, 0, 'locked', 0.0,
          dict(type='ice', seed=3180, palette=[(0, '#7f8fa0'), (0.5, '#cbd6e0'), (1, '#f4f8fb')], craters=80)),
        P('cairn', 'Cairn', 'moon', 'gelid', 430 * KM, 3.0e20, 2.7e8, 140, 0, 'locked', 0.0,
          dict(type='rocky', seed=3190, palette=[(0, '#3a332c'), (0.5, '#6f6356'), (1, '#a99a86')], craters=140)),
        P('wisp', 'Wisp', 'moon', 'gelid', 260 * KM, 7.0e19, 3.9e8, 260, 0, 'locked', 0.0,
          dict(type='ice', seed=3200, palette=ICE_GREY, craters=90)),
      ],
      belts=[dict(slug='icy_belt', inner=0.30 * AU, outer=0.46 * AU, kind='icy', seed=3301)]),

 dict(id='fic.caelum', name='Caelum', dir='Caelum', pos=[-28000, 14000, 180], color='#eaf0ff', seed=4100,
      star=P('sun', 'Caelum', 'star', None, 1.42 * R_SUN, 1.35 * M_SUN, 0, 0, 0, 96.0, 8.0,
             dict(type='star', seed=4101, palette=[(0, '#5a78c8'), (0.4, '#a8c0ff'), (0.7, '#e4ecff'), (0.9, '#f8faff'), (1, '#ffffff')])),
      feature='Two very different giants: Pyrrhus, an inflated hot giant so close to its star that its night side glows red, '
              'and Caelestis, a cold giant with a majestic multi-gap ring system and a geyser moon (Geyser) with blue fissures. '
              'Verdance is a living world whose vegetation is crimson and violet under the white F-type light.',
      bodies=[
        P('pyrrhus', 'Pyrrhus', 'planet', 'sun', 1.55 * R_J, 0.55 * M_J, 0.058 * AU, 50, 0.3, 'locked', 0.0,
          dict(type='gas', seed=4110, palette=[(0, '#2a0d08'), (0.3, '#5e1d10'), (0.55, '#8d3a1e'), (0.8, '#c06a3a'), (1, '#3a1209')],
               bands=7.0, turb=2.2, storms=[(0.05, 0.0, 0.5, 0.18, '#e8a050', 0.6)]), res=4096, glow='#ff4a1a'),
        P('solace', 'Solace', 'planet', 'sun', 0.7 * R_E, 0.33 * M_E, 0.85 * AU, 135, 2.0, 1250.0, 1.0,
          dict(type='rocky', seed=4120, palette=[(0, '#4a4a4c'), (0.5, '#8d8c8a'), (1, '#d6d4cf')], craters=300, streaks=0.3)),
        P('verdance', 'Verdance', 'planet', 'sun', 1.2 * R_E, 1.6 * M_E, 1.8 * AU, 210, 0.7, 27.3, 31.0,
          dict(type='terra', seed=4130, sea_pal=[(0, '#06283a'), (0.6, '#0d4f63'), (1, '#1f7f86')],
               land_pal=[(0, '#c49a5e'), (0.35, '#9a4a3a'), (0.65, '#6e1f3a'), (1, '#3e1238')], sea=0.02), res=4096,
          atmos='#9fc6ff', clouds=dict(seed=4131, cover=0.05, cfreq=3.0, zonal=2.4)),
        P('lumen', 'Lumen', 'moon', 'verdance', 1310 * KM, 2.4e22, 3.6e8, 300, 0, 'locked', 0.0,
          dict(type='rocky', seed=4140, palette=[(0, '#5a5650'), (0.5, '#9e978c'), (1, '#e0dad0')], craters=220, maria=True)),
        P('sere', 'Sere', 'planet', 'sun', 0.62 * R_E, 0.21 * M_E, 2.75 * AU, 285, 1.6, 30.5, 19.0,
          dict(type='rocky', seed=4150, palette=[(0, '#4a2a16'), (0.4, '#9a5a2a'), (0.7, '#d49a5a'), (1, '#f0d0a0')], craters=50, caps=1.3),
          atmos='#e0b07a', clouds=dict(seed=4151, cover=0.35, ctint='#d9a46a', zonal=1.6)),
        P('caelestis', 'Caelestis', 'planet', 'sun', 0.94 * R_J, 0.85 * M_J, 7.4 * AU, 340, 1.1, 10.1, 21.0,
          dict(type='gas', seed=4160, palette=[(0, '#8a8a78'), (0.3, '#c9c0a0'), (0.55, '#efe6cc'), (0.8, '#d3e2df'), (1, '#9fb4b0')],
               bands=15.0, turb=1.2, polar='#7f9aa8'), res=4096,
          ring=dict(inner=1.22, outer=2.95, seed=4161, opacity=0.95, palette=[(0, '#6e665a'), (0.3, '#c8b89a'), (0.55, '#efe2c8'), (0.8, '#d8ccb4'), (1, '#8c8274')],
                    bands=[(0.08, 0.05, 0.35), (0.3, 0.14, 0.85), (0.55, 0.12, 1.0), (0.78, 0.1, 0.8), (0.95, 0.03, 0.6)],
                    gaps=[(0.43, 0.025), (0.66, 0.015), (0.87, 0.01)])),
        P('geyser', 'Geyser', 'moon', 'caelestis', 290 * KM, 1.4e20, 2.5e8, 15, 0, 'locked', 0.0,
          dict(type='ice', seed=4170, palette=[(0, '#c7d3dd'), (0.5, '#eef3f7'), (1, '#ffffff')], stripes='#5f9fd6', craters=30)),
        P('tethra', 'Tethra', 'moon', 'caelestis', 660 * KM, 7.0e20, 3.6e8, 95, 0, 'locked', 0.0,
          dict(type='ice', seed=4180, palette=[(0, '#7f7f80'), (0.5, '#c4c4c2'), (1, '#ececea')], craters=120)),
        P('veil', 'Veil', 'moon', 'caelestis', 880 * KM, 2.2e21, 5.5e8, 175, 0, 'locked', 0.0,
          dict(type='ice', seed=4190, palette=[(0, '#706a66'), (0.5, '#b8b2aa'), (1, '#e6e0d8')], craters=160)),
        P('haze', 'Haze', 'moon', 'caelestis', 2520 * KM, 1.3e23, 1.25e9, 250, 0, 'locked', 0.0,
          dict(type='rocky', seed=4200, palette=[(0, '#2a1e14'), (0.5, '#5a4028'), (1, '#8a6a42')], craters=10),
          atmos='#e0a050', clouds=dict(seed=4201, cover=-0.6, ctint='#d9963f', zonal=3.0)),
        P('nyx', 'Nyx', 'moon', 'caelestis', 720 * KM, 1.8e21, 2.7e9, 320, 0, 'locked', 0.0,
          dict(type='rocky', seed=4210, palette=[(0, '#6a665e'), (0.5, '#b2aca0'), (1, '#e6e0d4')], craters=160, twotone=0.75)),
        P('azurine', 'Azurine', 'planet', 'sun', 3.95 * R_E, 16 * M_E, 15.8 * AU, 60, 1.9, 16.8, 29.0,
          dict(type='gas', seed=4220, palette=[(0, '#0f3a4a'), (0.4, '#1f6a7f'), (0.7, '#3f9aa8'), (1, '#9fdde0')], bands=5.0, turb=0.7,
               storms=[(-0.3, 2.0, 0.18, 0.08, '#0a2430', 0.9)]),
          ring=dict(inner=1.65, outer=2.25, seed=4221, opacity=0.5, palette=[(0, '#3a3a3a'), (0.5, '#5a5652'), (1, '#3a3836')],
                    bands=[(0.3, 0.08, 0.6), (0.7, 0.06, 0.5)])),
        P('triss', 'Triss', 'moon', 'azurine', 1360 * KM, 2.2e22, 3.6e8, 200, 0, 'locked', 0.0,
          dict(type='rocky', seed=4230, palette=[(0, '#8a6f6a'), (0.5, '#d6c2bd'), (1, '#f5ece8')], craters=40, streaks=0.2)),
        P('kyra', 'Kyra', 'planet', 'sun', 0.82 * R_E, 0.5 * M_E, 27.0 * AU, 160, 4.0, 41.0, 64.0,
          dict(type='ice', seed=4240, palette=[(0, '#5a5a66'), (0.5, '#9a9aa8'), (1, '#dcdce6')], cfreq=5.0, craters=60)),
      ],
      belts=[dict(slug='main_belt', inner=3.4 * AU, outer=4.5 * AU, kind='asteroid', seed=4301)]),

 dict(id='fic.morava', name='Morava', dir='Morava', pos=[35000, -22000, -80], color='#ffe3c0', seed=5100,
      star=P('sun', 'Morava', 'star', None, 0.88 * R_SUN, 0.9 * M_SUN, 0, 0, 0, 912.0, 5.0,
             dict(type='star', seed=5101, palette=[(0, '#9a3a14'), (0.35, '#ee8a3a'), (0.65, '#ffcf8a'), (0.9, '#fff0d6'), (1, '#ffffff')])),
      feature='A calm, old system with a vast icy outer belt. In the belt spins Selk, an elongated dwarf planet '
              '(3.9 h day) with a narrow ring and two small moons. Serein is a near-snowball world with dark '
              'equatorial highlands; Brannock keeps a family of small moons.',
      bodies=[
        P('amber', 'Amber', 'planet', 'sun', 0.55 * R_E, 0.15 * M_E, 0.38 * AU, 80, 2.3, 1630.0, 0.5,
          dict(type='rocky', seed=5110, palette=[(0, '#3a2614'), (0.4, '#7a5a32'), (0.7, '#b58f5a'), (1, '#e0c89a')], craters=220)),
        P('serein', 'Serein', 'planet', 'sun', 1.06 * R_E, 1.15 * M_E, 0.8 * AU, 165, 0.5, 29.4, 12.0,
          dict(type='rocky', seed=5120, palette=[(0, '#2e3238'), (0.4, '#4f555a'), (0.7, '#8a8f92'), (1, '#c8ced2')], caps=0.42, capcol='#e6eef5', craters=10),
          res=4096, atmos='#bcd6ff', clouds=dict(seed=5121, cover=0.15, zonal=2.0)),
        P('penny', 'Penny', 'moon', 'serein', 410 * KM, 3.0e20, 1.6e8, 60, 0, 'locked', 0.0,
          dict(type='rocky', seed=5130, palette=ROCK_GREY, craters=120)),
        P('brannock', 'Brannock', 'planet', 'sun', 0.8 * R_J, 0.45 * M_J, 3.3 * AU, 250, 1.2, 11.8, 7.0,
          dict(type='gas', seed=5140, palette=[(0, '#4a4a3a'), (0.3, '#7d7a62'), (0.55, '#b5b096'), (0.8, '#d8d4c0'), (1, '#5f5c4a')],
               bands=10.0, turb=1.4, storms=[(0.2, 2.5, 0.1, 0.05, '#e8e4d0', 0.7)]), res=4096,
          ring=dict(inner=1.5, outer=1.9, seed=5141, opacity=0.35, palette=[(0, '#5a5448'), (1, '#8a8272')], bands=[(0.5, 0.25, 0.5)])),
        P('kite', 'Kite', 'moon', 'brannock', 150 * KM, 1.5e19, 1.4e8, 30, 0, 'locked', 0.0,
          dict(type='rocky', seed=5150, palette=ROCK_GREY, craters=70), mesh=dict(seed=5151, elong=(1.0, 0.62, 0.5), rough=0.35)),
        P('tarn', 'Tarn', 'moon', 'brannock', 430 * KM, 2.9e20, 2.4e8, 110, 0, 'locked', 0.0,
          dict(type='ice', seed=5160, palette=ICE_GREY, craters=110)),
        P('rook', 'Rook', 'moon', 'brannock', 240 * KM, 5.0e19, 3.5e8, 200, 0, 'locked', 0.0,
          dict(type='rocky', seed=5170, palette=[(0, '#221f1c'), (0.5, '#4a443e'), (1, '#867c70')], craters=90)),
        P('pell', 'Pell', 'moon', 'brannock', 610 * KM, 7.6e20, 5.6e8, 290, 0, 'locked', 0.0,
          dict(type='ice', seed=5180, palette=[(0, '#7a7068'), (0.5, '#c2b8ae'), (1, '#eee6dc')], craters=130)),
        P('lethe', 'Lethe', 'planet', 'sun', 3.3 * R_E, 11 * M_E, 9.4 * AU, 30, 1.6, 18.2, 33.0,
          dict(type='gas', seed=5190, palette=[(0, '#2f4a66'), (0.5, '#5f86a8'), (1, '#b7d0e2')], bands=4.0, turb=0.7)),
        P('mire', 'Mire', 'moon', 'lethe', 380 * KM, 2.4e20, 2.0e8, 150, 0, 'locked', 0.0,
          dict(type='ice', seed=5200, palette=[(0, '#3e4248'), (0.5, '#7a8088'), (1, '#b8bec4')], craters=100)),
        P('selk', 'Selk', 'planet', 'sun', 1050 * KM, 4.0e21, 43.0 * AU, 120, 7.0, 3.92, 28.0,
          dict(type='rocky', seed=5210, palette=[(0, '#b9c3cc'), (0.5, '#e9eef2'), (1, '#ffffff')], craters=40, spot='#8a3a2a'),
          shape=(1.0, 0.78, 0.52), ring=dict(inner=2.18, outer=2.32, seed=5211, opacity=0.6, palette=[(0, '#8a8f94'), (1, '#c8ccd0')], bands=[(0.5, 0.3, 0.9)])),
        P('ise', 'Ise', 'moon', 'selk', 170 * KM, 1.8e19, 4.95e7, 40, 0, 'locked', 0.0,
          dict(type='ice', seed=5220, palette=ICE_GREY, craters=50)),
        P('nam', 'Nam', 'moon', 'selk', 90 * KM, 1.8e18, 2.55e7, 230, 0, 'locked', 0.0,
          dict(type='ice', seed=5230, palette=ICE_GREY, craters=30), mesh=dict(seed=5231, elong=(1.0, 0.7, 0.62), rough=0.3)),
      ],
      belts=[dict(slug='inner_belt', inner=1.6 * AU, outer=2.4 * AU, kind='asteroid', seed=5301),
             dict(slug='outer_icy_belt', inner=28.0 * AU, outer=56.0 * AU, kind='icy', seed=5302)]),
]


# ============================================================== physics helpers
def period_hours(a, M): return 2 * math.pi * math.sqrt(a ** 3 / (G * M)) / 3600.0


def hill(a, m, M): return a * (m / (3 * M)) ** (1 / 3)


def resolve_rotation(b, bodies, star):
    if b['rot'] == 'locked':
        parent = star if b['parent'] == 'sun' else bodies[b['parent']]
        return round(period_hours(b['a'], parent['mass'] + b['mass']), 2)
    if b['rot'] == 'locked_pair':   # double planet: both bodies spin with the pair period
        mate = next(o for o in bodies.values() if o['parent'] == b['slug'])
        return round(period_hours(mate['a'], b['mass'] + mate['mass']), 2)
    return b['rot']


def place(b, bodies):
    """Absolute local position (metres). Planets on inclined circular orbits; moons in the parent's equatorial plane."""
    th, inc = math.radians(b['ang']), math.radians(b['inc'])
    if b['parent'] == 'sun':
        return Vector((b['a'] * math.cos(th), b['a'] * math.sin(th) * math.cos(inc), b['a'] * math.sin(th) * math.sin(inc)))
    p = bodies[b['parent']]; tilt = math.radians(p['tilt'])
    off = Vector((b['a'] * math.cos(th), b['a'] * math.sin(th) * math.cos(tilt), b['a'] * math.sin(th) * math.sin(tilt)))
    return p['pos'] + off


# ============================================================== image I/O
def save_image(path, arr, alpha=False, jpeg=False):
    """arr: (H, W, C) sRGB float, row 0 = bottom (Blender order). Returns (W, H)."""
    H, W = arr.shape[:2]
    px = np.ones((H, W, 4), np.float32); px[..., :arr.shape[2]] = arr
    if not alpha: px[..., 3] = 1.0
    img = bpy.data.images.new(path.stem, W, H, alpha=alpha)
    img.pixels.foreach_set(px.ravel())
    path.parent.mkdir(parents=True, exist_ok=True)
    img.file_format = 'JPEG' if jpeg else 'PNG'
    if jpeg: img.save(filepath=str(path), quality=92)
    else: img.filepath_raw = str(path); img.save()
    bpy.data.images.remove(img)
    return W, H


def make_textures(sysd, b, tdir, bodies):
    """Paint the body maps. Returns dict of relative texture paths written."""
    sp = b['paint']; res = b['res'] if not FAST else b['res'] // 2
    W, H = res, res // 2
    rng = np.random.default_rng(sp['seed'] + 7)
    out = {}
    kind = sp['type']
    night = None
    if kind == 'star': col = paint_star(sp, W, H)
    elif kind == 'rocky': col = paint_rocky(sp, W, H, rng)
    elif kind == 'gas': col = paint_gas(sp, W, H)
    elif kind == 'ocean': col = paint_ocean(sp, W, H)
    elif kind == 'ice': col = paint_ice(sp, W, H, rng)
    elif kind == 'lava': col, night = paint_lava(sp, W, H)
    elif kind == 'terra': col = paint_terra(sp, W, H)
    else: raise ValueError(kind)
    if b.get('glow'):   # hot giant: dim thermal glow following the band structure (night side)
        lum = col.mean(-1, keepdims=True)
        night = np.clip(hexrgb(b['glow']) * (0.25 + 0.9 * lum), 0, 1)
    name = b['slug']
    save_image(tdir / f'{name}.jpg', col, jpeg=True); out['texture'] = f'Textures/{name}.jpg'
    if night is not None:
        save_image(tdir / f'{name}_night.jpg', night, jpeg=True); out['nightTexture'] = f'Textures/{name}_night.jpg'
    if b.get('clouds'):
        cw = min(W, 4096 if not FAST else 2048)
        cl = paint_clouds(b['clouds'], cw, cw // 2)
        save_image(tdir / f'{name}_clouds.png', cl, alpha=True); out['cloudTexture'] = f'Textures/{name}_clouds.png'
    if b.get('ring'):
        save_image(tdir / f'{name}_ring.png', paint_ring(b['ring']), alpha=True); out['ringTexture'] = f'Textures/{name}_ring.png'
    return out


# ============================================================== meshes
def uv_sphere():
    bpy.ops.mesh.primitive_uv_sphere_add(segments=128, ring_count=64, radius=1)
    o = bpy.context.object
    for f in o.data.polygons: f.use_smooth = True
    return o


def spherical_uv(me):
    """Equirectangular UVs from vertex directions (+Z pole, u = lon), with seam fix."""
    bm = bmesh.new(); bm.from_mesh(me)
    uvl = bm.loops.layers.uv.verify()
    for f in bm.faces:
        us = []
        for l in f.loops:
            d = l.vert.co.normalized()
            u = (math.atan2(d.y, d.x) + math.pi) / (2 * math.pi); v = (math.asin(max(-1, min(1, d.z))) + math.pi / 2) / math.pi
            l[uvl].uv = (u, v); us.append(u)
        if max(us) - min(us) > 0.5:
            for l in f.loops:
                if l[uvl].uv.x < 0.5: l[uvl].uv.x += 1.0
    bm.to_mesh(me); bm.free()


def irregular_mesh(name, seed, elong=(1.0, 0.7, 0.6), rough=0.3, subdiv=6):
    """Unit irregular body: icosphere, ellipsoid stretch, fBm relief and craters; normalised to max radius 1."""
    bm = bmesh.new(); bmesh.ops.create_icosphere(bm, subdivisions=subdiv, radius=1.0)
    nz = Noise(seed); rng = np.random.default_rng(seed)
    co = np.array([v.co[:] for v in bm.verts], np.float32)
    d = co / np.linalg.norm(co, axis=1, keepdims=True)
    r = 1 + rough * fbm(nz, d[:, 0], d[:, 1], d[:, 2], 6, 1.4, 0.5) + 0.06 * fbm(nz, d[:, 0], d[:, 1], d[:, 2], 4, 6.0)
    for _ in range(14):   # a few big impact basins
        c = rng.normal(size=3); c /= np.linalg.norm(c); rad = rng.uniform(0.15, 0.55)
        ang = np.arccos(np.clip(d @ c, -1, 1)) / rad
        r += np.where(ang < 1, (ang * ang - 1) * 0.12 * rad, 0) + np.exp(-((ang - 1) / 0.2) ** 2) * 0.03 * rad
    p = d * r[:, None] * np.array(elong, np.float32)
    p /= np.linalg.norm(p, axis=1).max()
    for v, q in zip(bm.verts, p): v.co = q
    me = bpy.data.meshes.new(name); bm.to_mesh(me); bm.free()
    for f in me.polygons: f.use_smooth = True
    spherical_uv(me)
    return me


def tex_material(name, tex_path, rough=0.9, metal=0.0, emission=None, alpha=False):
    m = bpy.data.materials.new(name); m.use_nodes = True
    nt = m.node_tree; bs = nt.nodes['Principled BSDF']
    bs.inputs['Roughness'].default_value = rough; bs.inputs['Metallic'].default_value = metal
    if tex_path:
        tn = nt.nodes.new('ShaderNodeTexImage'); tn.image = bpy.data.images.load(str(tex_path), check_existing=True)
        nt.links.new(tn.outputs['Color'], bs.inputs['Base Color'])
        if alpha:
            nt.links.new(tn.outputs['Alpha'], bs.inputs['Alpha']); m.surface_render_method = 'BLENDED'
    if emission is not None:
        et = nt.nodes.new('ShaderNodeTexImage'); et.image = bpy.data.images.load(str(emission[0]), check_existing=True)
        nt.links.new(et.outputs['Color'], bs.inputs['Emission Color']); bs.inputs['Emission Strength'].default_value = emission[1]
    return m


def export_glb(obj, path):
    bpy.ops.object.select_all(action='DESELECT'); obj.select_set(True); bpy.context.view_layer.objects.active = obj
    path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.export_scene.gltf(filepath=str(path), export_format='GLB', use_selection=True, export_extras=True)


def belt_rock(name, seed, icy, tdir):
    me = irregular_mesh(name, seed, elong=(1.0, 0.75 + 0.2 * ((seed % 7) / 7), 0.6 + 0.2 * ((seed % 5) / 5)), rough=0.32, subdiv=5)
    pal = ([(0, '#5f6b78'), (0.5, '#b3c0cc'), (1, '#e9f0f6')] if icy else [(0, '#2a2622'), (0.5, '#5c544b'), (1, '#9a8f80')])
    rng = np.random.default_rng(seed)
    col = paint_rocky(dict(seed=seed, palette=pal, craters=60), 1024, 512, rng)
    tp = tdir / f'{name}.jpg'; save_image(tp, col, jpeg=True)
    o = bpy.data.objects.new(name, me); bpy.context.scene.collection.objects.link(o)
    me.materials.append(tex_material('M_' + name, tp, rough=0.95))
    return o, f'Textures/{name}.jpg'


# ============================================================== schematic animated preview scene
def disp_radius(R):
    return 0.35 + 3.0 * (R / R_J) ** 0.6


def ring_mesh(name, rin, rout, seg=160):
    me = bpy.data.meshes.new(name); verts, faces, uvs = [], [], []
    for k in range(seg):
        a = 2 * math.pi * k / seg
        verts += [(rin * math.cos(a), rin * math.sin(a), 0), (rout * math.cos(a), rout * math.sin(a), 0)]
    for k in range(seg):
        i0, i1, j0, j1 = 2 * k, 2 * k + 1, 2 * ((k + 1) % seg), 2 * ((k + 1) % seg) + 1
        faces.append((i0, j0, j1, i1))
    me.from_pydata(verts, [], faces); me.update()
    uv = me.uv_layers.new(name='UVMap')
    for p in me.polygons:
        for li in p.loop_indices:
            vi = me.loops[li].vertex_index; uv.data[li].uv = (0.0 if vi % 2 == 0 else 1.0, 0.5)
    return me


def spin(obj, hours, frames_per_rev_min=60, frames_per_rev_max=480):
    """Looping preview rotation about local Z (schematic speed; sign kept)."""
    fr = max(frames_per_rev_min, min(frames_per_rev_max, abs(hours) / 2.0))
    sgn = 1 if hours >= 0 else -1
    obj.rotation_mode = 'XYZ'
    obj.keyframe_insert('rotation_euler', index=2, frame=1)
    obj.rotation_euler[2] += sgn * 2 * math.pi * 240 / fr
    obj.keyframe_insert('rotation_euler', index=2, frame=241)
    ad = obj.animation_data
    try: fcs = ad.action.fcurves
    except AttributeError:
        from bpy_extras import anim_utils
        fcs = anim_utils.action_get_channelbag_for_slot(ad.action, ad.action_slot).fcurves
    for fc in fcs:
        for kp in fc.keyframe_points: kp.interpolation = 'LINEAR'
        fc.modifiers.new('CYCLES')


def label(text, loc, size, col):
    cu = bpy.data.curves.new('L_' + text, 'FONT'); cu.body = text; cu.size = size; cu.align_x = 'CENTER'
    o = bpy.data.objects.new('Label_' + text, cu); bpy.context.scene.collection.objects.link(o); o.location = loc
    o.rotation_euler = (math.radians(70), 0, 0)
    m = bpy.data.materials.new('M_Label_' + text); m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']; b.inputs['Emission Color'].default_value = (*col, 1); b.inputs['Emission Strength'].default_value = 1.5
    b.inputs['Base Color'].default_value = (0, 0, 0, 1); cu.materials.append(m)
    return o


def build_preview(sysd, sdir, bodies, star, data_bodies, rocks):
    """Schematic: star at left, planets left to right by distance (not to scale), moons around parents,
    belts as scattered rocks, rings and cloud shells, looping rotation animation. Labels name each body."""
    sc = bpy.context.scene; tdir = sdir / 'Textures'
    col = bpy.data.collections.new('SchematicPreview'); sc.collection.children.link(col)
    def link(o):
        for c in o.users_collection: c.objects.unlink(o)
        col.objects.link(o); return o
    tex = {d['id'].split('.')[-1]: d for d in data_bodies}
    # star
    Rs = 7.0
    so = uv_sphere(); so.name = 'Preview_' + star['slug']; so.scale = (Rs,) * 3; link(so); so.visible_shadow = False
    so.data.materials.append(tex_material('M_star', sdir / tex['sun']['texture'], emission=(sdir / tex['sun']['texture'], 6.0)))
    spin(so, star['rot_final'])
    label(sysd['name'], (0, -Rs - 2.5, -Rs - 1.5), 1.6, (1, 0.9, 0.7))
    planets = sorted([b for b in bodies.values() if b['parent'] == 'sun'], key=lambda b: b['a'])
    x = Rs + 3.0; prev_ext = 0.0
    belts = sorted(sysd['belts'], key=lambda b: b['inner'])
    placed_belts = set()
    for p in planets:
        for bt in belts:   # belt gap before the first planet beyond it
            if bt['slug'] not in placed_belts and bt['outer'] <= p['a'] + 1e-6:
                x += prev_ext + 4.0; scatter_belt(col, rocks, bt, x, 3.0); x += 4.0; prev_ext = 0.0; placed_belts.add(bt['slug'])
        r = disp_radius(p['R'] * max(p.get('shape', (1, 1, 1))))
        moons = [m for m in bodies.values() if m['parent'] == p['slug']]
        ext = r * (p['ring']['outer'] if p.get('ring') else 1.0) + (2.2 if moons else 0.6)
        x += prev_ext + ext + 1.2
        if p.get('mesh'):
            po = bpy.data.objects.new('Preview_' + p['slug'], bpy.data.meshes[p['slug']].copy()); col.objects.link(po)
        else:
            po = uv_sphere(); po.name = 'Preview_' + p['slug']; link(po)
        po.location = (x, 0, 0)
        po.data.materials.clear()
        sh = p.get('shape', (1, 1, 1)); po.scale = (r * sh[0], r * sh[1], r * sh[2])
        po.rotation_euler = (math.radians(p['tilt']), 0, 0)
        t = tex[p['slug']]
        emi = (sdir / t['nightTexture'], 1.2) if 'nightTexture' in t else None
        po.data.materials.append(tex_material('M_' + p['slug'], sdir / t['texture'], emission=emi))
        spin(po, p['rot_final'])
        if 'cloudTexture' in t:
            co = uv_sphere(); co.name = 'Preview_' + p['slug'] + '_clouds'; link(co); co.parent = po
            co.scale = (1.012,) * 3; co.data.materials.append(tex_material('M_' + p['slug'] + '_clouds', sdir / t['cloudTexture'], rough=1.0, alpha=True))
            spin(co, p['rot_final'] * 0.87)
        if 'ringTexture' in t:
            ro = bpy.data.objects.new('Preview_' + p['slug'] + '_ring', ring_mesh('Ring_' + p['slug'], p['ring']['inner'], p['ring']['outer']))
            col.objects.link(ro); ro.location = po.location; ro.scale = (r, r, r); ro.rotation_euler = (math.radians(p['tilt']), 0, 0)
            ro.data.materials.append(tex_material('M_' + p['slug'] + '_ring', sdir / t['ringTexture'], rough=1.0, alpha=True)); ro.visible_shadow = False
        label(p['name'], (x, -r * 1.2 - 1.0, -r * (p['ring']['outer'] * 0.45 if p.get('ring') else 1.0) - 1.6), 0.7, (0.85, 0.9, 1.0))
        for k, m in enumerate(sorted(moons, key=lambda m: m['a'])):
            mr = 0.18 + 0.9 * (m['R'] / R_E) ** 0.6
            ang = math.radians(20 + 40 * k); dist = r * (p['ring']['outer'] if p.get('ring') else 1.0) + 0.8 + 0.7 * k
            mo = uv_sphere() if not m.get('mesh') else bpy.data.objects.new('tmp', bpy.data.meshes[m['slug']].copy())
            if m.get('mesh'): sc.collection.objects.link(mo)
            mo.name = 'Preview_' + m['slug']; link(mo)
            mo.location = (x + dist * math.cos(ang), dist * math.sin(ang) * 0.3, dist * math.sin(ang))
            mo.scale = (mr,) * 3
            mt = tex[m['slug']]
            emi = (sdir / mt['nightTexture'], 1.2) if 'nightTexture' in mt else None
            mo.data.materials.clear(); mo.data.materials.append(tex_material('M_' + m['slug'], sdir / mt['texture'], emission=emi))
            spin(mo, m['rot_final'])
            if 'cloudTexture' in mt:
                co = uv_sphere(); co.name = 'Preview_' + m['slug'] + '_clouds'; link(co); co.parent = mo; co.scale = (1.02,) * 3
                co.data.materials.append(tex_material('M_' + m['slug'] + '_clouds', sdir / mt['cloudTexture'], rough=1.0, alpha=True))
        prev_ext = ext
    for bt in belts:
        if bt['slug'] not in placed_belts:
            x += prev_ext + 4.0; scatter_belt(col, rocks, bt, x, 4.0); x += 4.0; prev_ext = 0.0
    for a in [b for b in bodies.values() if b['parent'] == 'sun' and b['kind'] == 'asteroid']:
        pass   # irregular belt members are shown inside their belt scatter
    # lighting, world, camera
    sun = bpy.data.objects.new('Preview_Sun', bpy.data.lights.new('Preview_Sun', 'SUN')); col.objects.link(sun)
    sun.data.energy = 3.2; sun.data.color = tuple(hexrgb(sysd['color']))
    sun.rotation_euler = (-Vector((1.0, 0.75, -0.35))).to_track_quat('Z', 'Y').to_euler()   # shadows fall behind the row, not on neighbours
    sc.world = bpy.data.worlds.new('Preview_Space'); sc.world.use_nodes = True
    sc.world.node_tree.nodes['Background'].inputs[0].default_value = (0.01, 0.012, 0.018, 1)
    cam = bpy.data.objects.new('Preview_Camera', bpy.data.cameras.new('Preview_Camera')); col.objects.link(cam)
    span = x + prev_ext + 4; cx = span / 2 - 2
    cam.data.type = 'ORTHO'; cam.data.ortho_scale = span * 1.02; cam.data.clip_end = 2000
    cam.location = (cx, -span * 0.9, span * 0.32)
    cam.rotation_euler = (Vector((cx, 0, -1.5)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    sc.camera = cam
    sc.render.engine = 'BLENDER_EEVEE'; sc.render.resolution_x, sc.render.resolution_y = 2400, 900
    sc.view_settings.view_transform = 'AgX'
    sc.render.fps = 24; sc.frame_start, sc.frame_end = 1, 240
    lab = label('SCHEMATIC PREVIEW - not to scale; physical metres are in system.json', (cx, 0, -9.5), 0.8, (0.7, 0.75, 0.8))
    for c in lab.users_collection: c.objects.unlink(lab)
    col.objects.link(lab)
    return col


def scatter_belt(col, rocks, bt, x, half):
    rng = np.random.default_rng(bt['seed'])
    for k in range(120):
        src = rocks[k % len(rocks)]
        o = bpy.data.objects.new(f'Preview_{bt["slug"]}_{k:03d}', src.data); col.objects.link(o)
        o.location = (x + rng.uniform(-half, half), rng.uniform(-3, 3), rng.uniform(-2.5, 2.5))
        s = rng.uniform(0.08, 0.28); o.scale = (s, s, s); o.rotation_euler = tuple(rng.uniform(0, 6.28, 3))
    label(bt['slug'].replace('_', ' '), (x, -3.5, -4.0), 0.55, (0.75, 0.8, 0.85))


# ============================================================== system build
def build_system(sysd):
    sdir = OUTROOT / sysd['dir']; tdir = sdir / 'Textures'; mdir = sdir / 'Models'
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene; sc.unit_settings.system = 'METRIC'; sc.unit_settings.scale_length = 1.0
    star = dict(sysd['star'])
    bodies = {b['slug']: dict(b) for b in sysd['bodies']}
    for b in bodies.values():
        assert b['parent'] == 'sun' or b['parent'] in bodies, b['slug']
    # positions: planets first, then moons (parents placed before children)
    for b in sorted(bodies.values(), key=lambda b: b['parent'] != 'sun'):
        b['pos'] = place(b, bodies)
    star['rot_final'] = star['rot']; star['pos'] = Vector((0, 0, 0))
    for b in bodies.values(): b['rot_final'] = resolve_rotation(b, bodies, star)
    # source collection: unit sphere, irregular bodies, belt rocks (exported)
    src = bpy.data.collections.new('Sources'); sc.collection.children.link(src)
    def to_src(o):
        for c in o.users_collection: c.objects.unlink(o)
        src.objects.link(o); return o
    sphere = to_src(uv_sphere()); sphere.name = 'SM_UnitSphere'; sphere.data.name = 'SM_UnitSphere'
    sphere.data.materials.append(tex_material('M_UnitSphere', None, rough=0.8))
    export_glb(sphere, mdir / 'UnitSphere.glb')
    data_bodies = []
    def rec(b, kind, parent_id, tex):
        d = {'id': f"{sysd['id']}.{b['slug']}", 'name': b['name'], 'parentId': parent_id, 'kind': kind,
             'radiusMetres': round(float(b['R']), 1), 'positionMetres': [round(float(c), 1) for c in b['pos']],
             'texture': tex['texture'], 'rotationHours': float(b['rot_final']), 'tiltDegrees': float(b['tilt']),
             'shapeScale': list(b.get('shape', (1.0, 1.0, 1.0))), 'surfaceQuality': 'fictional-authored'}
        for k in ('cloudTexture', 'nightTexture'):
            if k in tex: d[k] = tex[k]
        if b.get('atmos'): d['atmosphereColor'] = b['atmos']
        if b.get('ring'):
            d['ringInnerMetres'] = round(float(b['R'] * b['ring']['inner']), 1); d['ringOuterMetres'] = round(float(b['R'] * b['ring']['outer']), 1)
            d['ringTexture'] = tex['ringTexture']
        if b.get('mesh'): d['mesh'] = f"Models/{b['slug']}.glb"
        return d
    texs = {}
    texs['sun'] = make_textures(sysd, star, tdir, bodies)
    data_bodies.append(rec(star, 'star', '', texs['sun']))
    for b in sorted(bodies.values(), key=lambda b: (b['parent'] != 'sun', b['a'])):
        texs[b['slug']] = make_textures(sysd, b, tdir, bodies)
        if b.get('mesh'):
            ms = b['mesh']; me = irregular_mesh(b['slug'], ms['seed'], ms['elong'], ms['rough'])
            o = bpy.data.objects.new('SM_' + b['slug'], me); sc.collection.objects.link(o); to_src(o)
            me.materials.append(tex_material('M_' + b['slug'], tdir / Path(texs[b['slug']]['texture']).name, rough=0.95))
            export_glb(o, mdir / f"{b['slug']}.glb")
        parent_id = f"{sysd['id']}.sun" if b['parent'] == 'sun' else f"{sysd['id']}.{b['parent']}"
        data_bodies.append(rec(b, b['kind'], parent_id, texs[b['slug']]))
    rocks = []
    kinds = {bt['kind'] for bt in sysd['belts']}
    for k, icy in ((k, k == 'icy') for k in sorted(kinds)):
        for v in 'ABC':
            o, _ = belt_rock(f"Belt{'Ice' if icy else 'Rock'}_{v}", sysd['seed'] + ord(v) + (50 if icy else 0), icy, tdir)
            to_src(o); export_glb(o, mdir / f"{o.name}.glb"); rocks.append(o)
    # data file
    data = {'version': 1, 'id': sysd['id'], 'name': sysd['name'], 'source': 'fictional',
            'galaxyPositionLightYears': sysd['pos'], 'primaryStarId': f"{sysd['id']}.sun", 'stellarColor': sysd['color'],
            'bodies': data_bodies,
            'belts': [{'id': f"{sysd['id']}.{bt['slug']}", 'parentId': f"{sysd['id']}.sun", 'innerMetres': round(bt['inner'], 1),
                       'outerMetres': round(bt['outer'], 1), 'count': 600, 'seed': bt['seed'], 'kind': bt['kind']} for bt in sysd['belts']]}
    (sdir / 'system.json').write_text(json.dumps(data, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    # preview scene + render
    build_preview(sysd, sdir, bodies, star, data_bodies, rocks)
    sc.frame_set(1); sc.render.filepath = str(sdir / 'preview.png'); bpy.ops.render.render(write_still=True)
    # make texture paths relative and save the editable source
    blend = sdir / f"{sysd['dir']}.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend), compress=True)
    for img in bpy.data.images:
        if img.filepath and not img.filepath.startswith('//'):
            try: img.filepath = '//' + Path(img.filepath).resolve().relative_to(sdir.resolve()).as_posix()
            except ValueError: pass
    bpy.ops.wm.save_mainfile(compress=True)
    return data, bodies, star, blend


# ============================================================== validation
def validate(sysd, data, bodies, star, blend):
    sdir = OUTROOT / sysd['dir']; rep = {'system': sysd['id'], 'bodies': len(data['bodies']), 'issues': []}
    ids = [b['id'] for b in data['bodies']]
    assert len(ids) == len(set(ids)), 'duplicate ids'
    byid = {b['id']: b for b in data['bodies']}
    planets = [b for b in data['bodies'] if b['kind'] == 'planet']; moons = [b for b in data['bodies'] if b['kind'] == 'moon']
    rep['planets'], rep['moons'], rep['belts'] = len(planets), len(moons), len(data['belts'])
    rep['ringed'] = [b['name'] for b in data['bodies'] if 'ringOuterMetres' in b]
    assert 4 <= len(planets) <= 7 and len(moons) >= 5 and len(data['belts']) >= 1, (len(planets), len(moons))
    sun = byid[data['primaryStarId']]
    for b in data['bodies']:
        assert b['radiusMetres'] > 0 and all(math.isfinite(c) for c in b['positionMetres']), b['id']
        assert b['parentId'] == '' if b['kind'] == 'star' else b['parentId'] in byid, b['id']
        assert (sdir / b['texture']).exists(), b['texture']
        if b['kind'] == 'star': continue
        par = byid[b['parentId']]
        dist = (Vector(b['positionMetres']) - Vector(par['positionMetres'])).length
        guard = par['radiusMetres'] * max(1.0, par.get('ringOuterMetres', 0) / par['radiusMetres'] if 'ringOuterMetres' in par else 1.0)
        assert dist > guard + b['radiusMetres'] * 1.05, ('overlap', b['id'], dist, guard)
        assert b['radiusMetres'] < 0.2 * sun['radiusMetres'], ('star smaller than body', b['id'])
    # Hill stability and Kepler periods (masses from the script; not part of the JSON contract)
    M = star['mass']; kepler = []
    for s, b in bodies.items():
        if b['parent'] == 'sun':
            kepler.append((b['name'], b['a'] / AU, period_hours(b['a'], M + b['mass']) / 24))
        else:
            p = bodies[b['parent']]; rh = hill(p['a'], p['mass'], M)
            frac = b['a'] / rh
            assert frac < 0.5, ('outside stable Hill fraction', b['slug'], frac)
            kepler.append((b['name'], b['a'] / 1e3, period_hours(b['a'], p['mass'] + b['mass']) / 24))
    pl = sorted([b for b in bodies.values() if b['parent'] == 'sun' and b['kind'] == 'planet'], key=lambda b: b['a'])
    for p1, p2 in zip(pl, pl[1:]):   # mutual Hill spacing
        rhm = ((p1['mass'] + p2['mass']) / (3 * M)) ** (1 / 3) * (p1['a'] + p2['a']) / 2
        assert (p2['a'] - p1['a']) / rhm > 8, ('tight spacing', p1['slug'], p2['slug'])
    rep['kepler'] = kepler
    # textures decode
    texfiles = sorted({p for b in data['bodies'] for k in ('texture', 'cloudTexture', 'nightTexture', 'ringTexture') if (p := b.get(k))})
    for p in texfiles:
        img = bpy.data.images.load(str(sdir / p)); w, h = img.size
        assert w >= 2048 or 'ring' in p or FAST, (p, w, h)
        if 'ring' not in p: assert w == 2 * h, (p, w, h)
        bpy.data.images.remove(img)
    rep['textures'] = len(texfiles)
    # GLB bounds (unit radius)
    for g in sorted((sdir / 'Models').glob('*.glb')):
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.import_scene.gltf(filepath=str(g))
        ms = [o for o in bpy.context.scene.objects if o.type == 'MESH']
        rmax = max((o.matrix_world @ v.co).length for o in ms for v in o.data.vertices)
        assert abs(rmax - 1.0) < 0.01, (g.name, rmax)
    rep['glb'] = [g.name for g in sorted((sdir / 'Models').glob('*.glb'))]
    # reopen the blend
    bpy.ops.wm.open_mainfile(filepath=str(blend))
    assert 'SM_UnitSphere' in bpy.data.objects and 'SchematicPreview' in bpy.data.collections
    missing = [i.name for i in bpy.data.images if i.source == 'FILE' and not Path(bpy.path.abspath(i.filepath)).exists()]
    assert not missing, missing
    rep['blendReopen'] = True
    return rep


def write_sources(sysd, data, bodies, star, rep):
    sdir = OUTROOT / sysd['dir']
    lines = [f"# {sysd['name']} ({sysd['id']}) — sources and notes", '',
             'Fictional system for flight testing (task 0015). Everything here is original and procedural:',
             'maps, rings and meshes are generated by `Tools/Prepare-FiveSystems.py` (value-noise fBm on the',
             'sphere, craters, bands, storms, ice cracks, lava, clouds). No third-party images, models or data.',
             'The values are plausible, not claims about real exoplanets.', '',
             f"**What makes it special.** {sysd['feature']}", '',
             f"Galaxy position {sysd['pos']} ly; star colour {sysd['color']}. Star: {star['R'] / R_SUN:.2f} R_sun, {star['mass'] / M_SUN:.2f} M_sun.", '',
             '| Body | Kind | Parent | Radius km | Orbit | Period | Day h | Tilt |', '| --- | --- | --- | --- | --- | --- | --- | --- |']
    for name, a, per in rep['kepler']:
        b = next(x for x in bodies.values() if x['name'] == name)
        orbit = f"{a:.3f} AU" if b['parent'] == 'sun' else f"{a:,.0f} km"
        lines.append(f"| {name} | {b['kind']} | {b['parent']} | {b['R'] / 1e3:,.0f} | {orbit} | {per:.2f} d | {b['rot_final']} | {b['tilt']} |")
    lines += ['', 'Belts: ' + '; '.join(f"{bt['slug']} {bt['inner'] / AU:.2f}–{bt['outer'] / AU:.2f} AU ({bt['kind']}, 600 bodies, seed {bt['seed']})" for bt in sysd['belts']), '',
              'Notes:', '- Positions are system-local metres (star at the origin); moons are absolute positions.',
              '- `locked` days equal the orbital period (tidally locked). Tessaly and Tessa Minor share the pair period.' if sysd['id'] == 'fic.asterion' else '- `locked` days equal the orbital period (tidally locked).',
              '- Textures are equirectangular, +Z pole, u = longitude from -180 deg; rings are radial RGBA strips (x: inner to outer).',
              '- Models/*.glb are normalised to a 1 m maximum radius (UnitSphere for smooth bodies, irregular bodies and belt rocks).',
              '- The .blend holds the export sources and a SCHEMATIC animated preview (not to scale; rotation loops at preview speed).',
              f"- Checks: {rep['planets']} planets, {rep['moons']} moons, {rep['belts']} belts, ringed: {', '.join(rep['ringed']) or 'none'};"
              f" all moons inside 0.5 Hill radius; adjacent planets more than 8 mutual Hill radii apart; {rep['textures']} textures decoded.", '']
    (sdir / 'SOURCES.md').write_text('\n'.join(lines), encoding='utf-8')


# ============================================================== main
summary = []
for sysd in SYSTEMS:
    if ONLY and sysd['id'] not in ONLY: continue
    data, bodies, star, blend = build_system(sysd)
    rep = validate(sysd, data, bodies, star, blend)
    write_sources(sysd, data, bodies, star, rep)
    rep.pop('kepler'); summary.append(rep)
    print('SYSTEM_DONE', json.dumps(rep))
print('FIVE_SYSTEMS_PASS ' + json.dumps({'systems': len(summary), 'fast': FAST}))
