"""Owned Unreal commandlet: source import and repeatable solar content recipe."""
import unreal, json, struct, hashlib
from pathlib import Path

def main():
    base=Path(unreal.Paths.project_dir()).resolve().parent.parent
    fingerprint=hashlib.sha256(Path(__file__).read_bytes())
    for source in [base/'Art/Ships/Daedalus/Daedalus.glb',base/'Art/Space/SolarSphere.glb']+sorted((base/'Art/Space/Textures').glob('*.jpg')):
        fingerprint.update(source.read_bytes())
    recipe=fingerprint.hexdigest()
    existing=unreal.load_asset('/Game/Ships/Daedalus/SM_Daedalus')
    required=['M_Earth','M_Sun','M_Atmosphere','M_Star','M_Dust']
    if existing and unreal.EditorAssetLibrary.get_metadata_tag(existing,'SolarRecipe')==recipe and all(unreal.EditorAssetLibrary.does_asset_exist('/Game/Solar/Materials/'+name) for name in required) and unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/SolarFlight'):
        print('SOLAR_CONTENT_PASS cached verified recipe')
        return
    root=Path(unreal.Paths.project_dir()).resolve().parent.parent
    tools=unreal.AssetToolsHelpers.get_asset_tools()
    lib=unreal.MaterialEditingLibrary
    
    def imported(source,path,name,typ):
        task=unreal.AssetImportTask()
        task.filename=str(source);task.destination_path=path;task.destination_name=name
        task.automated=True;task.replace_existing=True;task.save=True
        tools.import_asset_tasks([task])
        assets=[unreal.load_asset(p) for p in task.imported_object_paths]
        selected=[a for a in assets if isinstance(a,typ)]
        assert len(selected)==1,(source,task.imported_object_paths)
        asset=selected[0]
        desired=path+'/'+name
        if asset.get_path_name().split('.')[0]!=desired:
            assert not unreal.EditorAssetLibrary.does_asset_exist(desired),'Refuse ambiguous target overwrite'
            assert unreal.EditorAssetLibrary.rename_asset(asset.get_path_name(),desired)
        return asset
    
    ship=imported(root/'Art/Ships/Daedalus/Daedalus.glb','/Game/Ships/Daedalus','SM_Daedalus',unreal.StaticMesh)
    assert abs(ship.get_bounds().box_extent.x*2-60000)<30,ship.get_bounds()
    sphere=imported(root/'Art/Space/SolarSphere.glb','/Game/Solar','SM_SolarSphere',unreal.StaticMesh)
    assert abs(sphere.get_bounds().box_extent.x-100)<.01
    # Preserve the full smooth sphere. Nanite fallback simplification makes the
    # astronomical silhouette visibly polygonal and cannot render additive air.
    nanite=sphere.get_editor_property('nanite_settings')
    nanite.set_editor_property('enabled',False)
    sphere.set_editor_property('nanite_settings',nanite)
    assert unreal.EditorAssetLibrary.save_loaded_asset(sphere)
    textures={}
    for name in ['earth_daymap','earth_nightmap','earth_clouds','sun']:
        textures[name]=imported(root/f'Art/Space/Textures/{name}.jpg','/Game/Solar/Textures','T_'+name,unreal.Texture2D)
    
    def material(name,unlit=False,blend=None,two=False):
        path='/Game/Solar/Materials/'+name
        mat=unreal.load_asset(path)
        if mat:
            assert isinstance(mat,unreal.Material)
            # Existing expressions loaded by CDO references can be rooted in this
            # commandlet. Reconnect the owned output graph; do not garbage-mark
            # rooted expressions through DeleteAll/DeleteUnused (UE asserts).
        else: mat=tools.create_asset(name,'/Game/Solar/Materials',unreal.Material,unreal.MaterialFactoryNew())
        mat.set_editor_property('two_sided',two)
        if unlit:mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
        if blend is not None:mat.set_editor_property('blend_mode',blend)
        return mat
    def node(mat,cls,**props):
        expr=lib.create_material_expression(mat,cls)
        for k,v in props.items():expr.set_editor_property(k,v)
        return expr
    def constant(mat,value):return node(mat,unreal.MaterialExpressionConstant,r=value)
    def vector(mat,value):return node(mat,unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(*value))
    def texture(mat,asset):return node(mat,unreal.MaterialExpressionTextureSample,texture=asset)
    def custom(mat,code,inputs,output=unreal.CustomMaterialOutputType.CMOT_FLOAT3):
        fields=[]
        for name in inputs:
            field=unreal.CustomInput()
            field.set_editor_property('input_name',name)
            fields.append(field)
        c=node(mat,unreal.MaterialExpressionCustom,code=code,output_type=output,
            inputs=fields)
        for name,(expr,channel) in inputs.items():assert lib.connect_material_expressions(expr,channel,c,name)
        return c
    def finish(mat,expr,prop=unreal.MaterialProperty.MP_EMISSIVE_COLOR):
        assert lib.connect_material_property(expr,'',prop)
        lib.layout_material_expressions(mat);lib.recompile_material(mat)
        assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
    
    earth=material('M_Earth',True)
    normal=node(earth,unreal.MaterialExpressionPixelNormalWS)
    sun=node(earth,unreal.MaterialExpressionVectorParameter,parameter_name='SunDirection',default_value=unreal.LinearColor(.7,-.7,0,0))
    surface=custom(earth,
        'float d=dot(normalize(N),normalize(S)); float light=smoothstep(-0.03,0.15,d); '
        'float3 ground=Day*max(0.035, d)*0.9; float cloud=saturate(Cloud.r)*0.72; '
        'float3 lit=lerp(ground,float3(0.85,0.89,0.93)*max(0.035,d),cloud); '
        'return lerp(Night*0.65,lit,light);',
        {'Day':(texture(earth,textures['earth_daymap']),'RGB'),'Night':(texture(earth,textures['earth_nightmap']),'RGB'),
         'Cloud':(texture(earth,textures['earth_clouds']),'RGB'),'N':(normal,''),'S':(sun,'RGB')})
    finish(earth,surface)
    atmos=material('M_Atmosphere',True,unreal.BlendMode.BLEND_ADDITIVE,False)
    normal=node(atmos,unreal.MaterialExpressionPixelNormalWS);view=node(atmos,unreal.MaterialExpressionCameraVectorWS)
    sun=node(atmos,unreal.MaterialExpressionVectorParameter,parameter_name='SunDirection',default_value=unreal.LinearColor(.7,-.7,0,0))
    rim=custom(atmos,'float rim=pow(1-saturate(abs(dot(normalize(N),normalize(V)))),4); '
        'return float3(0.08,0.35,0.8)*rim*(0.1+0.9*saturate(dot(normalize(N),normalize(S))+0.2));',
        {'N':(normal,''),'V':(view,''),'S':(sun,'RGB')})
    finish(atmos,rim)
    sunmat=material('M_Sun',True)
    sunexpr=custom(sunmat,'return lerp(float3(1,0.9,0.68),T*float3(1.4,1.05,0.72),0.35)*3;',{'T':(texture(sunmat,textures['sun']),'RGB')})
    finish(sunmat,sunexpr)
    star=material('M_Star',True,unreal.BlendMode.BLEND_MASKED,True)
    lib.set_base_material_usage(star,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    uv=node(star,unreal.MaterialExpressionTextureCoordinate)
    circle=custom(star,'return 1-smoothstep(0.22,0.48,length(U-0.5));',{'U':(uv,'')},unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    assert lib.connect_material_property(circle,'',unreal.MaterialProperty.MP_OPACITY_MASK)
    components=[]
    for i in range(3):
        value=node(star,unreal.MaterialExpressionPerInstanceCustomData,data_index=i,const_default_value=.5)
        interpolation=node(star,unreal.MaterialExpressionVertexInterpolator)
        assert lib.connect_material_expressions(value,'',interpolation,'')
        components.append(interpolation)
    col=custom(star,'return float3(R,G,B)*1.2;',dict(zip(['R','G','B'],[(n,'') for n in components])))
    finish(star,col)
    dust=material('M_Dust',True,unreal.BlendMode.BLEND_MASKED,True)
    lib.set_base_material_usage(dust,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    circle=custom(dust,'return 1-smoothstep(0.1,0.48,length(U-0.5));',{'U':(node(dust,unreal.MaterialExpressionTextureCoordinate),'')},unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    assert lib.connect_material_property(circle,'',unreal.MaterialProperty.MP_OPACITY_MASK)
    finish(dust,vector(dust,(.17,.23,.28,1)))
    
    # Stable owned materials on every slot, preserving imported vertex colors.
    source=(root/'Art/Ships/Daedalus/Daedalus.glb').read_bytes()
    doc=json.loads(source[20:20+struct.unpack_from('<I',source,12)[0]])
    source_mats={m['name']:m for m in doc['materials']}
    for index,slot in enumerate(ship.get_editor_property('static_materials')):
        name=str(slot.material_slot_name)
        assert name in source_mats,(name,list(source_mats))
        src=source_mats[name];pbr=src['pbrMetallicRoughness']
        m=material('M_'+name)
        lib.set_base_material_usage(m,unreal.MaterialUsage.MATUSAGE_NANITE)
        base=vector(m,tuple(pbr.get('baseColorFactor',[1,1,1,1])))
        color=node(m,unreal.MaterialExpressionVertexColor)
        mul=node(m,unreal.MaterialExpressionMultiply)
        assert lib.connect_material_expressions(base,'',mul,'A')
        assert lib.connect_material_expressions(color,'',mul,'B')
        assert lib.connect_material_property(mul,'',unreal.MaterialProperty.MP_BASE_COLOR)
        assert lib.connect_material_property(constant(m,pbr.get('metallicFactor',0)),'',unreal.MaterialProperty.MP_METALLIC)
        assert lib.connect_material_property(constant(m,pbr.get('roughnessFactor',.6)),'',unreal.MaterialProperty.MP_ROUGHNESS)
        strength=src.get('extensions',{}).get('KHR_materials_emissive_strength',{}).get('emissiveStrength',1)
        emission=[v*strength for v in src.get('emissiveFactor',[0,0,0])]
        finish(m,vector(m,(*emission,1)))
        # set_material edits the real StaticMesh slot, not a transient component.
        ship.set_material(index,m)
    assert unreal.EditorAssetLibrary.save_loaded_asset(ship)
    editor=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/SolarFlight'):
        assert editor.new_level('/Game/Maps/SolarFlight')
        assert editor.save_current_level()
    print('SOLAR_CONTENT_PASS',ship.get_path_name(),len(source_mats))
    
    unreal.EditorAssetLibrary.set_metadata_tag(ship,'SolarRecipe',recipe)
    assert unreal.EditorAssetLibrary.save_loaded_asset(ship)
main()
