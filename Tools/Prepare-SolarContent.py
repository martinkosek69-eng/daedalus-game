"""Owned Unreal commandlet: source import and repeatable solar content recipe."""
import unreal, json, struct, hashlib, importlib.util, os
from pathlib import Path

def main():
    base=Path(unreal.Paths.project_dir()).resolve().parent.parent
    fingerprint=hashlib.sha256(Path(__file__).read_bytes())
    for source in [base/'Art/Ships/Daedalus'/f for f in ['Daedalus.glb','DaedalusEngineGlow.glb','DaedalusLights.glb','ENGINE_MOUNTS.json']]+[base/'Art/Space/SolarSphere.glb']+sorted((base/'Art/Space/Textures').glob('*.jpg')):
        fingerprint.update(source.read_bytes())
    web_source=base/'Art/Ships/Daedalus/WebReference/WebDaedalus.glb'
    fingerprint.update(web_source.read_bytes())
    for source in sorted((base/'Art/Space/SolarSystem').rglob('*'))+sorted((base/'Art/Ships/Daedalus/Textures').glob('*.png'))+[base/'Art/Ships/Daedalus/DaedalusAddOns.glb',base/'Art/Ships/Daedalus/DaedalusTurrets.glb',base/'Game/Daedalus/Content/Data/Solar/system.json',base/'Tools/Prepare-SolarSystemMaterials.py',base/'Tools/Prepare-DaedalusMaterials.py']:
        if source.is_file():fingerprint.update(source.read_bytes())
    # Catalogs and explicit texture sources across independently delivered
    # systems participate in the same cache recipe as every ship dependency.
    expanded=[]
    for directory in ('Art/Space/SolarCatalog','Art/Space/SolarDetails','Art/Space/Systems','Game/Daedalus/Content/Data/Systems'):
        expanded+=list((base/directory).rglob('*'))
    expanded+=[base/'Game/Daedalus/Content/Data/Solar/universe.json',base/'Tools/Prepare-SolarDetails.py',base/'Tools/Fetch-SolarMinorCatalog.py']
    # Planet quality: prepared sources, recipe helpers and the planet texture group.
    expanded+=list((base/'Art/Space/PlanetQuality').rglob('*'))
    expanded+=[base/'Tools/Prepare-PlanetMaterials.py',base/'Tools/Prepare-PlanetSources.py',base/'Tools/Fetch-PlanetSources.ps1',
               base/'Game/Daedalus/Config/DefaultDeviceProfiles.ini']
    for source in sorted(set(expanded)):
        if source.is_file():
            fingerprint.update(source.relative_to(base).as_posix().encode('utf-8'))
            fingerprint.update(source.read_bytes())
    recipe=fingerprint.hexdigest()
    existing=unreal.load_asset('/Game/Ships/Daedalus/SM_Daedalus')
    required=['M_Earth','M_Sun','M_Atmosphere','M_Star','M_Dust','M_Daedalus_EngineGlowCore','M_Daedalus_EngineGlowPlume','M_Daedalus_Glass']
    materials_file=base/'Tools/Prepare-SolarSystemMaterials.py'
    materials_spec=importlib.util.spec_from_file_location('SolarMaterialsRecipe',materials_file)
    system_recipe=importlib.util.module_from_spec(materials_spec)
    materials_spec.loader.exec_module(system_recipe)
    paths=['/Game/Solar/Materials/'+name for name in required]
    paths+=['/Game/Solar/SM_SolarRock','/Game/Solar/Materials/M_Sky','/Game/Solar/Materials/M_Rock','/Game/Ships/Daedalus/Details/SM_DaedalusAddOns','/Game/Maps/SolarFlight','/Game/Ships/Daedalus/WebReference/SM_WebDaedalus']
    paths+=system_recipe.required_material_paths(base)
    paths+=system_recipe.required_mesh_paths(base)
    detail_data=base/'Game/Daedalus/Content/Data/Solar/ship-details.json'
    if detail_data.exists(): paths+=json.loads(detail_data.read_text())['meshes']
    for filename in ['Daedalus.glb','DaedalusAddOns.glb','DaedalusTurrets.glb']:
        data=(base/'Art/Ships/Daedalus'/filename).read_bytes()
        paths+=['/Game/Ships/Daedalus/Materials/M_'+m['name'] for m in json.loads(data[20:20+struct.unpack_from('<I',data,12)[0]])['materials']]
    paths+=['/Game/Ships/Daedalus/Textures/'+p.stem for p in (base/'Art/Ships/Daedalus/Textures').glob('*.png')]
    # Development iteration on planets only: rebuilds planet presentation assets, never imports or
    # re-saves ship/sky/star assets and never writes the shared recipe tag (full run still required).
    planet_only=os.environ.get('DAEDALUS_PLANET_ONLY')=='1'
    if not planet_only and existing and detail_data.exists() and unreal.EditorAssetLibrary.get_metadata_tag(existing,'SolarRecipe')==recipe and all(unreal.EditorAssetLibrary.does_asset_exist(path) for path in paths):
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
    
    if planet_only:return planet_pass(root,tools,lib,imported,system_recipe)
    ship=imported(root/'Art/Ships/Daedalus/Daedalus.glb','/Game/Ships/Daedalus','SM_Daedalus',unreal.StaticMesh)
    assert abs(ship.get_bounds().box_extent.x*2-60000)<30,ship.get_bounds()
    web=imported(web_source,'/Game/Ships/Daedalus/WebReference','SM_WebDaedalus',unreal.StaticMesh)
    assert abs(web.get_bounds().box_extent.x*2-60000)<30,web.get_bounds()
    ns=web.get_editor_property('nanite_settings');ns.set_editor_property('enabled',False)
    web.set_editor_property('nanite_settings',ns)
    sphere=imported(root/'Art/Space/SolarSphere.glb','/Game/Solar','SM_SolarSphere',unreal.StaticMesh)
    assert abs(sphere.get_bounds().box_extent.x-100)<.01
    # Preserve the full smooth sphere. Nanite fallback simplification makes the
    # astronomical silhouette visibly polygonal and cannot render additive air.
    nanite=sphere.get_editor_property('nanite_settings')
    nanite.set_editor_property('enabled',False)
    sphere.set_editor_property('nanite_settings',nanite)
    assert unreal.EditorAssetLibrary.save_loaded_asset(sphere)
    lights=imported(root/'Art/Ships/Daedalus/DaedalusLights.glb','/Game/Ships/Daedalus/Effects','SM_DaedalusLights',unreal.StaticMesh)
    glowtask=unreal.AssetImportTask()
    glowtask.filename=str(root/'Art/Ships/Daedalus/DaedalusEngineGlow.glb')
    glowtask.destination_path='/Game/Ships/Daedalus/Effects'
    glowtask.automated=True;glowtask.replace_existing=True;glowtask.save=True
    tools.import_asset_tasks([glowtask])
    glows=[unreal.load_asset(p) for p in glowtask.imported_object_paths if isinstance(unreal.load_asset(p),unreal.StaticMesh)]
    mounts=json.loads((root/'Art/Ships/Daedalus/ENGINE_MOUNTS.json').read_text())['outlets']
    assert len(glows)==len(mounts)==6
    for mesh in glows:
        matches=[m for m in mounts if mesh.get_name().endswith(m['glowObject'])]
        assert len(matches)==1,(mesh.get_name(),mounts)
        mount=matches[0]
        # Keep the importer's stable scene folder. Renaming a member of a
        # multi-mesh import makes subsequent imports create duplicate targets.
        assert mesh.get_path_name().split('.')[0].endswith('/DaedalusEngineGlow/StaticMeshes/'+mount['glowObject']),mesh.get_path_name()
        origin=mesh.get_bounds().origin
        expected=mount['centreMetres']
        # Default Interchange bake transforms preserves the complete ship frame.
        # Reject a local nozzle import instead of silently offsetting it twice.
        # Blender source +Y is port; Unreal +Y is starboard. Interchange's
        # handedness conversion maps (x,y,z) -> (x,-y,z), including baked nodes.
        assert abs(origin.x-expected[0]*100)<1000 and abs(origin.y+expected[1]*100)<1 and abs(origin.z-expected[2]*100)<1,(mesh.get_name(),origin,expected)
    for mesh in [lights]+glows:
        ns=mesh.get_editor_property('nanite_settings');ns.set_editor_property('enabled',False)
        mesh.set_editor_property('nanite_settings',ns)
    textures={}
    for name in ['sun']:
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
        for name,(expr,channel) in inputs.items():assert lib.connect_material_expressions(expr,channel,c,name),(name,expr.get_class().get_name(),channel)
        return c
    def finish(mat,expr,prop=unreal.MaterialProperty.MP_EMISSIVE_COLOR):
        assert lib.connect_material_property(expr,'',prop)
        lib.layout_material_expressions(mat);lib.recompile_material(mat)
        assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
    
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
    col=custom(star,'return float3(R,G,B)*2.4;',dict(zip(['R','G','B'],[(n,'') for n in components])))
    finish(star,col)
    dust=material('M_Dust',True,unreal.BlendMode.BLEND_MASKED,True)
    lib.set_base_material_usage(dust,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    circle=custom(dust,'return 1-smoothstep(0.1,0.48,length(U-0.5));',{'U':(node(dust,unreal.MaterialExpressionTextureCoordinate),'')},unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    assert lib.connect_material_property(circle,'',unreal.MaterialProperty.MP_OPACITY_MASK)
    finish(dust,vector(dust,(.45,.63,.8,1)))
    
    # Stable owned materials on every slot, preserving imported vertex colors.
    def assign_source_materials(mesh,filename,effect=False,web_recipe=False):
      source=(root/'Art/Ships/Daedalus'/filename).read_bytes()
      doc=json.loads(source[20:20+struct.unpack_from('<I',source,12)[0]])
      source_mats={m['name']:m for m in doc['materials']}
      for index,slot in enumerate(mesh.get_editor_property('static_materials')):
        name=str(slot.material_slot_name)
        assert name in source_mats,(name,list(source_mats))
        src=source_mats[name];pbr=src['pbrMetallicRoughness']
        m=material(('M_Web_' if web_recipe else 'M_')+name,effect,unreal.BlendMode.BLEND_TRANSLUCENT if src.get('alphaMode')=='BLEND' else unreal.BlendMode.BLEND_OPAQUE,src.get('doubleSided',False))
        if not effect:lib.set_base_material_usage(m,unreal.MaterialUsage.MATUSAGE_NANITE)
        base=vector(m,tuple(pbr.get('baseColorFactor',[1,1,1,1])))
        color=node(m,unreal.MaterialExpressionVertexColor)
        mul=node(m,unreal.MaterialExpressionMultiply)
        assert lib.connect_material_expressions(base,'',mul,'A')
        assert lib.connect_material_expressions(color,'',mul,'B')
        if web_recipe and name=='Daedalus_Armor':
            position=node(m,unreal.MaterialExpressionWorldPosition)
            local=node(m,unreal.MaterialExpressionTransformPosition,
                transform_source_type=unreal.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_WORLD,
                transform_type=unreal.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_LOCAL)
            assert lib.connect_material_expressions(position,'',local,'')
            normal=node(m,unreal.MaterialExpressionPixelNormalWS)
            normal_local=node(m,unreal.MaterialExpressionTransform,
                transform_source_type=unreal.MaterialVectorCoordTransformSource.TRANSFORMSOURCE_WORLD,
                transform_type=unreal.MaterialVectorCoordTransform.TRANSFORM_LOCAL)
            assert lib.connect_material_expressions(normal,'',normal_local,'')
            # Original web livery.mjs recipe, including derivative AA seams.
            # Imported UE (x,y,z) maps back to source (y,z,-x), cm / 600m.
            mul=custom(m,'''float3 p=float3(P.y,P.z,-P.x)/60000.;
                float3 n=normalize(float3(N.y,N.z,-N.x)),an=abs(n);
                float2 uv=an.y>max(an.x,an.z)?p.xz:an.x>an.z?p.zy:p.xy;
                float2 grid=uv*float2(27.,43.);
                grid.x+=fmod(floor(grid.y)+10000.,2.)*.37;
                float2 cell=frac(grid),edge=min(cell,1.-cell),aa=max(fwidth(grid),float2(.006,.006));
                float seam=1.-smoothstep(.014,.014+max(aa.x,aa.y),min(edge.x,edge.y));
                float panel=.76+frac(sin(dot(floor(grid),float2(127.1,311.7)))*43758.5453)*.22;
                float wear=sin(p.x*583.)*sin(p.z*769.)*sin(p.y*677.)*.012;
                float3 c=(panel+wear)*lerp(1.,.27,seam*.8);
                c*=p.y<-.018?.69:1.;
                if(p.z>.33)c*=float3(.58,.61,.65);
                if(abs(p.x)<.010&&p.z<.23&&p.z>-.32)c*=.55;
                if(abs(p.x)>.18&&abs(p.x)<.295&&p.z<.015&&abs(n.z)>.7&&p.y<.016)c*=.28;
                if(p.y>.04&&abs(n.y)<.35)c*=.64;
                return Base*c;''',{'P':(local,''),'N':(normal_local,''),'Base':(base,'')})
        assert lib.connect_material_property(mul,'',unreal.MaterialProperty.MP_BASE_COLOR)
        assert lib.connect_material_property(constant(m,pbr.get('metallicFactor',0)),'',unreal.MaterialProperty.MP_METALLIC)
        assert lib.connect_material_property(constant(m,pbr.get('roughnessFactor',.6)),'',unreal.MaterialProperty.MP_ROUGHNESS)
        strength=src.get('extensions',{}).get('KHR_materials_emissive_strength',{}).get('emissiveStrength',1)
        emission=[v*strength for v in src.get('emissiveFactor',[0,0,0])]
        emit=vector(m,(*emission,1))
        if web_recipe:
            # The web's blue ambient light remains readable in black space.
            emit=custom(m,'return C*float3(.025,.043,.070)+E;',{'C':(mul,''),'E':(emit,'')})
        if filename=='DaedalusEngineGlow.glb':
            level=node(m,unreal.MaterialExpressionScalarParameter,parameter_name='EngineLevel',default_value=.08)
            mod=node(m,unreal.MaterialExpressionMultiply)
            assert lib.connect_material_expressions(emit,'',mod,'A')
            assert lib.connect_material_expressions(level,'',mod,'B')
            emit=mod
        if src.get('alphaMode')=='BLEND':
            assert lib.connect_material_property(constant(m,pbr.get('baseColorFactor',[1,1,1,1])[3]),'',unreal.MaterialProperty.MP_OPACITY)
        finish(m,emit)
        # set_material edits the real StaticMesh slot, not a transient component.
        mesh.set_material(index,m)
      assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
      return len(source_mats)
    def module(filename):
        spec=importlib.util.spec_from_file_location(filename,root/'Tools'/filename)
        result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result
    ship_recipe=module('Prepare-DaedalusMaterials.py')
    count=ship_recipe.assign_materials(root,unreal,tools,lib,ship,'Daedalus.glb')
    addons=imported(root/'Art/Ships/Daedalus/DaedalusAddOns.glb','/Game/Ships/Daedalus/Details','SM_DaedalusAddOns',unreal.StaticMesh)
    ship_recipe.assign_materials(root,unreal,tools,lib,addons,'DaedalusAddOns.glb')
    turret_task=unreal.AssetImportTask();turret_task.filename=str(root/'Art/Ships/Daedalus/DaedalusTurrets.glb')
    turret_task.destination_path='/Game/Ships/Daedalus/Details';turret_task.automated=True;turret_task.replace_existing=True;turret_task.save=True
    tools.import_asset_tasks([turret_task])
    turrets=[unreal.load_asset(p) for p in turret_task.imported_object_paths if isinstance(unreal.load_asset(p),unreal.StaticMesh)]
    assert len(turrets)==38,(len(turrets),turret_task.imported_object_paths)
    for mesh in turrets:ship_recipe.assign_materials(root,unreal,tools,lib,mesh,'DaedalusTurrets.glb')
    (root/'Game/Daedalus/Content/Data/Solar/ship-details.json').write_text(json.dumps({'version':1,'meshes':[m.get_path_name() for m in turrets]},indent=2)+'\n')
    planet_maps=system_recipe.prepare(root,unreal,tools,lib,imported,material,node,constant,vector,texture,custom,finish)
    legacy_earth(root,unreal,lib,imported,material,node,texture,custom,finish,planet_maps)
    assign_source_materials(web,'WebReference/WebDaedalus.glb',web_recipe=True)
    assign_source_materials(lights,'DaedalusLights.glb',True)
    for mesh in glows:assign_source_materials(mesh,'DaedalusEngineGlow.glb',True)
    editor=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/SolarFlight'):
        assert editor.new_level('/Game/Maps/SolarFlight')
        assert editor.save_current_level()
    print('SOLAR_CONTENT_PASS',ship.get_path_name(),count,'glows',len(glows),'lights',lights.get_path_name())
    
    unreal.EditorAssetLibrary.set_metadata_tag(ship,'SolarRecipe',recipe)
    assert unreal.EditorAssetLibrary.save_loaded_asset(ship)

def legacy_earth(root,unreal,lib,imported,material,node,texture,custom,finish,planet_maps):
    """M_Earth is still required by the game mode. It shares the planet-quality Earth maps (no second
    copy of Earth textures in memory)."""
    maps=(planet_maps or {}).get('sol.earth')
    day=maps['day'] if maps else imported(root/'Art/Space/Textures/earth_daymap.jpg','/Game/Solar/Textures','T_earth_daymap',unreal.Texture2D)
    night=maps['night'] if maps and maps['night'] else imported(root/'Art/Space/Textures/earth_nightmap.jpg','/Game/Solar/Textures','T_earth_nightmap',unreal.Texture2D)
    earth=material('M_Earth',True)
    normal=node(earth,unreal.MaterialExpressionPixelNormalWS)
    sun=node(earth,unreal.MaterialExpressionVectorParameter,parameter_name='SunDirection',default_value=unreal.LinearColor(.7,-.7,0,0))
    surface=custom(earth,'float d=dot(normalize(N),normalize(S)); float light=smoothstep(-0.03,0.15,d); '
        'return lerp(Night*0.65,Day*max(0.035,d)*0.9,light);',
        {'Day':(texture(earth,day),'RGB'),'Night':(texture(earth,night),'RGB'),'N':(normal,''),'S':(sun,'RGB')})
    finish(earth,surface)
    if maps:
        # Accumulated (disconnected) legacy graph nodes of M_Earth/M_Map_earth still hold the old 8K
        # Earth imports; deleting nodes of the CDO-referenced M_Earth asserts in UE. Retire the old
        # imports to 64 px so they cost no memory; the original sources stay in Art/Space/Textures.
        for name in ('T_earth_daymap','T_earth_nightmap','T_earth_clouds'):
            path='/Game/Solar/Textures/'+name
            if unreal.EditorAssetLibrary.does_asset_exist(path):
                old=unreal.load_asset(path);old.set_editor_property('max_texture_size',64)
                assert unreal.EditorAssetLibrary.save_loaded_asset(old)
                print('PLANET_QUALITY legacy',path,'retired to 64 px')


def planet_pass(root,tools,lib,imported,system_recipe):
    """DAEDALUS_PLANET_ONLY=1: rebuild planet presentation assets with the same helpers as main()."""
    def material(name,unlit=False,blend=None,two=False):
        path='/Game/Solar/Materials/'+name
        mat=unreal.load_asset(path)
        if mat:assert isinstance(mat,unreal.Material)
        else:mat=tools.create_asset(name,'/Game/Solar/Materials',unreal.Material,unreal.MaterialFactoryNew())
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
            field=unreal.CustomInput();field.set_editor_property('input_name',name);fields.append(field)
        c=node(mat,unreal.MaterialExpressionCustom,code=code,output_type=output,inputs=fields)
        for name,(expr,channel) in inputs.items():assert lib.connect_material_expressions(expr,channel,c,name),(name,expr.get_class().get_name(),channel)
        return c
    def finish(mat,expr,prop=unreal.MaterialProperty.MP_EMISSIVE_COLOR):
        assert lib.connect_material_property(expr,'',prop)
        lib.layout_material_expressions(mat);lib.recompile_material(mat)
        assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
    planet_maps=system_recipe.prepare(root,unreal,tools,lib,imported,material,node,constant,vector,texture,custom,finish)
    legacy_earth(root,unreal,lib,imported,material,node,texture,custom,finish,planet_maps)
    print('PLANET_ONLY_PASS (recipe tag not written; run full Assets before integration)')


main()
