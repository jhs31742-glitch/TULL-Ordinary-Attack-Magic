from pathlib import Path
import ast,json,hashlib,struct,re,io,collections,zlib,shutil
from PIL import Image
P=Path(r'[PROJECT_ROOT]');M=Path(r'[LOCAL_PATH_REDACTED]
exec((S/'000_READ_AUDIT.py').read_text(encoding='utf-8').split('inventory=[]')[0],globals())
U=lambda x:struct.pack('<I',x);sha=lambda b:hashlib.sha256(b).hexdigest().upper();norm=lambda x:x.replace('\\','/').lower()
nodes=ast.parse((S/'005_CLEAN_AUTHOR_R2.py').read_text(encoding='utf-8')).body
exec(compile(ast.Module(body=[n for n in nodes if isinstance(n,ast.FunctionDef) and n.name in ['tree','encode','build_record','rewrite_nif']],type_ignores=[]),'helpers','exec'),globals())
plan=json.loads((S/'004_CLEAN_PLAN.json').read_text(encoding='utf-8'));BASE='TULL Ordinary Offensive Magic.esp';ADD='TULL Ordinary Offensive Magic - Stances Addon.esp'
def add(rel,b,origin):
 f=Q/'Data'/rel;f.parent.mkdir(parents=True,exist_ok=True);f.write_bytes(b)
 row=next((x for x in plan['files'] if x['path']==rel),None);e=dict(path=rel,sha256=sha(b),bytes=len(b))
 if row:row.update(e)
 else:plan['files'].append(e)
 plan['assetOrigins'][rel]=origin
# The selected built-in generated raster is copied non-destructively; all discarded drafts remain outside Production.
generated=Path(r'[USER_HOME]\.codex\generated_images\01a17025-b360-72f2-89b7-09fc96154453\exec-bb1c971d-a114-4b2e-9bbb-330630b45960.png')
assert generated.is_file();source_png=S/'021_Arcana_Tome_Source.png';shutil.copy2(generated,source_png)
im=Image.open(source_png).convert('RGBA');assert im.width==im.height
im=im.resize((1024,1024),Image.Resampling.LANCZOS)
def dds_mips(image,pixel_format):
 levels=[];m=image.copy()
 while True:
  b=io.BytesIO();m.save(b,format='DDS',pixel_format=pixel_format);raw=b.getvalue();assert raw[:4]==b'DDS ' and raw[84:88]==pixel_format.encode()
  levels.append(raw[128:])
  if m.size==(1,1):header=bytearray(raw[:128]);break
  m=m.resize((max(1,m.width//2),max(1,m.height//2)),Image.Resampling.LANCZOS)
 # Use the original top-level dimensions/header and actual compressed linear byte size.
 b=io.BytesIO();image.save(b,format='DDS',pixel_format=pixel_format);header=bytearray(b.getvalue()[:128])
 flags=struct.unpack_from('<I',header,8)[0]|0x20000;struct.pack_into('<I',header,8,flags)
 struct.pack_into('<I',header,20,len(levels[0]));struct.pack_into('<I',header,28,len(levels))
 struct.pack_into('<I',header,108,0x1000|0x8|0x400000)
 out=bytes(header)+b''.join(levels)
 check=Image.open(io.BytesIO(out));check.load();assert check.size==image.size
 return out
diffuse=dds_mips(im,'DXT5')
emission=dds_mips(Image.new('RGB',(128,128),(0,0,0)),'DXT1')
add('textures/TOOM/Books/Arcana_Tome_d.dds',diffuse,['Built-in image_gen; vanilla UV atlas reference; Hebrew introductory textbook cover'])
add('textures/TOOM/Books/Arcana_Tome_n.dds',(V/'textures/clutter/books/largespelltomedestructionnoback_n.dds').read_bytes(),['Vanilla plain-back leather normal; no flame-school relief'])
add('textures/TOOM/Books/Arcana_Tome_em.dds',emission,['Constant black emission mask; utilitarian non-glowing book'])
source=json.loads((S/'018_BOOK_SOURCE_MANIFEST.json').read_text(encoding='utf-8'));bookmap={}
for path in source['texturePaths']:
 key=path.replace('\\','/').lower();name=Path(path).name
 if 'spelltome' in name:
  if name.endswith('_em.dds'):new='textures/TOOM/Books/Arcana_Tome_em.dds'
  elif name.endswith('_n.dds'):new='textures/TOOM/Books/Arcana_Tome_n.dds'
  else:new='textures/TOOM/Books/Arcana_Tome_d.dds'
 else:
  role={'book01paper.dds':'Arcana_Tome_Paper_Low_d.dds','book01paper_n.dds':'Arcana_Tome_Paper_Low_n.dds','largebookpaper01.dds':'Arcana_Tome_Paper_d.dds','largebookpaper01_n.dds':'Arcana_Tome_Paper_n.dds','ore_silver_e.dds':'Arcana_Tome_Silver_Cubemap.dds','ore_steel_e.dds':'Arcana_Tome_Steel_Cubemap.dds'}[name]
  new='textures/TOOM/Books/'+role
  add(new,(V/path).read_bytes(),['Vanilla BSA '+path])
 bookmap[key]=new
bookmodels={}
for path,new in zip(source['meshPaths'],['meshes/TOOM/Books/Arcana_Tome_World.nif','meshes/TOOM/Books/Arcana_Tome_Inventory.nif']):
 b,count=rewrite_nif((V/path).read_bytes(),bookmap)
 add(new,b,['Vanilla BSA '+path+'; original geometry/UV/shader parameters preserved; texture paths redirected'])
 plan['nifEdits'].append(dict(source='VANILLA:'+path,sourceFile=str(V/path),target=new,pathStrings=count))
 bookmodels[path]=new
# Inventory Art is a separate high-poly vanilla STAT; one shared custom STAT is necessary for all fifteen books.
esm=Path(r'[LOCAL_PATH_REDACTED]
with esm.open('rb') as f:
 while True:
  h=f.read(24);assert h
  sig,n=struct.unpack_from('<4sI',h)
  if sig==b'GRUP':continue
  fid=struct.unpack_from('<I',h,12)[0]
  if fid!=0x2fbb5:f.seek(n,1);continue
  data=f.read(n);flags=struct.unpack_from('<I',h,8)[0]
  if flags&0x40000:data=zlib.decompress(data[4:]);h=h[:4]+U(len(data))+U(flags&~0x40000)+h[12:]
  template=h+data;break
assert template[:4]==b'STAT' and dict(subs(template))[b'EDID']==b'HighPolyDestructionBook\0'
(V/'Vanilla_Inventory_STAT.bin').write_bytes(template)
newfid=0x01000DC0
newstat=build_record(template,[(k,b'TOOM_Arcana_Tome_InventoryArt\0' if k==b'EDID' else b'TOOM\\Books\\Arcana_Tome_Inventory.nif\0' if k==b'MODL' else v) for k,v in subs(template) if k!=b'MODT'],newfid)
book_count=0
for lang in ['en','kr']:
 for name in [BASE,ADD]:
  h,g=tree((Q/'Data/Language'/lang/name).read_bytes())
  assert not any(struct.unpack_from('<I',r,12)[0]==newfid for _,rr in g.values() for r in rr)
  for rindex,r in enumerate(g[b'BOOK'][1]):
   fid=struct.unpack_from('<I',r,12)[0];ss=[]
   for k,v in subs(r):
    if k==b'MODT':continue # Retire optional stale vanilla texture-cache metadata.
    if k==b'MODL':v=b'TOOM\\Books\\Arcana_Tome_World.nif\0'
    if k==b'INAM':v=U(newfid)
    ss.append((k,v))
   g[b'BOOK'][1][rindex]=build_record(r,ss);book_count+=1
  if name==BASE:g[b'STAT'][1].append(newstat)
  b=encode(h,g);add('Language/'+lang+'/'+name,b,['TOOM CP022 '+lang+' plugin'])
  if lang=='en':add(name,b,['TOOM CP022 English plugin'])
assert book_count==30 # fifteen books in each language
# Update the exact pre/post record whitelist after the shared-book asset change.
plan['espEdits']=[]
for lang in ['en','kr']:
 for name in [BASE,ADD]:
  before={struct.unpack_from('<I',r,12)[0]:r for r in records((M/'Language'/lang/name).read_bytes())}
  after={struct.unpack_from('<I',r,12)[0]:r for r in records((Q/'Data/Language'/lang/name).read_bytes())}
  for fid,r in before.items():
   old=subs(r);new=subs(after[fid]);new_by=dict(new)
   for k,v in old:
    if k==b'MODT' and r[:4]==b'BOOK' and k not in new_by:plan['espEdits'].append(dict(lang=lang,plugin=name,record=f'{fid:08X}',subrecord='MODT',before=v.hex(),after=None));continue
    vv=new_by[k]
    if vv==v:continue
    if fid==0:assert k==b'HEDR';continue
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
