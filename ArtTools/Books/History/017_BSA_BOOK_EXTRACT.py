from pathlib import Path
import json,hashlib,struct,re,zlib
from PIL import Image
P=Path(r'[PROJECT_ROOT]');S=P/'evidence/STAGE_EVIDENCE/QA1_CP022_CLEAN_RELEASE';OUT=S/'Book_Source';OUT.mkdir(exist_ok=True)
exec((S/'000_READ_AUDIT.py').read_text(encoding='utf-8').split('inventory=[]')[0],globals())
root=Path(r'[LOCAL_PATH_REDACTED]
entries={}
for f in root.glob('Skyrim - *.bsa'):
 if not any(x in f.name for x in ['Textures','Meshes']):continue
 with f.open('rb') as stream:
  hdr=stream.read(36);magic,version,offset,flags,nf,nfiles,nfolder,nameslen,content=struct.unpack('<4s8I',hdr)
  assert magic==b'BSA\0' and version==105
  folders=stream.read(nf*24);blocks=stream.read(nf+nfolder+nfiles*16);names=stream.read(nameslen).split(b'\0')
  baseline=36+len(folders)+nameslen;cursor=0
  for i in range(nf):
   count=struct.unpack_from('<I',folders,i*24+8)[0];off=struct.unpack_from('<Q',folders,i*24+16)[0]-baseline
   ln=blocks[off];folder=blocks[off+1:off+1+ln].rstrip(b'\0').decode().replace('\\','/').lower()
   start=off+1+ln
   for j,filename in enumerate(names[cursor:cursor+count]):
    _,size,datapos=struct.unpack_from('<QII',blocks,start+j*16)
    entries[folder+'/'+filename.decode().lower()]=dict(bsa=str(f),flags=flags,size=size,offset=datapos)
   cursor+=count
def lz4_block(src,history=b''):
 out=bytearray(history);begin=len(out);p=0
 while p<len(src):
  token=src[p];p+=1;literal=token>>4
  if literal==15:
   while True:
    x=src[p];p+=1;literal+=x
    if x!=255:break
  assert p+literal<=len(src);out+=src[p:p+literal];p+=literal
  if p==len(src):break
  offset=int.from_bytes(src[p:p+2],'little');p+=2;assert 0<offset<=len(out)
  length=(token&15)+4
  if (token&15)==15:
   while True:
    x=src[p];p+=1;length+=x
    if x!=255:break
  assert len(out)+length<100000000
  for _ in range(length):out.append(out[-offset])
 return bytes(out[begin:])
def frame(src):
 assert src[:4]==bytes.fromhex('04224D18')
 flg,bd=src[4:6];p=6
 assert flg>>6==1 and not flg&1
 if flg&8:p+=8
 p+=1 # Descriptor checksum is retained in compressed-source provenance; extracted size/asset parsers are checked.
 result=bytearray()
 while True:
  n=struct.unpack_from('<I',src,p)[0];p+=4
  if n==0:break
  raw=bool(n&0x80000000);n&=0x7fffffff;chunk=src[p:p+n];assert len(chunk)==n;p+=n
  result+=chunk if raw else lz4_block(chunk,b'' if flg&32 else bytes(result[-65536:]))
  if flg&16:p+=4
 if flg&4:p+=4
 assert p==len(src)
 return bytes(result)
manifest=[]
def extract(path):
 e=entries[path]
 with Path(e['bsa']).open('rb') as f:f.seek(e['offset']);raw=f.read(e['size']&0x3fffffff)
 sha=lambda b:hashlib.sha256(b).hexdigest().upper()
 compressed=bool(e['flags']&4)^bool(e['size']&0x40000000)
 src=raw
 if e['flags']&0x100:src=src[1+src[0]:]
 if compressed:
  expected=struct.unpack_from('<I',src)[0];src=src[4:]
  data=frame(src) if src.startswith(bytes.fromhex('04224d18')) else lz4_block(src)
  assert len(data)==expected,(path,len(data),expected)
 else:data=src
 out=OUT/path;out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(data)
 manifest.append(dict(path=path,bsa=Path(e['bsa']).name,offset=e['offset'],archiveFlags=e['flags'],compressed=compressed,packedSHA256=sha(raw),sha256=sha(data),bytes=len(data)))
 return out
meshpaths=['meshes/clutter/books/spelltomedestructionlowpoly.nif','meshes/clutter/books/book02/character assets/spelltomedestruction.nif']
textures=set()
for path in meshpaths:
 out=extract(path)
 for r in nifpaths(out.read_bytes()):
  if r['path'].lower().endswith('.dds'):textures.add(r['path'].replace('\\','/').lower())
for path in sorted(textures):
 out=extract(path);im=Image.open(out);im.load()
 print('TEXTURE',path,im.size,im.mode,out.stat().st_size)
 im.convert('RGBA').save(out.with_suffix('.png'))
(S/'018_BOOK_SOURCE_MANIFEST.json').write_text(json.dumps(dict(files=manifest,meshPaths=meshpaths,texturePaths=sorted(textures)),indent=2),encoding='utf-8')
print('BOOK EXTRACTION PASS',len(manifest),'files')
