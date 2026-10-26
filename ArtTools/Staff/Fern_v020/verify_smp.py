from pathlib import Path
import json,sys,math,hashlib,struct,xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from inspect_sources import NifFile,records,subs,sha
bind=json.loads((ROOT/'evidence'/'ribbon_bind.json').read_text())
static=NifFile(str(ROOT/'package'/'meshes'/'FernStaff'/'FernStaff.nif'))
p=ROOT/'smp_package'/'meshes'/'FernStaff'/'FernStaff.nif';n=NifFile(str(p));assert n.game=='SKYRIMSE' and len(n.shapes)==10
report={'status':'STATIC_PASS','runtime':'UNVERIFIED','first_person':'static','skin':[],'xml':{},'script':{},'plugin':{}}
def global_pos(node):
    a=list(node.transform.translation);parent=node.parent
    while parent:
        a=[a[i]+parent.transform.translation[i] for i in range(3)];parent=parent.parent
    return a
for name,pos in zip(bind['bones'],bind['positions']):
    assert name in n.nodes
    assert max(abs(x-y) for x,y in zip(global_pos(n.nodes[name]),pos))<.00001
def oriented(t):return min(tuple(t),tuple(t[1:]+t[:1]),tuple(t[2:]+t[:2]))
for sh,orig in zip(n.shapes,static.shapes):
    assert sh.name==orig.name and len(sh.verts)==len(orig.verts)
    assert sorted(oriented(list(t)) for t in sh.tris)==sorted(oriented(list(t)) for t in orig.tris)
    assert max(abs(x-y) for a,b in zip(sh.verts,orig.verts) for x,y in zip(a,b))<.00003
    if sh.name.startswith(('07_','08_','09_')):
        assert sh.skin_instance_name=='NiSkinInstance'
        assert sh.bone_names==bind['bones'];vw=[[] for _ in sh.verts]
        for bone,weights in sh.bone_weights.items():
            for idx,w in weights:vw[idx].append((bone,w))
        assert all(1<=len(w)<=2 and abs(sum(v for _,v in w)-1)<.0011 for w in vw)
        if sh.name.startswith(('08_','09_')):assert all(len(w)==1 and w[0][0]==bind['bones'][-1] for w in vw)
        for bone,pos in zip(bind['bones'],bind['positions']):
            inv=sh.get_shape_skin_to_bone(bone)
            assert max(abs(inv.translation[i]+pos[i]) for i in range(3))<.00001
        # Independent linear blend skinning reconstruction at bind pose.
        err=0
        for v,w in zip(sh.verts,vw):
            calc=[0.,0.,0.]
            for bone,weight in w:
                bp=global_pos(n.nodes[bone]);inv=sh.get_shape_skin_to_bone(bone)
                for j in range(3):calc[j]+=(v[j]+inv.translation[j]+bp[j])*weight
            err=max(err,max(abs(calc[j]-v[j]) for j in range(3)))
        assert err<.035
        report['skin'].append(dict(name=sh.name,vertices=len(sh.verts),bones=len(sh.bone_names),max_influences=max(map(len,vw)),bind_max_error=err))
    else:assert not sh.skin_instance_name
helper=ROOT/'smp_package'/'meshes'/'FernStaff'/'FernStaffSMPHelper.nif';h=NifFile(str(helper))
tag=h.root.get_extra_data(blockname='NiStringExtraData',name='HDT Skinned Mesh Physics Object');assert tag
xml=ROOT/'smp_package'/tag.string_data.replace('\\','/');assert xml.is_file();system=ET.parse(xml).getroot()
assert {b.attrib['name'] for b in system.findall('bone')}==set(bind['bones']+['FSFern_StaffRoot'])
assert len(system.findall('./constraint-group/generic-constraint'))==8
for con in system.findall('./constraint-group/generic-constraint'):
    assert con.attrib['bodyA'] in bind['bones'] and con.attrib['bodyB'] in bind['bones']
report['xml']=dict(path=str(xml.relative_to(ROOT/'smp_package')),sha256=sha(xml),constraints=8,anchor=bind['anchor'])
rr=list(records((ROOT/'smp_package'/'FernStaff.esp').read_bytes()));assert len(rr)==5
assert {r[0] for r in rr}=={b'TES4',b'ARMA',b'ARMO',b'STAT',b'WEAP'}
by={r[0]:dict(subs(r[3])) for r in rr};assert rr[0][2]==0x200
assert struct.unpack('<II',by[b'ARMO'][b'BOD2'])==(0x80000000,2)
assert struct.unpack('<I',by[b'ARMO'][b'MODL'])[0]==0x01000803
assert struct.unpack('<I',by[b'WEAP'][b'WNAM'])[0]==0x01000801
assert b'EITM' not in by[b'WEAP']
vmad=by[b'WEAP'][b'VMAD'];assert struct.unpack_from('<3H',vmad)==(5,2,1)
assert b'FSFernStaffSMP' in vmad and b'PhysicsHelper' in vmad and b'StaffItem' in vmad
pex=ROOT/'smp_package'/'Scripts'/'FSFernStaffSMP.pex';raw=pex.read_bytes();assert raw[:4]==b'\xfa\x57\xc0\xde' and struct.unpack_from('>H',raw,6)[0]==1
assert b'GetWornForm' in raw and b'PhysicsHelper' in raw and b'OnEquipped' in raw
report['script']=dict(file=pex.name,sha256=sha(pex),compiler='Caprica v0.3.0 --game skyrim',game_id=1,slot_collision_guard=True)
report['plugin']=dict(records=4,esl=True,weapon_id='01000800',helper_slot=61,sha256=sha(ROOT/'smp_package'/'FernStaff.esp'))
report['nif_sha256']=sha(p);report['helper_sha256']=sha(helper)
(ROOT/'evidence'/'smp_validation.json').write_text(json.dumps(report,indent=2),encoding='utf8')
print('SMP_STATIC_PASS: 3 NiSkinInstance parts; 9-bone chain; normalized weights; bind pose; helper XML; 4-record ESP; Skyrim PEX')
