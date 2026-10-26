from pathlib import Path
import sys,struct,json,hashlib,re,subprocess,xml.etree.ElementTree as ET,importlib.util,collections
sys.dont_write_bytecode=True
P=Path(r'[PROJECT_ROOT]');S=P/'evidence/STAGE_EVIDENCE/QA1_CP026_POSTQA_STANCE_VFX';A=P/'assets/TOAM/Data'
Q=S/'033_STAFF_SMP_CANDIDATE';assert not Q.exists();D=Q/'Data';D.mkdir(parents=True);C=Q/'StancesPatchData';C.mkdir()
sha=lambda b:hashlib.sha256(b).hexdigest().upper()
def put(f,b):
 f.parent.mkdir(parents=True,exist_ok=True);f.write_bytes(b.encode('utf-8') if isinstance(b,str) else b)
old=(S/'015_FINALIZE_VALIDATE.py').read_text('utf-8');start=old.index("# Use the already-audited");end=old.index('def fixheader')
exec(old[start:end])
def sub(k,v):return k+struct.pack('<H',len(v))+v
def rawrec(k,fid,ss,flags=0):
 b=b''.join(sub(k,v) for k,v in ss);return struct.pack('<4sIIIIHH',k,len(b),flags,fid,0,44,0)+b
def group(k,rs):
 b=b''.join(rs);return struct.pack('<4sI4sIHHHH',b'GRUP',24+len(b),k,0,0,0,0,0)+b
U=lambda n:struct.pack('<I',n)
Z=lambda t:t.encode('utf-8')+b'\0'
s16=lambda t:struct.pack('<H',len(t.encode('utf-8')))+t.encode('utf-8')
for model in ['Silver','Gold']:
 mesh=D/f'meshes/TOAM/Staffs/SMP/Arcana_Staff_{model}.nif';helper=D/f'meshes/TOAM/Staffs/SMP/Staff{model}Bridge.nif';mesh.parent.mkdir(parents=True,exist_ok=True);bone=Q/f'{model}_bones.txt';xml=f'SKSE\\Plugins\\hdtSkinnedMeshConfigs\\TOAM\\Staff{model}.xml'
 args=[str(S/'036_SMP_MESH_R3.exe'),str(A/f'meshes/TOAM/Staffs/Arcana_Staff_{model}.nif'),str(mesh),str(helper),str(bone),model,xml,'author']
 r=subprocess.run(args,capture_output=True,timeout=60);put(S/f'037_SMP_{model}_AUTHOR.log',r.stdout+r.stderr);assert r.returncode==0,model
 for f in [mesh,helper]:
  b=f.read_bytes()
  for m in list(re.finditer(rb'(?i)textures[\\/][a-z0-9_ .()!+\-/\\]+\.dds',b)):b=b[:m.start()]+m.group().replace(b'/',b'\\')+b[m.end():]
  f.write_bytes(b)
 args[-1]='verify';r=subprocess.run(args,capture_output=True,timeout=60);put(S/f'037_SMP_{model}_VERIFY.log',r.stdout+r.stderr);assert r.returncode==0,(model,r.stdout,r.stderr)
 rows=[l.split() for l in bone.read_text().splitlines()[1:]];names=[r[0] for r in rows];pos=[[float(v) for v in r[1:]] for r in rows];assert len(names)==9
 root=ET.Element('system')
 def el(parent,tag,text=None,**attrs):
  e=ET.SubElement(parent,tag,{k:str(v) for k,v in attrs.items()})
  if text is not None:e.text=str(text)
  return e
 el(root,'bone',name='TOAM_SMP_'+model+'_Root')
 el(root,'bone',name=names[0])
 default=el(root,'bone-default');el(default,'mass',.04);el(default,'inertia',x=2,y=2,z=2)
 el(default,'linearDamping',.7);el(default,'angularDamping',.75);el(default,'friction',0);el(default,'restitution',0);el(default,'gravity-factor',1);el(default,'wind-factor',.05)
 for name in names[1:]:el(root,'bone',name=name)
 limits=el(root,'generic-constraint-default')
 el(limits,'linearLowerLimit',x=0,y=0,z=0);el(limits,'linearUpperLimit',x=0,y=0,z=0)
 el(limits,'angularLowerLimit',x=-.35,y=-.2,z=-.35);el(limits,'angularUpperLimit',x=.35,y=.2,z=.35)
 el(limits,'useLinearReferenceFrameA','false')
 el(limits,'angularStiffness',x=.05,y=.03,z=.05);el(limits,'angularDamping',x=.7,y=.7,z=.7)
 grp=el(root,'constraint-group')
 for i in range(1,9):
  con=el(grp,'generic-constraint',bodyA=names[i],bodyB=names[i-1])
  for tag,xyz in [('frameInA',[0,0,0]),('frameInB',[pos[i][j]-pos[i-1][j] for j in range(3)])]:
   frame=el(con,tag);el(frame,'basis',x=0,y=0,z=0,w=1);el(frame,'origin',**dict(zip(['x','y','z'],xyz)))
 ET.indent(root);put(D/xml.replace('\\','/'),ET.tostring(root,encoding='utf-8',xml_declaration=True))
script='''Scriptname TOAM_StaffSMP_Bridge extends ObjectReference
; Authored for TOAM; Riilungolz was inspected as a technical reference only.
; One delayed equipment transition, no polling, forced locks or movement calls.
Armor Property PhysicsHelper Auto
Keyword Property StaffCategory Auto
Int Property HelperSlotMask Auto
Actor ownerActor

Bool Function IsCard(Weapon akWeapon)
    Return akWeapon != None && akWeapon.HasKeyword(StaffCategory)
EndFunction

Function ClearOwnHelper(Actor akActor)
    If akActor.IsEquipped(PhysicsHelper)
        akActor.UnequipItem(PhysicsHelper, False, True)
    EndIf
    Int count = akActor.GetItemCount(PhysicsHelper)
    If count > 0
        akActor.RemoveItem(PhysicsHelper, count, True)
    EndIf
EndFunction

Function Reconcile(Actor akActor)
    If akActor == None || PhysicsHelper == None || StaffCategory == None
        Return
    EndIf
    Bool left = IsCard(akActor.GetEquippedWeapon(True))
    Bool right = IsCard(akActor.GetEquippedWeapon(False))
    ; Duplicate names in two identical weapon rigs cannot safely own one solver.
    If left && right
        ClearOwnHelper(akActor)
        Debug.Trace("[TOAM SMP] same-model dual wield: static fallback")
        Return
    EndIf
    If left || right
        Form occupied = akActor.GetWornForm(HelperSlotMask)
        If occupied != None && occupied != PhysicsHelper
            Debug.Trace("[TOAM SMP] helper slot occupied: static fallback")
            Return
        EndIf
        If akActor.GetItemCount(PhysicsHelper) == 0
            akActor.AddItem(PhysicsHelper, 1, True)
        EndIf
        If !akActor.IsEquipped(PhysicsHelper)
            akActor.EquipItem(PhysicsHelper, False, True)
        EndIf
    Else
        ClearOwnHelper(akActor)
    EndIf
EndFunction

Event OnEquipped(Actor akActor)
    ownerActor = akActor
    RegisterForSingleUpdate(0.20)
EndEvent

Event OnUnequipped(Actor akActor)
    ownerActor = akActor
    RegisterForSingleUpdate(0.20)
EndEvent

Event OnContainerChanged(ObjectReference akNewContainer, ObjectReference akOldContainer)
    Actor oldOwner = akOldContainer as Actor
    If oldOwner != None && akNewContainer != akOldContainer
        Reconcile(oldOwner)
    EndIf
EndEvent

Event OnUpdate()
    Reconcile(ownerActor)
EndEvent
'''
put(D/'Scripts/Source/TOAM_StaffSMP_Bridge.psc',script)
(D/'Scripts').mkdir(exist_ok=True)
compiler=Path(r'[LOCAL_PATH_REDACTED]
r=subprocess.run([str(compiler),'--game','skyrim','--import',str(imports),'--flags',str(imports/'TESV_Papyrus_Flags.flg'),'--output',str(D/'Scripts'),str(D/'Scripts/Source/TOAM_StaffSMP_Bridge.psc')],capture_output=True,timeout=120)
put(S/'037_PAPYRUS_COMPILE.log',r.stdout+r.stderr);assert r.returncode==0,(r.stdout,r.stderr)
pex=(D/'Scripts/TOAM_StaffSMP_Bridge.pex').read_bytes();assert pex[:4]==b'\xfa\x57\xc0\xde' and struct.unpack_from('>H',pex,6)[0]==1
base='TULL Ordinary Attack Magic.esp';stances='TULL Ordinary Attack Magic - Stances Addon.esp'
name='TULL Ordinary Attack Magic - Staff SMP.esp';patch='TULL Ordinary Attack Magic - Staff SMP - Stances Patch.esp'
def header(masters,n,nxt):
 return rawrec(b'TES4',0,[(b'HEDR',struct.pack('<fII',1.7,n,nxt)),(b'CNAM',Z('TULL / TOAM SMP candidate'))]+[(k,v) for m in masters for k,v in [(b'MAST',Z(m)),(b'DATA',b'\0'*8)]],0x200)
def vmad(model,index):
 silver=model=='Silver';out=struct.pack('<3H',5,2,1)+s16('TOAM_StaffSMP_Bridge')+b'\0'+struct.pack('<H',3)
 for key,fid in [('PhysicsHelper',(index<<24)+(0x802 if silver else 0x803)),('StaffCategory',(index<<24)+(0x800 if silver else 0x801))]:
  out+=s16(key)+b'\x01\x01'+struct.pack('<HHI',0,0xffff,fid)
 out+=s16('HelperSlotMask')+b'\x03\x01'+U(0x80000000 if silver else 0x40000000)
 return out
def weapon(r,index):
 ss=subs(r);d=dict(ss);model='Silver' if b'Silver' in d[b'MODL'] else 'Gold';assert model.encode() in d[b'MODL']
 assert b'VMAD' not in d
 kw=list(struct.unpack('<'+'I'*(len(d.get(b'KWDA',b''))//4),d.get(b'KWDA',b'')));kw.append((index<<24)+(0x800 if model=='Silver' else 0x801))
 out=[]
 for k,v in ss:
  if k==b'MODL':v=Z(f'TOAM\\Staffs\\SMP\\Arcana_Staff_{model}.nif')
  if k in [b'KSIZ',b'KWDA']:continue
  out.append((k,v))
  if k==b'EDID':out.append((b'VMAD',vmad(model,index)))
  if k==b'ETYP':out += [(b'KSIZ',U(len(kw))),(b'KWDA',b''.join(map(U,kw)))]
 assert len([1 for k,v in out if k==b'KWDA'])==1
 return rec(r,out)
# Standard Skyrim armor contract; authored records, no reference-mod assets or byte records copied.
helpergroups={b'KYWD':[],b'ARMA':[],b'ARMO':[]}
# Reuse the existing authored race list, not guessed FormIDs.
legacy=(Path(r'[LOCAL_PATH_REDACTED]
racevals=[v for r in records(legacy).values() if r[:4]==b'ARMA' for k,v in subs(r) if k==b'MODL'];assert len(racevals)>=20
for i,model in enumerate(['Silver','Gold']):
 key=0x02000800+i;armor=0x02000802+i;arma=0x02000804+i;mask=0x80000000 if i==0 else 0x40000000;bod=struct.pack('<II',mask,2)
 helpergroups[b'KYWD'].append(rawrec(b'KYWD',key,[(b'EDID',Z('TOAM_SMP_'+model+'_Category'))]))
 helpergroups[b'ARMO'].append(rawrec(b'ARMO',armor,[(b'EDID',Z('TOAM_SMP_'+model+'_Bridge')),(b'OBND',b'\0'*12),(b'FULL',Z('TOAM SMP '+model+' Bridge')),(b'BOD2',bod),(b'RNAM',U(0x19)),(b'DESC',b'\0'),(b'MODL',U(arma)),(b'DATA',struct.pack('<If',0,0)),(b'DNAM',U(0))],4))
 helpergroups[b'ARMA'].append(rawrec(b'ARMA',arma,[(b'EDID',Z('TOAM_SMP_'+model+'_BridgeAddon')),(b'BOD2',bod),(b'RNAM',U(0x19)),(b'DNAM',b'\0'*12),(b'MOD2',Z(f'TOAM\\Staffs\\SMP\\Staff{model}Bridge.nif')),(b'MOD3',Z(f'TOAM\\Staffs\\SMP\\Staff{model}Bridge.nif'))]+[(b'MODL',v) for v in racevals]))
for lang in ['en','kr']:
 root=A/'Language'/lang
 w=[weapon(r,2) for r in records((root/base).read_bytes()).values() if r[:4]==b'WEAP'];assert len(w)==8
 gr={**helpergroups,b'WEAP':w};blob=header(['Skyrim.esm',base],sum(map(len,gr.values()))+len(gr),0x806)+b''.join(group(k,v) for k,v in gr.items())
 put(D/'Language'/lang/name,blob)
 w=[weapon(r,3) for r in records((root/stances).read_bytes()).values() if r[:4]==b'WEAP'];assert len(w)==2
 blob=header(['Skyrim.esm',base,stances,name],3,0x800)+group(b'WEAP',w);put(C/'Language'/lang/patch,blob)
for root,name in [(D,name),(C,patch)]:put(root/name,(root/'Language/en'/name).read_bytes())
print('SMP candidate meshes+XML+Papyrus+8/2 typed overrides authored; not deployed')
