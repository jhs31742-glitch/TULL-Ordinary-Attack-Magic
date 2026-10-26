(function(root){'use strict';
const PROJECT='TULL_FORMATION_PROJECT_1',LIBRARY='TULL_FORMATION_LIBRARY_1';
const clone=x=>JSON.parse(JSON.stringify(x));
const add=(a,b)=>a.map((v,i)=>v+b[i]),sub=(a,b)=>a.map((v,i)=>v-b[i]),mul=(a,s)=>a.map(v=>v*s);
const dot=(a,b)=>a.reduce((s,v,i)=>s+v*b[i],0),cross=(a,b)=>[a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]];
const norm=v=>{const n=Math.hypot(...v);if(n<1e-7)throw Error('Zero normal');return mul(v,1/n);};
function yaw(v,degrees){const a=degrees*Math.PI/180,c=Math.cos(a),s=Math.sin(a);return [c*v[0]+s*v[1],-s*v[0]+c*v[1],v[2]];}
function origin(a){return add(a.position,add(a.offset||[0,0,0],[0,0,a.pivot==='center'?a.height/2:0]));}
const toWorld=(p,anchor,v)=>add(origin(p.anchors[anchor]),yaw(v,p.anchors[anchor].yaw));
const fromWorld=(p,anchor,v)=>yaw(sub(v,origin(p.anchors[anchor])),-p.anchors[anchor].yaw);
function euler(v,d){let[x,y,z]=v;const [a,b,c]=d.map(v=>v*Math.PI/180);[y,z]=[y*Math.cos(a)-z*Math.sin(a),y*Math.sin(a)+z*Math.cos(a)];[x,z]=[x*Math.cos(b)+z*Math.sin(b),-x*Math.sin(b)+z*Math.cos(b)];return [x*Math.cos(c)-y*Math.sin(c),x*Math.sin(c)+y*Math.cos(c),z];}
function localVector(asset,v){const n=norm(asset.normal),ref=Math.abs(n[2])>.95?[1,0,0]:[0,0,1],x=norm(cross(n,ref)),z=norm(cross(x,n)),r=euler(v,asset.rotation);return add(add(mul(x,r[0]),mul(n,r[1])),mul(z,r[2]));}
const vectorWorld=(p,a,v)=>yaw(localVector(a,v),p.anchors[a.anchor].yaw);
const assets=p=>p.formations[p.activeFormation].assets;
function reanchor(p,a,next){
 const w=toWorld(p,a.anchor,a.position),delta=p.anchors[a.anchor].yaw-p.anchors[next].yaw;
 const columns=[[1,0,0],[0,1,0],[0,0,1]].map(v=>yaw(localVector(a,v),delta));
 const n=yaw(a.normal,delta),base={normal:n,rotation:[0,0,0]},axes=[[1,0,0],[0,1,0],[0,0,1]].map(v=>localVector(base,v));
 const r=axes.map(axis=>columns.map(v=>dot(axis,v))),b=Math.asin(Math.max(-1,Math.min(1,-r[2][0])));let x,z;
 if(Math.abs(Math.cos(b))>1e-6){x=Math.atan2(r[2][1],r[2][2]);z=Math.atan2(r[1][0],r[0][0]);}else{x=Math.atan2(-r[1][2],r[1][1]);z=0;}
 a.anchor=next;a.position=fromWorld(p,next,w);a.normal=n;a.rotation=[x,b,z].map(v=>v*180/Math.PI);
}
function translate(p,ids,delta){for(const a of assets(p))if(ids.has(a.id))a.position=add(a.position,yaw(delta,-p.anchors[a.anchor].yaw));}
function finite(v){return typeof v==='number'&&Number.isFinite(v)&&Math.abs(v)<=100000;}
function vec(v){return Array.isArray(v)&&v.length===3&&v.every(finite);}
function validateProject(p){
 if(!p||p.schema!==PROJECT||!p.anchors||!p.formations)throw Error('Unsupported project schema');
 for(const key of ['actor','target','world']){const a=p.anchors[key];if(!a||!vec(a.position)||!vec(a.offset)||!finite(a.yaw)||!finite(a.height)||a.height<=0||!['feet','center'].includes(a.pivot))throw Error('Invalid anchor');}
 const forms=Object.keys(p.formations);if(!forms.length||forms.length>16||!Object.hasOwn(p.formations,p.activeFormation))throw Error('Invalid casting variants');let total=0;
 for(const k of forms){const f=p.formations[k];if(!f||!Array.isArray(f.assets)||f.assets.length>512)throw Error('Too many assets');const ids=new Set();
  for(const a of f.assets){if(!a||typeof a.id!=='string'||ids.has(a.id)||!['actor','target','world'].includes(a.anchor)||!vec(a.position)||!vec(a.normal)||Math.hypot(...a.normal)<1e-6||!vec(a.rotation)||!finite(a.scale)||a.scale<.001||a.scale>100||!finite(a.radius)||a.radius<=0||!vec(a.size)||!a.size.every(v=>v>0)||!['circle','sword','marker'].includes(a.kind)||!['left','right','shared'].includes(a.hand)||typeof a.group!=='string')throw Error('Invalid asset');ids.add(a.id);total++;}
 }if(total>4096)throw Error('Too many assets in project');return true;
}
function validateLibrary(l){if(!l||l.schema!==LIBRARY||typeof l.name!=='string'||!Array.isArray(l.spells)||!l.spells.length||l.spells.length>64)throw Error('Invalid preset set');const ids=new Set();for(const s of l.spells){if(!s||typeof s.id!=='string'||ids.has(s.id))throw Error('Duplicate spell ID');ids.add(s.id);validateProject(s.project);}if(!ids.has(l.activeSpell))throw Error('Invalid active spell');return true;}
function blank(){return {schema:PROJECT,coordinates:'Skyrim X right, Y forward, Z up;game units;degrees',anchors:{actor:{position:[0,0,0],offset:[0,0,0],yaw:0,height:128,pivot:'feet'},target:{position:[260,180,0],offset:[0,0,0],yaw:0,height:128,pivot:'center'},world:{position:[0,0,0],offset:[0,0,0],yaw:0,height:128,pivot:'feet'}},activeFormation:'single',formations:{single:{name:{en:'Single cast',kr:'단독 시전'},assets:[]}},metadata:{}};}
function newAsset(id,kind='circle'){return {id,name:id,kind,hand:'right',anchor:'actor',position:[0,64,100],normal:[0,1,0],rotation:[0,0,0],scale:1,radius:100,size:[12,8,64],group:'group1',attachment:'follow',modelPath:''};}
const api={PROJECT,LIBRARY,clone,add,sub,mul,dot,cross,norm,yaw,origin,toWorld,fromWorld,euler,localVector,vectorWorld,assets,reanchor,translate,validateProject,validateLibrary,blank,newAsset};
if(typeof module!=='undefined'&&module.exports)module.exports=api;else root.FormationEngine=api;
})(typeof globalThis!=='undefined'?globalThis:this);
