import bpy,json,sys,math
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,r'[USER_HOME]\AppData\Roaming\Blender Foundation\Blender\4.5\scripts\addons\io_scene_nifly')
from pyn.pynifly import NifFile
bpy.ops.wm.open_mainfile(filepath=str(ROOT/'evidence'/'Exported_NIF_Review.blend'))
control=bpy.data.collections.get('VANILLA_STAFF01_CONTROL')
if control:
    for ob in list(control.objects):bpy.data.objects.remove(ob,do_unlink=True)
    bpy.data.collections.remove(control)
for me in list(bpy.data.meshes):
    if me.users==0:bpy.data.meshes.remove(me)
bind=json.loads((ROOT/'evidence'/'ribbon_bind.json').read_text());points=[Vector((x,-z,y)) for x,y,z in bind['positions']]
rigdata=bpy.data.armatures.new('FSFern_SMP_Rig');rig=bpy.data.objects.new('FSFern_SMP_Rig',rigdata)
bpy.context.scene.collection.objects.link(rig);bpy.context.view_layer.objects.active=rig;rig.select_set(True)
bpy.ops.object.mode_set(mode='EDIT');root=rigdata.edit_bones.new('FSFern_StaffRoot');root.head=(0,0,0);root.tail=(0,0,3)
parent=root
for i,(name,p) in enumerate(zip(bind['bones'],points)):
    bone=rigdata.edit_bones.new(name);bone.head=p;bone.tail=points[i+1] if i<8 else p+Vector((0,0,-2.5));bone.parent=parent;parent=bone
bpy.ops.object.mode_set(mode='OBJECT');rig.show_in_front=True;rig['runtime_status']='SMP_NOT_YET_VERIFIED_IN_SKYRIM'
n=NifFile(str(ROOT/'smp_package'/'meshes'/'FernStaff'/'FernStaff.nif'));result=[]
for sh in n.shapes:
    if not sh.skin_instance_name:continue
    ob=bpy.data.objects[sh.name];assert len(ob.data.vertices)==len(sh.verts)
    for name,weights in sh.bone_weights.items():
        vg=ob.vertex_groups.new(name=name)
        for idx,w in weights:vg.add([idx],w,'REPLACE')
    mod=ob.modifiers.new('SMP_Weight_Rig_Preview','ARMATURE');mod.object=rig
    result.append(ob)
bpy.context.view_layer.update();deps=bpy.context.evaluated_depsgraph_get();binderr=0
for ob in result:
    evaluated=ob.evaluated_get(deps)
    binderr=max(binderr,max((a.co-b.co).length for a,b in zip(evaluated.data.vertices,ob.data.vertices)))
assert binderr<.04
rig.pose.bones[bind['bones'][4]].rotation_mode='XYZ';rig.pose.bones[bind['bones'][4]].rotation_euler.x=.28
bpy.context.view_layer.update();deps=bpy.context.evaluated_depsgraph_get();motion=0;anchor=0
for ob in result:
    evaluated=ob.evaluated_get(deps)
    for a,b in zip(evaluated.data.vertices,ob.data.vertices):
        dist=(a.co-b.co).length;motion=max(motion,dist)
        if ob.name.startswith('07_') and b.co.z>=28.8:anchor=max(anchor,dist)
assert motion>.5 and anchor<.04
rig.pose.bones[bind['bones'][4]].rotation_euler.x=0;bpy.context.view_layer.update()
cam=bpy.context.scene.camera;cam.location=(110,-160,65);cam.rotation_euler=(Vector((1,0,20))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=60
bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'source'/'FernStaff_SMP.blend'))
(ROOT/'evidence'/'rig_deformation_check.json').write_text(json.dumps(dict(status='OFFLINE_PASS',bind_max_error=binderr,posed_max_movement=motion,pinned_top_max_movement=anchor,skinned_objects=[ob.name for ob in result],skyrim_runtime='UNVERIFIED'),indent=2),encoding='utf8')
print('RIG_SOURCE_PASS',binderr,motion,anchor)
