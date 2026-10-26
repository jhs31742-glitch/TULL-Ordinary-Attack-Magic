from pathlib import Path
import json,struct,hashlib
P=Path(r'[PROJECT_ROOT]');M=Path(r'[LOCAL_PATH_REDACTED]
exec((S/'000_READ_AUDIT.py').read_text(encoding='utf-8').split('inventory=[]')[0],globals())
sha=lambda b:hashlib.sha256(b).hexdigest().upper()
plan=json.loads((S/'004_CLEAN_PLAN.json').read_text(encoding='utf-8'));BASE='TULL Ordinary Offensive Magic.esp';ADD='TULL Ordinary Offensive Magic - Stances Addon.esp'
source_png=S/'021_Arcana_Tome_Source.png';newfid=0x01000DC0
source=json.loads((S/'018_BOOK_SOURCE_MANIFEST.json').read_text(encoding='utf-8'))
for folder in ['meshes/TOOM/Books','textures/TOOM/Books']:
 for f in (Q/'Data'/folder).iterdir():
  rel=f.relative_to(Q/'Data').as_posix()
  assert not any(x['path']==rel for x in plan['files'])
  plan['files'].append(dict(path=rel,sha256=sha(f.read_bytes()),bytes=f.stat().st_size))
  plan['assetOrigins'][rel]=['Vanilla Skyrim BSA book structure; TOOM plain Hebrew textbook reskin; see book source manifest']
for old,new in zip(source['meshPaths'],['meshes/TOOM/Books/Arcana_Tome_World.nif','meshes/TOOM/Books/Arcana_Tome_Inventory.nif']):
 plan['nifEdits'].append(dict(source='VANILLA:'+old,sourceFile=str(V/old),target=new,pathStrings=len(nifpaths((Q/'Data'/new).read_bytes()))))
for row in plan['files']:
 if row['path'].endswith('.esp'):
  f=Q/'Data'/row['path'];row.update(sha256=sha(f.read_bytes()),bytes=f.stat().st_size)
plan['espEdits']=[]
for lang in ['en','kr']:
 for name in [BASE,ADD]:
  before={struct.unpack_from('<I',r,12)[0]:r for r in records((M/'Language'/lang/name).read_bytes())}
  after={struct.unpack_from('<I',r,12)[0]:r for r in records((Q/'Data/Language'/lang/name).read_bytes())}
  assert set(after)-set(before)==({newfid} if name==BASE else set())
  for fid,r in before.items():
   old=subs(r);new=subs(after[fid]);removed=[]
   if r[:4]==b'BOOK':
    removed=[(k,v) for k,v in old if k==b'MODT'];old=[(k,v) for k,v in old if k!=b'MODT']
   assert len(old)==len(new)
   for k,v in removed:plan['espEdits'].append(dict(lang=lang,plugin=name,record=f'{fid:08X}',subrecord='MODT',before=v.hex(),after=None))
   for (k,v),(kk,vv) in zip(old,new):
    assert k==kk
    if vv==v:continue
    if fid==0:assert k==b'HEDR';continue
    assert k in [b'EDID',b'MODL',b'INAM',b'ANAM'],('unexpected change',name,hex(fid),k)
    plan['espEdits'].append(dict(lang=lang,plugin=name,record=f'{fid:08X}',subrecord=k.decode(),before=v.hex(),after=vv.hex()))
# Stage developer authoring source and exact final prompt/text; they are not runtime Data clutter.
booktext='''Title: קסם התקפי רגיל
Meaning: Ordinary Offensive Magic / 일반적인 공격 마법
Original band source: assets/ChargeCannon/r3/Source/scripts/make_textures.py line62.
הקסם נאגר בטבעות
הטבעות מתיישרות לקנה אחד
הקרן נורית ועוברת בכל אויב
בפגיעה באדמה האור מתפוצץ
Author credit: מחברת: פרירן
Meaning: Author: Frieren / 저자: 프리렌 (in-universe flavor credit).
Art direction: plain required introductory coursebook; no ornate magic circle or gold filigree.
Generation: built-in image_gen, vanilla UV atlas edit reference, final selected source copied into the project.
Diffuse encoding: BC3/DXT5 1024x1024, complete11-level mip chain.
Mesh: vanilla world low-poly plus inventory high-poly; geometry/UV/collision/controller/shader parameters retained.
Normal: vanilla plain-back normal to avoid retaining the flame-school embossing. Emission: black mask.
'''
for target,b in [(P/'assets/TOOM/Authoring/Books/Arcana_Tome_Source.png',source_png.read_bytes()),(P/'assets/TOOM/Authoring/Books/ART_DIRECTION_AND_TEXT.txt',booktext.encode())]:
 q=Q/'Payload'/target.name;q.write_bytes(b)
 plan['write'].append(dict(target=str(target),candidate=str(q),before=sha(target.read_bytes()) if target.is_file() else None,after=sha(b),bytes=len(b)))
plan['book']=dict(title='קסם התקפי רגיל',author='מחברת: פרירן',instructions=['הקסם נאגר בטבעות','הטבעות מתיישרות לקנה אחד','הקרן נורית ועוברת בכל אויב','בפגיעה באדמה האור מתפוצץ'],worldModel='meshes/TOOM/Books/Arcana_Tome_World.nif',inventoryModel='meshes/TOOM/Books/Arcana_Tome_Inventory.nif',sharedInventorySTAT=f'{newfid:08X}',books=15,sourceManifest=str(S/'018_BOOK_SOURCE_MANIFEST.json'),generatedImageSHA256=sha(source_png.read_bytes()),textureSize=1024,mipLevels=11,shaderGeometryUV='unchanged; texture paths only',cachedMODTRetired=True)
plan['validation'].update(espCounts={BASE:1388,ADD:15},newStaffStatics=0,newSharedBookInventorySTAT=1,niflyParsed=41)
plan['assetManifest']['files']=sorted(plan['files'],key=lambda x:x['path'])
(S/'004_CLEAN_PLAN.json').write_text(json.dumps(plan,ensure_ascii=False,indent=2),encoding='utf-8')
print('BOOK CANDIDATE PASS',json.dumps(dict(books=15,sharedBookSTAT='01000DC0',staffSTATsAdded=0,NIFs=41,DDS=sum(x['path'].endswith('.dds') for x in plan['files']),files=len(plan['files'])),ensure_ascii=False))
