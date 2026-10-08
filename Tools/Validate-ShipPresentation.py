"""Read-only Unreal check of the integrated 0008 ship and animation dependencies."""
import hashlib
import json
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve().parent.parent
source = root / 'Art/Ships/Daedalus'
manifest = json.loads((root / 'Game/Daedalus/Content/Data/Solar/ship-details.json').read_text())
assert manifest['version'] == 2
assert len(manifest['meshes']) == 2 and len(manifest['doors']) == 4
assert len(manifest['pointLights']) == 10
for filename, expected in manifest['sources'].items():
    assert hashlib.sha256((source / filename).read_bytes()).hexdigest() == expected, filename
metadata = json.loads((source / 'asset-metadata.json').read_text())
assert manifest['sources']['Daedalus.glb'] == metadata['glbSHA256']
hull = unreal.load_asset('/Game/Ships/Daedalus/SM_Daedalus')
assert isinstance(hull, unreal.StaticMesh)
assert abs(hull.get_bounds().box_extent.x * 2 - 60000) < 30
assert len(hull.get_editor_property('static_materials')) == 5
assets = manifest['meshes'] + [d['mesh'] for d in manifest['doors']]
assets += [manifest['windowMesh'], manifest['beacon']['mesh']]
for path in assets:
    mesh = unreal.load_asset(path)
    assert isinstance(mesh, unreal.StaticMesh), path
    assert not mesh.get_editor_property('nanite_settings').enabled, path
    assert mesh.get_num_lods() == 1, path
    for slot in mesh.get_editor_property('static_materials'):
        material = slot.material_interface
        assert isinstance(material, unreal.Material), (path, slot.material_slot_name)
        assert material.get_path_name().startswith(('/Game/Ships/Daedalus/Materials/', '/Game/Solar/Materials/')), material.get_path_name()
    assert 'Turret_' not in path, path
for door in manifest['doors']:
    mesh = unreal.load_asset(door['mesh'])
    centre = mesh.get_bounds().origin
    # Door vertices already live in the complete hull frame. Runtime adds only
    # the opening offset; no second placement using node translation.
    expected_y = -13420.074 if door['bay'] == 'P' else 13420.076
    assert abs(centre.y - expected_y) < 1000, (door, centre)
    assert door['openOffsetCentimetres'] == [0, 0, 560 if door['half'] == 'upper' else -560]
assert manifest['beacon']['periodSeconds'] == 5
assert abs(manifest['beacon']['onSeconds'] - 1 / 3) < 1e-9
beacon = unreal.load_asset(manifest['beacon']['mesh']).get_material(0)
assert 'BeaconLevel' in [str(n) for n in unreal.MaterialEditingLibrary.get_scalar_parameter_names(beacon)]
for name in ('T_Daedalus_Plating_Normal', 'T_Daedalus_SiloHatch_Normal'):
    tex = unreal.load_asset('/Game/Ships/Daedalus/Textures/' + name)
    assert tex.get_editor_property('flip_green_channel') and not tex.get_editor_property('srgb'), name
for name in ('T_Daedalus_Plating_BaseColor', 'T_Daedalus_SiloHatch_Plain_BaseColor', 'T_Daedalus_SiloHatch_Striped_BaseColor'):
    tex = unreal.load_asset('/Game/Ships/Daedalus/Textures/' + name)
    assert tex.get_editor_property('srgb') and tex.get_editor_property('lod_bias') == 0, name
print('SHIP_INTEGRATION_PASS latest source SHA, 600m, materials, four doors, blink, ten point lights')
