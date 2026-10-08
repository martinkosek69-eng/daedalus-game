"""Read-only Unreal commandlet check of every planet-quality hookup.

UnrealEditor-Cmd <uproject> -run=pythonscript -script=Tools/Validate-PlanetQuality.py -unattended -nullrhi
Checks masters, every placed textured body instance (parent, bound sources, encoding, group),
cloud layers, relief, night maps, map discs, finer spheres and the retired legacy Earth imports.
Prints one PLANET_BINDING line per body (stable, comparable between repeated imports) and
PLANET_QUALITY_PASS. Never imports, saves or edits assets.
"""
import importlib.util
import json
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve().parent.parent


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, root / path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


system = module('SolarMaterialSources', 'Tools/Prepare-SolarSystemMaterials.py')
planet = module('PlanetQualityRecipe', 'Tools/Prepare-PlanetMaterials.py')
lib = unreal.MaterialEditingLibrary
manifest = planet.load_manifest(root, {row['id'] for row in system.bodies(root)})
TC = unreal.TextureCompressionSettings
GROUP = unreal.TextureGroup.TEXTUREGROUP_PROJECT01


def source_of(texture):
    data = texture.get_editor_property('asset_import_data')
    files = [Path(p).resolve() for p in data.extract_filenames()] if data else []
    assert len(files) == 1, (texture.get_path_name(), files)
    return files[0]


def check_texture(texture, kind, expected_source):
    assert isinstance(texture, unreal.Texture2D), texture
    assert source_of(texture) == expected_source.resolve(), ('wrong source', texture.get_path_name(), str(source_of(texture)), str(expected_source))
    compression = texture.get_editor_property('compression_settings')
    assert compression == (TC.TC_ALPHA if kind == 'mask' else TC.TC_BC7), (texture.get_path_name(), compression)
    assert texture.get_editor_property('srgb') == (kind == 'color'), (texture.get_path_name(), 'sRGB')
    assert texture.get_editor_property('lod_group') == GROUP, (texture.get_path_name(), 'group')
    assert texture.get_editor_property('max_texture_size') == 0, texture.get_path_name()
    return dict(path=texture.get_path_name().split('.')[0], size=[texture.blueprint_get_size_x(), texture.blueprint_get_size_y()])


masters = {}
for kind, name in planet.MASTER_NAMES.items():
    mat = unreal.load_asset(planet.MASTERS + '/' + name)
    assert isinstance(mat, unreal.Material), name
    assert mat.get_editor_property('shading_model') == unreal.MaterialShadingModel.MSM_UNLIT, name
    blend = unreal.BlendMode.BLEND_TRANSLUCENT if kind == 'clouds' else unreal.BlendMode.BLEND_OPAQUE
    assert mat.get_editor_property('blend_mode') == blend, name
    scalars = set(str(n) for n in lib.get_scalar_parameter_names(mat))
    textures = set(str(n) for n in lib.get_texture_parameter_names(mat))
    assert set(planet.TEXTURE_PARAMS[kind]) <= textures, (name, textures)
    expected_scalars = set(planet.SCALARS[kind]) if kind != 'clouds' else {'MeshUV', 'CloudSize', 'CloudDetail', 'CloudOpacity', 'Brightness', 'Ambient'}
    assert expected_scalars <= scalars, (name, expected_scalars - scalars)
    masters[kind] = mat

checked, clouds, reliefs, nights = 0, 0, 0, 0
for row in system.bodies(root):
    if not system.has_geometry(row) or not (row.get('texture') or row.get('textureSource')):
        continue
    if not planet.uses_planet_quality(row, manifest):
        continue
    key = system.material_key(row['id'])
    kind = planet.body_type(row)
    sources = system.planet_sources(root, row, manifest)
    mi = unreal.load_asset('/Game/Solar/Materials/M_Body_' + key)
    assert isinstance(mi, unreal.MaterialInstanceConstant), (row['id'], mi)
    assert mi.get_editor_property('parent') == masters[kind], (row['id'], 'parent')
    assert row.get('material', '/Game/Solar/Materials/M_Body_' + key) == '/Game/Solar/Materials/M_Body_' + key, row['id']
    binding = dict(id=row['id'], type=kind, day=check_texture(lib.get_material_instance_texture_parameter_value(mi, 'Day'), 'color', sources['day']['source']))
    if sources['night'] and kind != 'star':
        binding['night'] = check_texture(lib.get_material_instance_texture_parameter_value(mi, 'Night'), 'color', sources['night']['source'])
        assert lib.get_material_instance_scalar_parameter_value(mi, 'NightStrength') > 0, row['id']
        nights += 1
    if sources['relief']:
        assert kind == 'surface', ('relief only on solid surfaces', row['id'])
        binding['relief'] = check_texture(lib.get_material_instance_texture_parameter_value(mi, 'Relief'), 'mask', sources['relief']['source'])
        assert lib.get_material_instance_scalar_parameter_value(mi, 'ReliefScale') > 0, row['id']
        reliefs += 1
    cloud_mi = unreal.load_asset('/Game/Solar/Materials/M_Cloud_' + key)
    if sources['clouds'] and kind == 'surface':
        assert isinstance(cloud_mi, unreal.MaterialInstanceConstant) and cloud_mi.get_editor_property('parent') == masters['clouds'], row['id']
        layer = check_texture(lib.get_material_instance_texture_parameter_value(cloud_mi, 'Clouds'), 'mask', sources['clouds']['source'])
        shadow = lib.get_material_instance_texture_parameter_value(mi, 'Clouds')
        assert shadow is not None and shadow.get_path_name().split('.')[0] == layer['path'], ('cloud shadow uses the layer map', row['id'])
        assert lib.get_material_instance_scalar_parameter_value(mi, 'CloudShadow') > 0, row['id']
        binding['clouds'] = layer
        clouds += 1
    else:
        assert cloud_mi is None, ('cloud layer on a body type without clouds', row['id'])
    # The sharp map disc samples the same day texture.
    disc = unreal.load_asset(row.get('mapMaterial', '/Game/Solar/Materials/M_Map_' + key))
    assert isinstance(disc, unreal.Material), row['id']
    pending, seen, found = [lib.get_material_property_input_node(disc, unreal.MaterialProperty.MP_EMISSIVE_COLOR)], set(), set()
    while pending:
        expr = pending.pop()
        if not expr or expr.get_path_name() in seen:
            continue
        seen.add(expr.get_path_name())
        pending.extend(lib.get_inputs_for_material_expression(disc, expr))
        if isinstance(expr, unreal.MaterialExpressionTextureSample):
            found.add(expr.get_editor_property('texture').get_path_name().split('.')[0])
    assert binding['day']['path'] in found, ('map disc does not use the planet day map', row['id'], found)
    print('PLANET_BINDING ' + json.dumps(binding, sort_keys=True))
    checked += 1

for name in planet.SPHERES:
    mesh = unreal.load_asset(planet.SPHERE_FOLDER + '/' + name)
    assert isinstance(mesh, unreal.StaticMesh) and abs(mesh.get_bounds().box_extent.x - 100) < .01, name
    assert not mesh.get_editor_property('nanite_settings').enabled, name
for name in ('T_earth_daymap', 'T_earth_nightmap', 'T_earth_clouds'):
    old = unreal.load_asset('/Game/Solar/Textures/' + name)
    assert old is None or old.get_editor_property('max_texture_size') == 64, name
config = (root / 'Game/Daedalus/Config/DefaultDeviceProfiles.ini').read_text(encoding='utf-8')
assert 'Group=TEXTUREGROUP_Project01' in config and 'MipFilter=linear' in config and 'MaxLODSize=16384' in config
print('PLANET_QUALITY_PASS', checked, 'bodies;', clouds, 'cloud layers;', reliefs, 'relief maps;', nights, 'night maps; masters', len(masters))
