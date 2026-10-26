"""Render the shipped NIF geometry and DDS, with Blender material approximation."""
import bpy,sys,json,math
from mathutils import Vector
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,r'[USER_HOME]\AppData\Roaming\Blender Foundation\Blender\4.5\scripts\addons\io_scene_nifly')
from pyn.pynifly import NifFile
bpy.ops.wm.open_mainfile(filepath=str(ROOT/'source'/'FernStaff.blend'))
main=bpy.data.collections['FERN_STAFF_GAME_MESHES']
for ob in list(main.objects):bpy.data.objects.remove(ob,do_unlink=True)
for key in ['wood','cloth','silver','gem']:
    mat=bpy.data.materials[key]
    for node in mat.node_tree.nodes:
        if node.type=='TEX_IMAGE':
            stem=Path(node.image.name).stem
            if stem.endswith('_n'):
                im=bpy.data.images.load(str(ROOT/'package'/'textures'/'FernStaff'/f'{key}_n.dds'),check_existing=True);im.colorspace_settings.name='Non-Color'
            else:im=bpy.data.images.load(str(ROOT/'package'/'textures'/'FernStaff'/f'{key}_d.dds'),check_existing=True)
            node.image=im
        if node.type=='NORMAL_MAP':node.inputs['Strength'].default_value=1
n=NifFile(str(ROOT/'package'/'meshes'/'FernStaff'/'FernStaff.nif'))
custom=[]
def make(sh,col,material,offset=0):
    vs=[(x,-z,y) for x,y,z in sh.verts];me=bpy.data.meshes.new(sh.name);me.from_pydata(vs,[],sh.tris);me.update()
    ob=bpy.data.objects.new(sh.name,me);col.objects.link(ob);ob.location.x=offset;me.materials.append(material)
    uv=me.uv_layers.new(name='UVMap')
    for po in me.polygons:
        po.use_smooth=True
        for li in po.loop_indices:
            u,v=sh.uvs[me.loops[li].vertex_index];uv.data[li].uv=(u,1-v)
    normals=[Vector((x,-z,y)) for x,y,z in sh.normals]
    me.normals_split_custom_set([normals[loop.vertex_index] for loop in me.loops])
    return ob
for sh in n.shapes:
    key=Path(sh.textures['Diffuse'].replace('\\','/')).stem.removesuffix('_d')
    custom.append(make(sh,main,bpy.data.materials[key]))
scene=bpy.context.scene;cam=scene.camera;scene.cycles.samples=48
scene.world.node_tree.nodes['Background'].inputs['Color'].default_value=(.11,.13,.16,1)
bpy.data.objects['Studio_floor'].hide_render=True
def render(name,p,target,scale,w,h):
    cam.location=p;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=scale
    scene.render.resolution_x=w;scene.render.resolution_y=h;scene.render.resolution_percentage=100;scene.render.filepath=str(ROOT/'evidence'/name)
    bpy.ops.render.render(write_still=True)
render('fern_staff_nif_hero.png',(60,-300,8),(0,0,-20),146,1000,1700)
render('fern_staff_nif_details.png',(30,-160,22),(1,0,20),60,1400,1400)
# A measured side-by-side control uses the exact vanilla mesh, with neutral material.
control=bpy.data.collections.new('VANILLA_STAFF01_CONTROL');scene.collection.children.link(control)
baseline=json.loads((ROOT/'evidence'/'vanilla_staff01.json').read_text())['staff01.nif']
vmat=bpy.data.materials.new('Vanilla control neutral');vmat.diffuse_color=(.22,.27,.30,1);vmat.use_nodes=True;vmat.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value=(.22,.27,.30,1)
for s in baseline['shapes']:
    me=bpy.data.meshes.new('VanillaStaff01');me.from_pydata([(v[0]-18,-v[2],v[1]) for v in s['verts']],[],s['tris']);me.update()
    ob=bpy.data.objects.new('VanillaStaff01',me);control.objects.link(ob);me.materials.append(vmat)
for ob in custom:ob.location.x=18
render('fern_staff_vanilla_scale_comparison.png',(0,-300,-20),(0,0,-20),148,1200,1700)
for ob in custom:ob.location.x=0
control.hide_render=True
bpy.ops.file.pack_all()
bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'evidence'/'Exported_NIF_Review.blend'))
print('EXPORTED_NIF_REVIEW_COMPLETE: shipped NIF and shipped DDS rendered; Skyrim runtime unverified')
