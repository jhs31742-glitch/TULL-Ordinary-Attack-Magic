from pathlib import Path
import sys,struct,subprocess,collections
sys.dont_write_bytecode=True
P=Path(r'[PROJECT_ROOT]');S=P/'evidence/STAGE_EVIDENCE/QA1_CP026_POSTQA_STANCE_VFX';Q=S/'033_STAFF_SMP_CANDIDATE';D=Q/'Data';C=Q/'StancesPatchData'
old=(S/'015_FINALIZE_VALIDATE.py').read_text('utf-8');exec(old[old.index('# Use the already-audited'):old.index('def fixheader')])
f=D/'Scripts/Source/TOAM_StaffSMP_Bridge.psc';t=f.read_text();assert 'PhysicsEnabled' not in t
t=t.replace('Int Property HelperSlotMask Auto','Int Property HelperSlotMask Auto\nGlobalVariable Property PhysicsEnabled Auto')
t=t.replace('    Bool left = IsCard','    If PhysicsEnabled == None || PhysicsEnabled.GetValue() < 0.5\n        ClearOwnHelper(akActor)\n        Return\n    EndIf\n    Bool left = IsCard')
f.write_text(t,encoding='utf-8')
compiler=Path(r'[LOCAL_PATH_REDACTED]
r=subprocess.run([str(compiler),'--game','skyrim','--import',str(imports),'--flags',str(imports/'TESV_Papyrus_Flags.flg'),'--output',str(D/'Scripts'),f.name],cwd=f.parent,capture_output=True,timeout=120)
(S/'040_PAPYRUS_GATE_COMPILE.log').write_bytes(r.stdout+r.stderr);assert r.returncode==0
U=lambda n:struct.pack('<I',n)
s16=lambda s:struct.pack('<H',len(s))+s.encode()
for root,index in [(D,2),(C,3)]:
 for f in root.rglob('*.esp'):
  b=f.read_bytes();head,g=groups(b)
  for sig,(gh,rr) in g.items():
   if sig!=b'WEAP':continue
   for i,r in enumerate(rr):
    ss=[]
    for k,v in subs(r):
     if k==b'VMAD':
      n=struct.unpack_from('<H',v,6)[0];o=6+2+n+1;assert struct.unpack_from('<H',v,o)[0]==3
      v=v[:o]+struct.pack('<H',4)+v[o+2:]+s16('PhysicsEnabled')+b'\x01\x01'+struct.pack('<HHI',0,0xffff,(index<<24)+0x806)
     ss.append((k,v))
    rr[i]=rec(r,ss)
  if root==D:
   ss=[]
   for k,v in subs(head):
    if k==b'HEDR':v=v[:4]+U(20)+U(0x807)
    ss.append((k,v))
   head=rec(head,ss)
  b=encode(head,g)
  if root==D:
   payload=b''.join(k+struct.pack('<H',len(v))+v for k,v in [(b'EDID',b'TOAM_SMP_Enabled\0'),(b'FNAM',b'f'),(b'FLTV',struct.pack('<f',1))])
   record=struct.pack('<4sIIIIHH',b'GLOB',len(payload),0,0x02000806,0,44,0)+payload
   b+=struct.pack('<4sI4sIHHHH',b'GRUP',24+len(record),b'GLOB',0,0,0,0,0)+record
  f.write_bytes(b)
print('SMP optional per-session gate compiled; default ON only when optional plugin enabled; core unchanged')
