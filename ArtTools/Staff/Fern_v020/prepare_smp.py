from pathlib import Path
import json,math,xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[1];data=json.loads((ROOT/'source'/'mesh_data.json').read_text())
ribbon=next(s for s in data if s['name'].startswith('07_'));positions=[]
for i in range(9):
    target=29.035-23.2*i/8
    row=sorted(ribbon['verts'],key=lambda p:abs(p[1]-target))[:14]
    positions.append([sum(p[j] for p in row)/len(row) for j in range(3)])
names=[f'FSFern_Ribbon_{i:02}' for i in range(9)]
with (ROOT/'source'/'ribbon.skin').open('w',encoding='ascii') as f:
    f.write('BONES 9\n')
    for name,p in zip(names,positions):f.write(name+' '+' '.join(map(str,p))+'\n')
    for s in data:
        if not s['name'].startswith(('07_','08_','09_')):continue
        f.write(f"SKIN {s['name']} {len(s['verts'])}\n")
        for p in s['verts']:
            if s['name'].startswith(('08_','09_')):a=b=8;wa=1;wb=0
            else:
                t=max(0,min(8,(positions[0][1]-p[1])/(positions[0][1]-positions[-1][1])*8))
                a=int(math.floor(t));b=min(a+1,8);wb=t-a;wa=1-wb
                if p[1]>=28.8:a=b=0;wa=1;wb=0
            f.write(f'{a} {wa:.8f} {b} {wb:.8f}\n')
system=ET.Element('system')
def child(par,tag,text=None,**attrs):
    el=ET.SubElement(par,tag,{k:str(v) for k,v in attrs.items()})
    if text is not None:el.text=str(text)
    return el
child(system,'bone',name='FSFern_StaffRoot')
anchor=child(system,'bone',name=names[0]);child(anchor,'mass',0);child(anchor,'inertia',x=0,y=0,z=0)
default=child(system,'bone-default');child(default,'mass',.06);child(default,'inertia',x=3,y=3,z=3)
child(default,'linearDamping',.60);child(default,'angularDamping',.68);child(default,'friction',.2);child(default,'restitution',0);child(default,'gravity-factor',1);child(default,'wind-factor',.15)
cm=child(default,'centerOfMassTransform');child(cm,'basis',x=0,y=0,z=0,w=1);child(cm,'origin',x=0,y=-1.0,z=0)
for i,name in enumerate(names[1:],1):
    b=child(system,'bone',name=name)
    if i==8:child(b,'mass',.11)
limits=child(system,'generic-constraint-default')
child(limits,'linearLowerLimit',x=0,y=0,z=0);child(limits,'linearUpperLimit',x=0,y=0,z=0)
child(limits,'angularLowerLimit',x=-.85,y=-.60,z=-.85);child(limits,'angularUpperLimit',x=.85,y=.60,z=.85)
child(limits,'linearStiffness',x=0,y=0,z=0);child(limits,'angularStiffness',x=.06,y=.035,z=.06)
child(limits,'angularDamping',x=.65,y=.65,z=.65)
group=child(system,'constraint-group')
for i in range(1,9):
    con=child(group,'generic-constraint',bodyA=names[i],bodyB=names[i-1])
    for fn,origin in [('frameInA',[0,0,0]),('frameInB',[positions[i][j]-positions[i-1][j] for j in range(3)])]:
        frame=child(con,fn);child(frame,'basis',x=0,y=0,z=0,w=1);child(frame,'origin',**dict(zip(['x','y','z'],[round(v,7) for v in origin])))
out=ROOT/'smp_package'/'SKSE'/'Plugins'/'hdtSkinnedMeshConfigs'/'FernStaff'/'Ribbon.xml';out.parent.mkdir(parents=True,exist_ok=True)
ET.indent(system);ET.ElementTree(system).write(out,encoding='utf-8',xml_declaration=True)
(ROOT/'evidence'/'ribbon_bind.json').write_text(json.dumps(dict(bones=names,positions=positions,root='FSFern_StaffRoot',anchor=names[0],dynamic_bones=names[1:],chain_constraints=8,skin_parts=['07_Hanging_Purple_Ribbon','08_Pendant_Silver_Eyelet','09_Red_Faceted_Pendant'],activation='scripted invisible ARMO/ARMA helper in biped slot 61',first_person='static'),indent=2),encoding='utf8')
print('SMP_SPEC_READY: 1 kinematic anchor, 8 dynamic ribbon bones, pendant follows terminal bone')
