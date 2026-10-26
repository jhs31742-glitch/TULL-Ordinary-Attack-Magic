"""Reproducible NIF-only assembly from COPIED r3 component specs. No runtime code."""
from pathlib import Path
from copy import deepcopy
import argparse
import hashlib
import json
import math
import os
import subprocess

ROOT=Path(__file__).resolve().parents[1]
GEN=ROOT/'generator'
SPEC=ROOT/'work/nifspec'
OUT=ROOT/'out/Data/meshes/effects/ChargeCannon/Derived'
DEFAULT_FLAGS=524302

def read_spec(path):
    lines=path.read_text(encoding='utf-8').splitlines()
    records=[]
    i=0
    while i<len(lines):
        tokens=lines[i].split()
        if not tokens:
            i+=1
            continue
        size=1+(int(tokens[-2])+int(tokens[-1]) if tokens[0]=='MESH' else 0)
        records.append({'tokens':tokens,'data':lines[i+1:i+size]})
        i+=size
    return records

def serialize(records):
    return '\n'.join(line for r in records for line in [' '.join(r['tokens']),*r['data']])+'\n'

def node(name,parent='-',xyz=(0,0,0),scale=1):
    return {'tokens':['NODE',name,parent,*map(str,xyz),str(scale)],'data':[]}

def component(name,prefix,tintable,shift_y=0):
    suffix='_Tintable' if tintable else ''
    records=read_spec(GEN/'components'/f'{name}{suffix}.nifspec')
    result=[]
    for record in records:
        t=record['tokens']
        if t[0] not in ('NODE','MESH'):
            continue  # Deliberately remove ROOT and ALL controller commands.
        r=deepcopy(record)
        q=r['tokens']
        q[1]=prefix+'_'+q[1]
        q[2]=prefix if q[2]=='-' else prefix+'_'+q[2]
        if q[0]=='NODE' and t[2]=='-':
            q[4]=format(float(q[4])+shift_y,'.9g')
        result.append(r)
    return result

def rig(tintable):
    records=[{'tokens':['ROOT','OOM_MagicianCharge_Root'],'data':[]}]
    for i in range(1,9):
        control=f'MAG_Circle{i:02}'
        records.append(node(control))
        records.append({'tokens':['NODEFLAGS',control,str(DEFAULT_FLAGS | (1 if i>1 else 0))],'data':[]})
        records+=component('Charge_Circle_Core_Static' if i==1 else 'Charge_Circle_Static',control,tintable)
    return records

def emitter(tintable):
    records=[{'tokens':['ROOT','OOM_EmitterCircle_Root'],'data':[]},node('EMITTER_Visual'),node('FIRE_ORIGIN')]
    # Preserve relative layer spacing, put the original Face centre exactly at socket Y=0.
    records+=component('Charge_Circle_Small','EMITTER_Visual',tintable,shift_y=-0.1)
    return records

def cap_geometry(radius,half_length):
    # Three soft textured planes through endpoint origin: XZ cap + XY/YZ feathers.
    # Based on the r3 quad_xz winding/UV language. No rings, shards, flash or collision.
    vertices=[]
    triangles=[]
    for plane in range(3):
        base=len(vertices)
        for a,b,u,v in [(-1,1,0,0),(1,1,1,0),(1,-1,1,1),(-1,-1,0,1)]:
            if plane==0: p=(a*radius,0,b*radius,u,v,0,-1,0)
            elif plane==1: p=(a*radius,b*half_length,0,u,v,0,0,-1)
            else: p=(0,b*half_length,a*radius,u,v,-1,0,0)
            vertices.append(p)
        triangles += [(base,base+1,base+2),(base,base+2,base+3)]
    return vertices,triangles

def endfade(tintable):
    records=[{'tokens':['ROOT','OOM_BeamEndFade_Root'],'data':[]}]
    source=read_spec(GEN/'components'/('Beam_Body_Unit_Static_Tintable.nifspec' if tintable else 'Beam_Body_Unit_Static.nifspec'))
    source_meshes=[r for r in source if r['tokens'][0]=='MESH']
    for role,radius,emissive,source_mesh in zip(['Core','Main','Out','Aft'],[10.4,26,40,48],[2.0,1.3,1.0,.65],source_meshes):
        control='ENDF_'+role
        records.append(node(control))
        t=source_mesh['tokens']
        # Keep corresponding beam-layer RGB and alpha; only authored emissive is reduced.
        texture='textures\\effects\\ChargeCannon\\'+('Tintable\\' if tintable else '')+'Glow_Soft.dds'
        verts,tris=cap_geometry(radius,32)
        tokens=['MESH',control+'_Mesh',control,texture,*t[4:8],str(emissive),'1','0','1','1','1','1','0',str(len(verts)),str(len(tris))]
        data=[' '.join(format(value,'.8g') for value in vertex) for vertex in verts]
        data+=[' '.join(map(str,tri)) for tri in tris]
        records.append({'tokens':tokens,'data':data})
    return records

def tier_layouts():
    config=json.loads((GEN/'magician_tiers.candidate.json').read_text())
    layouts=[]
    for tier in config['tiers']:
        circles=[]
        for i in range(1,9):
            circles.append({'node':f'MAG_Circle{i:02}','visible':i<=tier['visible_count'],
                'translation':[0,0 if i==1 else round((i-1)*tier['spacing_y'],6),0],
                'scale':1 if i==1 else round((1-(i-1)*config['base_scale_step_per_circle'])*tier['radial_compaction'],6),
                'rotation_y_degrees':0 if i==1 else round((i-1)*config['relative_rotation_step_degrees'],6)})
        layouts.append({**tier,'circles':circles})
    assert [t['mp_authority'] for t in layouts]==[100,300,600,1200,2000,3000,4000,5000]
    assert all(t['circles'][0]==layouts[0]['circles'][0] for t in layouts)
    assert layouts[1]['circles'][1]['translation']==[0,96,0]
    for previous,current in zip(layouts[1:],layouts[2:]):
        assert current['spacing_y']<previous['spacing_y']
        assert current['spacing_y']*(current['visible_count']-1)<previous['spacing_y']*(previous['visible_count']-1)
        for i in range(1,previous['visible_count']):
            assert current['circles'][i]['translation'][1]<previous['circles'][i]['translation'][1]
            assert current['circles'][i]['scale']<previous['circles'][i]['scale']
    return config,layouts

def main():
    raise SystemExit('Historical 8-circle workflow retired: REJECTED_CANDIDATE / DO NOT PROMOTE. Existing NIF bytes are preserved. Use render_instance_composition.py for current data-only preview; historical specs remain available for separately authorized reproduction into work only.')
    parser=argparse.ArgumentParser()
    parser.add_argument('--repro',action='store_true')
    args=parser.parse_args()
    destination=ROOT/'work/repro' if args.repro else OUT
    assert destination.resolve().is_relative_to(ROOT)
    assert (ROOT/'reports/ASSET_GAP_AUDIT.md').is_file(), 'Gap audit must precede generation'
    SPEC.mkdir(parents=True,exist_ok=True)
    assets=[]
    for family,stem,factory,sources,purpose in [
        ('Magician','Magician_ChargeRig_8Circle_Static',rig,['Charge_Circle_Core_Static','Charge_Circle_Static'],'8 independent sibling charge-circle branches; no controllers'),
        ('Common','Beam_EndFade',endfade,['Beam_Body_Unit_Static','Glow_Soft.dds'],'Small static spatial soft cap centered on beam endpoint; no explosion'),
        ('Common','Emitter_Circle_Small_Static',emitter,['Charge_Circle_Small'],'Generic emitter visual and exact-centre FIRE_ORIGIN socket')]:
        for tintable in [False,True]:
            name=stem+('_Tintable' if tintable else '')+'.nif'
            records=factory(tintable)
            assert not any(r['tokens'][0] in ('SPIN','NODESCALE','SHFLOAT') for r in records)
            spec=SPEC/(Path(name).stem+'.nifspec')
            spec.write_text(serialize(records),encoding='utf-8')
            final=destination/family/name
            final.parent.mkdir(parents=True,exist_ok=True)
            result=subprocess.run([str(ROOT/'work/build/nifbuild.exe'),str(spec),str(final)],capture_output=True,text=True)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            assets.append({'filename':name,'relative_output_path':final.relative_to(ROOT).as_posix(),'family':family,'purpose':purpose,
                'source_components':sources,'tintable':tintable,'status':['GENERATED','RUNTIME UNVERIFIED'],
                'nifspec':spec.relative_to(ROOT).as_posix(),'spec_sha256':hashlib.sha256(spec.read_bytes()).hexdigest().upper()})
            print('GENERATED',final.relative_to(ROOT))
    if not args.repro:
        (ROOT/'work/generated-assets.json').write_text(json.dumps(assets,indent=2),encoding='utf-8')
        config,layouts=tier_layouts()
        (ROOT/'reports/MAGICIAN_TIER_PREVIEW_VALUES.json').write_text(json.dumps({'status':config['status'],'tiers':layouts},indent=2),encoding='utf-8')
        rows=['# Magician tier 구성 후보','', '**CANDIDATE VALUES ONLY**. MP와 circle count만 사용자 authority. Transform 수치는 개발 프리뷰 후보이다.','',
            '| Tier | MP | Visible nodes | Y spacing | Stack depth | Radial factor | Circle01 |','|---|---:|---|---:|---:|---:|---|']
        for t in layouts:
            rows.append(f"| T{t['tier']} | {t['mp_authority']} | 01..{t['visible_count']:02} | {t['spacing_y']} | {round(t['spacing_y']*(t['visible_count']-1),3)} | {t['radial_compaction']} | fixed: (0,0,0), scale=1, rot=0 |")
        rows+=['','Circle i>1: local Y=(i-1)*spacing; scale=(1-0.085*(i-1))*radial factor; local +Y rotation=(i-1)*7.5 degrees. X=Z=0. Full per-circle data: MAGICIAN_TIER_PREVIEW_VALUES.json.',
            '', 'T2 Circle02 keeps baseline Y=96 and radial factor=1; no compression. T3+ moves each existing Circle02..N inward and reduces total stack depth despite increasing circle count. Circle01 remains identical. These values are not stored in final rig transforms.',
            '', 'Preview: fixed camera, fixed exposure, actual generated NIF geometry/UV readback; software additive rendering. Engine brightness/culling/alpha sorting are RUNTIME UNVERIFIED. Runtime transitions must begin from CURRENT transforms; no transition/controller implementation is included.']
        (ROOT/'reports/MAGICIAN_TIER_COMPOSITION_MATRIX.md').write_text('\n'.join(rows)+'\n',encoding='utf-8')

if __name__=='__main__':
    main()
