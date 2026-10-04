"""Import the reviewed glTF ship's PBR material recipe into owned UE assets.

Called by Prepare-SolarContent.py after mesh imports. This module never opens
an editor or imports/replaces geometry. Source colours and UVs stay unchanged.
"""
import json
import struct
from pathlib import Path

_texture_cache = {}
_material_cache = {}
MATERIAL_PATH = '/Game/Ships/Daedalus/Materials'
TEXTURE_PATH = '/Game/Ships/Daedalus/Textures'


def assign_materials(root, unreal, tools, lib, mesh, filename, beacon=False):
    """Assign saved source-faithful materials, retaining the full imported mesh."""
    root = Path(root)
    source = (root / 'Art/Ships/Daedalus' / filename).read_bytes()
    doc = json.loads(source[20:20 + struct.unpack_from('<I', source, 12)[0]])
    source_materials = {material['name']: material for material in doc['materials']}
    colour_materials = {
        doc['materials'][primitive['material']]['name']
        for source_mesh in doc['meshes']
        for primitive in source_mesh['primitives']
        if 'COLOR_0' in primitive.get('attributes', {})
    }

    def expression(material, cls, **properties):
        result = lib.create_material_expression(material, cls)
        for key, value in properties.items():
            result.set_editor_property(key, value)
        return result

    def scalar(material, value):
        return expression(material, unreal.MaterialExpressionConstant, r=value)

    def vector(material, value):
        return expression(material, unreal.MaterialExpressionConstant3Vector,
                          constant=unreal.LinearColor(*value))

    def multiply(material, left, right):
        result = expression(material, unreal.MaterialExpressionMultiply)
        assert lib.connect_material_expressions(left, '', result, 'A')
        assert lib.connect_material_expressions(right, '', result, 'B')
        return result

    def import_texture(info, role):
        assert info.get('texCoord', 0) == 0, 'Only reviewed UVMap/UV0 is supported'
        image = doc['images'][doc['textures'][info['index']]['source']]
        allowed_names = {
            'base': {'T_Daedalus_Plating_BaseColor', 'T_Daedalus_SiloHatch_Plain_BaseColor', 'T_Daedalus_SiloHatch_Striped_BaseColor'},
            'orm': {'T_Daedalus_Plating_ORM'},
            'normal': {'T_Daedalus_Plating_Normal', 'T_Daedalus_SiloHatch_Normal'},
        }
        name = image['name']
        assert name in allowed_names[role], (filename, role, image)
        key = (str(root.resolve()), name)
        if key in _texture_cache:
            return _texture_cache[key]
        task = unreal.AssetImportTask()
        task.filename = str(root / 'Art/Ships/Daedalus/Textures' / (name + '.png'))
        task.destination_path = TEXTURE_PATH
        task.destination_name = name
        task.automated = True
        task.replace_existing = True
        task.save = True
        tools.import_asset_tasks([task])
        texture = unreal.load_asset(TEXTURE_PATH + '/' + name)
        assert isinstance(texture, unreal.Texture2D), task.imported_object_paths
        texture.set_editor_property('srgb', role == 'base')
        texture.set_editor_property('compression_settings', {
            'base': unreal.TextureCompressionSettings.TC_DEFAULT,
            'orm': unreal.TextureCompressionSettings.TC_MASKS,
            'normal': unreal.TextureCompressionSettings.TC_NORMALMAP,
        }[role])
        # Blender/glTF tangent normals are OpenGL (+Y); UE expects DirectX (-Y).
        if role == 'normal':
            texture.set_editor_property('flip_green_channel', True)
        texture.set_editor_property('lod_bias', 0)
        assert unreal.EditorAssetLibrary.save_loaded_asset(texture)
        _texture_cache[key] = texture
        return texture

    def sample(material, info, role):
        return expression(material, unreal.MaterialExpressionTextureSample,
                          texture=import_texture(info, role), sampler_type={
                              'base': unreal.MaterialSamplerType.SAMPLERTYPE_COLOR,
                              'orm': unreal.MaterialSamplerType.SAMPLERTYPE_MASKS,
                              'normal': unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL,
                          }[role])

    def connect(material, node, output, property_name):
        assert lib.connect_material_property(node, output, property_name)

    for index, slot in enumerate(mesh.get_editor_property('static_materials')):
        name = str(slot.material_slot_name)
        assert name in source_materials, (filename, name, list(source_materials))
        src = source_materials[name]
        has_colour = name in colour_materials
        recipe = json.dumps([src, has_colour, beacon], sort_keys=True)
        asset_name = 'M_' + name + ('_' + Path(filename).stem if filename != 'Daedalus.glb' else '')
        key = (str(root.resolve()), asset_name)
        if key in _material_cache:
            material, previous_recipe = _material_cache[key]
            assert previous_recipe == recipe, 'Conflicting shared source material: ' + name
            mesh.set_material(index, material)
            continue
        material = unreal.load_asset(MATERIAL_PATH + '/' + asset_name)
        if material is None:
            material = tools.create_asset(asset_name, MATERIAL_PATH,
                                          unreal.Material, unreal.MaterialFactoryNew())
        assert isinstance(material, unreal.Material)
        # Do not delete rooted expressions loaded by CDOs in the commandlet.
        # Reconnect this recipe's outputs; within the commandlet reuse the graph.
        material.set_editor_property('two_sided', src.get('doubleSided', False))
        material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
        material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_OPAQUE)
        pbr = src.get('pbrMetallicRoughness', {})
        base = vector(material, tuple(pbr.get('baseColorFactor', [1, 1, 1, 1])))
        if 'baseColorTexture' in pbr:
            base = multiply(material, base, sample(material, pbr['baseColorTexture'], 'base'))
        if has_colour:
            base = multiply(material, base, expression(material, unreal.MaterialExpressionVertexColor))
        connect(material, base, '', unreal.MaterialProperty.MP_BASE_COLOR)
        metallic = scalar(material, pbr.get('metallicFactor', 1))
        roughness = scalar(material, pbr.get('roughnessFactor', 1))
        if 'metallicRoughnessTexture' in pbr:
            orm = sample(material, pbr['metallicRoughnessTexture'], 'orm')
            for channel, factor, output in [
                ('B', metallic, unreal.MaterialProperty.MP_METALLIC),
                ('G', roughness, unreal.MaterialProperty.MP_ROUGHNESS),
            ]:
                product = expression(material, unreal.MaterialExpressionMultiply)
                assert lib.connect_material_expressions(orm, channel, product, 'A')
                assert lib.connect_material_expressions(factor, '', product, 'B')
                connect(material, product, '', output)
            connect(material, orm, 'R', unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
        else:
            connect(material, metallic, '', unreal.MaterialProperty.MP_METALLIC)
            connect(material, roughness, '', unreal.MaterialProperty.MP_ROUGHNESS)
        if 'normalTexture' in src:
            normal_info = src['normalTexture']
            assert normal_info.get('scale', 1) == 1, 'Review non-unit normal scale before import'
            connect(material, sample(material, normal_info, 'normal'), 'RGB', unreal.MaterialProperty.MP_NORMAL)
        strength = src.get('extensions', {}).get('KHR_materials_emissive_strength', {}).get('emissiveStrength', 1)
        emission = [value * strength for value in src.get('emissiveFactor', [0, 0, 0])]
        emit = vector(material, (*emission, 1))
        if beacon:
            # Source blink extras become a runtime parameter, not a baked Blender timeline.
            level = expression(material, unreal.MaterialExpressionScalarParameter,
                               parameter_name='BeaconLevel', default_value=0)
            emit = multiply(material, base, level)
        connect(material, emit, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        lib.layout_material_expressions(material)
        lib.recompile_material(material)
        assert unreal.EditorAssetLibrary.save_loaded_asset(material)
        _material_cache[key] = (material, recipe)
        mesh.set_material(index, material)

    nanite = mesh.get_editor_property('nanite_settings')
    nanite.set_editor_property('enabled', False)
    mesh.set_editor_property('nanite_settings', nanite)
    assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    return len(mesh.get_editor_property('static_materials'))
