"""Read-only Unreal commandlet check of imported world/map source dependencies.

Run through UnrealEditor-Cmd -run=pythonscript -script=<this file> -nullrhi.
Does not import, save, change a scene, or modify worker files.
"""
import importlib.util
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve().parent.parent
spec = importlib.util.spec_from_file_location('SolarMaterialSources', root / 'Tools/Prepare-SolarSystemMaterials.py')
recipe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(recipe)
planet_manifest = recipe.planet_module(root).load_manifest(root)
checked = 0
for row in recipe.bodies(root):
    if not recipe.has_geometry(row) or not (row.get('texture') or row.get('textureSource')):
        continue
    source = recipe.texture_source(root, row.get('texture'), row.get('textureSource')).resolve()
    for field in ('material', 'mapMaterial'):
        mat = unreal.load_asset(row[field])
        if isinstance(mat, unreal.MaterialInstanceConstant):
            # Planet-quality instance (Tools/Validate-PlanetQuality.py checks it in full): its Day map
            # must be the canonical surface unless planets.json names a prepared replacement.
            texture = unreal.MaterialEditingLibrary.get_material_instance_texture_parameter_value(mat, 'Day')
            data = texture.get_editor_property('asset_import_data')
            imported_sources = {Path(p).resolve() for p in data.extract_filenames()}
            replacement = planet_manifest['bodies'].get(row['id'], {}).get('day')
            assert (root / replacement).resolve() in imported_sources if replacement else source in imported_sources, (row['id'], field, imported_sources)
            checked += 1
            continue
        assert isinstance(mat, unreal.Material), (row['id'], field)
        imported_sources = set()
        # Walk the connected output graph, including Custom HLSL inputs. A
        # commandlet's uncompiled RHI resource may report no used textures.
        pending = [unreal.MaterialEditingLibrary.get_material_property_input_node(mat, unreal.MaterialProperty.MP_EMISSIVE_COLOR)]
        visited = set()
        while pending:
            node = pending.pop()
            if not node or node.get_path_name() in visited:
                continue
            visited.add(node.get_path_name())
            pending.extend(unreal.MaterialEditingLibrary.get_inputs_for_material_expression(mat, node))
            if isinstance(node, unreal.MaterialExpressionTextureSample):
                texture = node.get_editor_property('texture')
                assert isinstance(texture, unreal.Texture2D)
                data = texture.get_editor_property('asset_import_data')
                if data:
                    imported_sources.update(Path(p).resolve() for p in data.extract_filenames())
        replacement = planet_manifest['bodies'].get(row['id'], {}).get('day')
        expected = (root / replacement).resolve() if replacement else source
        assert expected in imported_sources, ('Material does not use canonical surface', row['id'], field, str(expected), [str(p) for p in imported_sources])
        checked += 1
sphere = unreal.load_asset('/Game/Solar/SM_SolarSphere')
assert isinstance(sphere, unreal.StaticMesh)
slots = len(sphere.get_editor_property('static_materials'))
assert slots >= 1, 'Sphere must expose a material slot for body presentation'
print('SOLAR_IMPORTED_SOURCES_PASS', checked, 'world/map bindings; sphere slots', slots)
