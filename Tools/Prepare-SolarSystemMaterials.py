"""Canonical catalog -> detailed, independently lit world materials."""
import json

def prepare(root, unreal, tools, lib, imported, material, node, constant, vector, texture, custom, finish):
    catalog=json.loads((root/'Game/Daedalus/Content/Data/Solar/system.json').read_text(encoding='utf-8-sig'))
    maps={}
    def tex(filename):
        if filename not in maps:
            source=root/'Art/Space/SolarSystem/Textures'/filename
            if not source.exists():source=root/'Art/Space/Textures'/filename
            asset=imported(source,'/Game/Solar/Textures','T_'+source.stem,unreal.Texture2D)
            asset.set_editor_property('max_texture_size',8192)
            assert unreal.EditorAssetLibrary.save_loaded_asset(asset)
            maps[filename]=asset
        return maps[filename]
    for row in catalog['bodies']:
        key=row['id'][4:]; mat=material('M_Body_'+key,True)
        n=node(mat,unreal.MaterialExpressionPixelNormalWS)
        s=node(mat,unreal.MaterialExpressionVectorParameter,parameter_name='SunDirection',default_value=unreal.LinearColor(.7,-.7,0,0))
        inputs={'T':(texture(mat,tex(row['texture'])),'RGB'),'N':(n,''),'S':(s,'RGB')}
        if key=='earth':
            inputs['Cloud']=(texture(mat,tex(row['cloudTexture'])),'RGB')
            inputs['Night']=(texture(mat,tex(row['nightTexture'])),'RGB')
            code='float d=dot(normalize(N),normalize(S)); float l=smoothstep(-.03,.15,d); float3 c=lerp(T,float3(.88,.92,.97),Cloud.r*.7); return lerp(Night*.65,c*max(.026,d)*.95,l);'
        elif row['kind']=='star':code='return T*float3(2.1,1.75,1.25);'
        else:code='float d=dot(normalize(N),normalize(S));return T*(.026+.96*max(0.,d));'
        finish(mat,custom(mat,code,inputs))
        if 'atmosphereColor' in row:
            air=material('M_Air_'+key,True,unreal.BlendMode.BLEND_ADDITIVE)
            rgb=tuple(int(row['atmosphereColor'][i:i+2],16)/255 for i in (1,3,5))
            rim=custom(air,'float r=pow(1-saturate(abs(dot(normalize(N),normalize(V)))),4); return C*r*(.10+.6*saturate(dot(normalize(N),normalize(S))+.2));',
                {'N':(node(air,unreal.MaterialExpressionPixelNormalWS),''),'V':(node(air,unreal.MaterialExpressionCameraVectorWS),''),
                'C':(vector(air,(*rgb,1)),''),'S':(node(air,unreal.MaterialExpressionVectorParameter,parameter_name='SunDirection',default_value=unreal.LinearColor(.7,-.7,0,0)),'RGB')})
            finish(air,rim)
        if 'ringTexture' in row:
            ring=material('M_Ring_'+key,True,unreal.BlendMode.BLEND_TRANSLUCENT,True)
            uv=node(ring,unreal.MaterialExpressionTextureCoordinate)
            ratio=row['ringInnerMetres']/row['ringOuterMetres']
            coord=custom(ring,f'float r=length(U*2-1);return float2(saturate((r-{ratio})/(1-{ratio})),.5);',{'U':(uv,'')},unreal.CustomMaterialOutputType.CMOT_FLOAT2)
            sample=texture(ring,tex(row['ringTexture']));assert lib.connect_material_expressions(coord,'',sample,'UVs')
            opacity=custom(ring,f'float r=length(U*2-1);return A*step({ratio},r)*step(r,1.);',{'U':(uv,''),'A':(sample,'A')},unreal.CustomMaterialOutputType.CMOT_FLOAT1)
            assert lib.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
            finish(ring,custom(ring,'return C*.7;',{'C':(sample,'RGB')}))
    sky=material('M_Sky',True,two=True)
    finish(sky,custom(sky,'return pow(max(T,0.),1.15)*.85;',{'T':(texture(sky,tex('stars_milky_way.jpg')),'RGB')}))
    rock=imported(root/'Art/Space/SolarSystem/SolarRock.glb','/Game/Solar','SM_SolarRock',unreal.StaticMesh)
    ns=rock.get_editor_property('nanite_settings');ns.set_editor_property('enabled',False);rock.set_editor_property('nanite_settings',ns)
    rm=material('M_Rock');assert lib.connect_material_property(texture(rm,tex('rock_schematic.png')),'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
    finish(rm,constant(rm,.86),unreal.MaterialProperty.MP_ROUGHNESS)
    lib.set_base_material_usage(rm,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    rock.set_material(0,rm);assert unreal.EditorAssetLibrary.save_loaded_asset(rock)
    print('SOLAR_SYSTEM_MATERIALS_PASS',len(catalog['bodies']),len(maps))
