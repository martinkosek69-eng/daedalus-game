"""Canonical available catalogs -> detailed world and sharp map materials."""
import hashlib
import importlib.util
import json
import math
from pathlib import Path


def planet_module(root):
    """Shared planet-quality masters/instances (Tools/Prepare-PlanetMaterials.py)."""
    spec=importlib.util.spec_from_file_location('PlanetQualityRecipe',Path(root)/'Tools/Prepare-PlanetMaterials.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module


def material_key(ident):
    return ident.removeprefix('sol.').replace('.', '_').replace('-', '_')


def load_catalogs(root):
    content=root/'Game/Daedalus/Content'
    paths=[content/'Data/Solar/system.json']
    universe=content/'Data/Solar/universe.json'
    if universe.exists():
        for entry in json.loads(universe.read_text(encoding='utf-8-sig')).get('systems',[]):
            if not entry.get('available',True) or not entry.get('catalog'):continue
            path=content/entry['catalog']
            if path.is_file() and path not in paths:paths.append(path)
    return [json.loads(path.read_text(encoding='utf-8-sig')) for path in paths]


def bodies(root):
    rows=[row for catalog in load_catalogs(root) for row in catalog.get('bodies',[])]
    assert len({row['id'] for row in rows})==len(rows),'Duplicate canonical body IDs'
    assert len({material_key(row['id']) for row in rows})==len(rows),'Material key collision'
    return rows


def has_geometry(row):
    radius=row.get('radiusMetres');position=row.get('positionMetres')
    return (isinstance(radius,(float,int)) and math.isfinite(radius) and radius>0 and
        isinstance(position,list) and len(position)==3 and all(isinstance(v,(float,int)) and math.isfinite(v) for v in position))


def texture_source(root,filename,override=None):
    if override:
        source=root/override
        assert source.is_file(),('Missing explicit textureSource',source)
        return source
    if not filename:return None
    for directory in ('Art/Space/SolarSystem/Textures','Art/Space/SolarDetails/Textures','Art/Space/Textures'):
        source=root/directory/filename
        if source.is_file():return source
    raise FileNotFoundError('Missing canonical texture: '+str(filename))


def ring_views(root):
    path=root/'Art/Space/SolarDetails/ring-views.json'
    return {v['parentId']:v for v in json.loads(path.read_text(encoding='utf-8-sig'))['views']} if path.exists() else {}


def ring_spec(root,row,views):
    view=views.get(row['id'])
    if view:
        return dict(inner=view['innerMetres'],outer=view['outerMetres'],
            source=root/'Art/Space/SolarDetails'/view['texture'])
    inner,outer=row.get('ringInnerMetres'),row.get('ringOuterMetres')
    if row.get('ringTexture') and inner is not None and outer is not None and 0<inner<outer:
        return dict(inner=inner,outer=outer,
            source=texture_source(root,row['ringTexture'],row.get('ringTextureSource')))
    return None


def planet_sources(root,row,manifest):
    """Planet-quality sources for one row: manifest overrides, else the canonical catalog maps."""
    entry=manifest['bodies'].get(row['id'],{})
    def pick(key,field,override):
        if key in entry:return dict(source=root/entry[key],filename=None,override=None)
        if row.get(field) or row.get(override):
            return dict(source=texture_source(root,row.get(field),row.get(override)),filename=row.get(field),override=row.get(override))
        return None
    return dict(day=pick('day','texture','textureSource'),night=pick('night','nightTexture','nightTextureSource'),
                clouds=pick('clouds','cloudTexture','cloudTextureSource'),relief=pick('relief','',''))


def required_material_paths(root):
    names={'M_FallbackRock','M_FallbackIce','M_MapRock','M_UnknownMarker','M_Sky','M_Rock'}
    views=ring_views(root);planet=planet_module(root);manifest=planet.load_manifest(root)
    paths=[]
    for row in bodies(root):
        if not has_geometry(row):continue
        key=material_key(row['id'])
        if row.get('texture') or row.get('textureSource'):names.update(('M_Body_'+key,'M_Map_'+key))
        if (row.get('texture') or row.get('textureSource')) and planet.uses_planet_quality(row,manifest):
            master=planet.MASTERS+'/'+planet.MASTER_NAMES[planet.body_type(row)]
            if master not in paths:paths.append(master)
            if planet.body_type(row)=='surface' and planet_sources(root,row,manifest)['clouds']:
                names.add('M_Cloud_'+key);paths.append(planet.MASTERS+'/'+planet.MASTER_NAMES['clouds'])
        if row.get('atmosphereColor'):names.add('M_Air_'+key)
        if ring_spec(root,row,views):names.add('M_Ring_'+key)
    return ['/Game/Solar/Materials/'+name for name in sorted(names)]+sorted(set(paths))


def required_mesh_paths(root):
    paths=[]
    for row in bodies(root):
        if has_geometry(row) and row.get('meshSource'):
            assert (root/row['meshSource']).is_file(),('Missing body meshSource',row['id'])
            expected='/Game/Solar/Models/SM_Body_'+material_key(row['id'])
            assert row.get('meshAsset',expected)==expected,('Unexpected owned body mesh path',row['id'])
            paths.append(expected)
    planet=planet_module(root)
    paths+=[planet.SPHERE_FOLDER+'/'+name for name in planet.SPHERES]
    return sorted(paths)


def cloud_channel(root,row):
    source=texture_source(root,row.get('cloudTexture'),row.get('cloudTextureSource'))
    with source.open('rb') as stream:header=stream.read(26)
    # PNG grayscale+alpha or RGBA uses authored coverage alpha. Legacy RGB
    # Earth JPEG encodes its mask in luminance; never treat opaque JPEG alpha
    # as a full blanket of cloud. No Pillow dependency inside Unreal Python.
    return 'A' if header[:8]==b'\x89PNG\r\n\x1a\n' and len(header)>25 and header[25] in (4,6) else 'R'


def prepare(root,unreal,tools,lib,imported,material,node,constant,vector,texture,custom,finish):
    rows=bodies(root);maps={};views=ring_views(root)
    planet=planet_module(root)
    quality=planet.Recipe(root,unreal,tools,lib,imported,node,custom,finish,material)
    planet.load_manifest(root,{row['id'] for row in rows})
    quality.spheres();planet_maps={}
    def tex(filename=None,override=None,source=None):
        source=source or texture_source(root,filename,override)
        assert source is not None
        identity=source.relative_to(root).as_posix()
        if identity not in maps:
            # Stable legacy names; path suffix isolates independent system maps.
            suffix='_'+hashlib.sha256(identity.encode()).hexdigest()[:8] if override or source.parent==root/'Art/Space/SolarDetails/Textures' else ''
            asset=imported(source,'/Game/Solar/Textures','T_'+source.stem+suffix,unreal.Texture2D)
            asset.set_editor_property('max_texture_size',8192)
            assert unreal.EditorAssetLibrary.save_loaded_asset(asset)
            maps[identity]=asset
        return maps[identity]
    def world(mat,asset=None,color=(.6,.6,.6,1),star=False,body=None):
        n=node(mat,unreal.MaterialExpressionPixelNormalWS)
        sun=node(mat,unreal.MaterialExpressionVectorParameter,parameter_name='SunDirection',default_value=unreal.LinearColor(.7,-.7,0,0))
        inputs={'T':(texture(mat,asset),'RGB') if asset else (vector(mat,color),''),'N':(n,''),'S':(sun,'RGB')}
        if body and body['id']=='sol.earth' and body.get('cloudTexture') and body.get('nightTexture'):
            inputs['Cloud']=(texture(mat,tex(body['cloudTexture'],body.get('cloudTextureSource'))),'RGB')
            inputs['Night']=(texture(mat,tex(body['nightTexture'],body.get('nightTextureSource'))),'RGB')
            code='float d=dot(normalize(N),normalize(S)); float l=smoothstep(-.03,.15,d); float3 c=lerp(T,float3(.88,.92,.97),Cloud.r*.7); return lerp(Night*.65,c*max(.026,d)*.95,l);'
        elif star:
            code='return T*float3(2.1,1.75,1.25);' if body and body['id']=='sol.sun' else 'return T*2.1;'
        else:
            code='float d=dot(normalize(N),normalize(S));float3 c=T;float coverage=0.;'
            if body and (body.get('cloudTexture') or body.get('cloudTextureSource')):
                sample=texture(mat,tex(body.get('cloudTexture'),body.get('cloudTextureSource')))
                inputs['CloudColor']=(sample,'RGB')
                inputs['CloudCoverage']=(sample,cloud_channel(root,body))
                code+='coverage=saturate(CloudCoverage)*.72;c=lerp(c,CloudColor,coverage);'
            code+='float3 result=c*(.026+.96*max(0.,d));'
            if body and (body.get('nightTexture') or body.get('nightTextureSource')):
                inputs['Night']=(texture(mat,tex(body.get('nightTexture'),body.get('nightTextureSource'))),'RGB')
                code+='result+=Night*.85*(1-smoothstep(-.08,.12,d))*(1-coverage*.9);'
            code+='return result;'
        finish(mat,custom(mat,code,inputs))
    def map_disc(name,asset=None,star=False):
        mat=material(name,True,unreal.BlendMode.BLEND_TRANSLUCENT,True)
        lib.set_base_material_usage(mat,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
        uv=node(mat,unreal.MaterialExpressionTextureCoordinate)
        opacity=custom(mat,'float r=length(U*2-1);return 1-smoothstep(1-max(fwidth(r),.0001),1,r);',
            {'U':(uv,'')},unreal.CustomMaterialOutputType.CMOT_FLOAT1)
        assert lib.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
        # Orthographic hemisphere projected from cylindrical source, not a square.
        coord=custom(mat,'float2 p=U*2-1;p.y=-p.y;float z=sqrt(saturate(1-dot(p,p)));return float2(.5+atan2(p.x,max(z,.000001))/6.28318530718,.5-asin(clamp(p.y,-1,1))/3.14159265359);',
            {'U':(uv,'')},unreal.CustomMaterialOutputType.CMOT_FLOAT2)
        if asset:
            sample=texture(mat,asset);assert lib.connect_material_expressions(coord,'',sample,'UVs')
            # Fixed illustrative map light conveys a round globe. This display
            # light is independent of canonical world Sun/state; stars emit.
            code='return C*1.15;' if star else 'float2 p=U*2-1;float3 n=normalize(float3(p.x,-p.y,sqrt(saturate(1-dot(p,p)))));return C*(.24+.76*saturate(dot(n,normalize(float3(-.4,.35,.85)))));'
            finish(mat,custom(mat,code,{'C':(sample,'RGB'),'U':(uv,'')}))
        else:
            finish(mat,custom(mat,'float2 p=U*2-1;float3 n=normalize(float3(p.x,-p.y,sqrt(saturate(1-dot(p,p)))));return float3(.48,.47,.45)*(.32+.68*saturate(dot(n,normalize(float3(-.4,.35,.85)))));',{'U':(uv,'')}))
    world(material('M_FallbackRock',True),tex('rock_schematic.png'))
    world(material('M_FallbackIce',True),color=(.70,.74,.75,1))
    map_disc('M_MapRock')
    marker=material('M_UnknownMarker',True,unreal.BlendMode.BLEND_MASKED,True)
    marker_uv=node(marker,unreal.MaterialExpressionTextureCoordinate)
    marker_mask=custom(marker,'float2 p=abs(U*2-1);float d=p.x+p.y;float diamond=step(.64,d)*step(d,.91);float cross=step(min(p.x,p.y),.07)*step(max(p.x,p.y),.43);return max(diamond,cross);',
        {'U':(marker_uv,'')},unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    assert lib.connect_material_property(marker_mask,'',unreal.MaterialProperty.MP_OPACITY_MASK)
    finish(marker,custom(marker,'return U.x<.5?float3(.22,.65,1.):float3(1.,.54,.13);',{'U':(marker_uv,'')}))
    lib.set_base_material_usage(marker,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    for row in rows:
        if not has_geometry(row):continue
        key=material_key(row['id'])
        if (row.get('texture') or row.get('textureSource')) and planet.uses_planet_quality(row,quality.manifest):
            # Shared planet masters: M_Body_<key> (and M_Cloud_<key>) become material instances.
            sources=planet_sources(root,row,quality.manifest)
            def load(spec,kind):
                if spec is None:return None
                if spec['filename'] is None:return quality.texture(spec['source'],kind)
                # Existing catalog map: same stable asset name as before, planet settings applied.
                return quality.configure(tex(spec['filename'],spec['override']),kind)
            day=load(sources['day'],'color');night=load(sources['night'],'color')
            clouds=load(sources['clouds'],'mask');relief=load(sources['relief'],'mask')
            quality.body(row,day,night,clouds,relief)
            map_disc('M_Map_'+key,day,star=row.get('kind')=='star')
            planet_maps[row['id']]=dict(day=day,night=night,clouds=clouds,relief=relief)
        elif row.get('texture') or row.get('textureSource'):
            asset=tex(row.get('texture'),row.get('textureSource'))
            world(material('M_Body_'+key,True),asset,star=row.get('kind')=='star',body=row)
            map_disc('M_Map_'+key,asset,star=row.get('kind')=='star')
        if row.get('meshSource'):
            expected='/Game/Solar/Models/SM_Body_'+key
            assert row.get('meshAsset',expected)==expected,('Unexpected owned body mesh path',row['id'])
            mesh=imported(root/row['meshSource'],'/Game/Solar/Models','SM_Body_'+key,unreal.StaticMesh)
            bounds=mesh.get_bounds();extent=bounds.box_extent
            assert 0<max(extent.x,extent.y,extent.z)<=100.05,('Body mesh must retain normalized <=1m radius',row['id'],bounds)
            settings=mesh.get_editor_property('nanite_settings');settings.set_editor_property('enabled',False)
            mesh.set_editor_property('nanite_settings',settings)
            path=row.get('material') or ('/Game/Solar/Materials/M_Body_'+key if row.get('texture') or row.get('textureSource') else '/Game/Solar/Materials/M_FallbackRock')
            owned=unreal.load_asset(path);assert isinstance(owned,unreal.MaterialInterface),(row['id'],path)
            for index in range(len(mesh.get_editor_property('static_materials'))):mesh.set_material(index,owned)
            assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        if row.get('atmosphereColor'):
            air=material('M_Air_'+key,True,unreal.BlendMode.BLEND_ADDITIVE)
            rgb=tuple(int(row['atmosphereColor'][i:i+2],16)/255 for i in (1,3,5))
            rim=custom(air,'float r=pow(1-saturate(abs(dot(normalize(N),normalize(V)))),4); return C*r*(.10+.6*saturate(dot(normalize(N),normalize(S))+.2));',
                {'N':(node(air,unreal.MaterialExpressionPixelNormalWS),''),'V':(node(air,unreal.MaterialExpressionCameraVectorWS),''),
                 'C':(vector(air,(*rgb,1)),''),'S':(node(air,unreal.MaterialExpressionVectorParameter,parameter_name='SunDirection',default_value=unreal.LinearColor(.7,-.7,0,0)),'RGB')})
            finish(air,rim)
        spec=ring_spec(root,row,views)
        if spec:
            assert spec['source'].is_file(),spec['source']
            ring=material('M_Ring_'+key,True,unreal.BlendMode.BLEND_TRANSLUCENT,True)
            uv=node(ring,unreal.MaterialExpressionTextureCoordinate);ratio=spec['inner']/spec['outer']
            coord=custom(ring,f'float r=length(U*2-1);return float2(saturate((r-{ratio})/(1-{ratio})),.5);',{'U':(uv,'')},unreal.CustomMaterialOutputType.CMOT_FLOAT2)
            ring_tex=tex(source=spec['source'])
            ring_tex.set_editor_property('address_x',unreal.TextureAddress.TA_CLAMP)
            ring_tex.set_editor_property('address_y',unreal.TextureAddress.TA_CLAMP)
            assert unreal.EditorAssetLibrary.save_loaded_asset(ring_tex)
            sample=texture(ring,ring_tex);assert lib.connect_material_expressions(coord,'',sample,'UVs')
            opacity=custom(ring,f'float r=length(U*2-1);return A*step({ratio},r)*step(r,1.);',{'U':(uv,''),'A':(sample,'A')},unreal.CustomMaterialOutputType.CMOT_FLOAT1)
            assert lib.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
            finish(ring,custom(ring,'return C*.7;',{'C':(sample,'RGB')}))
    sky=material('M_Sky',True,two=True)
    finish(sky,custom(sky,'float3 d=normalize(N);float plane=exp(-pow(d.z/.12,2));float cone=pow(saturate(dot(d,normalize(S))*.5+.5),8);return pow(max(T,0.),1.15)*.85+float3(.028,.024,.018)*plane*cone*Z;',
        {'T':(texture(sky,tex('stars_milky_way.jpg')),'RGB'),
         'N':(node(sky,unreal.MaterialExpressionPixelNormalWS),''),
         'S':(node(sky,unreal.MaterialExpressionVectorParameter,parameter_name='SunDirection',default_value=unreal.LinearColor(.7,-.7,0,0)),'RGB'),
         'Z':(node(sky,unreal.MaterialExpressionScalarParameter,parameter_name='ZodiacalStrength',default_value=0.),'')}))
    rock=imported(root/'Art/Space/SolarSystem/SolarRock.glb','/Game/Solar','SM_SolarRock',unreal.StaticMesh)
    ns=rock.get_editor_property('nanite_settings');ns.set_editor_property('enabled',False);rock.set_editor_property('nanite_settings',ns)
    rm=material('M_Rock');assert lib.connect_material_property(texture(rm,tex('rock_schematic.png')),'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
    finish(rm,constant(rm,.86),unreal.MaterialProperty.MP_ROUGHNESS)
    lib.set_base_material_usage(rm,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    lib.recompile_material(rm)
    assert unreal.EditorAssetLibrary.save_loaded_asset(rm)
    rock.set_material(0,rm);assert unreal.EditorAssetLibrary.save_loaded_asset(rock)
    print('SOLAR_SYSTEM_MATERIALS_PASS',len(rows),'definitions',sum(has_geometry(r) for r in rows),'placed',len(maps),'textures',len(planet_maps),'planet-quality bodies')
    for entry in quality.report:print('PLANET_QUALITY',*entry)
    return planet_maps
