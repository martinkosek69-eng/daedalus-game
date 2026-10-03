"""Daedalus engine outlet measurement and small throttle-ready glow meshes (task 0008).

Library used by Tools/Prepare-DaedalusDetail.py; it can also run alone on an
existing detailed source to rebuild only the glow export:
  blender --background --factory-startup --python Tools/Prepare-DaedalusEngineEffects.py
Outlets are measured from the hull mesh with ray-cast depth maps (the source is
triangle soup, so topology cannot be used). Units: metres, +X forward, +Z up.
"""
import bpy, math, json
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix
from mathutils.bvhtree import BVHTree
from numpy.lib.stride_tricks import sliding_window_view as swv

GLOW_HEX = 'ffd394'   # warm white-orange core, from rear stills (yellow-white centre, orange halo)
PLUME_HEX = 'ff9a4a'


def depth_map(bvh, axis, sign, a_rng, b_rng, step, start):
    """Rays along `sign` * axis. Returns grid coords and height toward the camera."""
    other = [i for i in range(3) if i != axis]
    A = np.arange(a_rng[0], a_rng[1], step); B = np.arange(b_rng[0], b_rng[1], step)
    H = np.full((len(A), len(B)), np.nan, np.float32)
    d = [0, 0, 0]; d[axis] = sign; dv = Vector(d)
    for i, a in enumerate(A):
        for j, b in enumerate(B):
            o = [0, 0, 0]; o[axis] = start; o[other[0]] = a; o[other[1]] = b
            hit = bvh.ray_cast(Vector(o), dv, 4000)[0]
            if hit is not None: H[i, j] = -sign * hit[axis]
    return A, B, H


def _minf(x, r):
    p = np.pad(x, ((r, r), (0, 0)), mode='edge'); x = swv(p, 2 * r + 1, axis=0).min(-1)
    p = np.pad(x, ((0, 0), (r, r)), mode='edge'); return swv(p, 2 * r + 1, axis=1).min(-1)


def _maxf(x, r): return -_minf(-x, r)


def _label(mask):
    lab = np.zeros(mask.shape, np.int32); n = 0
    for i, j in zip(*np.nonzero(mask)):
        if lab[i, j]: continue
        n += 1; stack = [(i, j)]; lab[i, j] = n
        while stack:
            a, b = stack.pop()
            for da, db in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                c, e = a + da, b + db
                if 0 <= c < mask.shape[0] and 0 <= e < mask.shape[1] and mask[c, e] and not lab[c, e]:
                    lab[c, e] = n; stack.append((c, e))
    return lab, n


def blobs(H, A, B, step, r, mode, thr, dmin, dmax):
    """'bump' = top-hat (H - opening); 'pit' = bottom-hat (closing - H). Returns region stats."""
    h = np.where(np.isnan(H), np.nanmin(H) - 50, H)
    t = h - _maxf(_minf(h, r), r) if mode == 'bump' else _minf(_maxf(h, r), r) - h
    lab, n = _label(t > thr); out = []
    for k in range(1, n + 1):
        ii, jj = np.nonzero(lab == k); area = len(ii) * step * step
        d = 2 * math.sqrt(area / math.pi)
        if not (dmin <= d <= dmax): continue
        ea, eb = (ii.max() - ii.min() + 1) * step, (jj.max() - jj.min() + 1) * step
        surf = float(np.nanmax(H[ii, jj])) if mode == 'bump' else float(np.nanmedian(H[ii, jj]))
        out.append({'a': float(A[ii].mean()), 'b': float(B[jj].mean()), 'aMin': float(A[ii].min()), 'aMax': float(A[ii].max()),
                    'bMin': float(B[jj].min()), 'bMax': float(B[jj].max()), 'diam': d, 'extentA': ea, 'extentB': eb,
                    'fill': area / (ea * eb), 'relief': float(t[ii, jj].max()), 'surface': surf})
    return out


def measure_engines(bvh):
    """Find circular rear outlets, then refine each with a radial depth profile."""
    A, B, H = depth_map(bvh, 0, +1, (-192, 192), (-48, 48), 0.5, -500)
    pits = [p for p in blobs(H, A, B, 0.5, 40, 'pit', 1.5, 6, 45) if p['fill'] > 0.7]
    engines = []
    for p in pits:
        y0, z0 = p['a'], p['b']; rmax = p['diam'] * 0.95
        radii = np.arange(0, rmax, 0.25); prof = []
        for r in radii:
            xs = []
            for k in range(24):
                th = 2 * math.pi * k / 24
                hit = bvh.ray_cast(Vector((-500, y0 + r * math.cos(th), z0 + r * math.sin(th))), Vector((1, 0, 0)), 4000)[0]
                if hit is not None: xs.append(hit.x)
            prof.append(np.mean(xs) if xs else np.nan)
        prof = np.array(prof); lip_i = int(np.nanargmin(prof)); lip_x = float(prof[lip_i])
        on_lip = np.nonzero(prof <= lip_x + 0.6)[0]
        inner = float(radii[on_lip.min()]); outer = float(radii[on_lip.max()])
        core_x = float(np.nanmax(prof[:max(1, on_lip.min())]))
        engines.append({'y': y0, 'z': z0, 'lipX': lip_x, 'coreX': core_x, 'apertureRadius': inner,
                        'lipOuterRadius': outer, 'recessDepth': core_x - lip_x})
    engines.sort(key=lambda e: (-e['y']))
    return engines


def name_engines(engines):
    for e in engines:
        side = 'Port' if e['y'] > 0 else 'Starboard'
        if abs(e['y']) < 80: e['id'] = f'ENG_Main_{side}'
        else: e['id'] = f'ENG_Pod_{side}_' + ('Outer' if abs(e['y']) > 140 else 'Inner')
    return engines


def srgb_lin(h):
    q = [int(h[i:i + 2], 16) / 255 for i in (0, 2, 4)]
    return tuple(c / 12.92 if c <= .04045 else ((c + .055) / 1.055) ** 2.4 for c in q)


def glow_materials():
    core = bpy.data.materials.get('Daedalus_EngineGlowCore') or bpy.data.materials.new('Daedalus_EngineGlowCore')
    core.use_nodes = True; b = core.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*srgb_lin(GLOW_HEX), 1)
    b.inputs['Emission Color'].default_value = (*srgb_lin(GLOW_HEX), 1); b.inputs['Emission Strength'].default_value = 6.0
    b.inputs['Roughness'].default_value = 1.0
    plume = bpy.data.materials.get('Daedalus_EngineGlowPlume') or bpy.data.materials.new('Daedalus_EngineGlowPlume')
    plume.use_nodes = True; b = plume.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*srgb_lin(PLUME_HEX), 1)
    b.inputs['Emission Color'].default_value = (*srgb_lin(PLUME_HEX), 1); b.inputs['Emission Strength'].default_value = 0.9
    b.inputs['Alpha'].default_value = 0.16; plume.surface_render_method = 'BLENDED'
    return core, plume


def build_glow(engines, collection):
    """Per outlet: emissive core disc just inside the lip + short translucent haze (0.35 x aperture radius).
    Object origin = outlet centre on the lip plane; local -X = exhaust direction, so later throttle can
    scale X (plume length) and emission without touching the hull."""
    core, plume = glow_materials(); objs = []
    for e in engines:
        r = e['apertureRadius']; seg = 48
        verts = [(0.25 * min(e['recessDepth'], 3.0), 0, 0)] + [(0.25 * min(e['recessDepth'], 3.0), r * .96 * math.cos(2 * math.pi * k / seg), r * .96 * math.sin(2 * math.pi * k / seg)) for k in range(seg)]
        faces = [(0, 1 + k, 1 + (k + 1) % seg) for k in range(seg)]
        L = 0.35 * r; tipr = 0.75 * r; base = len(verts)
        verts += [(0.0, r * .9 * math.cos(2 * math.pi * k / seg), r * .9 * math.sin(2 * math.pi * k / seg)) for k in range(seg)]
        verts += [(-L, tipr * math.cos(2 * math.pi * k / seg), tipr * math.sin(2 * math.pi * k / seg)) for k in range(seg)]
        cap = len(verts); verts.append((-L, 0, 0))
        pf = [(base + k, base + (k + 1) % seg, base + seg + (k + 1) % seg, base + seg + k) for k in range(seg)]
        pf += [(base + seg + k, base + seg + (k + 1) % seg, cap) for k in range(seg)]
        me = bpy.data.meshes.new(e['id'] + '_Glow'); me.from_pydata(verts, [], faces + pf); me.update()
        me.materials.append(core); me.materials.append(plume)
        for i, poly in enumerate(me.polygons): poly.material_index = 0 if i < len(faces) else 1
        ob = bpy.data.objects.new(e['id'] + '_Glow', me); collection.objects.link(ob)
        ob.location = (e['lipX'], e['y'], e['z'])
        ob['outlet_id'] = e['id']; ob['exhaust_axis'] = '-X'; ob['throttle_hint'] = 'scale local X for plume length; scale emission for brightness'
        objs.append(ob)
    return objs


def engine_manifest(engines):
    return {'version': 1, 'units': 'metres', 'coordinateSystem': 'ship-local, +X forward, +Z up, origin = hull bounding-box centre',
            'method': 'ray-cast depth map from behind; radial profile of 24 rays per radius; lip = most rearward ring',
            'glowExport': 'DaedalusEngineGlow.glb (objects <id>_Glow; origin at outlet centre, exhaust along local -X)',
            'outlets': [{'id': e['id'], 'centreMetres': [round(e['lipX'], 3), round(e['y'], 3), round(e['z'], 3)],
                         'outwardAxis': [-1, 0, 0], 'upAxis': [0, 0, 1],
                         'apertureDiameterMetres': round(2 * e['apertureRadius'], 2),
                         'lipOuterDiameterMetres': round(2 * e['lipOuterRadius'], 2),
                         'recessDepthMetres': round(e['recessDepth'], 2),
                         'glowObject': e['id'] + '_Glow'} for e in engines]}


if __name__ == '__main__':
    ROOT = Path(__file__).resolve().parent.parent
    OUT = ROOT / 'Art/Ships/Daedalus'
    bpy.ops.wm.open_mainfile(filepath=str(OUT / 'Daedalus.blend'))
    ship = bpy.data.objects['SM_Daedalus']
    bvh = BVHTree.FromObject(ship, bpy.context.evaluated_depsgraph_get())
    engines = name_engines(measure_engines(bvh))
    col = bpy.data.collections.get('EngineGlow')
    if col:
        for o in list(col.objects): bpy.data.objects.remove(o)
    else:
        col = bpy.data.collections.new('EngineGlow'); bpy.context.scene.collection.children.link(col)
    objs = build_glow(engines, col)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / 'Daedalus.blend'), compress=True)
    bpy.ops.object.select_all(action='DESELECT')
    for o in objs: o.select_set(True)
    bpy.ops.export_scene.gltf(filepath=str(OUT / 'DaedalusEngineGlow.glb'), export_format='GLB', use_selection=True, export_extras=True)
    (OUT / 'ENGINE_MOUNTS.json').write_text(json.dumps(engine_manifest(engines), indent=2) + '\n')
    print('ENGINE_EFFECTS_PASS', len(engines))
