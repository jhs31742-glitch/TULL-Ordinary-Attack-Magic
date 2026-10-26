"""Repair only inconsistent winding on an owned, closed, static SSE100 BSTriShape.
No NIF reserialization, optimizer, shader change, skinning or physics dependency.
Usage: python repair_sse_ribbon.py INPUT OUTPUT --shape 10_Hanging_Red_Ribbon
A full deployment caller must supply its own baseline/backup/inventory transaction.
"""
from pathlib import Path
import argparse, collections, hashlib, json, math, struct

def sha(b): return hashlib.sha256(b).hexdigest().upper()
def add(a,b): return tuple(x+y for x,y in zip(a,b))
def sub(a,b): return tuple(x-y for x,y in zip(a,b))
def mul(a,b): return tuple(x*b for x in a)
def dot(a,b): return sum(x*y for x,y in zip(a,b))
def cross(a,b): return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def norm(a):
 d=math.sqrt(dot(a,a))
 if d<1e-12: raise ValueError('Zero direction')
 return mul(a,1/d)
def nifparse(b):
 p=b.index(b'\n')+1
 assert b[:p-1]==b'Gamebryo File Format, Version 20.2.0.7'
 def val(fmt):
  nonlocal p
  v=struct.unpack_from('<'+fmt,b,p)[0];p+=struct.calcsize('<'+fmt);return v
 def string(fmt):
  nonlocal p
  n=val(fmt);s=b[p:p+n];p+=n;return s
 assert val('I')==0x14020007 and val('B')==1 and val('I')==12
 count=val('I');assert val('I')==100
 for _ in range(3):string('B')
 types=[string('I').decode() for _ in range(val('H'))];ids=[val('H') for _ in range(count)];sizes=[val('I') for _ in range(count)]
 n=val('I');val('I');strings=[string('I') for _ in range(n)];groups=val('I');p+=4*groups;blocks=[]
 for tid,size in zip(ids,sizes):blocks.append((types[tid],p,b[p:p+size]));p+=size
 assert b[p:]==struct.pack('<II',1,0)
 return strings,blocks

def decode_shape(raw):
 p=4;n=struct.unpack_from('<I',raw,p)[0];p+=4+4*n+4
 # SSE100 NiObjectNET -> BSTriShape fields;all are explicit on-disk sizes.
 p+=4+12+36+4+4+16
 skin,shader,alpha=struct.unpack_from('<III',raw,p);p+=12
 desc=struct.unpack_from('<Q',raw,p)[0];p+=8;nt,nv,size=struct.unpack_from('<HHI',raw,p);p+=8
 flags=desc>>44;stride=(desc&15)*4
 assert skin==0xFFFFFFFF and not flags&64,'Skinned geometry excluded'
 assert flags&1 and flags&2 and flags&8 and flags&16,'Position/UV/normal/tangent required'
 assert stride>=28 and size==nv*stride+nt*6
 uv=(desc>>6)&60;normal=(desc>>14)&60;tangent=(desc>>18)&60
 assert uv==16 and normal==20 and tangent==24,'Unsupported vertex layout'
 verts=[struct.unpack_from('<fff',raw,p+i*stride) for i in range(nv)]
 uvs=[struct.unpack_from('<ee',raw,p+i*stride+uv) for i in range(nv)]
 triOffset=p+nv*stride;triangles=[struct.unpack_from('<HHH',raw,triOffset+i*6) for i in range(nt)]
 assert all(max(t)<nv and len(set(t))==3 for t in triangles)
 assert all(math.isfinite(x) for v in verts+uvs for x in v)
 return dict(start=p,stride=stride,normal=normal,tangent=tangent,triOffset=triOffset,verts=verts,uvs=uvs,triangles=triangles)

def orient(vertices,triangles):
 edges=collections.defaultdict(list)
 for i,t in enumerate(triangles):
  for a,b in zip(t,t[1:]+t[:1]):edges[tuple(sorted((a,b)))].append((i,a,b))
 assert all(len(v)==2 for v in edges.values()),'A closed two-manifold ribbon is required'
 conflicts=sum(v[0][1:]==v[1][1:] for v in edges.values());adj=collections.defaultdict(list)
 for a,b in edges.values():
  parity=int(a[1:]==b[1:]);adj[a[0]].append((b[0],parity));adj[b[0]].append((a[0],parity))
 flip={};components=[]
 for root in range(len(triangles)):
  if root in flip:continue
  flip[root]=0;q=[root];component=[]
  while q:
   i=q.pop();component.append(i)
   for other,parity in adj[i]:
    want=flip[i]^parity
    if other in flip:assert flip[other]==want,'Non-orientable surface'
    else:flip[other]=want;q.append(other)
  components.append(component)
 out=[(t[0],t[2],t[1]) if flip[i] else t for i,t in enumerate(triangles)]
 for comp in components:
  volume=sum(dot(vertices[out[i][0]],cross(vertices[out[i][1]],vertices[out[i][2]])) for i in comp)/6
  assert abs(volume)>1e-7,'Zero-volume shell'
  if volume<0:
   for i in comp:t=out[i];out[i]=(t[0],t[2],t[1])
 return out,conflicts,len(components)

def tangent_frame(vertices,uvs,triangles):
 count=len(vertices);zero=(0.,0.,0.);ns=[zero]*count;ts=[zero]*count;bs=[zero]*count
 for a,b,c in triangles:
  e1=sub(vertices[b],vertices[a]);e2=sub(vertices[c],vertices[a]);n=cross(e1,e2);assert dot(n,n)>1e-14,'Degenerate face'
  for i in (a,b,c):ns[i]=add(ns[i],n)
  s1,t1=sub(uvs[b],uvs[a]);s2,t2=sub(uvs[c],uvs[a]);r=1. if s1*t2-s2*t1>=0 else -1.
  sd=mul(sub(mul(e1,t2),mul(e2,t1)),r);td=mul(sub(mul(e2,s1),mul(e1,s2)),r)
  if dot(sd,sd)>1e-16:sd=norm(sd)
  if dot(td,td)>1e-16:td=norm(td)
  for i in (a,b,c):ts[i]=add(ts[i],td);bs[i]=add(bs[i],sd)
 result=[]
 for n,t,b in zip(ns,ts,bs):
  n=norm(n);t=sub(t,mul(n,dot(n,t)))
  if dot(t,t)<1e-14:
   axis=min([(1.,0.,0.),(0.,1.,0.),(0.,0.,1.)],key=lambda a:abs(dot(a,n)));t=cross(n,axis)
  t=norm(t);b=sub(sub(b,mul(n,dot(n,b))),mul(t,dot(t,b)))
  if dot(b,b)<1e-14:b=cross(n,t)
  result.append((n,t,norm(b)))
 return result

def repair(data,shape_name):
 strings,blocks=nifparse(data);matches=[(i,o,b) for i,(kind,o,b) in enumerate(blocks) if kind=='BSTriShape' and strings[struct.unpack_from('<I',b)[0]].decode()==shape_name];assert len(matches)==1
 block,offset,raw=matches[0];d=decode_shape(raw);tri,conflicts,components=orient(d['verts'],d['triangles']);flips=sum(a!=b for a,b in zip(tri,d['triangles']))
 if conflicts==0:return data,dict(status='NO_FIX_NEEDED',block=block,conflicts=0)
 frame=tangent_frame(d['verts'],d['uvs'],tri);out=bytearray(data);allowed=set()
 pack=lambda v:bytes(max(0,min(255,int(math.floor((x+1)*127.5+.5)))) for x in v)
 for i,(n,t,b) in enumerate(frame):
  at=offset+d['start']+i*d['stride'];out[at+12:at+16]=struct.pack('<f',b[0]);out[at+d['normal']:at+d['normal']+4]=pack(n)+pack((b[1],));out[at+d['tangent']:at+d['tangent']+4]=pack(t)+pack((b[2],));allowed.update(range(at+12,at+16));allowed.update(range(at+d['normal'],at+d['normal']+8))
 for i,t in enumerate(tri):at=offset+d['triOffset']+i*6;out[at:at+6]=struct.pack('<HHH',*t);allowed.update(range(at,at+6))
 delta={i for i,(a,b) in enumerate(zip(data,out)) if a!=b};assert delta and delta<=allowed and len(data)==len(out)
 ns,nblocks=nifparse(out);assert ns==strings and len(nblocks)==len(blocks)
 for i,(a,b) in enumerate(zip(blocks,nblocks)):
  if i!=block:assert a==b,'Non-ribbon block changed'
 nd=decode_shape(nblocks[block][2]);assert d['verts']==nd['verts'] and d['uvs']==nd['uvs'];assert orient(nd['verts'],nd['triangles'])[1]==0
 return bytes(out),dict(status='REPAIRED',block=block,shape=shape_name,vertices=len(d['verts']),triangles=len(tri),components=components,originalConflictingEdges=conflicts,flippedTriangles=flips,changedBytes=len(delta),preserved='every byte except ribbon triangle indices and normal/tangent/bitangent fields',beforeSHA256=sha(data),afterSHA256=sha(out))

def main():
 p=argparse.ArgumentParser();p.add_argument('input',type=Path);p.add_argument('output',type=Path);p.add_argument('--shape',default='10_Hanging_Red_Ribbon');args=p.parse_args();assert args.input.resolve()!=args.output.resolve(),'Use a separate candidate output';before=args.input.read_bytes();after,proof=repair(before,args.shape);args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_bytes(after);assert args.output.read_bytes()==after;print(json.dumps(proof,indent=2))
if __name__=='__main__':main()
