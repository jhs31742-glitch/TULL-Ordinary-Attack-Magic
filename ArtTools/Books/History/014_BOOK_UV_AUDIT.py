from pathlib import Path
import collections
txt=(Path(__file__).parent/'007_MESH_AUDIT.txt').read_text();cur='';d=collections.defaultdict(list);alluv=collections.defaultdict(list)
for l in txt.splitlines():
 if l.startswith('SHAPE '):cur=l.split(' id=')[0]
 if l.startswith('VERT '):
  f=l.split();p=list(map(float,f[2].split(',')));uv=list(map(float,f[4].split(',')));no=list(map(float,f[6].split(',')))
  alluv[cur].append(uv)
  if (cur.endswith(':0') or cur.endswith(':4') or cur.endswith(':6')) and abs(no[2])>.9:d[(cur,round(p[2]),int(uv[0]),int(uv[1]))].append(uv)
print([(k,len(v)) for k,v in d.items()])
print([(k,[min(x[0] for x in v),max(x[0] for x in v),min(x[1] for x in v),max(x[1] for x in v)]) for k,v in alluv.items()])
