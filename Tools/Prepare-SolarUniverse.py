"""Compile reviewed source catalogs into one canonical local-metre universe.
Source epochs/unknown sizes are retained; visuals never invent measurements.
Run after source workers have handed off. No applications/network are launched.
"""
import json,math,copy
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
AU=149597870700.0
SYSTEMS=[('Asterion','fic.asterion',[25970,40,20]),('Velara','fic.velara',[25400,300,100]),('Nivara','fic.nivara',[7000,-7000,-120]),('Caelum','fic.caelum',[-28000,14000,180]),('Morava','fic.morava',[35000,-22000,-80])]
def load(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def save(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2,allow_nan=False)+'\n',encoding='utf-8')
def key(ident):return ident.removeprefix('sol.').replace('.','_').replace('-','_')
def transform(v):
    # Explicit orthonormal ecliptic-J2000 -> authored left-handed engine frame.
    # Rotation is illustrative, not a current-date ephemeris calibration.
    a=math.radians(94.89295434084237);c,s=math.cos(a),math.sin(a)
    return [c*v[0]-s*v[1],-(s*v[0]+c*v[1]),v[2]]
def add(a,b):return [x+y for x,y in zip(a,b)]
def shader_path(row):
    if row.get('texture'):return '/Game/Solar/Materials/M_Body_'+key(row['id'])
    return '/Game/Solar/Materials/M_FallbackIce' if row.get('icy') else '/Game/Solar/Materials/M_FallbackRock'
def main():
    sol=load(ROOT/'Art/Space/SolarSystem/system-reference.json');rows=sol['bodies'];ids={r['id']:r for r in rows}
    sun=ids['sol.sun']['positionMetres']
    meta_keys=['radiusSigmaMetres','radiusStatus','radiusProvenance','shapeProvenance','positionEpochJdTdb','positionStatus','sourceUrl','orbit','jplSatelliteCode','jplSmallBodyId','designation']
    def enrich(row,src):
        for k in meta_keys:
            if k in src:row[k]=copy.deepcopy(src[k])
        r=src.get('radiusMetres');row['radiusMetres']=r;row['radiusKnown']=r is not None
        row['positionKnown']=row.get('positionMetres') is not None
        axes=(src.get('shapeProvenance') or {}).get('semiaxesMetres')
        if axes and r:row['shapeScale']=[x/r for x in axes]
        row.setdefault('shapeScale',[1,1,1]);row.setdefault('tiltDegrees',0)
        if row.get('rotationHours') is None:row['rotationHours']=src.get('rotationHours') or 0
        row.setdefault('texture',None);row.setdefault('surfaceQuality','schematic-unmapped' if r else 'unknown-size-marker')
        return row
    minor=load(ROOT/'Art/Space/SolarCatalog/minor-bodies.json')['bodies']
    for src in minor:
        row=ids.get(src['id'])
        if row is None:
            row={'id':src['id'],'name':src['name'],'kind':src['kind'],'parentId':'sol.sun','rotationHours':src.get('rotationHours') or 0}
            row['icy']=src['kind'] in ('dwarf-planet','comet')
            rows.append(row);ids[row['id']]=row
        row['kind']=src['kind']
        row['positionMetres']=add(sun,transform(src['positionMetres']))
        enrich(row,src)
    for src in load(ROOT/'Art/Space/SolarCatalog/satellites.json')['bodies']:
        assert src['parentId'] in ids,src['parentId']
        row=ids.get(src['id'])
        if row is None:
            row={k:src[k] for k in ('id','name','kind','parentId')}
            row['icy']=src['parentId'] in ('sol.saturn','sol.uranus','sol.neptune','sol.pluto')
            rows.append(row);ids[row['id']]=row
        row['positionMetres']=add(ids[src['parentId']]['positionMetres'],transform(src['relativePositionMetres']))
        enrich(row,src)
    for src in load(ROOT/'Art/Space/SolarCatalog/minor-body-satellites.json')['bodies']:
        row={k:src[k] for k in ('id','name','kind','parentId')};row['positionMetres']=None
        enrich(row,src);rows.append(row);ids[row['id']]=row
    details=ROOT/'Art/Space/SolarDetails'
    if (details/'surface-views.json').exists():
        for view in load(details/'surface-views.json')['surfaces']:
            row=ids[view['bodyId']];row['textureSource']='Art/Space/SolarDetails/'+view['texture']
            row['texture']=Path(view['texture']).name;row['surfaceQuality']=view['surfaceQuality']
            row['sourceUrl']=view.get('sourceUrl') or ''
            if view.get('observationMask'):row['observationMaskSource']='Art/Space/SolarDetails/'+view['observationMask']
    if (details/'ring-views.json').exists():
        for view in load(details/'ring-views.json')['views']:
            row=ids[view['parentId']];row['ringInnerMetres']=view['innerMetres'];row['ringOuterMetres']=view['outerMetres']
            row['ringTexture']=Path(view['texture']).name;row['ringTextureSource']='Art/Space/SolarDetails/'+view['texture'];row['ringComponents']=view['components']
    sol.update(primaryStarId='sol.sun',source='real-source-approximate-static-epoch',galaxyPositionLightYears=[26000,0,0])
    sol['coordinateTransform']={'source':'right-handed-ecliptic-J2000','target':'authored-left-handed-UE-local-metres-Earth-origin','rotationDegrees':94.89295434084237,'flipRotatedY':True,'epochPolicy':'mixed-source-epoch-illustrative; not current ephemerides'}
    for row in rows:
        row.setdefault('radiusKnown',row.get('radiusMetres') is not None);row.setdefault('positionKnown',row.get('positionMetres') is not None)
        row['material']=shader_path(row);row['mapMaterial']=('/Game/Solar/Materials/M_Map_'+key(row['id'])) if row.get('texture') else '/Game/Solar/Materials/M_MapRock'
    regions=load(ROOT/'Art/Space/SolarCatalog/distributions.json')['regions']
    sol['sourceRegions']=regions
    # Envelopes are labeled display/sampling guides, never measured hard walls.
    sol['regions']=[{'id':'sol.mainbelt','name':'Hlavní pás asteroidů (oblast)','geometry':'disk','centreMetres':sun,'innerMetres':2.1*AU,'outerMetres':3.3*AU,'count':600,'seed':3040012},
        {'id':'sol.kuiper','name':'Kuiperův pás (přibližná oblast)','geometry':'disk','centreMetres':sun,'innerMetres':30*AU,'outerMetres':50*AU,'count':600,'seed':3040016},
        {'id':'sol.scattered','name':'Rozptýlená populace (odhad oblasti)','geometry':'disk','centreMetres':sun,'innerMetres':50*AU,'outerMetres':1000*AU,'count':400,'seed':3040025},
        {'id':'sol.oort','name':'Oortovo mračno (odhad hranic)','geometry':'sphere','centreMetres':sun,'innerMetres':2000*AU,'outerMetres':100000*AU,'count':400,'seed':3040026}]
    sol['belts']=[{'id':r['id'],'parentId':'sol.sun','innerMetres':r['innerMetres'],'outerMetres':r['outerMetres'],'count':r['count'],'seed':r['seed'],'kind':'icy' if r['geometry']=='sphere' or r['id'].endswith('kuiper') else 'asteroid','geometry':r['geometry']} for r in sol['regions']]
    sol['belts'] += [{'id':'sol.trojan.leading','parentId':'sol.sun','innerMetres':4.9*AU,'outerMetres':5.5*AU,'count':180,'seed':3040044,'kind':'asteroid','geometry':'trojan','longitudeOffsetDegrees':60},
        {'id':'sol.trojan.trailing','parentId':'sol.sun','innerMetres':4.9*AU,'outerMetres':5.5*AU,'count':180,'seed':3040045,'kind':'asteroid','geometry':'trojan','longitudeOffsetDegrees':-60}]
    sol['unlocatedBodies']=[r['id'] for r in rows if not r['positionKnown']]
    registry={'version':1,'galaxyModel':'illustrative-barred-spiral-informed-by-ESA-Gaia','systems':[{'id':'sol','name':'Sluneční soustava','galaxyPositionLightYears':[26000,0,0],'primaryStarId':'sol.sun','available':True,'catalog':'Data/Solar/system.json','stellarColor':'#fff4df'}]}
    for name,ident,gal in SYSTEMS:
        source=ROOT/'Art/Space/Systems'/name/'system.json'
        entry={'id':ident,'name':name,'galaxyPositionLightYears':gal,'primaryStarId':ident+'.sun','stellarColor':'#ffffff','available':False,'catalog':'Data/Systems/'+name+'/system.json'}
        if source.exists():
            data=load(source);assert data['id']==ident and data['galaxyPositionLightYears']==gal
            for row in data['bodies']:
                row.setdefault('radiusKnown',True);row.setdefault('positionKnown',True)
                row['textureSource']=str((source.parent/row['texture']).relative_to(ROOT)).replace('\\','/')
                row['texture']=Path(row['texture']).name;row['material']=shader_path(row);row['mapMaterial']='/Game/Solar/Materials/M_Map_'+key(row['id'])
                if row.get('mesh'):
                    row['meshSource']=str((source.parent/row['mesh']).relative_to(ROOT)).replace('\\','/')
                    row['meshAsset']='/Game/Solar/Models/SM_Body_'+key(row['id'])
                for field in ('ringTexture','cloudTexture','nightTexture'):
                    if row.get(field):
                        row[field+'Source']=str((source.parent/row[field]).relative_to(ROOT)).replace('\\','/')
                        row[field]=Path(row[field]).name
            entry.update(available=True,stellarColor=data.get('stellarColor','#ffffff'))
            by_id={r['id']:r for r in data['bodies']}
            data['regions']=[dict(name=b['id'].split('.')[-1]+' (fiktivní pás)',geometry='disk',centreMetres=by_id[b['parentId']]['positionMetres'],innerMetres=b['innerMetres'],outerMetres=b['outerMetres']) for b in data['belts']]
            save(ROOT/'Game/Daedalus/Content'/entry['catalog'],data)
        registry['systems'].append(entry)
    for row in rows:
        if row['parentId']:assert row['parentId'] in ids
        for field in ('positionMetres','shapeScale'):
            if row.get(field) is not None:assert len(row[field])==3 and all(math.isfinite(v) for v in row[field])
    assert len(ids)==len(rows)==506,(len(ids),len(rows))
    assert sum(r['kind']=='moon' for r in rows)==468
    save(ROOT/'Game/Daedalus/Content/Data/Solar/system.json',sol)
    save(ROOT/'Game/Daedalus/Content/Data/Solar/universe.json',registry)
    print('UNIVERSE_DATA_PASS',len(rows),'bodies;468moons;6systemslots',sum(e['available'] for e in registry['systems']),'available')
if __name__=='__main__':main()

