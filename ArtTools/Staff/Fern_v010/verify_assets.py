import sys, json, math, struct, hashlib
from pathlib import Path
from collections import Counter
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from inspect_sources import NifFile,subs,records,sha,buf
budget=json.loads((ROOT/'evidence'/'mesh_budget.json').read_text())
source=json.loads((ROOT/'source'/'mesh_data.json').read_text())
baseline=json.loads((ROOT/'evidence'/'vanilla_staff01.json').read_text())
report={'status':'PASS','runtime':'UNVERIFIED','nifs':{},'dds':{},'plugin':{},'protected_inputs':[]}
for bn,on in [('staff01.nif','FernStaff.nif'),('1stpersonstaff01.nif','1stPersonFernStaff.nif')]:
    p=ROOT/'package'/'meshes'/'FernStaff'/on;n=NifFile(str(p));assert n.game=='SKYRIMSE'
    raw=p.read_bytes();hp=raw.index(b'\n')+1
    assert struct.unpack_from('<I',raw,hp)[0]==0x14020007 and raw[hp+4]==1
    assert struct.unpack_from('<I',raw,hp+5)[0]==12 and struct.unpack_from('<I',raw,hp+13)[0]==100
    assert n.root.blockname=='BSFadeNode' and n.root.flags==14
    assert n.root.get_extra_data(blockname='NiStringExtraData',name='Prn').string_data=='WeaponStaff'
    assert n.root.get_extra_data(blockname='BSXFlags').properties.integerData==194
    inv=n.root.get_extra_data(blockname='BSInvMarker').properties
    oldinv=next(x['properties'] for x in baseline[bn]['extras'] if x['type']=='BSInvMarker')
    assert all(getattr(inv,k)==oldinv[k] for k in ['rot0','rot1','rot2','zoom'])
    assert len(n.shapes)==len(source)==9
    result=[]
    for sh,expected in zip(n.shapes,source):
        assert sh.name==expected['name']
        v,ts,uv,norm=sh.verts,sh.tris,sh.uvs,sh.normals
        assert len(v)==len(expected['verts']) and len(ts)==len(expected['tris'])
        assert len(v)==len(uv)==len(norm) and len(v)<15000 and len(ts)<15000
        poserr=max(abs(a-b) for p,q in zip(v,expected['verts']) for a,b in zip(p,q));assert poserr<.00003,(sh.name,poserr)
        assert ts==[tuple(t) for t in expected['tris']]
        assert all(math.isfinite(x) for row in [*v,*uv,*norm] for x in row)
        assert min(sum(x*x for x in a) for a in norm)>.95
        normalerr=max(abs(a-b) for p,q in zip(norm,expected['normals']) for a,b in zip(p,q));assert normalerr<.01
        uverr=max(max(abs(a[0]-b[0]),abs(a[1]-(1-b[1]))) for a,b in zip(uv,expected['uvs']));assert uverr<.0005
        center=sh.properties.boundingSphereCenter;radius=sh.properties.boundingSphereRadius
        maxexcess=max(math.sqrt(sum((v[i]-center[i])**2 for i in range(3)))-radius for v in v)
        assert radius>0 and maxexcess<.00003,(sh.name,maxexcess)
        assert not sh.skin_instance_name
        assert sh.shader_block_name=='BSLightingShaderProperty'
        assert sh.shader.properties.Emissive_Mult==0
        for tri in ts:
            assert len(set(tri))==3 and min(tri)>=0 and max(tri)<len(v)
            a,b,c=[v[i] for i in tri];u=[b[j]-a[j] for j in range(3)];w=[c[j]-a[j] for j in range(3)]
            cross=[u[1]*w[2]-u[2]*w[1],u[2]*w[0]-u[0]*w[2],u[0]*w[1]-u[1]*w[0]]
            assert sum(x*x for x in cross)>1e-16,(sh.name,tri)
        for k,t in sh.textures.items():
            if t:assert (ROOT/'package'/Path(t.replace('\\','/'))).is_file(),t
        result.append(dict(name=sh.name,vertices=len(v),triangles=len(ts),position_max_error=poserr,normal_max_error=normalerr,uv_max_error=uverr,bound_max_excess=maxexcess,shader=sh.shader_block_name,textures=sh.textures))
    col=n.root.collision_object;assert col and col.body.shape.blockname=='bhkCapsuleShape'
    before=baseline[bn]['collision']
    assert {k:v for k,v in buf(col.body.properties).items() if k!='shapeID'}=={k:v for k,v in before['body'].items() if k!='shapeID'}
    assert buf(col.body.shape.properties)==before['shape']
    vv=[v for s in n.shapes for v in s.verts]
    bounds=[[min(v[i] for v in vv),max(v[i] for v in vv)] for i in range(3)]
    assert abs((bounds[1][1]-bounds[1][0])-134.60506057739258)<.02
    report['nifs'][on]=dict(sha256=sha(p),parts=result,bounds=bounds,collision='vanilla capsule preserved',Prn='WeaponStaff')
for p in sorted((ROOT/'package'/'textures'/'FernStaff').glob('*.dds')):
    d=p.read_bytes();assert d[:4]==b'DDS ' and struct.unpack_from('<I',d,4)[0]==124
    h,w,_,_,mip=struct.unpack_from('<5I',d,12)
    cc=d[84:88].decode();assert cc==('DXT5' if '_n.' in p.name else 'DXT1')
    assert mip==int(math.log2(max(w,h)))+1
    block=16 if cc=='DXT5' else 8
    expected=128+sum(max(1,(max(1,w>>i)+3)//4)*max(1,(max(1,h>>i)+3)//4)*block for i in range(mip))
    assert len(d)==expected,(p,len(d),expected)
    report['dds'][p.name]=dict(width=w,height=h,format=cc,mip_levels=mip,sha256=sha(p))
assert len(report['dds'])==8
p=ROOT/'package'/'FernStaff.esp';rr=list(records(p.read_bytes()));assert len(rr)==3
assert [r[0] for r in rr]==[b'TES4',b'STAT',b'WEAP']
header=dict(subs(rr[0][3]));assert rr[0][2]==0x200 and header[b'MAST']==b'Skyrim.esm\0'
assert struct.unpack('<fII',header[b'HEDR'])[1:]==(2,0x802)
stat,wp=dict(subs(rr[1][3])),dict(subs(rr[2][3]))
assert rr[1][1]==0x01000801 and rr[2][1]==0x01000800
assert struct.unpack('<I',wp[b'WNAM'])[0]==rr[1][1]
assert b'EITM' not in wp and b'EAMT' not in wp and b'VMAD' not in wp
assert wp[b'DNAM'][0]==8 and not(struct.unpack_from('<H',wp[b'DNAM'],12)[0]&0x88)
assert wp[b'FULL']==b"Fern's Staff\0"
assert (ROOT/'package'/'meshes'/wp[b'MODL'].rstrip(b'\0').decode().replace('\\','/')).is_file()
assert (ROOT/'package'/'meshes'/stat[b'MODL'].rstrip(b'\0').decode().replace('\\','/')).is_file()
report['plugin']=dict(sha256=sha(p),records=2,overrides=0,enchantment=None,edid=wp[b'EDID'].rstrip(b'\0').decode(),esl=True)
for k,b in baseline.items():
    assert sha(b['path'])==b['sha256'];report['protected_inputs'].append(dict(path=b['path'],sha256=b['sha256'],unchanged=True))
db=json.loads((ROOT/'evidence'/'vanilla_staff_records.json').read_text())
assert sha(db['esm_path'])==db['esm_sha256'];report['protected_inputs'].append(dict(path=db['esm_path'],sha256=db['esm_sha256'],unchanged=True))
report['total_vertices']=sum(x['vertices'] for x in budget);report['total_triangles']=sum(x['triangles'] for x in budget)
(ROOT/'evidence'/'validation.json').write_text(json.dumps(report,indent=2),encoding='utf8')
print('STATIC_PASS',report['total_vertices'],'vertices',report['total_triangles'],'triangles; 9 parts; 2 NIFs; 8 mipmapped DDS; 2 new ESP records; protected inputs unchanged')
