const fs=require('fs'),path=require('path');
const out=__dirname;
const outline=JSON.parse(fs.readFileSync(path.join(out,'outline.json'),'utf8'));
let html=fs.readFileSync(path.join(out,'hud-template.html'),'utf8');
let seed=812;const rand=()=>{seed=(Math.imul(seed,1664525)+1013904223)>>>0;return seed/4294967296;};
const stars=Array.from({length:350},()=>`<circle cx="${(rand()*1600).toFixed(1)}" cy="${(rand()*900).toFixed(1)}" r="${(.25+rand()*.6).toFixed(2)}"/>`).join('');
const scale=150/outline.height;
html=html.replace('__STARS__',stars).replace('__HULL_PATH__',outline.path).replace('__HULL_TRANSFORM__',`translate(${(120-outline.width*scale/2).toFixed(2)},45) scale(${scale.toFixed(5)})`);
const mounts=JSON.parse(fs.readFileSync(path.resolve(out,'../../../Art/Ships/Daedalus/WEAPON_MOUNTS.json'),'utf8'));
const categories={dorsal_railguns:'turret',ventral_railguns:'turret',bow_vls:'missile',asgard_beams:'beam'};
const projected=mounts.mounts.filter(m=>categories[m.group]).map(m=>{
 const x=(m.centerMetres[1]-outline.projectionLower[0])*outline.projectionScale+2;
 const y=(-m.centerMetres[0]-outline.projectionLower[1])*outline.projectionScale+2;
 return {x,y,id:m.mountID,group:categories[m.group]};
});
const zones=projected.map(m=>`<circle class="zone" data-weapon="${m.group}" cx="${m.x.toFixed(2)}" cy="${m.y.toFixed(2)}" r="3.6"><title>${m.id}</title></circle>`).join('');
html=html.replace('__WEAPON_ZONES__',zones);
for(const group of ['turret','missile','beam']){
 const points=projected.filter(m=>m.group===group);
 // All diagram geometry and locations come from the same model projection.
 // Zoom the missile diagram to its bow cluster; the others show the full hull.
 const view=group==='missile'?'52 7 90 137':`-65 -8 ${outline.width+130} ${outline.height+16}`;
 const marks=points.map(m=>`<circle class="weapon-point" cx="${m.x.toFixed(2)}" cy="${m.y.toFixed(2)}" r="${group==='missile'?2.8:group==='beam'?7:4.2}"/>`).join('');
 const diagram=`<svg class="weapon-map" viewBox="${view}" aria-hidden="true"><path class="weapon-hull" d="${outline.path}"/>${marks}</svg>`;
 html=html.replace(`__${group.toUpperCase()}_DIAGRAM__`,diagram);
}
fs.writeFileSync(path.join(out,'hud-fragment.html'),html.replace('__BACKGROUND__','').replace(/^[ \t]+$/gm,''));
html=html.replace('__BACKGROUND__','<img class="scene" src="background.png" alt="Referenční herní scéna; původní spodní HUD je mimo náhled"/>');
fs.writeFileSync(path.join(out,'hud-concept.html'),`<!doctype html><html lang="cs"><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><title>Daedalus — návrh HUD</title><style>html,body{margin:0;background:#020504;}button,input{cursor:pointer;}button:focus-visible,input:focus-visible{outline:2px solid #b7e5ba;outline-offset:3px;}</style>${html}</html>`);
console.log('Generated design preview. No game changes.');
