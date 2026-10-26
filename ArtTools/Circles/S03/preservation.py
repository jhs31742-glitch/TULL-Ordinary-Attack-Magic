"""Hash protected workspace/MO2 files, excluding this task's sole output root."""
from pathlib import Path
import hashlib
import json
import os
import sys

ROOT = Path(__file__).resolve().parents[1]
WORKSPACE = ROOT.parents[1]
MO2 = Path(r'[LOCAL_PATH_REDACTED]

def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for data in iter(lambda:f.read(1024*1024),b''):
            h.update(data)
    return h.hexdigest().upper()

def snapshot():
    records=[]
    directories=[]
    for scope,base in [('workspace',WORKSPACE),('mo2',MO2)]:
        assert base.is_dir()
        for current,dirs,files in os.walk(base,followlinks=False):
            current=Path(current)
            dirs[:]=sorted(d for d in dirs if (current/d)!=ROOT and not (current/d).is_junction() and not (current/d).is_symlink() and d!='.git')
            if scope=='mo2':
                directories.append(str(current))
            for name in sorted(files):
                p=current/name
                s=p.stat()
                records.append({'scope':scope,'path':str(p),'bytes':s.st_size,'sha256':digest(p),'mtime_ns':s.st_mtime_ns})
    return {'files':sorted(records,key=lambda r:(r['scope'],r['path'])),'mo2_directories':sorted(directories)}

def main():
    for name in ['generator','work','reports','out/Data/meshes/effects/ChargeCannon/Derived/Common','out/Data/meshes/effects/ChargeCannon/Derived/Magician']:
        (ROOT/name).mkdir(parents=True,exist_ok=True)
    phase=sys.argv[1]
    value=snapshot()
    destination=ROOT/('work/protected-before.json' if phase=='before' else 'reports/IMMUTABILITY_AFTER.json')
    if phase=='before':
        assert not destination.exists(), 'Never overwrite the original baseline'
    destination.write_text(json.dumps(value,indent=2),encoding='utf-8')
    if phase=='after':
        before=json.loads((ROOT/'work/protected-before.json').read_text())
        ai={(r['scope'],r['path']):r for r in before['files']}
        bi={(r['scope'],r['path']):r for r in value['files']}
        changes=[{'path':k[1],'before':ai.get(k),'after':bi.get(k)} for k in sorted(ai.keys()|bi.keys()) if ai.get(k)!=bi.get(k)]
        directory_changes=before['mo2_directories']!=value['mo2_directories']
        status='FAIL' if changes or directory_changes else 'STATIC QA PASS'
        result={'status':status,'protected_file_count':len(value['files']),'byte_or_metadata_changes':changes,'mo2_directory_structure_changed':directory_changes,
            'scope':'Workspace real files (junctions and .git directories excluded), MO2 recursive files and directory inventory; excludes current task root only'}
        (ROOT/'reports/IMMUTABILITY_GATE.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
        if changes or directory_changes:
            raise SystemExit('STOP: protected input changed; do not repair production.')
    print(phase,len(value['files']),'protected files')

if __name__=='__main__':
    main()
