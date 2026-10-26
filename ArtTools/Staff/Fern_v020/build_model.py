"""Blender 4.5: original Fern staff geometry; no reference-image pixels used as textures."""
import bpy, math, json, sys
from mathutils import Vector
from pathlib import Path
from collections import Counter
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'source';EV=ROOT/'evidence';PI=math.pi
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
for col in list(bpy.data.collections):
    if col.name!='Collection' and col.users==0:bpy.data.collections.remove(col)
main=bpy.data.collections.new('FERN_STAFF_GAME_MESHES');bpy.context.scene.collection.children.link(main)
stage=bpy.data.collections.new('PRESENTATION_ONLY');bpy.context.scene.collection.children.link(stage)
materials={}
for key,metal,rough,col in [('wood',0,.72,(.16,.10,.055,1)),('endgrain',0,.78,(.26,.16,.085,1)),('cloth',0,.66,(.12,.025,.27,1)),('silver',.78,.40,(.22,.25,.20,1)),('gem',.45,.12,(.45,.006,.016,1))]:
    m=bpy.data.materials.new(key);m.use_nodes=True;m.diffuse_color=col
    n=m.node_tree.nodes;p=n.get('Principled BSDF');p.inputs['Metallic'].default_value=metal;p.inputs['Roughness'].default_value=rough
    tex=n.new('ShaderNodeTexImage');tex.image=bpy.data.images.load(str(OUT/'textures'/f'{key}_d.png'))
    m.node_tree.links.new(tex.outputs['Color'],p.inputs['Base Color'])
    nm=n.new('ShaderNodeTexImage');nm.image=bpy.data.images.load(str(OUT/'textures'/f'{key}_n.png'));nm.image.colorspace_settings.name='Non-Color'
    normal=n.new('ShaderNodeNormalMap');normal.inputs['Strength'].default_value=.55
    m.node_tree.links.new(nm.outputs['Color'],normal.inputs['Color']);m.node_tree.links.new(normal.outputs['Normal'],p.inputs['Normal'])
    materials[key]=m

parts=[]
def mesh(name,verts,faces,uvs,mat,smooth=True):
    me=bpy.data.meshes.new(name);me.from_pydata(verts,[],faces);me.update()
    ob=bpy.data.objects.new(name,me);main.objects.link(ob);me.materials.append(materials[mat])
    layer=me.uv_layers.new(name='UVMap')
    for po in me.polygons:
        po.use_smooth=smooth
        for li in po.loop_indices:layer.data[li].uv=uvs[me.loops[li].vertex_index]
    # Recalculate winding consistently, including closed caps and solid ribbon sides.
    import bmesh
    bm=bmesh.new();bm.from_mesh(me)
    bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.000001)
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(me);bm.free();me.update()
    me.set_sharp_from_angle(angle=math.radians(55))
    ob['material_key']=mat;ob['export_budget_vertices']=14999;ob['export_budget_triangles']=14999
    parts.append(ob);return ob

PROFILE=[(-87.90222168,.12,.12,.12),(-85,.18,.58,.55),(-78,.16,.92,.86),(-65,.08,1.02,.93),(-45,-.10,1.16,1.04),(-25,-.12,1.3,1.13),(-14,0,1.4,1.21),(0,0,1.44,1.25),(10,.12,1.54,1.29),(14,-.5,1.60,1.28),(18,-3.15,1.77,1.34),(20,-3.65,1.76,1.34),(24,-2.32,1.65,1.30),(28,-.60,1.57,1.26),(32,.10,1.58,1.38),(35,.42,1.80,1.57),(38,1.05,2.50,2.18),(42,1.75,3.35,2.90),(46.70283890,2.10,3.85,3.25)]
def thickness_factor(z):
    if z<=-14:return 1.35
    if z<0:return 1.2+.15*(-z/14)
    if z<=35:return 1.2
    return 1.2-.2*(z-35)/(46.7028389-35)
PROFILE=[(z,x,rx*thickness_factor(z),ry*thickness_factor(z)) for z,x,rx,ry in PROFILE]
def sample(z):
    for a,b in zip(PROFILE,PROFILE[1:]):
        if a[0]<=z<=b[0]:
            f=(z-a[0])/(b[0]-a[0]);return tuple(a[j]*(1-f)+b[j]*f for j in range(1,4))
    return tuple(PROFILE[0 if z<PROFILE[0][0] else -1][1:])
def frame(z):
    x,rx,ry=sample(z);dx=(sample(z+.12)[0]-sample(z-.12)[0])/.24
    return Vector((x,0,z)),Vector((1,0,-dx)).normalized(),Vector((0,1,0)),Vector((dx,0,1)).normalized()
def surface(z,theta,offset=0):
    c,a,b,t=frame(z);_,rx,ry=sample(z)
    return c+a*((rx+offset)*math.cos(theta))+b*((ry+offset)*math.sin(theta))

def loft(name,stations,mat,N=40,wrap=False,head=False):
    v=[];uv=[];faces=[]
    for i,(z,rx,ry,offset) in enumerate(stations):
        c,a,b,t=frame(z)
        if (head or wrap) and z>35:a=Vector((1,0,0));b=Vector((0,1,0))
        for j in range(N+1):
            th=2*PI*j/N
            bump=0
            if wrap:
                phase=(2*PI*(z-stations[0][0])/2.75-th)%(2*PI)
                bump=.025*math.cos(phase)-.030*math.exp(-((phase-PI)/.38)**2)
            elif head:
                strength=.045 if z<35 else .13
                bump=strength*(math.sin(5*th+.16*z)+.45*math.sin(9*th-.10*z))
            p=c+a*((rx+offset+bump)*math.cos(th))+b*((ry+offset+bump)*math.sin(th))
            if head and z>44:
                f=(z-44)/(46.70283890-44);p.z-=f*1.8*(math.cos(th)+1)/2
            v.append(tuple(p));uv.append((j/N,(z-stations[0][0])/(26.0 if head else (stations[-1][0]-stations[0][0]))))
        if i:
            for j in range(N):
                a0=(i-1)*(N+1)+j;b0=i*(N+1)+j
                faces.append((a0,a0+1,b0+1,b0))
    # Separate cap vertices give a hard edge at cut wood and band ends.
    endcap=None
    for i,reverse in [(0,True),(len(stations)-1,False)]:
        if head and not reverse:
            capv=[v[i*(N+1)+j] for j in range(N)]
            center=tuple(sum((Vector(p) for p in capv),Vector())/N)
            capuv=[(.5+.46*math.cos(2*PI*j/N),.5+.46*math.sin(2*PI*j/N)) for j in range(N)]+[(.5,.5)]
            capv.append(center);capfaces=[(N,j,(j+1)%N) for j in range(N)]
            endcap=(capv,capfaces,capuv)
            continue
        ring=[]
        for j in range(N):
            ring.append(len(v));v.append(v[i*(N+1)+j]);uv.append((.5+.46*math.cos(2*PI*j/N),.5+.46*math.sin(2*PI*j/N)))
        ci=len(v);v.append(tuple(sum((Vector(v[k]) for k in ring),Vector())/N));uv.append((.5,.5))
        for j in range(N):faces.append((ci,ring[(j+1)%N],ring[j]) if reverse else (ci,ring[j],ring[(j+1)%N]))
    result=mesh(name,v,faces,uv,mat)
    if endcap:mesh('10_Cut_Endgrain_Surface',*endcap,'endgrain',False)
    return result

zs=sorted({round(z,8) for z in [a[0] for a in PROFILE]+[-87.90222168+(46.70283890+87.90222168)*i/105 for i in range(106)]})
wood=loft('01_Carved_Wood_Core',[(z,*sample(z)[1:],0) for z in zs],'wood',40,head=True)
def wrapping(name,lo,hi,rows):
    return loft(name,[(lo+(hi-lo)*i/rows,*sample(lo+(hi-lo)*i/rows)[1:],.22) for i in range(rows+1)],'cloth',32,wrap=True)
wrapping('02_Lower_Purple_Spiral_Wrap',-78,-14.0,208)
wrapping('03_Upper_Purple_Wrap',26.65,35.6,76)

def lathe_local(name,z,profile,mat,N=48):
    center,a,b,t=frame(z);v=[];uv=[];faces=[]
    for i,(h,r) in enumerate(profile):
        if mat=='silver':r*=1.20
        for j in range(N+1):
            th=2*PI*j/N;p=center+t*h+a*(r*math.cos(th))+b*(r*math.sin(th))
            v.append(tuple(p));uv.append((j/N,i/(len(profile)-1)))
        if i:
            for j in range(N):
                k=(i-1)*(N+1)+j;l=i*(N+1)+j;faces.append((k,k+1,l+1,l))
    return mesh(name,v,faces,uv,mat)

# The grip is a closed hollow metal sleeve; end lips hide the seam with the wood.
sleeve_profile=[(-5.05,1.52),(-5.05,1.95),(-4.95,2.07),(-4.7,2.10),(-4.5,1.99),(-4.25,1.94),(-3.9,1.92),(-2,1.92),(0,1.92),(2,1.92),(3.9,1.92),(4.25,1.94),(4.5,1.99),(4.7,2.10),(4.95,2.07),(5.05,1.95),(5.05,1.52),(-5.05,1.52)]
lathe_local('04_Engraved_Silver_Grip_Sleeve',5.25,sleeve_profile,'silver',56)
collars=[]
def band(z,r,width):
    p=[(-width/2,r-.30),(-width/2,r-.04),(-width/2+.11,r+.08),(-width/2+.25,r+.13),(width/2-.25,r+.13),(width/2-.11,r+.08),(width/2,r-.04),(width/2,r-.3),(-width/2,r-.3)]
    ob=lathe_local('collar',z,p,'silver',48);collars.append(ob)
band(12.45,1.86,.68);band(13.45,1.89,.68);band(22.5,2.18,3.05);band(31.4,1.96,1.35)
# Raised rims on the wide riveted cuff.
band(21.20,2.26,.52);band(23.78,2.26,.52)

def tube_path(name,points,r,mat,sides=5,closed=False):
    v=[];uv=[];faces=[];pts=[Vector(p) for p in points]
    if closed:pts.append(pts[0])
    for i,p in enumerate(pts):
        tangent=(pts[min(i+1,len(pts)-1)]-pts[max(0,i-1)]).normalized()
        ref=Vector((0,1,0)) if abs(tangent.y)<.9 else Vector((1,0,0))
        a=tangent.cross(ref).normalized();b=tangent.cross(a).normalized()
        for j in range(sides+1):
            q=p+r*(math.cos(2*PI*j/sides)*a+math.sin(2*PI*j/sides)*b);v.append(tuple(q));uv.append((j/sides,i/max(1,len(pts)-1)))
        if i:
            for j in range(sides):
                k=(i-1)*(sides+1)+j;l=i*(sides+1)+j;faces.append((k,k+1,l+1,l))
    # Tubes terminate inside the sleeve/cuff; explicit end caps preserve manifold structure.
    if not closed:
        faces.append(tuple(reversed(range(sides))))
        faces.append(tuple((len(pts)-1)*(sides+1)+j for j in range(sides)))
    return mesh(name,v,faces,uv,mat)

orn=[]
def sleevept(z,th,r=1.965):return (r*1.20*math.cos(th)+sample(z)[0],r*1.20*math.sin(th),z)
for sg in [-1,1]:
    pts=[sleevept(.9+8.8*i/120,sg*(2*PI*i/120)+.25) for i in range(121)]
    orn.append(tube_path('interlaced_border',pts,.073,'silver'))
for row in [2.95,7.55]:
    for j in range(5):
        a=2*PI*j/5+.18
        pts=[sleevept(row+1.10*math.sin(2*PI*i/40),a+.215*math.cos(2*PI*i/40),1.936) for i in range(40)]
        orn.append(tube_path('oval_inlay',pts,.066,'silver',5,True))

def sphere(name,p,r,mat):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=12,ring_count=6,radius=r,location=p)
    ob=bpy.context.object
    for c in list(ob.users_collection):c.objects.unlink(ob)
    main.objects.link(ob);ob.name=name;ob.data.materials.append(materials[mat]);ob['material_key']=mat
    for po in ob.data.polygons:po.use_smooth=True
    parts.append(ob);return ob
for i in range(6):
    c,a,b,t=frame(22.5);th=2*PI*i/6
    ob=sphere('cuff_rivet',tuple(c+a*(2.32*1.2*math.cos(th))+b*(2.32*1.2*math.sin(th))),.216,'silver');collars.append(ob)

def join_objects(name,obs):
    bpy.ops.object.select_all(action='DESELECT')
    for ob in obs:ob.select_set(True)
    bpy.context.view_layer.objects.active=obs[0];bpy.ops.object.join();ob=bpy.context.object;ob.name=name
    for x in obs[1:]:
        if x in parts:parts.remove(x)
    return ob
join_objects('05_Silver_Collars_and_Rivets',collars)
join_objects('06_Silver_Interlace_and_Eye_Ornaments',orn)

# Static ribbon with real thickness: visible from either side without alpha blending.
rv=[];ruv=[];rf=[];rows=64;cols=6
for side in [-1,1]:
    for i in range(rows+1):
        t=i/rows;z=29.0-23.2*t
        cx=sample(29)[0]+2.45+3.9*math.sin(t*PI*.65)+.7*math.sin(t*PI*3)
        cy=-.38+.72*math.sin(t*PI*2.1)
        ang=.18+.7*math.sin(t*PI*1.7)
        width=1.85*(1-.12*t)
        if t>.90:width*=max(.035,(1-t)/.10)
        for j in range(cols+1):
            u=j/cols-.5
            yy=cy+u*width*math.sin(ang)+.12*math.sin(u*PI)*math.sin(t*PI*3)
            rv.append((cx+u*width*math.cos(ang),yy+side*.034,z+.07*math.cos(u*PI)))
            ruv.append((j/cols,t))
            if i and j:
                k=(0 if side==-1 else (rows+1)*(cols+1))+(i-1)*(cols+1)+j-1
                a=k;b=k+1;c=k+cols+2;d=k+cols+1;rf.append((a,d,c,b) if side==-1 else (a,b,c,d))
offset=(rows+1)*(cols+1)
for i in range(rows):
    for j in [0,cols]:
        a=i*(cols+1)+j;b=(i+1)*(cols+1)+j;rf.append((a,b,b+offset,a+offset))
for i in [0,rows]:
    for j in range(cols):
        a=i*(cols+1)+j;rf.append((a,a+offset,a+offset+1,a+1))
ribbon=mesh('07_Hanging_Purple_Ribbon',rv,rf,ruv,'cloth')
tip=Vector(rv[rows*(cols+1)+cols//2]);gemx=tip.x;gemy=tip.y
# A small silver eyelet joins the tapered cloth and red faceted pendant.
eye=tube_path('08_Pendant_Silver_Eyelet',[(gemx+.26*math.cos(2*PI*i/32),gemy-.05,5.65+.38*math.sin(2*PI*i/32)) for i in range(32)],.075,'silver',6,True)
gv=[];guv=[];gf=[];N=10
for iz,(z,r) in enumerate([(5.36,.08),(4.45,.67),(3.82,.60),(2.35,.025)]):
    for j in range(N+1):
        a=2*PI*j/N;gv.append((gemx+r*math.cos(a),gemy+r*.68*math.sin(a),z));guv.append((j/N,iz/3))
    if iz:
        for j in range(N):
            k=(iz-1)*(N+1)+j;l=iz*(N+1)+j;gf.extend([(k,k+1,l+1),(k,l+1,l)])
gf.append(tuple(reversed(range(N))));gf.append(tuple(3*(N+1)+j for j in range(N)))
mesh('09_Red_Faceted_Pendant',gv,gf,guv,'gem',False)

# Export evaluated mesh corners with distinct UV/normal seams, exactly as NIF needs them.
bpy.context.view_layer.update()
export=[];stats=[]
for ob in parts:
    if ob.name not in bpy.data.objects:continue
    me=ob.data;me.calc_loop_triangles();uv=me.uv_layers.active.data
    verts=[];normals=[];uvs=[];triangles=[];keys={}
    for tri in me.loop_triangles:
        ids=[]
        for li in tri.loops:
            idx=me.loops[li].vertex_index;p=ob.matrix_world@me.vertices[idx].co
            no=ob.matrix_world.to_3x3()@me.corner_normals[li].vector;no.normalize()
            pos=(p.x,p.z,-p.y);n=(no.x,no.z,-no.y);u=tuple(uv[li].uv)
            key=tuple(round(x,6) for x in (*pos,*n,*u))
            if key not in keys:
                keys[key]=len(verts);verts.append(pos);normals.append(n);uvs.append(u)
            ids.append(keys[key])
        triangles.append(ids)
    assert len(verts)<15000 and len(triangles)<15000,(ob.name,len(verts),len(triangles))
    e=dict(name=ob.name,material=ob['material_key'],verts=verts,normals=normals,uvs=uvs,tris=triangles)
    export.append(e);stats.append(dict(name=ob.name,material=e['material'],vertices=len(verts),triangles=len(triangles)))
    ob['nif_vertex_count']=len(verts);ob['nif_triangle_count']=len(triangles)
    print('PART',stats[-1])
(OUT/'mesh_data.json').write_text(json.dumps(export,separators=(',',':')),encoding='utf8')
(EV/'mesh_budget.json').write_text(json.dumps(stats,indent=2),encoding='utf8')
with (OUT/'fern_staff.meshspec').open('w',encoding='ascii') as f:
    for e in export:
        gloss,spec={'wood':(13,.12),'endgrain':(10,.10),'cloth':(12,.20),'silver':(65,.65),'gem':(210,1.6)}[e['material']]
        f.write(f"MESH {e['name']} {e['material']} {gloss} {spec} 0 {len(e['verts'])} {len(e['tris'])}\n")
        for p,u,n in zip(e['verts'],e['uvs'],e['normals']):f.write(' '.join(f'{x:.8f}' for x in (*p,u[0],1-u[1],*n))+'\n')
        for t in e['tris']:f.write(' '.join(map(str,t))+'\n')

# Model metadata and guide markers stay outside the game-mesh collection.
for name,loc in [('VANILLA_GRIP_ORIGIN',(0,0,0)),('STAFF_HEAD_LIMIT',(0,0,46.7028389)),('STAFF_BUTT_LIMIT',(0,0,-87.90222168))]:
    ob=bpy.data.objects.new(name,None);stage.objects.link(ob);ob.location=loc;ob.empty_display_size=3

scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=40
scene.cycles.use_denoising=True;scene.render.image_settings.file_format='PNG';scene.render.film_transparent=False
scene.world.color=(.16,.16,.16)
world=scene.world;world.use_nodes=True;world.node_tree.nodes['Background'].inputs['Color'].default_value=(.12,.14,.18,1);world.node_tree.nodes['Background'].inputs['Strength'].default_value=.55
def move_stage(ob):
    for c in list(ob.users_collection):c.objects.unlink(ob)
    stage.objects.link(ob)
def area(name,p,power,size,color,target):
    d=bpy.data.lights.new(name,'AREA');ob=bpy.data.objects.new(name,d);stage.objects.link(ob);ob.location=p;d.energy=power;d.shape='DISK';d.size=size;d.color=color;ob.rotation_euler=(Vector(target)-ob.location).to_track_quat('-Z','Y').to_euler()
area('Key',(45,-80,70),240000,80,(1,.90,.78),(0,0,-15))
area('Fill',(-55,-30,-20),155000,65,(.66,.78,1),(0,0,-10))
area('Rim',(30,45,55),300000,60,(.91,.92,1),(0,0,5))
bpy.ops.mesh.primitive_plane_add(size=1200,location=(0,0,-88.65));floor=bpy.context.object;floor.name='Studio_floor';move_stage(floor)
fm=bpy.data.materials.new('Studio_Backdrop');fm.diffuse_color=(.065,.082,.105,1);fm.use_nodes=True;fm.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(.065,.082,.105,1);fm.node_tree.nodes.get('Principled BSDF').inputs['Roughness'].default_value=.83;floor.data.materials.append(fm)
d=bpy.data.cameras.new('Review_Camera');cam=bpy.data.objects.new('Review_Camera',d);stage.objects.link(cam);scene.camera=cam
def render(name,position,target,scale,res):
    cam.location=position;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();d.type='ORTHO';d.ortho_scale=scale;d.lens=70
    scene.render.resolution_x=res[0];scene.render.resolution_y=res[1];scene.render.resolution_percentage=100;scene.render.filepath=str(EV/name)
    bpy.ops.render.render(write_still=True)
scene.view_settings.view_transform='AgX'
# Save the editable geometry before rendering; packed textures allow portable review.
bpy.ops.file.pack_all()
cam.location=(110,-320,35);cam.rotation_euler=(Vector((0,0,-20))-cam.location).to_track_quat('-Z','Y').to_euler();d.type='ORTHO';d.ortho_scale=149
scene.render.resolution_x=1000;scene.render.resolution_y=1700
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'FernStaff.blend'))
render('fern_staff_hero.png',(75,-300,25),(0,0,-20),149,(1000,1700))
render('fern_staff_details.png',(38,-160,25),(1,0,20),62,(1450,1450))
render('fern_staff_reverse.png',(-75,300,25),(0,0,-20),149,(1000,1700))
print('MODEL_COMPLETE',len(export),'parts',sum(s['vertices'] for s in stats),'verts',sum(s['triangles'] for s in stats),'tris')
