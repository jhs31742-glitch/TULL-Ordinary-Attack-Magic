"""TOAM release verification and deterministic ZIP assembly.
Mutation callers must wrap writes, backup, rollback and 008 synchronization
in the project's complete PowerShell transaction. CLI verification is read-only.
"""
from pathlib import Path
import io,json,hashlib,struct,zipfile,argparse
def digest(b):return hashlib.sha256(b).hexdigest().upper()
def verify_data(root,manifest):
 root=Path(root);rows=json.loads(Path(manifest).read_text(encoding="utf-8"))["files"]
 expected={r["path"].replace("\\","/"):r for r in rows}
 actual={p.relative_to(root).as_posix():p for p in root.rglob("*") if p.is_file()}
 if actual.keys()!=expected.keys():raise ValueError("Data file set differs from release manifest")
 for rel,p in actual.items():
  b=p.read_bytes();r=expected[rel]
  if len(b)!=r["bytes"] or digest(b)!=r["sha256"]:raise ValueError("Data drift: "+rel)
 return {rel:p.read_bytes() for rel,p in actual.items()}
def assemble(data,dll):
 b=bytes(dll);off=struct.unpack_from("<I",b,0x3c)[0]
 if b[off:off+4]!=b"PE\0\0" or struct.unpack_from("<H",b,off+4)[0]!=0x8664:raise ValueError("AMD64 TOAM.dll required")
 out={rel:value for rel,value in data.items() if not rel.replace("\\","/").lower().startswith("docs/")};out["SKSE/Plugins/TOAM.dll"]=b
 forbidden=(".pdb",".bak",".ps1",".py",".cpp",".obj",".lib",".exp")
 for rel in out:
  p=Path(rel)
  if p.is_absolute() or ".." in p.parts or p.suffix.lower() in forbidden:raise ValueError("Unsafe release path "+rel)
 if any(k.lower().endswith(("arcanastaffs.esp","arcanastaffs-stances.esp","ordinaryoffensivemagic.dll","ordinaryattackmagic.dll","toom.dll","arcanastaffs.dll")) for k in out):raise ValueError("Legacy executable/plugin in package")
 return out
def zip_bytes(files):
 buff=io.BytesIO()
 with zipfile.ZipFile(buff,"w",compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for rel,b in sorted(files.items()):
   info=zipfile.ZipInfo(rel,(2026,10,25,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o644<<16;z.writestr(info,b)
 result=buff.getvalue()
 with zipfile.ZipFile(io.BytesIO(result)) as z:
  if set(z.namelist())!=set(files):raise ValueError("ZIP path set mismatch")
  for rel,b in files.items():
   if z.read(rel)!=b:raise ValueError("ZIP identity mismatch "+rel)
 return result

def validate_aimed_charge_carriers(data):
 """SE97 input GetAVEffect requires a projectile before update-timer suppression."""
 checked=0
 for rel,b in data.items():
  if not rel.lower().endswith("tull ordinary attack magic.esp"):continue
  pos=0;records={}
  while pos<len(b):
   sig,n=struct.unpack_from("<4sI",b,pos)
   if sig==b"GRUP":pos+=24;continue
   end=pos+24+n;q=pos+24;ss=[]
   while q<end:
    key,length=struct.unpack_from("<4sH",b,q);q+=6;ss.append((key,b[q:q+length]));q+=length
   if q!=end:raise ValueError("Carrier ESP length: "+rel)
   records[struct.unpack_from("<I",b,pos+12)[0]]=(sig,ss);pos=end
  for local in (0x800,0x801):
   sig,ss=records[0x01000000|local];d=dict(ss)
   if sig!=b"SPEL" or struct.unpack_from("<II",d[b"SPIT"],16)!=(2,2):raise ValueError("Aimed charge carrier metadata mismatch: "+rel)
   effects=[struct.unpack("<I",v)[0] for k,v in ss if k==b"EFID"]
   if effects!=[0x01000DCE]:raise ValueError("Aimed charge carrier effect owner mismatch: "+rel)
   efits=[struct.unpack_from("<fII",v) for k,v in ss if k==b"EFIT"]
   if len(efits)!=1 or efits[0][0]!=0 or efits[0][1]!=0:raise ValueError("Aimed charge carrier must be damage-free: "+rel)
   ms,fields=records[effects[0]];md=dict(fields);raw=md[b"DATA"]
   projectile=struct.unpack_from("<I",raw,72)[0];flags=struct.unpack_from("<I",raw)[0]
   if ms!=b"MGEF" or struct.unpack_from("<II",raw,80)!=(2,2) or not projectile or flags&0x01000000:raise ValueError("SE97 aimed carrier has no eligible projectile effect: "+rel)
   if struct.unpack_from("<I",raw,64)[0]!=1 or not(flags&0x10 and flags&0x08000000):raise ValueError("Aimed charge carrier no-hit Script contract mismatch: "+rel)
   ps,pfields=records.get(projectile,(None,[]))
   if ps!=b"PROJ" or dict(pfields).get(b"MODL")!=b"\0":raise ValueError("Aimed carrier projectile must be existing empty-model metadata: "+rel)
   checked+=1
 if checked!=6:raise ValueError("Expected EN/root/KR aimed charge carriers: "+str(checked))
 return checked

def engine_dependencies(data,source_root):
 """Check raw shipped strings, without a NIF reader's implicit path repair."""
 import re, json
 validate_aimed_charge_carriers(data)
 files={k.replace("\\","/").lower():k for k in data}
 # Exact vanilla entries were verified in the installed Skyrim BSAs during CP022.
 external={
  "meshes/armor/amuletsandrings/goldringgo.nif",
  "meshes/clutter/books/spelltomedestructionlowpoly.nif",
  "textures/effects/fxfireatlas04.dds","textures/effects/glowslightflash.dds",
  "textures/effects/gradients/gradfirecloak01.dds",
  "textures/effects/gradients/gradfireexplosion.dds",
  "textures/effects/gradients/gradflame01.dds","textures/effects/vaportile01.dds"}

 nifs=textures=models=audio=0
 def present(p,kind):
  key=p.replace("\\","/").lower()
  if key not in files and key not in external:raise ValueError(kind+" dependency missing: "+p)
 for rel,b in data.items():
  if rel.lower().endswith(".nif"):
   nifs+=1
   for m in re.finditer(rb'(?i)textures[\\/][a-z0-9_ .()!+\-/\\]+\.dds',b):
    raw=m.group()
    if m.start()<4 or struct.unpack_from("<I",b,m.start()-4)[0]!=len(raw):raise ValueError("Untyped texture string: "+rel)
    p=raw.decode("ascii")
    if "/" in p:raise ValueError("Noncanonical raw texture path: "+rel+" -> "+p)
    present(p,"NIF texture");textures+=1
  elif rel.lower().endswith(".esp"):
   pos=0
   while pos<len(b):
    sig,length=struct.unpack_from("<4sI",b,pos)
    if sig==b"GRUP":pos+=24;continue
    end=pos+24+length;q=pos+24
    while q<end:
     k,n=struct.unpack_from("<4sH",b,q);q+=6;v=b[q:q+n];q+=n
     if k==b"MODL" and v.lower().endswith(b".nif\0"):
      p=v[:-1].decode()
      if "/" in p:raise ValueError("Noncanonical ESP model: "+p)
      present("meshes\\"+p,"ESP model");models+=1
     elif sig==b"SNDR" and k==b"ANAM":
      p=v.rstrip(b"\0").decode()
      present("sound\\"+p,"ESP sound");audio+=1
    if q!=end:raise ValueError("ESP subrecord length: "+rel)
    pos=end
   if pos!=len(b):raise ValueError("ESP record length: "+rel)
 native=0
 for f in (Path(source_root)/"src").rglob("*"):
  if not f.is_file() or f.suffix.lower() not in (".h",".cpp",".inl"):continue
  text=f.read_text(encoding="utf-8-sig")
  for m in re.finditer(r'"((?:[^"\\]|\\.)*\.(?:nif|dds|wav|xwm))"',text,re.I):
   p=json.loads('"'+m.group(1)+'"')
   if p.lower().startswith(("meshes\\","textures\\","sound\\")):
    present(p,"Native "+str(f));native+=1
 return dict(nifs=nifs,rawTextureReferences=textures,espModels=models,espSounds=audio,nativeLiterals=native)

if __name__=="__main__":
 parser=argparse.ArgumentParser();parser.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[1]/"assets/TOAM/Data")
 parser.add_argument("--manifest",type=Path,default=Path(__file__).resolve().parents[1]/"assets/TOAM/release_manifest.json")
 args=parser.parse_args();files=verify_data(args.root,args.manifest);engine_dependencies(files,args.root.parents[2]);print("TOAM DATA VERIFY PASS",len(files))
