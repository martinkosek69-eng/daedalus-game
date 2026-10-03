"""Reference-based Daedalus colour, lighting, engine glow and weapon-mount manifest (task 0008).

Run from the repository root:
  blender --background --factory-startup --python Tools/Prepare-DaedalusDetail.py
Optional: set DAEDALUS_LIVE=1 to save .local/daedalus-detail/live.blend after
each stage for a separate GUI viewer (preview only, never an asset).

Geometry is NOT redesigned. The hull is rebuilt from Original/daedalus.glb with
the same orientation and join as Tools/Prepare-DaedalusSource.py, giving the exact
Astrofossil geometry (222330 triangles). Baseline hatch squares and vent slats are
not original and are omitted; baseline window cubes become DaedalusLights.glb.
Only vertex colours, material assignment and separate effect objects change.
Prepare-DaedalusSource.py is the baseline reconstruction and must not be run
over the accepted detailed source (it would overwrite these outputs).

Outputs (Art/Ships/Daedalus): Daedalus.blend, Daedalus.glb (one hull mesh),
DaedalusEngineGlow.glb, DaedalusLights.glb, ENGINE_MOUNTS.json, WEAPON_MOUNTS.json,
asset-metadata.json. Previews: .local/daedalus-detail/renders (ignored).
"""
import bpy, json, math, hashlib, os, sys, runpy
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


def pbr(name, hexcol, metal, rough, emission=None, strength=0.0, vertex=False):
    m = bpy.data.materials.new(name); m.use_nodes = True
    rgb = srgb(hexcol); m.diffuse_color = (*rgb, 1)
    bs = m.node_tree.nodes.get('Principled BSDF')
    bs.inputs['Base Color'].default_value = (*rgb, 1)
    bs.inputs['Metallic'].default_value = metal; bs.inputs['Roughness'].default_value = rough
    if emission:
        bs.inputs['Emission Color'].default_value = (*srgb(emission), 1); bs.inputs['Emission Strength'].default_value = strength
    if vertex:
        vc = m.node_tree.nodes.new('ShaderNodeVertexColor'); vc.layer_name = 'COLOR_0'
        mix = m.node_tree.nodes.new('ShaderNodeMixRGB'); mix.blend_type = 'MULTIPLY'; mix.inputs[0].default_value = 1
        mix.inputs[2].default_value = (*rgb, 1)
        m.node_tree.links.new(vc.outputs['Color'], mix.inputs[1]); m.node_tree.links.new(mix.outputs[0], bs.inputs['Base Color'])
    m['base_hex'] = hexcol
    return m


# Hull slots. Vertex COLOR_0 multiplies every slot; the hull albedo lives in COLOR_0 (base factor white).
# Baseline Recess/Trim slots are gone with the removed fittings; Daedalus_Glass now lives on the window-light object.
MAT = {
    'Armor': pbr('Daedalus_Armor', 'ffffff', .25, .55, vertex=True),
    'EngineMetal': pbr('Daedalus_EngineMetal', '303336', .82, .42, vertex=True),
    'Hangar': pbr('Daedalus_HangarInterior', '9a958a', .3, .6, 'ffdcaa', 0.45, vertex=True),
    'LightWhite': pbr('Daedalus_LightWhite', 'e6f2ff', .1, .3, 'eef7ff', 6.0, vertex=True),
}
SLOT = {k: i for i, k in enumerate(MAT)}
for m in MAT.values(): mesh.materials.append(m)
for poly in mesh.polygons: poly.material_index = SLOT['Armor']
GLASS = pbr('Daedalus_Glass', '20323a', .2, .25, 'dff2ff', 4.0)

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
# door centre until the deck level to get the plate, then select the plate faces (top and sides).
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
silo_mask = np.zeros(npoly, bool)
for s in silos:
    s['plate'] = silo_plate(s); x0, x1, y0, y1 = s['plate']
    silo_mask |= ((pc[:, 0] > x0 - 0.3) & (pc[:, 0] < x1 + 0.3) & (pc[:, 1] > y0 - 0.3) & (pc[:, 1] < y1 + 0.3)
                  & (pc[:, 2] > s['surface'] - 0.5) & (pc[:, 2] < s['surface'] + 1.0) & (pn[:, 2] > -0.3))
slot_mask = np.zeros(npoly, bool)
for s in bow_slot:
    slot_mask |= ((pc[:, 1] > s['aMin']) & (pc[:, 1] < s['aMax']) & (pc[:, 2] > s['bMin']) & (pc[:, 2] < s['bMax'])
                  & (pn[:, 0] > 0.7) & (pc[:, 0] < s['surface'] + 0.2) & (pc[:, 0] > s['surface'] - 3.0))
matidx[hull & eng_mask] = SLOT['EngineMetal']
matidx[hull & hang_mask] = SLOT['Hangar']
matidx[hull & slot_mask] = SLOT['LightWhite']
mesh.polygons.foreach_set('material_index', matidx)
assign_stats = {'engineMetalFaces': int((hull & eng_mask).sum()), 'hangarInteriorFaces': int((hull & hang_mask).sum()),
                'bowLightFaces': int((hull & slot_mask).sum()), 'siloOrangeFaces': int((hull & silo_mask).sum())}

# ---------------------------------------------------------------- 4. reference palette into COLOR_0
# Stills show a dark charcoal hull with a slight olive/green cast, a lighter sunlit
# deck with dark and light plate patches, very dark recesses and dark greebles.
TOP, SIDE, UNDER = srgb('777d7a'), srgb('5a605d'), srgb('3f4442')
SILO_ORANGE = 'c98f35'   # amber-orange of the VLS hatch markings in the user's reference sheet (shade estimated by eye)
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
k = 0.92 + 0.16 * fine
k = np.where(patch < 0.22, k * 0.55, np.where(patch > 0.85, k * 1.25, k))
k = np.where((pa < 5.0) & (up > 0.3) & (sec != 6), k * 0.66, k)          # dark deck greebles
k = np.where(sec == 4, k * 0.82, k)                                   # engine section darker metal
k = np.where((sec == 1) | (sec == 2), k * 0.93, k)                    # hangar pods
k = np.where((sec == 3) & (up > 0.55), k * 1.06, k)                   # sunlit bridge deck
albedo = np.clip(base * k[:, None], 0, 1)
tint = np.array(srgb('868f8a')) / max(srgb('868f8a'))                  # olive-green cast on sides/undersides
albedo = np.where((up[:, None] < 0.55), albedo * tint, albedo)
albedo[silo_mask] = srgb(SILO_ORANGE)                                   # user art direction: orange VLS hatch plates, plain paint (not emissive)
non_hull = matidx != SLOT['Armor']
albedo[non_hull] = 1.0                                                   # other slots carry their own base colour
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
                   'intendedMovingMesh': 'SM_Daedalus_RailgunTurret (to be authored; not part of hull)',
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
                   'hatchPlateBoundsXY': [round(c, 2) for c in s['plate']], 'hatchPlateColour': '#' + SILO_ORANGE,
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
GROUP_EFFECTS = {'dorsal_railguns': 'orange tracer projectiles (user, from on-screen footage)',
                 'ventral_railguns': 'orange tracer projectiles (user, from on-screen footage)',
                 'bow_vls': 'missiles leave the dorsal bow silos upward (+Z), then turn to the target (user)',
                 'asgard_beams': 'continuous blue-white beam: near-white core with a blue halo (user stills U9, U10)',
                 'f302_bays': 'F-302 fighters exit along +X'}
for m in mounts: m['fireEffectHint'] = GROUP_EFFECTS[m['group']]
counters = {}
for m in mounts:
    key = {'dorsal_railguns': 'RG_D', 'ventral_railguns': 'RG_V', 'bow_vls': 'VLS', 'f302_bays': 'BAY', 'asgard_beams': 'ASG'}[m['group']]
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

# ---------------------------------------------------------------- 7. save, export, reopen checks
ship['source_creator'] = 'Astrofossil'; ship['source_url'] = 'https://www.thingiverse.com/thing:2256025'
ship['license'] = 'CC BY-NC 4.0'; ship['forward_axis'] = '+X'; ship['up_axis'] = '+Z'; ship['length_metres'] = 600.0
ship['detail_pass'] = 'task 0008: reference palette, AO, lights; exact Astrofossil geometry; baseline fittings removed or moved to lights'
scene.render.engine = 'BLENDER_EEVEE'
blend = OUT / 'Daedalus.blend'; glb = OUT / 'Daedalus.glb'; glow_glb = OUT / 'DaedalusEngineGlow.glb'; lights_glb = OUT / 'DaedalusLights.glb'
bpy.ops.wm.save_as_mainfile(filepath=str(blend), compress=True)
# glTF: constant base colour per slot + exported COLOR_0 (the Unreal parent multiplies them).
for m in mesh.materials:
    bs = m.node_tree.nodes.get('Principled BSDF')
    for link in list(bs.inputs['Base Color'].links): m.node_tree.links.remove(link)
    bs.inputs['Base Color'].default_value = (*srgb(m['base_hex']), 1)
bpy.ops.object.select_all(action='DESELECT'); ship.select_set(True); bpy.context.view_layer.objects.active = ship
bpy.ops.export_scene.gltf(filepath=str(glb), export_format='GLB', use_selection=True, export_vertex_color='ACTIVE',
                          export_materials='EXPORT', export_extras=True)
bpy.ops.object.select_all(action='DESELECT')
for o in glow_objs: o.select_set(True)
bpy.ops.export_scene.gltf(filepath=str(glow_glb), export_format='GLB', use_selection=True, export_extras=True)
bpy.ops.object.select_all(action='DESELECT'); bpy.data.objects['Daedalus_WindowLights'].select_set(True)
bpy.ops.export_scene.gltf(filepath=str(lights_glb), export_format='GLB', use_selection=True, export_extras=True)
bpy.ops.wm.open_mainfile(filepath=str(blend))
ship = bpy.data.objects['SM_Daedalus']
assert abs(ship.dimensions.x - 600) < .01
hull_meshes = [o for o in bpy.data.objects if o.type == 'MESH' and o.name not in bpy.data.collections['EngineGlow'].objects and o.name not in bpy.data.collections['HullLights'].objects]
assert [o.name for o in hull_meshes] == ['SM_Daedalus'], [o.name for o in hull_meshes]
check = {'blendReopen': True, 'dimensionsMetres': [round(c, 4) for c in ship.dimensions], 'materialSlots': [m.name for m in ship.data.materials],
         'colorAttributes': [c.name for c in ship.data.color_attributes]}
# Re-import exports into empty scenes.
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(glb))
ims = [o for o in bpy.context.scene.objects if o.type == 'MESH']
check['glbMeshes'] = [o.name for o in ims]; check['glbDimensions'] = [round(c, 3) for c in ims[0].dimensions]
check['glbTriangles'] = sum(len(o.data.polygons) for o in ims); check['glbColorAttributes'] = [c.name for c in ims[0].data.color_attributes]
check['glbMaterials'] = sorted({m.name for o in ims for m in o.data.materials})
assert len(ims) == 1 and abs(ims[0].dimensions.x - 600) < .05 and check['glbTriangles'] == 222330
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(lights_glb))
check['lightsObjects'] = sorted(o.name for o in bpy.context.scene.objects if o.type == 'MESH')
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(glow_glb))
gl = [o for o in bpy.context.scene.objects if o.type == 'MESH']
check['glowObjects'] = sorted(o.name for o in gl)
check['glowMaterials'] = sorted({m.name for o in gl for m in o.data.materials})

engine_manifest = FX['engine_manifest'](engines)
(OUT / 'ENGINE_MOUNTS.json').write_text(json.dumps(engine_manifest, indent=2) + '\n')
weapon_manifest = {
    'version': 1, 'units': 'metres', 'coordinateSystem': 'ship-local, +X forward, +Z up, origin = hull bounding-box centre (same as Daedalus.glb)',
    'method': 'ray-cast height maps (0.5 m grid) with top-hat (domes) / bottom-hat (recesses) detection on the unchanged hull',
    'axisConvention': 'forwardAxis = mount local +X (turret barrel rest / silo launch / bay exit); upAxis = local +Z (turret yaw axis); empties MOUNT_<id> in Daedalus.blend carry the same frame',
    'azimuthConvention': 'traverseFreeAzimuthDeg: degrees about the ship +Z, 0 = +X bow, 90 = +Y port; inclusive [start, end] ranges sampled every 5 deg at 5 deg elevation; a negative start wraps through 0 (e.g. [-295, 25] = 65..360 and 0..25)',
    'notes': ['Positions are measured modelled features, not canon proofs. Wiki lists 32 railguns and 16 VLS tubes; a fan sheet lists 26 twin railguns. The model contains its own dome count (see group counts).',
              'Turret mounts: centre = dome base on the hull; upAxis is the turret yaw axis; forwardAxis is the rest direction of the barrels.',
              'No turret/barrel geometry was added (user instruction: do not change the model). intendedMovingMesh names are proposals for later separate meshes.',
              'Asgard plasma beam weapons (4 per wiki): bow pair = outer dome on each lower side ledge (labelled on the SGA Tech Journal schematic); ventral pair = chamfered block fronts between the hangar pods (user, from stills; no dedicated emitter modelled, see zone and surfaceNormal).',
              'fireEffectHint describes the look of each weapon as reported by the user from on-screen footage; it is not a game rule.'],
    'groupCounts': {g: sum(1 for m in mounts if m['group'] == g) for g in sorted({m['group'] for m in mounts})},
    'unplacedReferenceSystems': [{'system': 'two long forward rods at the bow', 'count': 2, 'referenceURL': REF_DECK, 'reason': 'visible in stills but not modelled; no geometry is added (user instruction)'}],
    'mounts': mounts}
(OUT / 'WEAPON_MOUNTS.json').write_text(json.dumps(weapon_manifest, indent=2) + '\n')
report = {'sourceSHA256': hashlib.sha256(SOURCE.read_bytes()).hexdigest(), 'blendSHA256': hashlib.sha256(blend.read_bytes()).hexdigest(),
          'glbSHA256': hashlib.sha256(glb.read_bytes()).hexdigest(), 'engineGlowGlbSHA256': hashlib.sha256(glow_glb.read_bytes()).hexdigest(),
          'dimensionsMetres': check['dimensionsMetres'], 'originalTriangles': 222330, 'finalTriangles': FINAL['finalTriangles'],
          'vertices': FINAL['vertices'], 'originalAstrofossilGeometryChanged': False,
          'changesFromBaseline': 'hull is the exact Astrofossil geometry: the baseline hatch squares and vent slats (not in the original) were removed at the user request; baseline window cubes moved to the separate DaedalusLights.glb', 'materialSlots': check['materialSlots'],
          'vertexColour': 'COLOR_0 linear albedo x baked AO (hull slot base factor white; other slots carry base colour)',
          'fittings': stats, 'materialAssignment': assign_stats, 'engineOutlets': len(engines), 'mountGroups': weapon_manifest['groupCounts'],
          'exports': {'hull': 'Daedalus.glb', 'engineGlow': 'DaedalusEngineGlow.glb', 'windowLights': 'DaedalusLights.glb'}, 'lightsGlbSHA256': hashlib.sha256(lights_glb.read_bytes()).hexdigest(), 'checks': check,
          'coordinateMapping': 'source (x,y,z) -> Unreal/Blender (-z,-x,y)', 'detailScript': 'Tools/Prepare-DaedalusDetail.py',
          'baselineScript': 'Tools/Prepare-DaedalusSource.py (baseline only; do not run over this detailed source)'}
(OUT / 'asset-metadata.json').write_text(json.dumps(report, indent=2) + '\n')
print('DAEDALUS_DETAIL_PASS ' + json.dumps({'engines': len(engines), 'groups': weapon_manifest['groupCounts'], 'assign': assign_stats, 'check': check}))
