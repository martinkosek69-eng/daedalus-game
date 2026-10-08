"""Owned UE commandlet, independent Aurora recipe: no planet/Daedalus imports."""
import unreal, json, struct, hashlib
from pathlib import Path

def main():
    root=Path(unreal.Paths.project_dir()).resolve().parent.parent
    source=root/'Art/Ships/Aurora'; raw=(source/'Aurora.glb').read_bytes()
    doc=json.loads(raw[20:20+struct.unpack_from('<I',raw,12)[0]])
    fingerprint=hashlib.sha256(raw+Path(__file__).read_bytes())
    for file in sorted((source/'Textures').glob('T_Aurora_*.png')):fingerprint.update(file.read_bytes())
    recipe=fingerprint.hexdigest(); folder='/Game/Ships/Aurora'
    mesh=unreal.load_asset(folder+'/SM_Aurora')
    if mesh and unreal.EditorAssetLibrary.get_metadata_tag(mesh,'AuroraRecipe')==recipe:
        print('AURORA_CONTENT_PASS cached');return
    tools=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
    def imported(file,path,name,cls):
        task=unreal.AssetImportTask();task.filename=str(file);task.destination_path=path;task.destination_name=name
        task.automated=True;task.replace_existing=True;task.save=True;tools.import_asset_tasks([task])
        assets=[unreal.load_asset(p) for p in task.imported_object_paths]
        selected=[a for a in assets if isinstance(a,cls)];assert len(selected)==1,(file,task.imported_object_paths)
        asset=selected[0];desired=path+'/'+name
        if asset.get_path_name().split('.')[0]!=desired:
            assert not unreal.EditorAssetLibrary.does_asset_exist(desired),'Ambiguous overwrite'
            assert unreal.EditorAssetLibrary.rename_asset(asset.get_path_name(),desired)
        return asset
    mesh=imported(source/'Aurora.glb',folder,'SM_Aurora',unreal.StaticMesh)
    assert abs(mesh.get_bounds().box_extent.x*2-350000)<50,mesh.get_bounds()
    textures={}
    for role in ['BaseColor','ORM','Normal','Emission']:
        name='T_Aurora_'+role;texture=imported(source/'Textures'/(name+'.png'),folder+'/Textures',name,unreal.Texture2D)
        texture.set_editor_property('srgb',role in ('BaseColor','Emission'))
        texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP if role=='Normal' else unreal.TextureCompressionSettings.TC_MASKS if role=='ORM' else unreal.TextureCompressionSettings.TC_DEFAULT)
        if role=='Normal':texture.set_editor_property('flip_green_channel',True)
        texture.set_editor_property('lod_bias',0);texture.set_editor_property('never_stream',True)
        texture.set_editor_property('max_texture_size',8192 if role=='BaseColor' else 4096)
        texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_PROJECT02)
        unreal.EditorAssetLibrary.save_loaded_asset(texture);textures[role]=texture
    def node(m,cls,**kw):
        n=lib.create_material_expression(m,cls)
        for k,v in kw.items():n.set_editor_property(k,v)
        return n
    def sample(m,role):
        return node(m,unreal.MaterialExpressionTextureSample,texture=textures[role],sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if role=='Normal' else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS if role=='ORM' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
    def constant(m,v):return node(m,unreal.MaterialExpressionConstant,r=v)
    def vector(m,v):return node(m,unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(*v,1))
    sources={m['name']:m for m in doc['materials']}
    for slot,s in enumerate(mesh.get_editor_property('static_materials')):
        name=str(s.material_slot_name);assert name in sources,name;src=sources[name];pbr=src.get('pbrMetallicRoughness',{})
        m=unreal.load_asset(folder+'/Materials/'+name) or tools.create_asset(name,folder+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
        m.set_editor_property('two_sided',False);m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_OPAQUE)
        m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
        if 'baseColorTexture' in pbr:
            for role,prop,channel in [('BaseColor',unreal.MaterialProperty.MP_BASE_COLOR,'RGB'),('Normal',unreal.MaterialProperty.MP_NORMAL,'RGB'),('Emission',unreal.MaterialProperty.MP_EMISSIVE_COLOR,'RGB')]:
                assert lib.connect_material_property(sample(m,role),channel,prop)
            orm=sample(m,'ORM')
            for channel,prop in [('G',unreal.MaterialProperty.MP_ROUGHNESS),('B',unreal.MaterialProperty.MP_METALLIC)]:assert lib.connect_material_property(orm,channel,prop)
        else:
            assert lib.connect_material_property(vector(m,pbr.get('baseColorFactor',[1,1,1,1])[:3]),'',unreal.MaterialProperty.MP_BASE_COLOR)
            strength=src.get('extensions',{}).get('KHR_materials_emissive_strength',{}).get('emissiveStrength',1)
            assert lib.connect_material_property(vector(m,[x*strength for x in src.get('emissiveFactor',[0,0,0])]),'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            for v,prop in [(pbr.get('roughnessFactor',1),unreal.MaterialProperty.MP_ROUGHNESS),(pbr.get('metallicFactor',1),unreal.MaterialProperty.MP_METALLIC)]:assert lib.connect_material_property(constant(m,v),'',prop)
        lib.layout_material_expressions(m);lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m);mesh.set_material(slot,m)
    ns=mesh.get_editor_property('nanite_settings');ns.set_editor_property('enabled',False);mesh.set_editor_property('nanite_settings',ns)
    unreal.EditorAssetLibrary.set_metadata_tag(mesh,'AuroraRecipe',recipe)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    print('AURORA_CONTENT_PASS',mesh.get_path_name(),len(mesh.get_editor_property('static_materials')))
if __name__=='__main__':main()
