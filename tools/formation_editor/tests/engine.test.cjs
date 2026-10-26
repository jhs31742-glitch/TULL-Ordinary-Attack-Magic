const assert=require('node:assert/strict'),fs=require('node:fs'),path=require('node:path');
const E=require('../engine.js'),library=JSON.parse(fs.readFileSync(path.join(__dirname,'../presets/toam-reviewed.json'),'utf8'));
assert(E.validateLibrary(library));const air=library.spells.find(s=>s.name.en.includes('airborne')).project;
assert(E.assets(air).some(a=>a.position[2]<0));assert(E.assets(air).length===12);
let p=E.blank(),a=E.newAsset('a');p.formations.single.assets.push(a);p.anchors.actor.yaw=37;p.anchors.target.yaw=-61;
for(let i=0;i<80;i++){a.position=[i-40,72-i,i*.1];a.normal=E.norm([Math.sin(i*.12),1,Math.cos(i*.3)]);a.rotation=[i*.7,-40+i,78-i*.1];
 const point=E.toWorld(p,a.anchor,a.position),vectors=[[1,0,0],[0,1,0],[0,0,1]].map(v=>E.vectorWorld(p,a,v));
 E.reanchor(p,a,a.anchor==='actor'?'target':'actor');assert(E.sub(E.toWorld(p,a.anchor,a.position),point).every(v=>Math.abs(v)<1e-6));
 vectors.forEach((v,j)=>assert(E.sub(E.vectorWorld(p,a,[[1,0,0],[0,1,0],[0,0,1]][j]),v).every(n=>Math.abs(n)<1e-6)));
}
a=E.newAsset('b');p.formations.single.assets.push(a);const before=E.toWorld(p,'actor',a.position);E.translate(p,new Set(['b']),[10,20,-5]);assert(E.sub(E.toWorld(p,'actor',a.position),before).every((x,i)=>Math.abs(x-[10,20,-5][i])<1e-7));
const round=JSON.parse(JSON.stringify(library));assert(E.validateLibrary(round));let bad=E.clone(library);bad.spells[0].project.formations.single.assets[0].scale=0;assert.throws(()=>E.validateLibrary(bad));bad=E.clone(library);bad.spells[0].project.formations.single.assets[0].anchor='__proto__';assert.throws(()=>E.validateLibrary(bad));
const dual=library.spells.find(s=>s.name.en.startsWith('Chariot')).project;assert(dual.formations.single.assets.length===5&&dual.formations.dual.assets.length===10);assert(dual.formations.dual.assets.filter(a=>a.hand==='right').length===5);
console.log(JSON.stringify({result:'PASS',coordinateRoundtrips:80,fullOrientationReanchor:true,librarySpells:library.spells.length,dualCount:10,airBelowFeet:true}));
