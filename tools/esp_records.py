"""Reconstruct owned TOAM plugin records to an explicit new output directory."""
from pathlib import Path
import json,struct,hashlib,argparse
def reconstruct(d):
 assert d['format']=='TOAM_ORDERED_ESP_V1';out=bytes.fromhex(d['header'])
 for g in d['groups']:
  records=[]
  for r in g['records']:
   h=bytes.fromhex(r['header']);body=b''.join(s['signature'].encode()+struct.pack('<H',len(bytes.fromhex(s['hex'])))+bytes.fromhex(s['hex']) for s in r['subrecords'])
   records.append(h[:4]+struct.pack('<I',len(body))+h[8:]+body)
  h=bytes.fromhex(g['header']);body=b''.join(records);out+=h[:4]+struct.pack('<I',24+len(body))+h[8:]+body
 assert len(out)==d['bytes'] and hashlib.sha256(out).hexdigest().upper()==d['sha256'];return out
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=Path(__file__).resolve().parents[1]/'DataSources/PluginRecords');p.add_argument('--output',type=Path,required=True);a=p.parse_args();assert not a.output.exists(),'Choose a new output directory'
 candidates=[]
 for f in a.source.rglob('*.json'):
  d=json.loads(f.read_text(encoding='utf-8'));rel=Path(d['path']);assert not rel.is_absolute() and '..' not in rel.parts;candidates.append((a.output/rel,reconstruct(d)))
 for f,b in candidates:f.parent.mkdir(parents=True,exist_ok=True);f.write_bytes(b)
 print('ESP byte-identical reconstruction PASS',len(candidates))
