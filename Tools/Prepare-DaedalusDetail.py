"""Reference-based Daedalus colour, lighting, engine glow and weapon-mount manifest (task 0008).

Run from the repository root:
  blender --background --factory-startup --python Tools/Prepare-DaedalusDetail.py
Optional: set DAEDALUS_LIVE=1 to save .local/daedalus-detail/live.blend after
each stage for a separate GUI viewer (preview only, never an asset).

Geometry is NOT redesigned. The hull is rebuilt from Original/daedalus.glb with
the same orientation and join as Tools/Prepare-DaedalusSource.py, giving the exact
Astrofossil geometry (222330 triangles). Baseline hatch squares and vent slats are
not original and are omitted; baseline window cubes become DaedalusLights.glb.
Hull vertices never move: the look comes from a procedural plating texture set
(box-projected UVMap), vertex colours, material assignment, and separate objects
for parts the stills show but the model lacks (masts, bow rods, hatches, hangar walls, lights).
Prepare-DaedalusSource.py is the baseline reconstruction and must not be run
over the accepted detailed source (it would overwrite these outputs).

Outputs (Art/Ships/Daedalus): Daedalus.blend (with LookDev_Series lighting),
Daedalus.glb (one 600 m hull mesh, textures embedded), Textures/*.png,
DaedalusEngineGlow.glb, DaedalusLights.glb, DaedalusAddOns.glb, DaedalusBeacons.glb,
ENGINE_MOUNTS.json, WEAPON_MOUNTS.json, asset-metadata.json.
Previews: .local/daedalus-detail (ignored).
"""
import bpy, bmesh, json, math, hashlib, os, sys, runpy
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix
from mathutils.bvhtree import BVHTree

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'Art/Ships/Daedalus'
TMP = ROOT / '.local/daedalus-detail'
(TMP / 'renders').mkdir(parents=True, exist_ok=True)
SOURCE = OUT / 'Original/daedalus.glb'
FX = runpy.run_path(str(ROOT / 'Tools/Prepare-DaedalusEngineEffects.py'))
LIVE = os.environ.get('DAEDALUS_LIVE') == '1'
BASELINE = {'finalTriangles': 223458, 'vertices': 504598}   # published baseline incl. its 14 hatch fittings (removed here)
SECTIONS = ['Predni trup', 'Pravy hangar', 'Levy hangar', 'Mustek a nadstavby', 'Motorova sekce', 'Centralni trup']


def live(caption):
    if not LIVE: return
    sc = bpy.context.scene
    cu = bpy.data.curves.new('LIVE_CAPTION', 'FONT'); cu.body = caption; cu.size = 20; cu.align_x = 'CENTER'
    ob = bpy.data.objects.new('LIVE_CAPTION', cu); sc.collection.objects.link(ob)
    ob.location = (0, 0, 110); ob.rotation_euler = (math.radians(70), 0, 0)
    m = bpy.data.materials.new('LIVE_CAPTION_MAT'); m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']; b.inputs['Emission Color'].default_value = (1, .85, .3, 1); b.inputs['Emission Strength'].default_value = 3
    cu.materials.append(m)
    tmp = TMP / 'live.tmp.blend'
    bpy.ops.wm.save_as_mainfile(filepath=str(tmp), copy=True, compress=False)
    os.replace(tmp, TMP / 'live.blend')
    bpy.data.objects.remove(ob); bpy.data.curves.remove(cu); bpy.data.materials.remove(m)


srgb = FX['srgb_lin']
TEX_DIR = OUT / 'Textures'
TEX_N, TILE_M = 2048, 32.0          # one tiling plating texture covers 32 x 32 m of hull


def periodic_noise(N, fmax, terms, rng):
    """Tileable smooth noise in [-1, 1] (integer frequencies only, so it wraps)."""
    u = (np.arange(N, dtype=np.float32) / N)
    acc = np.zeros((N, N), np.float32)
    for _ in range(terms):
        fx, fy = rng.integers(-fmax, fmax + 1, 2)
        if fx == 0 and fy == 0: continue
        ph = rng.uniform(0, 2 * math.pi)
        acc += np.cos(2 * math.pi * (fx * u[None, :] + fy * u[:, None]) + ph).astype(np.float32) / math.hypot(fx, fy)
    return acc / np.abs(acc).max()


def plating_maps(N=TEX_N, tile=TILE_M, seed=304):
    """Procedural BC-304 style hull plating, tileable. Returns (albedo sRGB, roughness, metallic, height).
    Dense rectangular plates of varied tone (patchwork seen in the stills), dark panel seams, inset
    hatches, vent rows and small raised boxes. Rows = V, columns = U; seams sit on the tile border."""
    rng = np.random.default_rng(seed); ppm = N / tile
    tone = np.ones((N, N), np.float32); hgt = np.zeros((N, N), np.float32)
    rgh = np.full((N, N), 0.55, np.float32); seam = np.zeros((N, N), bool)
    def snap(m): return max(16, int(round(m * ppm / 8)) * 8)
    # Plates in staggered rows (1.2-3.5 m high, 1.2-5.5 m long), like the dense plating in the stills;
    # some plates split once more into two strips. The tile border is always a seam (tileable).
    rects = []; y = 0
    while y < N:
        bh = snap(rng.choice([1.2, 1.5, 2.0, 2.5, 3.0, 3.5])); bh = N - y if y + bh > N - snap(1.2) else bh
        x = 0
        while x < N:
            bw = snap(rng.uniform(1.2, 5.5)); bw = N - x if x + bw > N - snap(1.2) else bw
            if rng.random() < 0.22 and bh >= snap(2.5):
                c = y + bh // 2 // 8 * 8; rects += [(x, y, x + bw, c), (x, c, x + bw, y + bh)]
            else:
                rects.append((x, y, x + bw, y + bh))
            x += bw
        y += bh
    t_plate = rng.normal(1.0, 0.05, len(rects)); r = rng.random(len(rects))
    t_plate *= np.where(r < 0.14, 0.85, np.where(r > 0.92, 1.06, 1.0))
    # Patchwork: groups of plates in a darker or lighter paint (the light/dark patches in the stills).
    cx = np.array([(a + c) / 2 for a, b, c, d in rects]); cy = np.array([(b + d) / 2 for a, b, c, d in rects])
    for _ in range(18):
        px, py = rng.uniform(0, N, 2); pw, ph = rng.uniform(4, 12, 2) * ppm
        dx = np.minimum(np.abs(cx - px), N - np.abs(cx - px)); dy = np.minimum(np.abs(cy - py), N - np.abs(cy - py))
        t_plate[(dx < pw / 2) & (dy < ph / 2)] *= rng.choice([0.90, 0.93, 1.06])
    t_plate = np.clip(t_plate, 0.76, 1.10)
    for (x0, y0, x1, y1), t in zip(rects, t_plate):
        tone[y0:y1, x0:x1] = t
        rgh[y0:y1, x0:x1] = 0.48 + 0.16 * rng.random()
        hgt[y0:y1, x0:x1] = rng.choice([0.0, 0.0, 0.0, 0.45, -0.3])
        w, h = x1 - x0, y1 - y0
        if rng.random() < 0.25 and w > snap(2.5):                      # fine panel line across the plate
            c = x0 + int(w * rng.uniform(0.3, 0.7)); tone[y0:y1, c:c + 1] *= 0.6; hgt[y0:y1, c:c + 1] = -0.6
        if rng.random() < 0.20 and min(w, h) > 1.6 * ppm:              # inset hatch / access panel
            ix, iy = int(w * rng.uniform(0.15, 0.3)), int(h * rng.uniform(0.15, 0.3))
            a0, b0, a1, b1 = x0 + ix, y0 + iy, x1 - ix, y1 - iy
            tone[b0:b1, a0:a1] *= rng.uniform(0.82, 1.08); hgt[b0:b1, a0:a1] -= 0.35
            for sl in (np.s_[b0:b0 + 2, a0:a1], np.s_[b1 - 2:b1, a0:a1], np.s_[b0:b1, a0:a0 + 2], np.s_[b0:b1, a1 - 2:a1]):
                tone[sl] *= 0.55; hgt[sl] = -0.9
        if rng.random() < 0.12 and w > 1.5 * ppm and h > 0.8 * ppm:    # vent row
            n = int(w / (0.55 * ppm)); vy = y0 + int(h * rng.uniform(0.2, 0.6))
            for k in range(1, n):
                vx = x0 + int(k * 0.55 * ppm); tone[vy:vy + int(0.5 * ppm), vx:vx + int(0.18 * ppm)] *= 0.45
                hgt[vy:vy + int(0.5 * ppm), vx:vx + int(0.18 * ppm)] = -0.8
        for _ in range(rng.integers(1, 4) if rng.random() < 0.30 else 0):   # small raised boxes (greebles)
            bw, bh = int(rng.uniform(0.3, 1.1) * ppm), int(rng.uniform(0.3, 1.1) * ppm)
            if w - bw < 10 or h - bh < 10: continue
            bx, by = rng.integers(x0 + 4, x1 - bw - 3), rng.integers(y0 + 4, y1 - bh - 3)
            tone[by:by + bh, bx:bx + bw] *= rng.uniform(0.78, 0.94); hgt[by:by + bh, bx:bx + bw] += 1.0
        seam[y0:y0 + 3, x0:x1] = True; seam[y0:y1, x0:x0 + 3] = True     # seams on the low edges (tile border wraps)
        tone[y0 + 3:y0 + 4, x0 + 3:x1] *= 1.07; tone[y0 + 3:y1, x0 + 3:x0 + 4] *= 1.07   # worn paint along the plate edge
    tone[seam] *= 0.38; hgt[seam] = -1.2; rgh[seam] = 0.8
    grime = 1.0 - 0.08 * periodic_noise(N, 6, 48, rng) - 0.035 * periodic_noise(N, 24, 48, rng)
    alb = np.clip(0.78 * tone * grime + rng.normal(0, 0.018, (N, N)).astype(np.float32), 0.10, 0.95)
    rgh = np.clip(rgh + 0.06 * (1 - grime) / 0.1 + rng.normal(0, 0.02, (N, N)).astype(np.float32), 0.25, 0.9)
    hgt = (hgt + np.roll(hgt, 1, 0) + np.roll(hgt, -1, 0) + np.roll(hgt, 1, 1) + np.roll(hgt, -1, 1)) / 5.0
    metal = np.clip(0.35 + 0.1 * periodic_noise(N, 12, 24, rng), 0.2, 0.5)
    return alb.astype(np.float32), rgh.astype(np.float32), metal.astype(np.float32), hgt.astype(np.float32), len(rects)


def save_map(name, rgb, colorspace):
    """rgb: (H, W, 3) values as stored (sRGB-encoded for 'sRGB', raw for 'Non-Color'); row 0 = bottom."""
    H, W = rgb.shape[:2]
    img = bpy.data.images.get(name) or bpy.data.images.new(name, W, H, alpha=False)
    img.colorspace_settings.name = colorspace
    px = np.ones((H, W, 4), np.float32); px[:, :, :3] = rgb
    img.pixels.foreach_set(px.ravel())
    TEX_DIR.mkdir(parents=True, exist_ok=True)
    img.filepath_raw = str(TEX_DIR / f'{name}.png'); img.file_format = 'PNG'; img.save()
    return img


def build_plating_images():
    alb, rgh, metal, hgt, nrect = plating_maps()
    gx = (np.roll(hgt, -1, 1) - np.roll(hgt, 1, 1)) * 0.5; gy = (np.roll(hgt, -1, 0) - np.roll(hgt, 1, 0)) * 0.5
    n = np.stack([-3.5 * gx, -3.5 * gy, np.ones_like(gx)], -1); n /= np.linalg.norm(n, axis=-1, keepdims=True)
    imgs = {'base': save_map('T_Daedalus_Plating_BaseColor', np.repeat(alb[:, :, None], 3, -1), 'sRGB'),
            'orm': save_map('T_Daedalus_Plating_ORM', np.stack([np.ones_like(rgh), rgh, metal], -1), 'Non-Color'),
            'normal': save_map('T_Daedalus_Plating_Normal', n * 0.5 + 0.5, 'Non-Color')}
    return imgs, {'plates': nrect, 'albedoMeanSRGB': round(float(alb.mean()), 3), 'roughnessMean': round(float(rgh.mean()), 3)}


HATCH_L, HATCH_W = 15.45, 6.2      # silo hatch overlay size in metres (length across the bow, width along it)


def hatch_maps(N=2048, seed=16):
    """BC-304 VLS hatch after the close-up still U16: a long worn taupe door in a dark frame with a raised
    grey lip, a dark slot with tick marks along one long side, and an end tab at each short end with two
    bolts. The striped variant has yellow/black hazard stripes on both tabs. Final albedo (sRGB),
    isotropic pixels; u (columns) = hatch length, v (rows) = width."""
    rng = np.random.default_rng(seed); M = int(round(N * HATCH_W / HATCH_L)); ppm = N / HATCH_L
    U, V = np.meshgrid((np.arange(N) + 0.5) / HATCH_L / ppm, (np.arange(M) + 0.5) / ppm)   # metres along length / width
    def R(u0, u1, v0, v1): return (U >= u0) & (U < u1) & (V >= v0) & (V < v1)
    def ring(u0, u1, v0, v1, t): return R(u0, u1, v0, v1) & ~R(u0 + t, u1 - t, v0 + t, v1 - t)
    U = U * HATCH_L; L, W = HATCH_L, HATCH_W; tab = 2.0
    col = np.ones((M, N, 3), np.float32) * np.array([104, 104, 102]) / 255; hgt = np.zeros((M, N), np.float32)
    def paint(mask, rgb, h=None):
        col[mask] = np.array(rgb) / 255
        if h is not None: hgt[mask] = h
    grime = 1.0 + 0.07 * periodic_noise(max(M, N), 10, 30, rng)[:M, :N] + rng.normal(0, 0.03, (M, N))
    # end tabs
    tabs = R(0.15, tab, 0.22 * W, 0.78 * W) | R(L - tab, L - 0.15, 0.22 * W, 0.78 * W)
    paint(tabs, (124, 124, 120), 0.3)
    # door frame: dark outline, raised grey lip, recessed worn taupe door
    paint(R(tab - 0.1, L - tab + 0.1, 0.03 * W, 0.97 * W), (48, 48, 48), -0.6)
    paint(R(tab, L - tab, 0.06 * W, 0.94 * W), (117, 116, 112), 0.5)
    door = R(tab + 0.35, L - tab - 0.35, 0.06 * W + 0.35, 0.94 * W - 0.35)
    paint(door, (96, 88, 80), -0.3)
    col[door] *= grime[door][:, None]
    for u0 in np.arange(tab + 0.6, L - tab - 0.6, 1.9):              # worn plate lines across the door
        paint(door & R(u0, u0 + 0.05, 0, W), (78, 72, 66), -0.45)
    paint(ring(0.38 * L, 0.62 * L, 0.38 * W, 0.62 * W, 0.12), (66, 60, 55), -0.55)   # inner access plate
    paint(R(0.25 * L, 0.75 * L, 0.80 * W, 0.845 * W), (40, 40, 40), -0.8)          # dark slot
    for u0 in np.arange(0.27 * L, 0.73 * L, 0.32):
        paint(R(u0, u0 + 0.12, 0.81 * W, 0.835 * W), (128, 128, 122))             # tick marks
    col *= 0.9                                                         # matches the darker hull
    plain = col.copy()
    # striped variant: 45 deg yellow/black stripes, 0.5 m period, on both tabs
    stripe = ((((U + V) / 0.25).astype(int)) % 2 == 0)
    col[tabs & stripe] = np.array([180, 142, 40]) / 255; col[tabs & ~stripe] = np.array([33, 30, 27]) / 255
    striped = col
    for c in (plain, striped):                                          # tab outlines and bolts
        c[ring(0.15, tab, 0.22 * W, 0.78 * W, 0.08) | ring(L - tab, L - 0.15, 0.22 * W, 0.78 * W, 0.08)] = np.array([58, 58, 56]) / 255
        for uc in (tab / 2 + 0.05, L - tab / 2 - 0.05):
            for vc in (0.36 * W, 0.64 * W):
                bolt = (U - uc) ** 2 + (V - vc) ** 2 < 0.12 ** 2; c[bolt] = np.array([45, 45, 44]) / 255; hgt[bolt] = -0.6
    gy, gx = np.gradient(hgt)
    n = np.stack([-3.0 * gx, -3.0 * gy, np.ones_like(gx)], -1); n /= np.linalg.norm(n, axis=-1, keepdims=True)
    return {'plain': save_map('T_Daedalus_SiloHatch_Plain_BaseColor', np.clip(plain, 0, 1), 'sRGB'),
            'striped': save_map('T_Daedalus_SiloHatch_Striped_BaseColor', np.clip(striped, 0, 1), 'sRGB'),
            'normal': save_map('T_Daedalus_SiloHatch_Normal', n * 0.5 + 0.5, 'Non-Color')}


def box_uv(me, tile=TILE_M):
    """World-scale box projection per face (dominant normal axis), 1 UV unit = `tile` metres.
    Continuous across coplanar faces, so the plating flows over large panels without seams."""
    nl = len(me.loops); npl = len(me.polygons)
    lv = np.empty(nl, np.int32); me.loops.foreach_get('vertex_index', lv)
    co = np.empty(len(me.vertices) * 3, np.float32); me.vertices.foreach_get('co', co); co = co.reshape(-1, 3)
    fn = np.empty(npl * 3, np.float32); me.polygons.foreach_get('normal', fn); fn = fn.reshape(-1, 3)
    lt = np.empty(npl, np.int32); me.polygons.foreach_get('loop_total', lt)
    n = fn[np.repeat(np.arange(npl), lt)]; p = co[lv]; ax = np.abs(n).argmax(1); s = np.sign(n[np.arange(nl), ax]); s[s == 0] = 1
    u = np.where(ax == 2, p[:, 0], np.where(ax == 1, -s * p[:, 0], s * p[:, 1]))
    v = np.where(ax == 2, s * p[:, 1], p[:, 2])
    uv = me.uv_layers.get('UVMap') or me.uv_layers.new(name='UVMap')
    uv.data.foreach_set('uv', (np.stack([u, v], 1) / tile).astype(np.float32).ravel())
    me.uv_layers.active = uv

# ---------------------------------------------------------------- 1. baseline geometry (unchanged shape)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(SOURCE))
scene = bpy.context.scene
scene.unit_settings.system = 'METRIC'; scene.unit_settings.scale_length = 1.0
parts = [o for o in scene.objects if o.type == 'MESH']
assert sum(len(o.data.polygons) for o in parts) == 222330
for o in parts:
    sec = SECTIONS.index(o.name) if o.name in SECTIONS else 5
    a = o.data.attributes.new('section', 'INT', 'FACE'); a.data.foreach_set('value', [sec] * len(o.data.polygons))
    o.matrix_world = Matrix.Rotation(-math.pi / 2, 4, 'Z') @ o.matrix_world
bpy.ops.object.select_all(action='DESELECT')
for o in parts: o.select_set(True)
bpy.context.view_layer.objects.active = parts[0]
bpy.ops.object.join()
ship = bpy.context.object; ship.name = 'SM_Daedalus'
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
ship.data.name = 'Daedalus_OriginalHull'; ship.data.materials.clear()
mesh = ship.data; mesh.update()
assert abs(ship.dimensions.x - 600) < .01


def pbr(name, hexcol, metal, rough, emission=None, strength=0.0, vertex=False, tex=None):
    m = bpy.data.materials.new(name); m.use_nodes = True
    rgb = srgb(hexcol); m.diffuse_color = (*rgb, 1)
    nt = m.node_tree; bs = nt.nodes.get('Principled BSDF')
    bs.inputs['Base Color'].default_value = (*rgb, 1)
    bs.inputs['Metallic'].default_value = metal; bs.inputs['Roughness'].default_value = rough
    if emission:
        bs.inputs['Emission Color'].default_value = (*srgb(emission), 1); bs.inputs['Emission Strength'].default_value = strength
    base = None
    if tex:   # tiling plating: base colour, ORM (G roughness, B metallic) and normal map on UVMap
        uvn = nt.nodes.new('ShaderNodeUVMap'); uvn.uv_map = 'UVMap'
        def img(key):
            n = nt.nodes.new('ShaderNodeTexImage'); n.image = tex[key]; n.name = 'TEX_' + key
            nt.links.new(uvn.outputs['UV'], n.inputs['Vector']); return n
        base = img('base'); nrm = img('normal')
        if 'orm' in tex:
            orm = img('orm'); sep = nt.nodes.new('ShaderNodeSeparateColor'); nt.links.new(orm.outputs['Color'], sep.inputs['Color'])
            nt.links.new(sep.outputs['Green'], bs.inputs['Roughness']); nt.links.new(sep.outputs['Blue'], bs.inputs['Metallic'])
        nm = nt.nodes.new('ShaderNodeNormalMap'); nm.uv_map = 'UVMap'
        nt.links.new(nrm.outputs['Color'], nm.inputs['Color']); nt.links.new(nm.outputs['Normal'], bs.inputs['Normal'])
    if vertex:
        vc = nt.nodes.new('ShaderNodeVertexColor'); vc.layer_name = 'COLOR_0'
        mix = nt.nodes.new('ShaderNodeMixRGB'); mix.blend_type = 'MULTIPLY'; mix.inputs[0].default_value = 1
        mix.inputs[2].default_value = (*rgb, 1)
        if base: nt.links.new(base.outputs['Color'], mix.inputs[2])
        nt.links.new(vc.outputs['Color'], mix.inputs[1]); nt.links.new(mix.outputs[0], bs.inputs['Base Color'])
    elif base:
        nt.links.new(base.outputs['Color'], bs.inputs['Base Color'])
    m['base_hex'] = hexcol; m['textured'] = bool(tex)
    return m


# Hull slots. Vertex COLOR_0 multiplies every slot; the hull albedo lives in COLOR_0 (base factor white).
# Armor and HangarInterior also carry the tiling plating maps (UVMap, 1 UV = 32 m):
# final albedo = plating base colour x COLOR_0 (the glTF rule for baseColorTexture and COLOR_0).
PLATING, PLATING_STATS = build_plating_images()
TEX_NAMES = {k: img.name for k, img in PLATING.items()}     # names survive scene resets
MAT = {
    'Armor': pbr('Daedalus_Armor', 'ffffff', .35, .55, vertex=True, tex=PLATING),
    'EngineMetal': pbr('Daedalus_EngineMetal', '2e2f30', .82, .42, vertex=True),
    'Hangar': pbr('Daedalus_HangarInterior', 'ffffff', .35, .6, 'e6eef5', 0.0, vertex=True, tex=PLATING),
    # bow window band: dark recess; its small cyan windows are separate light objects (stills U18)
    'LightWhite': pbr('Daedalus_LightWhite', '15191c', .3, .4, '3aa8c8', 0.0, vertex=True),
    # nozzle turbine (faces of the radial vanes and hub, facing aft): warm-lit metal with a yellow glow (U21)
    'EngineInner': pbr('Daedalus_EngineInner', 'a08a60', .3, .5, 'ffc070', 0.6, vertex=True),
}
SLOT = {k: i for i, k in enumerate(MAT)}
for m in MAT.values(): mesh.materials.append(m)
for poly in mesh.polygons: poly.material_index = SLOT['Armor']
box_uv(mesh)
GLASS = pbr('Daedalus_Glass', '0e1c22', .2, .25, '57c6dd', 0.4)   # small, faint blue windows (stills U18-U20)

# Hull = exact Astrofossil geometry. The baseline script added 14 black hatch squares and
# 36 black vent slats that are not in the original model: both omitted (user request).
# Its 30 window cubes are kept as LIGHTING in a separate object/export (not welded to the hull),
# placed with the same ray casts as the baseline.
bvh = BVHTree.FromObject(ship, bpy.context.evaluated_depsgraph_get())
verts = []; faces = []
def cv(p): return Vector((-p[2], -p[0], p[1]))
def ray(o, d):
    hit, normal, index, dist = bvh.ray_cast(cv(o), cv(d), 2000)
    if hit is None: return None
    return hit, Vector((-normal.y, normal.z, -normal.x))
def cube(center, dims):
    d = Vector((dims[2], dims[0], dims[1])) * .5; start = len(verts)
    verts.extend([tuple(center + Vector((sx * d.x, sy * d.y, sz * d.z))) for sx, sy, sz in [(-1, -1, -1), (-1, -1, 1), (-1, 1, -1), (-1, 1, 1), (1, -1, -1), (1, -1, 1), (1, 1, -1), (1, 1, 1)]])
    for f in [(0, 4, 6, 2), (1, 3, 7, 5), (0, 1, 5, 4), (2, 6, 7, 3), (0, 2, 3, 1), (4, 5, 7, 6)]: faces.append(tuple(start + i for i in f))
stats = {'windowLights': 0, 'removedBaselineHatchSquares': 14, 'removedBaselineVentSlats': 36}
for side in [-1, 1]:
    for row in range(24):
        for y in [.046, .052]:
            h = ray((side * .16 * 600, y * 600, (-.015 + row * .006) * 600), (-side, 0, 0))
            if h is None or abs(h[0].y) > .10 * 600 or abs(h[1].x) < .4: continue
            cube(h[0] + cv((side * .0004 * 600, 0, 0)), (.00065 * 600, .00135 * 600, .0028 * 600)); stats['windowLights'] += 1
    for i in range(9):
        h = ray(((side * .245 + (i - 4) * .004) * 600, -.006 * 600, -600), (0, 0, 1))
        if h:
            cube(h[0] + cv((0, 0, -.0004 * 600)), (.0018 * 600, .001 * 600, .0007 * 600)); stats['windowLights'] += 1
wmesh = bpy.data.meshes.new('Daedalus_WindowLights'); wmesh.from_pydata(verts, [], faces); wmesh.update()
wmesh.materials.append(GLASS)
lights_col = bpy.data.collections.new('HullLights'); scene.collection.children.link(lights_col)
window_obj = bpy.data.objects.new('Daedalus_WindowLights', wmesh); lights_col.objects.link(window_obj)
window_obj['note'] = 'window light effect; positions from the baseline web-livery ray casts; separate from hull'
assert len(mesh.polygons) == 222330, len(mesh.polygons)
FINAL = {'finalTriangles': len(mesh.polygons), 'vertices': len(mesh.vertices)}
live('Krok 1: presna puvodni geometrie Astrofossil (bez pridanych ctvercu a lamel)')

# ---------------------------------------------------------------- 2. measure features (no geometry change)
bvh = BVHTree.FromObject(ship, bpy.context.evaluated_depsgraph_get())
engines = FX['name_engines'](FX['measure_engines'](bvh))
# Front openings: hangar bays (deep pits in the pods) and the bow light slot.
A, B, Hf = FX['depth_map'](bvh, 0, -1, (-192, 192), (-48, 48), 0.5, 500)
front_pits = FX['blobs'](Hf, A, B, 0.5, 55, 'pit', 6.0, 10, 120)
hangars = []
for h in front_pits:
    ii = (A >= h['aMin']) & (A <= h['aMax']); jj = (B >= h['bMin']) & (B <= h['bMax'])
    sub = Hf[np.ix_(ii, jj)]
    if abs(h['a']) < 100 or np.isnan(sub).mean() > 0.05: continue   # pods only; rays must hit a back wall
    h['backX'] = float(np.nanmin(sub))
    rim = Hf[np.ix_((A >= h['aMin'] - 4) & (A <= h['aMax'] + 4), (B >= h['bMin'] - 4) & (B <= h['bMax'] + 4))]
    h['rimX'] = float(np.nanmax(rim)); hangars.append(h)
bow_slot = FX['blobs'](Hf, A, B, 0.5, 8, 'pit', 0.5, 2.0, 40)
bow_slot = [p for p in bow_slot if abs(p['a']) < 45 and p['extentA'] >= 20 and p['extentA'] > 3 * p['extentB'] and p['fill'] > 0.8 and p['surface'] > 270]
# Top/bottom turret domes and bow silo recesses.
At, Bt, Ht = FX['depth_map'](bvh, 2, -1, (-302, 302), (-192, 192), 0.5, 300)
Ab, Bb, Hb = FX['depth_map'](bvh, 2, +1, (-302, 302), (-192, 192), 0.5, -300)
def roundish(b): return b['fill'] > 0.55 and max(b['extentA'], b['extentB']) / min(b['extentA'], b['extentB']) < 1.7
def turret_family(b): return roundish(b) and 3.0 <= b['diam'] <= 4.7 and 3.3 <= b['relief'] <= 4.0
top_turrets = [b for b in FX['blobs'](Ht, At, Bt, 0.5, 10, 'bump', 0.9, 2.0, 9.0) if turret_family(b)]
bottom_turrets = [b for b in FX['blobs'](Hb, Ab, Bb, 0.5, 10, 'bump', 0.9, 2.0, 9.0) if turret_family(b)]
silos = [p for p in FX['blobs'](Ht, At, Bt, 0.5, 14, 'pit', 0.4, 4.0, 18.0)
         if 100 < p['a'] < 240 and abs(p['b']) < 20 and p['relief'] < 1.0 and p['fill'] > 0.8]
if os.environ.get('DAEDALUS_MEASURE_ONLY') == '1':
    print('MEASURE_ONLY ' + json.dumps({'engines': engines, 'hangars': hangars, 'bowSlot': bow_slot, 'top': len(top_turrets), 'bottom': len(bottom_turrets), 'silos': len(silos)}, default=float))
    sys.exit(0)

# ---------------------------------------------------------------- 3. material assignment (existing faces only)
npoly = len(mesh.polygons)
pc = np.empty(npoly * 3, np.float32); mesh.polygons.foreach_get('center', pc); pc = pc.reshape(-1, 3)
pn = np.empty(npoly * 3, np.float32); mesh.polygons.foreach_get('normal', pn); pn = pn.reshape(-1, 3)
pa = np.empty(npoly, np.float32); mesh.polygons.foreach_get('area', pa)
sec = np.empty(npoly, np.int32); mesh.attributes['section'].data.foreach_get('value', sec)
matidx = np.empty(npoly, np.int32); mesh.polygons.foreach_get('material_index', matidx)
hull = matidx == SLOT['Armor']
eng_mask = np.zeros(npoly, bool)
for e in engines:
    r = np.hypot(pc[:, 1] - e['y'], pc[:, 2] - e['z'])
    eng_mask |= (r < e['lipOuterRadius'] + 0.6) & (pc[:, 0] > e['lipX'] - 1.0) & (pc[:, 0] < e['coreX'] + 2.0)
hang_mask = np.zeros(npoly, bool)
DIRS = [Vector((dx, dy, dz)).normalized() for dx in (-1, 0, 1) for dy in (-1, 0, 1) for dz in (-1, 0, 1) if (dx, dy, dz) != (0, 0, 0)]
for h in hangars:   # interior = faces actually hit by rays cast from inside the bay volume
    cy, cz = (h['aMin'] + h['aMax']) / 2, (h['bMin'] + h['bMax']) / 2
    for x in np.arange(h['rimX'] - 3, h['backX'] + 2, -3.0):
        for oy in (-0.3, 0.0, 0.3):
            o = Vector((x, cy + oy * h['extentA'], cz))
            for d in DIRS:
                hit, nrm, idx, dist = bvh.ray_cast(o, d, 120)
                if idx is not None and hit.x < h['rimX'] - 0.5 and nrm.dot(d) < -0.2: hang_mask[idx] = True   # front face toward the bay only (single-layer skin stays dark)
# Faces that are also visible from outside (single-layer skin) must not glow: drop any candidate
# that a ray from above, below, either side or behind reaches first. The front stays open (bay mouth).
for idx in np.nonzero(hang_mask)[0]:
    c = Vector(pc[idx])
    for d in (Vector((0, 0, -1)), Vector((0, 0, 1)), Vector((0, 1, 0)), Vector((0, -1, 0)), Vector((1, 0, 0))):
        if bvh.ray_cast(c - d * 800, d, 2000)[2] == idx: hang_mask[idx] = False; break
# Bow VLS: each silo door sits on a raised hatch plate (about 0.6 m above the deck). Walk out from the
# door centre until the deck level to get the plate; the series-style hatch overlay (6b) covers it.
def silo_plate(s):
    x0, y0, top = s['a'], s['b'], s['surface']
    def zt(x, y):
        hit = bvh.ray_cast(Vector((x, y, top + 50)), Vector((0, 0, -1)), 200)[0]
        return hit.z if hit is not None else -1e9
    ext = []
    for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
        t = 0.0
        while t < 15 and zt(x0 + dx * (t + 0.25), y0 + dy * (t + 0.25)) > top - 0.3: t += 0.25
        ext.append(t)
    return x0 - ext[0], x0 + ext[1], y0 - ext[2], y0 + ext[3]
for s in silos: s['plate'] = silo_plate(s)
slot_mask = np.zeros(npoly, bool)
for s in bow_slot:
    slot_mask |= ((pc[:, 1] > s['aMin']) & (pc[:, 1] < s['aMax']) & (pc[:, 2] > s['bMin']) & (pc[:, 2] < s['bMax'])
                  & (pn[:, 0] > 0.7) & (pc[:, 0] < s['surface'] + 0.2) & (pc[:, 0] > s['surface'] - 3.0))
inner_mask = np.zeros(npoly, bool)
for e in engines:   # inside the aperture, behind the lip, facing along the axis: the turbine vanes and hub
    r = np.hypot(pc[:, 1] - e['y'], pc[:, 2] - e['z'])     # (the inner cylinder walls stay dark EngineMetal)
    inner_mask |= (r < e['apertureRadius'] - 0.05) & (pc[:, 0] > e['lipX'] + 0.3) & (pc[:, 0] < e['coreX'] + 0.5) & (np.abs(pn[:, 0]) > 0.5)
matidx[hull & eng_mask] = SLOT['EngineMetal']
matidx[hull & inner_mask] = SLOT['EngineInner']
matidx[hull & hang_mask] = SLOT['Hangar']
matidx[hull & slot_mask] = SLOT['LightWhite']
mesh.polygons.foreach_set('material_index', matidx)
assign_stats = {'engineMetalFaces': int((hull & eng_mask & ~inner_mask).sum()), 'engineInnerFaces': int((hull & inner_mask).sum()), 'hangarInteriorFaces': int((hull & hang_mask).sum()),
                'bowLightFaces': int((hull & slot_mask).sum())}

# ---------------------------------------------------------------- 4. reference palette into COLOR_0
# Sampled from the on-screen still U13 (sunlit dorsal three-quarter view above Earth): a neutral,
# very slightly warm grey (R >= G >= B by a few levels), lit decks around sRGB 145-170, sides around
# 105-110, strong plate-to-plate contrast. COLOR_0 carries the large-scale tone (orientation, plate
# patches, AO); the plating texture (mean about 0.8 sRGB) multiplies it with the per-plate detail.
TOP, SIDE, UNDER = srgb('8c8c8a'), srgb('7f7f7d'), srgb('707070')   # user: overall a bit darker
HANGAR_GREY = srgb('6a6c6e')
def hashf(ix, iy, seed):
    h = np.sin(ix * 127.1 + iy * 311.7 + seed * 74.7) * 43758.5453
    return h - np.floor(h)
up = pn[:, 2]
base = np.where(up[:, None] > 0.55, TOP, np.where(up[:, None] < -0.55, UNDER, SIDE)).astype(np.float32)
a = np.abs(pn)
# Panel key = minimum corner of each triangle in its dominant-plane projection, so both
# triangles of a rectangular panel fall into the same cell (no diagonal colour splits).
fv = np.empty(npoly * 3, np.int32); mesh.polygons.foreach_get('vertices', fv)
vco = np.empty(len(mesh.vertices) * 3, np.float32); mesh.vertices.foreach_get('co', vco); tri = vco.reshape(-1, 3)[fv.reshape(-1, 3)]
zdom = a[:, 2] >= np.maximum(a[:, 0], a[:, 1]); ydom = (~zdom) & (a[:, 1] >= a[:, 0])
u = np.where(zdom | ydom, tri[:, :, 0].min(1), tri[:, :, 1].min(1)) + 0.01
v = np.where(zdom, tri[:, :, 1].min(1), tri[:, :, 2].min(1)) + 0.01
fine = hashf(np.floor(u / 6.5), np.floor(v / 4.5), 1.0)
patch = hashf(np.floor(u / 23.0), np.floor(v / 17.0), 2.0)
k = 0.95 + 0.10 * fine
k = np.where(patch < 0.20, k * 0.70, np.where(patch > 0.86, k * 1.15, k))
k = np.where((pa < 5.0) & (up > 0.3) & (sec != 6), k * 0.80, k)          # darker deck greebles
k = np.where(sec == 4, k * 0.88, k)                                   # engine section darker metal
k = np.where((sec == 3) & (up > 0.55), k * 1.04, k)                   # sunlit bridge deck
albedo = np.clip(base * k[:, None], 0, 1)
albedo[matidx == SLOT['Hangar']] = HANGAR_GREY                          # dark plated bay interior (texture adds the plates)
non_hull = (matidx != SLOT['Armor']) & (matidx != SLOT['Hangar'])   # EngineMetal, EngineInner, LightWhite
albedo[non_hull] = 1.0                                                   # untextured slots carry their own base colour
# Ambient occlusion baked per corner gives recess/panel-edge contrast.
ao_attr = mesh.color_attributes.new(name='AO_BAKE', type='FLOAT_COLOR', domain='CORNER')
mesh.color_attributes.active_color = ao_attr
scene.render.engine = 'CYCLES'; scene.cycles.device = 'CPU'; scene.cycles.samples = 48
scene.world = bpy.data.worlds.new('BakeWorld'); scene.world.light_settings.distance = 7.0
bpy.ops.object.select_all(action='DESELECT'); ship.select_set(True); bpy.context.view_layer.objects.active = ship
bpy.ops.object.bake(type='AO', target='VERTEX_COLORS')
nl = len(mesh.loops)
ao = np.empty(nl * 4, np.float32); ao_attr.data.foreach_get('color', ao); ao = ao.reshape(-1, 4)[:, 0]
loop_face = np.repeat(np.arange(npoly), [p.loop_total for p in mesh.polygons])
# Large flat triangles keep little AO (their corners interpolate badly); greebles keep full AO.
strength = np.clip(1.0 - (pa[loop_face] - 25.0) / 250.0, 0.2, 1.0)
occl = np.clip(1.0 - strength * (1.0 - (0.30 + 0.70 * ao ** 0.9)), 0.25, 1.0)
rgba = np.ones((nl, 4), np.float32); rgba[:, :3] = albedo[loop_face] * occl[:, None]
mesh.color_attributes.remove(ao_attr)
col = mesh.color_attributes.new(name='COLOR_0', type='FLOAT_COLOR', domain='CORNER')
col.data.foreach_set('color', rgba.ravel()); mesh.color_attributes.active_color = col
mesh.color_attributes.render_color_index = mesh.color_attributes.active_color_index
mesh.attributes.remove(mesh.attributes['section'])
live('Krok 2: barvy podle fotek + stineni prohlubni')

# ---------------------------------------------------------------- 5. engine glow (separate effect objects)
glow_col = bpy.data.collections.new('EngineGlow'); scene.collection.children.link(glow_col)
glow_objs = FX['build_glow'](engines, glow_col)
live('Krok 3: zare motoru (samostatny efekt)')

# ---------------------------------------------------------------- 6. weapon / launch mounts
def free_arc(origin, upz):
    """Azimuth ranges (deg, 0=+X, 90=+Y) where a ray at 5 deg above the local horizon leaves the hull."""
    free = []
    for az in range(0, 360, 5):
        el = math.radians(5) * (1 if upz > 0 else -1)
        d = Vector((math.cos(math.radians(az)) * math.cos(el), math.sin(math.radians(az)) * math.cos(el), math.sin(el)))
        free.append(bvh.ray_cast(origin, d, 3000)[0] is None)
    ranges = []; az = 0
    while az < 72:
        if free[az]:
            s = az
            while az < 72 and free[az]: az += 1
            ranges.append([s * 5, az * 5 - 5])
        az += 1
    if len(ranges) > 1 and ranges[0][0] == 0 and ranges[-1][1] == 355:
        ranges[0][0] = ranges[-1][0] - 360; ranges.pop()
    return ranges, round(sum(free) * 5, 0)
REF_DECK = 'https://stargate.fandom.com/wiki/BC-304 (infobox still Daedalus.jpg; archived 2024-01-12)'
mounts = []
def add_turret(b, zsign, prefix):
    x, y = b['a'], b['b']; top = b['surface'] * zsign
    basez = top - zsign * b['relief']
    origin = Vector((x, y, top + zsign * 0.5))
    arc, total = free_arc(origin, zsign)
    mounts.append({'kind': 'turret', 'group': prefix, 'centerMetres': [round(x, 2), round(y, 2), round(basez, 2)],
                   'domeTopMetres': round(top, 2), 'domeDiameterMetres': round(b['diam'], 2),
                   'forwardAxis': [1, 0, 0], 'upAxis': [0, 0, zsign], 'traverseFreeAzimuthDeg': arc,
                   'freeAzimuthTotalDeg': total, 'arcNote': 'rays at 5 deg elevation; superstructure occlusion only, no game rules',
                   'intendedMovingMesh': 'none yet: the original dome turret with its barrels is part of the hull mesh (extract it to rotate)',
                   'weaponHint': 'railgun turret (references: twin-barrel dome turrets along deck and bow edges)',
                   'referenceURL': REF_DECK, 'confidence': 'medium: modelled dome present; exact canon placement/count unconfirmed'})
# The bow's lower side ledges (z about -18.6, below the main deck) carry two domes per side. The SGA
# "Tech Journal" schematic (REFERENCES.md U12) labels the Asgard beam turret exactly there: the outer
# dome of each side becomes the bow Asgard beam turret, the inner one stays a railgun proposal.
ledge = [b for b in top_turrets if b['a'] > 150 and b['surface'] < 0]
asg_bow = [max((b for b in ledge if b['b'] * side > 0), key=lambda b: abs(b['b'])) for side in (+1, -1)]
assert len(ledge) == 4 and all(abs(b['b']) > 40 for b in asg_bow), [(b['a'], b['b'], b['surface']) for b in ledge]
for b in sorted(top_turrets, key=lambda b: (b['a'], b['b'])):
    if not any(b is c for c in asg_bow): add_turret(b, +1, 'dorsal_railguns')
for b in sorted(bottom_turrets, key=lambda b: (b['a'], b['b'])): add_turret(b, -1, 'ventral_railguns')
for i, s in enumerate(sorted(silos, key=lambda s: (-s['a'], s['b']))):
    mounts.append({'kind': 'missile_silo', 'group': 'bow_vls', 'centerMetres': [round(s['a'], 2), round(s['b'], 2), round(s['surface'], 2)],
                   'hatchSizeMetres': [round(s['extentA'], 2), round(s['extentB'], 2)],
                   'hatchPlateBoundsXY': [round(c, 2) for c in s['plate']], 'hatchMarking': 'faint ochre tint on the door frame ridges (COLOR_0)',
                   'forwardAxis': [0, 0, 1], 'upAxis': [1, 0, 0],
                   'launchDirection': [0, 0, 1], 'intendedMovingMesh': 'hatch door optional; missile spawns at centre',
                   'referenceURL': 'https://stargate.fandom.com/wiki/BC-304#Missiles (16 VLS missile tubes; archived 2024-01-12)',
                   'confidence': 'high for zone and count (16 modelled recesses match 16 VLS); door detail interpretive'})
for h in sorted(hangars, key=lambda h: -h['a']):
    mounts.append({'kind': 'hangar_launch', 'group': 'f302_bays', 'centerMetres': [round(h['rimX'], 2), round(h['a'], 2), round((h['bMin'] + h['bMax']) / 2, 2)],
                   'openingSizeMetres': [round(h['extentA'], 2), round(h['extentB'], 2)], 'bayDepthMetres': round(h['rimX'] - h['backX'], 2),
                   'forwardAxis': [1, 0, 0], 'upAxis': [0, 0, 1], 'intendedMovingMesh': 'fighter spawn point (no moving hull part)',
                   'referenceURL': 'https://stargate.fandom.com/wiki/BC-304#Battle_complement (F-302 launch from side bays)',
                   'confidence': 'high: opening measured from mesh'})
# Asgard plasma beam weapons (4 per wiki): a mirrored pair at the bow and a mirrored pair under the hull
# between the hangar pods (user, from on-screen stills U9/U10; bow position labelled on schematic U12).
ASG_URL = 'https://stargate.fandom.com/wiki/BC-304#Asgard_beam_weapons (4 Asgard plasma beam weapons; archived 2024-01-12)'
for b in asg_bow:
    x, y = b['a'], b['b']; top = b['surface']; basez = top - b['relief']
    arc, total = free_arc(Vector((x, y, top + 0.5)), +1)
    mounts.append({'kind': 'beam_turret', 'group': 'asgard_beams', 'zone': 'bow, outer dome on the lower side ledge',
                   'centerMetres': [round(x, 2), round(y, 2), round(basez, 2)], 'domeTopMetres': round(top, 2),
                   'domeDiameterMetres': round(b['diam'], 2), 'forwardAxis': [1, 0, 0], 'upAxis': [0, 0, 1],
                   'traverseFreeAzimuthDeg': arc, 'freeAzimuthTotalDeg': total,
                   'arcNote': 'rays at 5 deg elevation; hull occlusion only, no game rules',
                   'intendedMovingMesh': 'SM_Daedalus_AsgardBeamTurret (to be authored; not part of hull)', 'referenceURL': ASG_URL,
                   'evidence': 'SGA Tech Journal schematic (U12) labels the Asgard beam turret on the bow lower side; user stills U9/U10 show beams leaving the bow',
                   'confidence': 'medium: labelled zone matches a modelled dome; which of the two ledge domes is the emitter is a choice'})
# The ventral pair has no dedicated emitter on the model: each mount snaps (ray cast) to the chamfered
# front of the block between the hangar pods, between its two capsules. That face points forward and down.
for side in (+1, -1):
    hit = bvh.ray_cast(Vector((-20.0, side * 79.5, -300)), Vector((0, 0, 1)), 1000)[0]          # between the capsules
    nrm = bvh.ray_cast(Vector((-20.0, side * 66.0, -300)), Vector((0, 0, 1)), 1000)[1]          # plain chamfer inboard of them
    assert hit is not None and nrm.z < -0.3 and nrm.x > 0.3 and abs(nrm.y) < 0.05, (side, hit, nrm)
    arc, total = free_arc(hit + nrm * 0.5, -1)
    mounts.append({'kind': 'beam_emitter', 'group': 'asgard_beams', 'zone': 'ventral, chamfered front of the block between the hangar pods',
                   'centerMetres': [round(c, 2) for c in hit], 'surfaceNormal': [round(c, 3) for c in nrm],
                   'forwardAxis': [1, 0, 0], 'upAxis': [0, 0, -1], 'traverseFreeAzimuthDeg': arc, 'freeAzimuthTotalDeg': total,
                   'arcNote': 'rays at 5 deg below the horizon; hull occlusion only, no game rules',
                   'intendedMovingMesh': 'none (fixed emitter; the beam effect starts here)', 'referenceURL': ASG_URL,
                   'evidence': 'user, from on-screen stills U9/U10: one beam leaves the underside between the hangar pods; mirrored on both sides',
                   'confidence': 'low-medium: zone from stills; exact emitter not modelled'})

# ---------------------------------------------------------------- 6b. add-on detail seen in the stills
# User: "free hand, make it look as close to the series as possible" (reference: the 4K still U15 and
# close-ups U16-U20). Parts the stills show but the Astrofossil model lacks are SEPARATE objects in the
# hull frame, so Daedalus.glb stays the exact 600 m hull. The model's own dome turrets (with their
# barrels) are kept as they are; no extra barrels are added (user).
class Geo:
    """Face soup with a material key and an optional COLOR_0 per face; outward winding by construction."""
    def __init__(self): self.v, self.f, self.mat, self.col, self.uv = [], [], [], [], {}
    def poly(self, pts, mat, col=None, uv=None):
        s = len(self.v); self.v.extend(tuple(p) for p in pts); self.f.append(tuple(range(s, s + len(pts))))
        self.mat.append(mat); self.col.append(col)
        if uv: self.uv[len(self.f) - 1] = uv
    def obox(self, c, ex, ey, half, mat, col=None):          # oriented box, right-handed (ex, ey, ex x ey)
        ez = ex.cross(ey); c = Vector(c); hx, hy, hz = half
        P = [c + ex * (sx * hx) + ey * (sy * hy) + ez * (sz * hz) for sx in (-1, 1) for sy in (-1, 1) for sz in (-1, 1)]
        for f in [(0, 1, 3, 2), (4, 6, 7, 5), (0, 4, 5, 1), (2, 3, 7, 6), (0, 2, 6, 4), (1, 5, 7, 3)]: self.poly([P[i] for i in f], mat, col)
    def box(self, c, size, mat, col=None): self.obox(c, Vector((1, 0, 0)), Vector((0, 1, 0)), Vector(size) * 0.5, mat, col)
    def tube(self, p0, p1, r0, r1, mat, col=None, seg=10):
        d = (p1 - p0).normalized(); a = Vector((0, 0, 1)) if abs(d.z) < 0.9 else Vector((1, 0, 0))
        b1 = d.cross(a).normalized(); b2 = d.cross(b1)
        ring = lambda p, r: [p + (b1 * math.cos(2 * math.pi * k / seg) + b2 * math.sin(2 * math.pi * k / seg)) * r for k in range(seg)]
        R0, R1 = ring(p0, r0), ring(p1, r1)
        for k in range(seg): self.poly([R0[k], R0[(k + 1) % seg], R1[(k + 1) % seg], R1[k]], mat, col)
        self.poly(R1, mat, col); self.poly(list(reversed(R0)), mat, col)
    def bar(self, x, p0, p1, w, depth, mat, col=None):        # bar standing on a wall that faces +X at plane x
        a = Vector((0, p0[0], p0[1])); b = Vector((0, p1[0], p1[1])); ey = (b - a).normalized()
        self.obox(Vector((x + depth / 2, (a.y + b.y) / 2, (a.z + b.z) / 2)), Vector((1, 0, 0)), ey, ((depth / 2), (b - a).length / 2 + w / 2, w / 2), mat, col)
    def disc(self, c, n, r, mat, col=None, seg=12):
        n = n.normalized(); a = Vector((0, 0, 1)) if abs(n.z) < 0.9 else Vector((1, 0, 0)); b1 = n.cross(a).normalized(); b2 = n.cross(b1)
        self.poly([c + (b1 * math.cos(2 * math.pi * k / seg) + b2 * math.sin(2 * math.pi * k / seg)) * r for k in range(seg)], mat, col)
    def build(self, name, mats, default_col):
        me = bpy.data.meshes.new(name); me.from_pydata(self.v, [], self.f)
        keys = list(dict.fromkeys(self.mat))
        for k in keys: me.materials.append(mats[k])
        me.polygons.foreach_set('material_index', [keys.index(k) for k in self.mat]); me.update()
        box_uv(me); uvl = me.uv_layers['UVMap']
        for fi, uvs in self.uv.items():
            for j, li in enumerate(me.polygons[fi].loop_indices): uvl.data[li].uv = uvs[j]
        cols = np.array([list(c if c is not None else default_col.get(m, (1.0, 1.0, 1.0))) + [1.0] for c, m in zip(self.col, self.mat)], np.float32)
        ca = me.color_attributes.new(name='COLOR_0', type='FLOAT_COLOR', domain='CORNER')
        ca.data.foreach_set('color', np.repeat(cols, [len(f) for f in self.f], axis=0).ravel())
        return me
def down(x, y):
    hit = bvh.ray_cast(Vector((x, y, 300)), Vector((0, 0, -1)), 1000)[0]; assert hit is not None, (x, y); return hit.z

HATCH = hatch_maps()
MAT_ADD = {'Armor': MAT['Armor'], 'Hangar': MAT['Hangar'],
           'Trim': pbr('Daedalus_TrimLight', '7d7f80', .5, .45), 'DarkPanel': pbr('Daedalus_DarkPanel', '1a1e21', .4, .4),
           'HatchPlain': pbr('Daedalus_SiloHatch_Plain', 'ffffff', .3, .6, tex={'base': HATCH['plain'], 'normal': HATCH['normal']}),
           'HatchStriped': pbr('Daedalus_SiloHatch_Striped', 'ffffff', .3, .6, tex={'base': HATCH['striped'], 'normal': HATCH['normal']})}
LMAT = {'WindowCyan': pbr('Daedalus_WindowCyan', '0e1c22', .2, .25, '57c6dd', 0.32),      # small faint blue windows (U18-U20)
        'SpotWhite': pbr('Daedalus_SpotWhite', 'e8eef5', .1, .3, 'f2f6ff', 8.0),        # spot and flood lights (U17)
        'NavGreen': pbr('Daedalus_NavGreen', '1f5a2c', .1, .3, '39ff6a', 8.0),          # starboard (U15, U17)
        'NavRed': pbr('Daedalus_NavRed', '5a1f1f', .1, .3, 'ff3a30', 8.0),              # port, by convention
        'CyanUnit': pbr('Daedalus_CyanUnit', '123a3a', .2, .3, '3fe0d0', 2.5)}          # small cyan-lit deck units (U19)
BEACON_MAT = pbr('Daedalus_Beacon', '5a2a1a', .1, .3, 'ff6a2a', 8.0)                     # orange mast-tip beacons (U20)
add, lit, bea = Geo(), Geo(), Geo()
addon_info = {'masts': [], 'rods': [], 'siloHatches': [], 'hangarInserts': [], 'lights': {}}

# Bridge masts (U15, U20): two thin masts on the top tier with blinking tip lights, and a cluster of three
# very tall thin masts beside the tower; the tallest carries a beacon too.
for x, y, h, r, beacon in [(-215.0, 40.0, 14.0, 0.35, True), (-215.0, 60.0, 14.0, 0.35, True), (-246.0, 76.0, 50.0, 0.45, True),
                           (-248.5, 78.0, 44.0, 0.4, False), (-244.0, 79.5, 38.0, 0.4, False)]:
    z0 = down(x, y); add.tube(Vector((x, y, z0 - 0.3)), Vector((x, y, z0 + h)), r, r * 0.4, 'Armor', seg=8)
    addon_info['masts'].append({'baseMetres': [x, y, round(z0, 2)], 'heightMetres': h, 'beacon': beacon})
    if beacon: bea.tube(Vector((x, y, z0 + h)), Vector((x, y, z0 + h + 0.9)), 0.45, 0.45, 'Beacon', seg=8)

# Two long forward rods at the bow (R2, U2, U15: thin rods well ahead of the bow; their tips glow when firing).
ROD_Y, ROD_Z, ROD_LEN = 29.5, -14.0, 40.0
for side in (+1, -1):
    hit = bvh.ray_cast(Vector((400, side * ROD_Y, ROD_Z)), Vector((-1, 0, 0)), 1000)[0]
    assert hit is not None and hit.x > 280, (side, hit)
    b0 = Vector((hit.x - 0.5, side * ROD_Y, ROD_Z)); tip = b0 + Vector((ROD_LEN, 0, 0))
    add.tube(b0, b0 + Vector((3.5, 0, 0)), 1.4, 1.1, 'Armor', seg=12)
    add.tube(b0 + Vector((3.0, 0, 0)), tip, 0.6, 0.35, 'Armor')
    add.tube(tip - Vector((1.6, 0, 0)), tip, 0.5, 0.5, 'Armor')
    addon_info['rods'].append({'baseMetres': [round(c, 2) for c in b0], 'tipMetres': [round(c, 2) for c in tip]})
    mounts.append({'kind': 'fixed_barrel', 'group': 'bow_rods', 'centerMetres': [round(c, 2) for c in tip],
                   'forwardAxis': [1, 0, 0], 'upAxis': [0, 0, 1], 'intendedMovingMesh': 'none (static rod in DaedalusAddOns.glb)',
                   'referenceURL': REF_DECK, 'evidence': 'long rods ahead of the bow in R2, U2 and U15; U15/U21 show glowing tips',
                   'confidence': 'low: rods are visible, their exact base and function are an interpretation'})

# VLS hatches like the close-up U16: an overlay slab on each measured hatch plate (covering the model's
# older door design) with a long taupe door, end tabs and, in a checkerboard, yellow/black hazard stripes.
for i, s in enumerate(sorted(silos, key=lambda s: (-s['a'], s['b']))):
    x0, x1, y0, y1 = s['plate']; zt, zb = s['surface'] + 0.55, s['surface'] - 0.1
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2; L = min(HATCH_L, y1 - y0 - 0.6); W = min(HATCH_W, x1 - x0 - 1.0)
    hx0, hx1, hy0, hy1 = cx - W / 2, cx + W / 2, cy - L / 2, cy + L / 2
    for a, b in (((x0, y0), (x1, y0)), ((x1, y0), (x1, y1)), ((x1, y1), (x0, y1)), ((x0, y1), (x0, y0))):
        add.poly([(a[0], a[1], zb), (b[0], b[1], zb), (b[0], b[1], zt), (a[0], a[1], zt)], 'Armor', tuple(np.array(SIDE) * 0.9))
    top = tuple(np.array(TOP) * 0.95)
    for q in ([(x0, y0), (hx0, y0), (hx0, y1), (x0, y1)], [(hx1, y0), (x1, y0), (x1, y1), (hx1, y1)],
              [(hx0, y0), (hx1, y0), (hx1, hy0), (hx0, hy0)], [(hx0, hy1), (hx1, hy1), (hx1, y1), (hx0, y1)]):
        add.poly([(p[0], p[1], zt) for p in q], 'Armor', top)
    striped = (i // 2 + (1 if cy > 0 else 0)) % 2 == 0
    add.poly([(hx0, hy0, zt), (hx1, hy0, zt), (hx1, hy1, zt), (hx0, hy1, zt)], 'HatchStriped' if striped else 'HatchPlain',
             uv=[(0, 0), (0, 1), (1, 1), (1, 0)])
    addon_info['siloHatches'].append({'centreMetres': [round(cx, 2), round(cy, 2), round(zt, 2)], 'sizeMetres': [round(W, 2), round(L, 2)], 'hazardStripes': striped})

# Hangar bays like U17/U26-U27: the model's bays end in an older X-truss frame. A new back wall just in front
# of it is clipped to the measured bay cross-section (the bay is asymmetric: vertical inboard wall, sloped
# outboard wall) and carries the split observation window with a heavy frame, a ledge, wall ribs and
# fixtures; the bay also gets wall ribs, floor rails and ceiling beams so it does not read as bare skin.
BAY_LIGHTS, DOORS = [], []
DOOR_TONE = tuple(np.clip(np.array(srgb('8a8886')) / 0.46, 0, 1))   # U33: dark grey with a faint teal cast under the bay light
def bay_span(x, yc, z, reach=120):
    """Free y interval at (x, z) seen from the bay centre line (inboard, outboard wall hits)."""
    out = []
    for sg in (-1, 1):
        hit = bvh.ray_cast(Vector((x, yc, z)), Vector((0, sg, 0)), reach)[0]
        out.append(hit.y if hit is not None else yc + sg * reach)
    return out
for h in hangars:
    c = (h['aMin'] + h['aMax']) / 2; B = h['aMax'] - h['aMin']; z0, z1 = h['bMin'], h['bMax']; H = z1 - z0; zc = (z0 + z1) / 2
    hits = [bvh.ray_cast(Vector((h['rimX'] - 1, y, z)), Vector((-1, 0, 0)), 400)[0]
            for y in np.arange(h['aMin'] + 12, h['aMax'] - 12, 4) for z in np.arange(z0 + 5, z1 - 4, 2)]   # central region (corners are chamfered)
    xw = max(p.x for p in hits if p is not None and p.x < h['rimX'] - 30) + 0.8
    assert h['rimX'] - xw > 35, (h['rimX'], xw)
    # back wall: star polygon from the bay centre to the measured walls, 0.15 m inset (never pokes out)
    ctr = Vector((xw + 0.3, c, zc)); ring = []
    for k in range(72):
        d = Vector((0, math.cos(2 * math.pi * k / 72), math.sin(2 * math.pi * k / 72)))
        hit = bvh.ray_cast(ctr, d, 120)[0]
        p = hit if hit is not None else ctr + d * 0.5 * H
        ring.append(Vector((xw, ctr.y + (p.y - ctr.y) * 0.995 - d.y * 0.15, ctr.z + (p.z - ctr.z) * 0.995 - d.z * 0.15)))
    add.poly(ring, 'Hangar')
    # window fitted between the walls at its three heights
    gb, gt = z0 + 0.40 * H, z0 + 0.86 * H; gm = (gb + gt) / 2
    (lb, rb), (lm, rm), (lt, rt) = (bay_span(xw + 0.3, c, z) for z in (gb, gm, gt))
    hexa = [(lb + 2.0, gb), (rb - 2.0, gb), (rm - 1.0, gm), (rt - 2.0, gt), (lt + 2.0, gt), (lm + 1.0, gm)]
    for k in range(6): add.bar(xw, hexa[k], hexa[(k + 1) % 6], 0.9, 0.7, 'Trim')          # heavy window surround
    cw, Wm = (lm + rm) / 2, rm - lm
    # split hangar doors (user): two plated metal halves meeting at a seam with a raised trapezoid; the
    # upper half opens up, the lower half down. Separate objects so the game can slide them apart.
    ztr = gt - 0.12 * (gt - gb)
    seam = [(lm + 1.0, gm), (cw - 0.13 * Wm, gm), (cw - 0.08 * Wm, ztr), (cw + 0.08 * Wm, ztr), (cw + 0.13 * Wm, gm), (rm - 1.0, gm)]
    side = 'P' if c > 0 else 'S'; up, dn = Geo(), Geo()
    up.poly([(xw + 0.12, y, z) for y, z in seam + [(rt - 2.0, gt), (lt + 2.0, gt)]], 'Hangar', DOOR_TONE)
    dn.poly([(xw + 0.12, y, z) for y, z in [(lb + 2.0, gb), (rb - 2.0, gb)] + seam[::-1]], 'Hangar', DOOR_TONE)
    for k in range(5): up.bar(xw + 0.12, seam[k], seam[k + 1], 0.45, 0.4, 'Trim')        # seam lip on the upper half
    for sg in (-1, 1): dn.bar(xw + 0.12, (cw + sg * 0.30 * Wm, gb), (cw + sg * 0.22 * Wm, gm), 0.4, 0.4, 'Trim')
    for half, g, axis, dist in (('upper', up, 1, gt - gm + 1.0), ('lower', dn, -1, gm - gb + 1.0)):
        DOORS.append((f'HangarDoor_{side}_{half.capitalize()}', g, Vector((xw, cw, gm)),
                      {'doorHalf': half, 'bay': side, 'openAxis': [0, 0, axis], 'openDistanceMetres': round(dist, 2), 'seamZMetres': round(gm, 2),
                       'note': 'split hangar door: the halves open apart at the seam, upper up, lower down (user)'}))
    for k in (-1, 0, 1): add.box(Vector((xw + 0.45, cw + k * 0.2 * Wm, gt + 0.06 * H)), (0.9, 3.5, 0.9), 'Trim')
    lz = gb - 1.2; ll, lr = bay_span(xw + 0.3, c, lz)
    add.box(Vector((xw + 0.35, (ll + lr) / 2, lz)), (0.7, lr - ll - 1.0, 0.8), 'Trim')     # ledge under the window
    for k in (-0.5, 0.5): add.box(Vector((xw + 0.9, cw + k * 0.3 * Wm, lz - 1.3)), (1.0, 4.5, 1.0), 'Trim')
    for y in np.arange(ll + 3.0, lr - 2.9, 7.0):                                             # wall ribs below the ledge
        add.box(Vector((xw + 0.3, y, (z0 + 0.4 + lz - 0.4) / 2)), (0.6, 0.7, lz - 0.4 - (z0 + 0.4)), 'Trim')
    # bay: vertical ribs on the vertical walls, floor rails and ceiling beams
    for x in np.arange(h['rimX'] - 5, xw + 2.5, -7.0):
        for sg in (-1, 1):
            hit, n, _, _ = bvh.ray_cast(Vector((x, c, zc)), Vector((0, sg, 0)), 120)
            if hit is not None and abs(n.y) > 0.9:
                add.box(Vector((x, hit.y - sg * 0.35, zc)), (0.8, 0.7, H - 5.0), 'Trim')
        top = bvh.ray_cast(Vector((x, c, zc)), Vector((0, 0, 1)), 60)[0]
        if top is not None:
            yl, yr = bay_span(x, c, top.z - 0.8)
            add.box(Vector((x, (yl + yr) / 2, top.z - 0.5)), (0.9, yr - yl - 0.6, 0.8), 'Trim')
    floor = bvh.ray_cast(Vector((h['rimX'] - 5, c, zc)), Vector((0, 0, -1)), 60)[0]
    if floor is not None:
        for k in (-0.18, 0.18):
            add.box(Vector(((h['rimX'] - 2 + xw + 1) / 2, c + k * B, floor.z + 0.2)), (h['rimX'] - 2 - xw - 1, 0.8, 0.4), 'Trim')
    for k, lx in enumerate((h['rimX'] - 15, xw + 12)):                                       # dim teal interior lights (U17, U26-U27)
        bl = bpy.data.lights.new(f'BayLight_{"P" if c > 0 else "S"}_{k + 1}', 'POINT'); bl.energy = 6000; bl.color = (0.6, 0.85, 0.8)
        bl.use_custom_distance = True; bl.cutoff_distance = 70; bl.shadow_soft_size = 3
        lo = bpy.data.objects.new(bl.name, bl); lo.location = (lx, cw, z1 - 3); BAY_LIGHTS.append(lo)
    addon_info['hangarInserts'].append({'bayCentreY': round(c, 2), 'backWallX': round(xw, 2), 'windowZ': [round(gb, 2), round(gt, 2)],
                                        'windowY': [round(min(lb, lt) + 2, 2), round(max(rb, rt) - 2, 2)]})
    # lights on the pod: two round spotlights above the opening, floodlights under the front edge, nav light
    for dy in (-0.25 * B, 0.25 * B):
        hit, n, _, _ = bvh.ray_cast(Vector((200, c + dy, z1 + 2.5)), Vector((-1, 0, 0)), 1000)
        if hit is not None and n.x > 0.3: lit.disc(hit + n * 0.06, n, 0.9, 'SpotWhite')
    for k in (-1, 0, 1):
        hit = bvh.ray_cast(Vector((h['rimX'] - 4, c + k * 0.3 * B, -300)), Vector((0, 0, 1)), 1000)[0]
        if hit is not None: lit.box(Vector((h['rimX'] - 4, c + k * 0.3 * B, hit.z - 0.2)), (1.6, 2.6, 0.3), 'SpotWhite')
    sg = 1 if c > 0 else -1
    hit, n, _, _ = bvh.ray_cast(Vector((h['rimX'] - 12, sg * 400, (z0 + z1) / 2 - 4)), Vector((0, -sg, 0)), 1000)
    if hit is not None: lit.box(hit + n * 0.45, (1.0, 1.0, 1.0), 'NavRed' if sg > 0 else 'NavGreen')

# Forward superstructure (U19, U31): a dark window panel with faint cyan windows on the front face of the
# upper tier, and two small cyan-lit units on the deck in front of it.
hit, n, _, _ = bvh.ray_cast(Vector((110, 0, 22.0)), Vector((-1, 0, 0)), 200)
assert hit is not None and n.x > 0.9 and 55 < hit.x < 75, hit
add.obox(Vector((hit.x + 0.08, 0, 22.0)), Vector((1, 0, 0)), Vector((0, 1, 0)), (0.08, 4.6, 0.9), 'DarkPanel')
for y in np.linspace(-3.6, 3.6, 5):
    lit.obox(Vector((hit.x + 0.18, y, 22.0)), Vector((1, 0, 0)), Vector((0, 1, 0)), (0.03, 0.55, 0.4), 'WindowCyan')
front = bvh.ray_cast(Vector((110, 0, 18.0)), Vector((-1, 0, 0)), 200)[0]
for y in (-6.0, 6.0):
    deck = bvh.ray_cast(Vector((front.x + 4.5, y, 60)), Vector((0, 0, -1)), 200)[0]
    add.box(Vector((front.x + 4.5, y, deck.z + 0.5)), (2.0, 1.8, 1.0), 'Trim')
    lit.box(Vector((front.x + 4.5, y, deck.z + 1.15)), (1.4, 1.2, 0.3), 'CyanUnit')
addon_info['forwardSuperstructure'] = {'windowFaceX': round(hit.x, 2), 'cyanUnitsX': round(front.x + 4.5, 2)}

# Small faint blue windows (U18-U20): a row in the dark bow band and rows around the bridge tower tiers.
for s in bow_slot:
    zc = (s['bMin'] + s['bMax']) / 2
    for y in np.linspace(s['aMin'] + 1.5, s['aMax'] - 1.5, 7):
        lit.obox(Vector((s['surface'] + 0.05, y, zc)), Vector((1, 0, 0)), Vector((0, 1, 0)), (0.04, 0.6, 0.35), 'WindowCyan')
nwin = 0
for z in (35.0, 40.0, 44.0):
    rays = ([(Vector((0, y, z)), Vector((-1, 0, 0))) for y in np.arange(26, 74, 1.8)] +
            [(Vector((x, -100, z)), Vector((0, 1, 0))) for x in np.arange(-238, -160, 1.8)] +
            [(Vector((x, 200, z)), Vector((0, -1, 0))) for x in np.arange(-238, -160, 1.8)])
    for k, (o, d) in enumerate(rays):
        hit, n, _, _ = bvh.ray_cast(o, d, 1000)
        if hit is None or not (-240 < hit.x < -150 and 20 < hit.y < 80) or abs(n.z) > 0.3 or k % 4 == 3: continue
        lit.obox(hit + n * 0.04, n, Vector((0, 0, 1)).cross(n).normalized(), (0.03, 0.5, 0.25), 'WindowCyan'); nwin += 1
addon_info['lights'] = {'bridgeWindows': nwin, 'bowWindows': 7 * len(bow_slot), 'faces': len(lit.f)}
GROUP_EFFECTS = {'dorsal_railguns': 'orange tracer projectiles (user, from on-screen footage)',
                 'ventral_railguns': 'orange tracer projectiles (user, from on-screen footage)',
                 'bow_vls': 'missiles leave the dorsal bow silos upward (+Z), then turn to the target (user)',
                 'asgard_beams': 'continuous blue-white beam: near-white core with a blue halo (user stills U9, U10)',
                 'f302_bays': 'F-302 fighters exit along +X',
                 'bow_rods': 'orange projectile from the rod tip (glowing tips in U13, U14); interpretation'}
for m in mounts: m['fireEffectHint'] = GROUP_EFFECTS[m['group']]
counters = {}
for m in mounts:
    key = {'dorsal_railguns': 'RG_D', 'ventral_railguns': 'RG_V', 'bow_vls': 'VLS', 'f302_bays': 'BAY', 'asgard_beams': 'ASG', 'bow_rods': 'ROD'}[m['group']]
    counters[key] = counters.get(key, 0) + 1
    side = '' if key in ('VLS',) else ('_P' if m['centerMetres'][1] > 0.5 else '_S' if m['centerMetres'][1] < -0.5 else '_C')
    m['mountID'] = f'{key}_{counters[key]:02d}{side}'
mount_col = bpy.data.collections.new('WeaponMounts'); scene.collection.children.link(mount_col)
for m in mounts:
    e = bpy.data.objects.new('MOUNT_' + m['mountID'], None); mount_col.objects.link(e)
    e.empty_display_type = 'ARROWS'; e.empty_display_size = 6
    fwd = Vector(m['forwardAxis']); upv = Vector(m['upAxis']); rgt = upv.cross(fwd)
    M = Matrix.Identity(4)
    M.col[0] = (*fwd, 0); M.col[1] = (*rgt, 0); M.col[2] = (*upv, 0); M.col[3] = (*m['centerMetres'], 1)
    e.matrix_world = M
    for k2 in ('mountID', 'kind', 'group'): e[k2] = m[k2]
bpy.context.view_layer.update()
for m in mounts:   # empties must carry exactly the JSON frame (local +X = forwardAxis, +Z = upAxis)
    mw = bpy.data.objects['MOUNT_' + m['mountID']].matrix_world
    assert (mw.col[0].xyz - Vector(m['forwardAxis'])).length < 1e-4 and (mw.col[2].xyz - Vector(m['upAxis'])).length < 1e-4
    assert (mw.translation - Vector(m['centerMetres'])).length < 1e-3, m['mountID']
live('Krok 4: zmerene body zbrani (sipky)')

# Add-on object (masts, rods, silo hatches, hangar inserts), detail lights and blinking beacons.
addon_col = bpy.data.collections.new('AddOns'); scene.collection.children.link(addon_col)
addon_me = add.build('Daedalus_AddOns', MAT_ADD, {'Armor': tuple(np.array(SIDE) * 0.85), 'Hangar': tuple(HANGAR_GREY)})
addon_obj = bpy.data.objects.new('Daedalus_AddOns', addon_me); addon_col.objects.link(addon_obj)
addon_obj['note'] = 'detail seen in the stills but absent from the Astrofossil model: bridge masts, forward bow rods, U16-style VLS hatches, U17-style hangar back walls'
DOOR_OBJS = []
for name, g, origin, extras in DOORS:
    dme = g.build(name, MAT_ADD, {'Hangar': DOOR_TONE}); dme.transform(Matrix.Translation(-origin))
    dob = bpy.data.objects.new(name, dme); addon_col.objects.link(dob); dob.location = origin
    for k, v in extras.items(): dob[k] = v
    DOOR_OBJS.append(dob)
lights_obj = bpy.data.objects.new('Daedalus_DetailLights', lit.build('Daedalus_DetailLights', LMAT, {})); lights_col.objects.link(lights_obj)
lights_obj['note'] = 'faint blue windows (bow band, bridge, forward superstructure), pod spot and flood lights, nav lights (port red, starboard green), cyan deck units'
for lo in BAY_LIGHTS: lights_col.objects.link(lo)
beacon_col = bpy.data.collections.new('Beacons'); scene.collection.children.link(beacon_col)
beacon_obj = bpy.data.objects.new('Daedalus_Beacons', bea.build('Daedalus_Beacons', {'Beacon': BEACON_MAT}, {})); beacon_col.objects.link(beacon_obj)
BLINK = {'blinkPeriodSeconds': 5.0, 'onSeconds': 1 / 3, 'colour': '#ff6a2a', 'emissionOn': 8.0, 'emissionOff': 0.0}
for k, v in BLINK.items(): beacon_obj[k] = v
# Preview animation in Blender (24 fps): on for 8 frames, off for 112 frames, repeating. Unreal reads BLINK.
scene.render.fps = 24; scene.frame_start, scene.frame_end = 1, 240
inp = BEACON_MAT.node_tree.nodes['Principled BSDF'].inputs['Emission Strength']
for frame, val in ((1, 8.0), (8, 8.0), (9, 0.0), (120, 0.0)):
    inp.default_value = val; inp.keyframe_insert('default_value', frame=frame)
ad = BEACON_MAT.node_tree.animation_data
try: fcurves = ad.action.fcurves
except AttributeError:
    from bpy_extras import anim_utils
    fcurves = anim_utils.action_get_channelbag_for_slot(ad.action, ad.action_slot).fcurves
for fc in fcurves:
    for kp in fc.keyframe_points: kp.interpolation = 'CONSTANT'
    fc.modifiers.new('CYCLES')
inp.default_value = 8.0

# Look-development lighting like the stills (not exported): hard key sun from port-aft above, a blue
# Earth bounce from below-starboard that keeps the sides readable (U13 sides ~105-110 sRGB), a weak
# camera-side fill, near-black space, AgX with higher contrast. Camera CAM_SeriesStill frames
# the hull like the on-screen still U13 (dorsal three-quarter from front starboard).
look_col = bpy.data.collections.new('LookDev_Series'); scene.collection.children.link(look_col)
def sun(name, energy, color, travel, angle):
    l = bpy.data.lights.new(name, 'SUN'); l.energy = energy; l.color = color; l.angle = math.radians(angle)
    o = bpy.data.objects.new(name, l); look_col.objects.link(o)
    o.rotation_euler = (-Vector(travel)).normalized().to_track_quat('Z', 'Y').to_euler(); return o
sun('SUN_Key', 4.6, (1.0, 0.98, 0.95), (0.3, -0.5, -0.81), 0.5)
sun('SUN_EarthBounce', 2.4, (0.62, 0.8, 1.0), (-0.25, 0.6, 0.6), 3.0)     # cool blue sky/Earth light (U22 tone)
sun('SUN_CamFill', 0.6, (0.8, 0.9, 1.0), (-0.65, 0.45, -0.5), 5.0)
scene.world = bpy.data.worlds.new('Space_Dark'); scene.world.use_nodes = True
scene.world.node_tree.nodes['Background'].inputs[0].default_value = (0.008, 0.011, 0.016, 1)
scene.view_settings.view_transform = 'AgX'
try: scene.view_settings.look = 'AgX - Medium High Contrast'
except TypeError: pass
cam = bpy.data.objects.new('CAM_SeriesStill', bpy.data.cameras.new('CAM_SeriesStill')); look_col.objects.link(cam)
cam.data.lens = 28; cam.data.clip_end = 5000; scene.camera = cam
cam.location = (470, -300, 330)
cam.rotation_euler = (Vector((-30, 10, -10)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
scene.render.resolution_x, scene.render.resolution_y = 2000, 1125
live('Krok 5: textura plat, stozary, tyce a hlavne vezi')

# ---------------------------------------------------------------- 7. save, export, reopen checks
ship['source_creator'] = 'Astrofossil'; ship['source_url'] = 'https://www.thingiverse.com/thing:2256025'
ship['license'] = 'CC BY-NC 4.0'; ship['forward_axis'] = '+X'; ship['up_axis'] = '+Z'; ship['length_metres'] = 600.0
ship['detail_pass'] = 'task 0008: plating texture (box UVs), reference palette, AO, lights; exact Astrofossil geometry; add-ons, detail lights and beacons are separate objects'

scene.render.engine = 'BLENDER_EEVEE'
blend = OUT / 'Daedalus.blend'; glb = OUT / 'Daedalus.glb'; glow_glb = OUT / 'DaedalusEngineGlow.glb'; lights_glb = OUT / 'DaedalusLights.glb'
addons_glb = OUT / 'DaedalusAddOns.glb'; beacons_glb = OUT / 'DaedalusBeacons.glb'
bpy.ops.wm.save_as_mainfile(filepath=str(blend), compress=True)
TEX_FILES = sorted(img.name for img in bpy.data.images if img.filepath_raw and Path(bpy.path.abspath(img.filepath_raw)).resolve().parent == TEX_DIR.resolve())
for n in TEX_FILES: bpy.data.images[n].filepath = '//Textures/' + n + '.png'   # relative, forward slashes (portable)
bpy.ops.wm.save_mainfile(compress=True)
assert all(bpy.data.images[PLATING[k].name].filepath.startswith('//') for k in PLATING)
# glTF: textured slots export the plating maps (baseColorTexture x COLOR_0 per the glTF spec);
# untextured slots export a constant base colour; COLOR_0 is the active colour attribute.
for m in {*mesh.materials, *addon_me.materials}:
    nt = m.node_tree; bs = nt.nodes.get('Principled BSDF')
    for link in list(bs.inputs['Base Color'].links): nt.links.remove(link)
    if m['textured']: nt.links.new(nt.nodes['TEX_base'].outputs['Color'], bs.inputs['Base Color'])
    else: bs.inputs['Base Color'].default_value = (*srgb(m['base_hex']), 1)
def export(path, objs, **kw):
    bpy.ops.object.select_all(action='DESELECT')
    for o in objs: o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.export_scene.gltf(filepath=str(path), export_format='GLB', use_selection=True, export_extras=True, **kw)
export(glb, [ship], export_vertex_color='ACTIVE', export_materials='EXPORT')
export(glow_glb, glow_objs + [c for o in glow_objs for c in o.children], export_lights=True)
export(lights_glb, [bpy.data.objects['Daedalus_WindowLights'], lights_obj] + BAY_LIGHTS, export_lights=True)
export(addons_glb, [addon_obj] + DOOR_OBJS, export_vertex_color='ACTIVE', export_materials='EXPORT')
export(beacons_glb, [beacon_obj], export_animations=False)
bpy.ops.wm.open_mainfile(filepath=str(blend))
ship = bpy.data.objects['SM_Daedalus']
assert abs(ship.dimensions.x - 600) < .01
skip = {o.name for c in ('EngineGlow', 'HullLights', 'AddOns', 'Beacons') for o in bpy.data.collections[c].objects}
hull_meshes = [o for o in bpy.data.objects if o.type == 'MESH' and o.name not in skip]
assert [o.name for o in hull_meshes] == ['SM_Daedalus'], [o.name for o in hull_meshes]
missing = [i.name for i in bpy.data.images if i.source == 'FILE' and not Path(bpy.path.abspath(i.filepath)).exists()]
assert not missing, missing
check = {'blendReopen': True, 'dimensionsMetres': [round(c, 4) for c in ship.dimensions], 'materialSlots': [m.name for m in ship.data.materials],
         'colorAttributes': [c.name for c in ship.data.color_attributes], 'uvMaps': [u.name for u in ship.data.uv_layers],
         'imageDependencies': sorted(i.filepath for i in bpy.data.images if i.source == 'FILE')}
# Re-import exports into empty scenes.
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(glb))
ims = [o for o in bpy.context.scene.objects if o.type == 'MESH']
check['glbMeshes'] = [o.name for o in ims]; check['glbDimensions'] = [round(c, 3) for c in ims[0].dimensions]
check['glbTriangles'] = sum(len(o.data.polygons) for o in ims); check['glbColorAttributes'] = [c.name for c in ims[0].data.color_attributes]
check['glbMaterials'] = sorted({m.name for o in ims for m in o.data.materials})
check['glbUVMaps'] = [u.name for u in ims[0].data.uv_layers]; check['glbImages'] = len([i for i in bpy.data.images if i.size[0] > 0])
assert len(ims) == 1 and abs(ims[0].dimensions.x - 600) < .05 and check['glbTriangles'] == 222330
assert check['glbUVMaps'] and check['glbImages'] >= 3, check
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(addons_glb))
check['addOnObjects'] = sorted(o.name for o in bpy.context.scene.objects if o.type == 'MESH')
check['addOnMaterials'] = sorted({m.name for o in bpy.context.scene.objects if o.type == 'MESH' for m in o.data.materials})
assert 'Daedalus_SiloHatch_Striped' in check['addOnMaterials'], check['addOnMaterials']
check['hangarDoors'] = sorted(o.name for o in bpy.context.scene.objects if 'doorHalf' in o)
assert len(check['hangarDoors']) == 2 * len(hangars), check['hangarDoors']
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(beacons_glb))
bo = [o for o in bpy.context.scene.objects if o.type == 'MESH']
check['beaconObjects'] = sorted(o.name for o in bo); check['beaconBlinkExtras'] = bool(bo) and 'blinkPeriodSeconds' in bo[0]
assert check['beaconBlinkExtras'], check
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(lights_glb))
check['lightsObjects'] = sorted(o.name for o in bpy.context.scene.objects if o.type == 'MESH')
check['bayLights'] = sum(1 for o in bpy.context.scene.objects if o.type == 'LIGHT')
assert check['bayLights'] == 2 * len(hangars), check['bayLights']
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(glow_glb))
gl = [o for o in bpy.context.scene.objects if o.type == 'MESH']
check['glowObjects'] = sorted(o.name for o in gl)
check['glowMaterials'] = sorted({m.name for o in gl for m in o.data.materials})
check['glowLights'] = sum(1 for o in bpy.context.scene.objects if o.type == 'LIGHT')
assert check['glowLights'] == len(engines), check['glowLights']

engine_manifest = FX['engine_manifest'](engines)
(OUT / 'ENGINE_MOUNTS.json').write_text(json.dumps(engine_manifest, indent=2) + '\n')
weapon_manifest = {
    'version': 1, 'units': 'metres', 'coordinateSystem': 'ship-local, +X forward, +Z up, origin = hull bounding-box centre (same as Daedalus.glb)',
    'method': 'ray-cast height maps (0.5 m grid) with top-hat (domes) / bottom-hat (recesses) detection on the unchanged hull',
    'axisConvention': 'forwardAxis = mount local +X (turret barrel rest / silo launch / bay exit); upAxis = local +Z (turret yaw axis); empties MOUNT_<id> in Daedalus.blend carry the same frame',
    'azimuthConvention': 'traverseFreeAzimuthDeg: degrees about the ship +Z, 0 = +X bow, 90 = +Y port; inclusive [start, end] ranges sampled every 5 deg at 5 deg elevation; a negative start wraps through 0 (e.g. [-295, 25] = 65..360 and 0..25)',
    'notes': ['Positions are measured modelled features, not canon proofs. Wiki lists 32 railguns and 16 VLS tubes; a fan sheet lists 26 twin railguns. The model contains its own dome count (see group counts).',
              'Turret mounts: centre = dome base on the hull; upAxis is the turret yaw axis; forwardAxis is the rest direction of the barrels.',
              'Railgun turrets are the original dome turrets with their barrels (part of the hull mesh); no extra barrels are added (user). A rotating version would need the dome extracted as a separate mesh.',
              'ROD mounts sit at the tips of the two forward bow rods (static, DaedalusAddOns.glb).',
              'Asgard plasma beam weapons (4 per wiki): bow pair = outer dome on each lower side ledge (labelled on the SGA Tech Journal schematic); ventral pair = chamfered block fronts between the hangar pods (user, from stills; no dedicated emitter modelled, see zone and surfaceNormal).',
              'fireEffectHint describes the look of each weapon as reported by the user from on-screen footage; it is not a game rule.'],
    'groupCounts': {g: sum(1 for m in mounts if m['group'] == g) for g in sorted({m['group'] for m in mounts})},
    'unplacedReferenceSystems': [],
    'mounts': mounts}
(OUT / 'WEAPON_MOUNTS.json').write_text(json.dumps(weapon_manifest, indent=2) + '\n')
report = {'sourceSHA256': hashlib.sha256(SOURCE.read_bytes()).hexdigest(), 'blendSHA256': hashlib.sha256(blend.read_bytes()).hexdigest(),
          'glbSHA256': hashlib.sha256(glb.read_bytes()).hexdigest(), 'engineGlowGlbSHA256': hashlib.sha256(glow_glb.read_bytes()).hexdigest(),
          'dimensionsMetres': check['dimensionsMetres'], 'originalTriangles': 222330, 'finalTriangles': FINAL['finalTriangles'],
          'vertices': FINAL['vertices'], 'originalAstrofossilGeometryChanged': False,
          'changesFromBaseline': 'hull is the exact Astrofossil geometry: the baseline hatch squares and vent slats (not in the original) were removed at the user request; baseline window cubes moved to the separate DaedalusLights.glb; parts the stills show but the model lacks (bridge masts, bow rods, U16-style VLS hatches, U17-style hangar back walls, detail lights, blinking mast beacons) are separate exports', 'materialSlots': check['materialSlots'],
          'vertexColour': 'COLOR_0 linear large-scale albedo x baked AO; textured slots multiply it with the plating base colour (glTF rule), untextured slots carry their base colour',
          'textures': {'files': {k: 'Textures/' + n + '.png' for k, n in TEX_NAMES.items()}, 'sizePixels': TEX_N, 'tileMetres': TILE_M,
                       'siloHatchFiles': ['Textures/' + n + '.png' for n in TEX_FILES if n not in TEX_NAMES.values()], 'siloHatchSizeMetres': [HATCH_W, HATCH_L],
                       'uv': 'UVMap: world-scale box projection per face (dominant normal axis), 1 UV unit = 32 m', 'generator': 'procedural, plating_maps() in this script (seed 304); no third-party images',
                       'stats': PLATING_STATS, 'sha256': {n: hashlib.sha256((TEX_DIR / (n + '.png')).read_bytes()).hexdigest() for n in TEX_FILES}},
          'addOns': addon_info, 'beacons': {'objects': check['beaconObjects'], **BLINK, 'note': 'blink in the engine: emissionOn for onSeconds once every blinkPeriodSeconds; Blender .blend has a preview animation'},
          'fittings': stats, 'materialAssignment': assign_stats, 'engineOutlets': len(engines), 'mountGroups': weapon_manifest['groupCounts'],
          'exports': {'hull': 'Daedalus.glb', 'engineGlow': 'DaedalusEngineGlow.glb', 'lights': 'DaedalusLights.glb', 'addOns': 'DaedalusAddOns.glb', 'beacons': 'DaedalusBeacons.glb'},
          'lightsGlbSHA256': hashlib.sha256(lights_glb.read_bytes()).hexdigest(), 'addOnsGlbSHA256': hashlib.sha256(addons_glb.read_bytes()).hexdigest(),
          'beaconsGlbSHA256': hashlib.sha256(beacons_glb.read_bytes()).hexdigest(), 'checks': check,
          'coordinateMapping': 'source (x,y,z) -> Unreal/Blender (-z,-x,y)', 'detailScript': 'Tools/Prepare-DaedalusDetail.py',
          'baselineScript': 'Tools/Prepare-DaedalusSource.py (baseline only; do not run over this detailed source)'}
(OUT / 'asset-metadata.json').write_text(json.dumps(report, indent=2) + '\n')
print('DAEDALUS_DETAIL_PASS ' + json.dumps({'engines': len(engines), 'groups': weapon_manifest['groupCounts'], 'assign': assign_stats, 'check': check}))
