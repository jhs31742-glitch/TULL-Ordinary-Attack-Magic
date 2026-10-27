#include "VFX/CircleTint.h"
#include "VFX/CircleOwnershipPolicy.h"
#include <array>
#include <chrono>
#include <cstring>
#include <vector>
namespace TOAM::VFX::CircleTint {
// Keep the engine root and its aliases untouched. A lease replaces only its
// child edges and restores the original branches when the last owner retires.
struct Lease {
 struct Branch {std::uint32_t index{};RE::NiPointer<RE::NiAVObject> original{},replacement{};};
 RE::NiPointer<RE::NiNode> root{};std::array<Branch,32> branches{};std::size_t count{};
 ~Lease(){if(root)for(std::size_t i=0;i<count;++i){auto& b=branches[i];const auto& children=root->GetChildren();
  if(b.index<children.size()&&children[b.index].get()==b.replacement.get())root->SetAt(b.index,b.original.get());
 }}
};
namespace {
constexpr std::array<const char*,4> originalPaths{
 "textures\\TOAM\\VFX\\TOAM_Circle_Face.dds","textures\\TOAM\\VFX\\TOAM_Circle_Rings.dds","textures\\TOAM\\VFX\\TOAM_Glow_Soft.dds","textures\\TOAM\\VFX\\TOAM_Hebrew_Band.dds"};
constexpr std::array<const char*,4> neutralPaths{
 "textures\\TOAM\\VFX\\Arcana_Circle_Face.dds","textures\\TOAM\\VFX\\Arcana_Circle_Rings.dds","textures\\TOAM\\VFX\\Arcana_Glow_Soft.dds","textures\\TOAM\\VFX\\Arcana_Hebrew_Band.dds"};
std::array<RE::NiPointer<RE::NiSourceTexture>,4> neutral{};

struct Entry {RE::BSGeometry* geometry{};RE::NiPointer<RE::BSShaderProperty> original{},replacement{};};
struct Plan {
 const char* failure="none";unsigned rejectedRefs{};const char* rejectedName="";
 std::array<Entry,32> entries{};std::size_t count{},nodes{},assigned{};bool committed=false;
 ~Plan(){if(!committed)for(std::size_t i=0;i<assigned;++i){auto& e=entries[i];e.geometry->GetGeometryRuntimeData().shaderProperty=e.original;}}
};
bool Reject(Plan& plan,const char* reason,RE::NiAVObject* object=nullptr){plan.failure=reason;if(object){plan.rejectedRefs=object->GetRefCount();plan.rejectedName=object->name.c_str();}return false;}
bool PrepareGeometry(RE::NiAVObject* o,Plan& plan,unsigned depth){
 if(!o||depth>24||++plan.nodes>256)return Reject(plan,"traversal_bound",o);
 if(depth&&o->AsNode()&&o->GetRefCount()!=1)return Reject(plan,"private_clone_node_refs",o);
 if(auto* g=o->AsGeometry()){
  // One parent reference is required. Shared cached geometry fails closed.
  if(g->GetRefCount()!=1||plan.count==plan.entries.size())return Reject(plan,"private_clone_geometry_refs_or_bound",o);
  auto* property=g->GetGeometryRuntimeData().shaderProperty.get();
  if(!property||!property->GetRTTI()||std::strcmp(property->GetRTTI()->GetName(),"BSEffectShaderProperty"))return Reject(plan,"property_type",o);
  auto* p=static_cast<RE::BSEffectShaderProperty*>(property);auto* m=p->GetMaterial();
  if(!m||m->GetType()!=RE::BSShaderMaterial::Type::kEffect||!m->sourceTexture)return Reject(plan,"material_type_or_texture",o);
  RE::NiPointer<RE::NiObject> copy(p->Clone());
  if(!copy||copy.get()==p||!copy->GetRTTI()||std::strcmp(copy->GetRTTI()->GetName(),"BSEffectShaderProperty"))return Reject(plan,"property_clone",o);
  auto* c=static_cast<RE::BSEffectShaderProperty*>(copy.get());
  // Native SE98897 -> material manager Create/CopyMembers with a fresh unique seed.
  c->SetMaterial(m,true);auto* cm=c->GetMaterial();
  if(!cm||cm==m||!cm->DoIsCopy(m)||c->flags!=p->flags||c->alpha!=p->alpha||cm->baseColor.alpha!=m->baseColor.alpha||cm->baseColorScale!=m->baseColorScale)return Reject(plan,"material_copy_flags_alpha_gain",o);
  // Alpha-controller clones must also be private and target the cloned property.
  auto* oc=p->controllers.get();auto* cc=c->controllers.get();unsigned controls=0;
  while(oc&&cc){
   if(++controls>16||oc==cc||cc->target!=c||!cc->GetRTTI()||std::strcmp(cc->GetRTTI()->GetName(),"BSEffectShaderPropertyFloatController")||cc->flags!=oc->flags||cc->frequency!=oc->frequency||cc->phase!=oc->phase||cc->loKeyTime!=oc->loKeyTime||cc->hiKeyTime!=oc->hiKeyTime)return Reject(plan,"controller_clone_target_or_timing",o);
   oc=oc->GetNext();cc=cc->GetNext();
  }
  if(oc||cc)return Reject(plan,"controller_chain_length",o);
  for(std::size_t i=0;i<plan.count;++i)if(plan.entries[i].geometry==g||plan.entries[i].replacement.get()==c||plan.entries[i].replacement->GetBaseMaterial()==cm)return Reject(plan,"duplicate_clone_identity",o);
  plan.entries[plan.count++]={g,RE::NiPointer<RE::BSShaderProperty>(p),RE::NiPointer<RE::BSShaderProperty>(c)};
 }
 if(auto* n=o->AsNode())for(auto& ch:n->GetChildren())if(ch&&!PrepareGeometry(ch.get(),plan,depth+1))return false;
 return true;
}
struct TreeAudit {
 std::array<RE::NiAVObject*,256> original{},copied{};std::size_t originalCount{},copiedCount{};
 bool Collect(RE::NiAVObject* o,unsigned depth=0){
  if(!o||depth>24||originalCount==original.size()||o->collisionObject||o->controllers)return false;
  for(std::size_t i=0;i<originalCount;++i)if(original[i]==o)return false;
  const char* type=o->GetRTTI()?o->GetRTTI()->GetName():"";
  if(std::strcmp(type,"NiNode")&&std::strcmp(type,"BSTriShape"))return false;
  original[originalCount++]=o;
  if(auto* g=o->AsGeometry())if(g->GetGeometryRuntimeData().skinInstance)return false;
  if(auto* n=o->AsNode())for(auto& c:n->GetChildren())if(c&&!Collect(c.get(),depth+1))return false;
  return true;
 }
 bool Pair(RE::NiAVObject* a,RE::NiAVObject* b,unsigned depth=0){
  if(!a||!b||depth>24||copiedCount==copied.size()||b->GetRefCount()!=1||b->collisionObject||b->controllers)return false;
  for(std::size_t i=0;i<originalCount;++i)if(original[i]==b)return false;
  for(std::size_t i=0;i<copiedCount;++i)if(copied[i]==b)return false;
  if(!b->GetRTTI()||std::strcmp(a->GetRTTI()->GetName(),b->GetRTTI()->GetName())||a->name!=b->name)return false;
  copied[copiedCount++]=b;
  // Native clones copy authored local pose; preserve the current owned pose and
  // per-instance fade, without touching immutable mesh buffers or original art.
  b->local=a->local;b->fadeAmount=a->fadeAmount;
  if(auto* ag=a->AsGeometry()){
   auto* bg=b->AsGeometry();if(!bg||bg->GetGeometryRuntimeData().skinInstance||std::memcmp(&ag->GetGeometryRuntimeData().vertexDesc,&bg->GetGeometryRuntimeData().vertexDesc,sizeof(RE::BSGraphics::VertexDesc)))return false;
   if(std::memcmp(&ag->GetModelData().modelBound,&bg->GetModelData().modelBound,sizeof(RE::NiBound)))return false;
  }
  if(auto* an=a->AsNode()){
   auto* bn=b->AsNode();if(!bn||an->GetChildren().size()!=bn->GetChildren().size())return false;
   for(std::size_t i=0;i<an->GetChildren().size();++i){auto* ac=an->GetChildren()[i].get();auto* bc=bn->GetChildren()[i].get();
    if(bool(ac)!=bool(bc))return false;if(ac&&(bc->parent!=bn||!Pair(ac,bc,depth+1)))return false;
   }
  }
  return true;
 }
};
std::shared_ptr<Lease> PrivateBranches(RE::NiAVObject* root){
 auto* node=root->AsNode();if(!node||root->controllers||root->collisionObject||node->GetChildren().size()>32)return {};
 TreeAudit audit;for(auto& child:node->GetChildren())if(child){
  // SetAt updates the detached branch's parent. Its top node must therefore
  // already be owned exclusively by THIS root edge; deeper shared objects
  // are only read and cloned, never detached or modified.
  if(child->parent!=node||child->GetRefCount()!=1||!audit.Collect(child.get()))return {};
 }
 auto lease=std::make_shared<Lease>();lease->root.reset(node);
 for(std::size_t index=0;index<node->GetChildren().size();++index){auto* child=node->GetChildren()[index].get();if(!child)continue;
  auto& b=lease->branches[lease->count++];b.index=static_cast<std::uint32_t>(index);b.original.reset(child);
  RE::NiPointer<RE::NiObject> cloned(child->Clone());
  if(!cloned||cloned.get()==child||!cloned->GetRTTI()||std::strcmp(cloned->GetRTTI()->GetName(),child->GetRTTI()->GetName()))return {};
  b.replacement.reset(static_cast<RE::NiAVObject*>(cloned.get()));cloned.reset();
  if(b.replacement->parent||!audit.Pair(child,b.replacement.get()))return {};
 }
 return lease->count?lease:std::shared_ptr<Lease>{};
}
bool Configure(Entry& e,Presentation::Color color){
 auto* original=static_cast<RE::BSEffectShaderProperty*>(e.original.get());auto* copy=static_cast<RE::BSEffectShaderProperty*>(e.replacement.get());
 auto* m=copy->GetMaterial();const auto* before=original->GetMaterial();const char* path=before->sourceTexturePath.c_str();bool known=false;
 for(std::size_t i=0;i<originalPaths.size();++i){
  if(!_stricmp(path,originalPaths[i])){if(!neutral[i])return false;m->sourceTexture=neutral[i];m->sourceTexturePath=neutralPaths[i];known=true;break;}
  if(!_stricmp(path,neutralPaths[i])){known=true;break;}
 }
 // Smoke on existing circle sidewalls is already neutral. Its alpha/falloff remains exact.
 if(!_stricmp(path,"textures\\TOAM\\VFX\\TOAM_Smoke_Cloud_Neutral.dds"))known=true;
 if(!known)return false;
 const auto rgb=Presentation::colorRGB[std::size_t(color)];const float high=(std::max)({before->baseColor.red,before->baseColor.green,before->baseColor.blue});
 if(!std::isfinite(high)||high<0)return false;
 m->baseColor.red=rgb.red*high;m->baseColor.green=rgb.green*high;m->baseColor.blue=rgb.blue*high;
 return m->baseColor.alpha==before->baseColor.alpha&&m->baseColorScale==before->baseColorScale&&copy->flags==original->flags;
}
}
void Prepare() noexcept {
 if(REL::Module::get().version()!=SKSE::RUNTIME_SSE_1_5_97)return;
 for(std::size_t i=0;i<neutral.size();++i){RE::NiPointer<RE::NiTexture> texture;RE::BSShaderManager::GetTexture(neutralPaths[i],true,texture,false);
  if(texture&&texture->GetRTTI()&&!std::strcmp(texture->GetRTTI()->GetName(),"NiSourceTexture"))neutral[i]=RE::NiPointer<RE::NiSourceTexture>(static_cast<RE::NiSourceTexture*>(texture.get()));
 }
}
void Apply(State& state,RE::ModelReferenceEffect* effect) noexcept {
 auto* root=effect&&!effect->finished?effect->Get3D():nullptr;
 if(!root||state.color==Presentation::Color::Default||state.color>=Presentation::Color::Count||root==state.attemptedRoot)return;
 state.lease.reset();state.attemptedRoot=root;const auto start=std::chrono::steady_clock::now();
 if(REL::Module::get().version()!=SKSE::RUNTIME_SSE_1_5_97||!root->GetObjectByName(RE::BSFixedString{"AS_CirclePose"}))return;
 // Pinned SE: effect owns artObject3D(+C8), plus AttachTechniqueInput
 // current3DRoot(+70) and attachedArt(+78). Count an alias only when its
 // actual pointer equals THIS root. Parent must contain exactly one edge.
 if(effect->artObject3D.get()!=root){logger::warn("[TOAM][CircleColor] REJECT reason=effect_root_mismatch");return;}
 const bool attachedAlias=effect->hitEffectArtData.attachedArt.get()==root;
 const bool currentAlias=effect->hitEffectArtData.current3DRoot.get()==root;
 unsigned parentReferences=0;
 if(root->parent){const auto& children=root->parent->GetChildren();if(children.size()>256)return;for(const auto& child:children)if(child.get()==root)++parentReferences;if(parentReferences!=1)return;}
 const auto actual=root->GetRefCount();
 const auto expected=CircleOwnership::Expected(root->parent!=nullptr,attachedAlias,currentAlias);
 if(!CircleOwnership::IsPrivate(actual,parentReferences,root->parent!=nullptr,attachedAlias,currentAlias)){logger::warn("[TOAM][CircleColor] REJECT reason=unknown_root_owner refs={} expected={} parent={} attachedArt={} current3DRoot={} original_retained=true",actual,expected,parentReferences,attachedAlias,currentAlias);return;}
 logger::info("[TOAM][CircleColor] OWNERSHIP_PROVED refs={} expected={} parent={} attachedArt={} current3DRoot={}",actual,expected,parentReferences,attachedAlias,currentAlias);

 try{
  auto lease=PrivateBranches(root);if(!lease){logger::warn("[TOAM][CircleColor] REJECT reason=private_branch_clone_or_structure original_retained=true");return;}
  Plan plan;
  for(std::size_t i=0;i<lease->count;++i)if(!PrepareGeometry(lease->branches[i].replacement.get(),plan,0)){
   logger::warn("[TOAM][CircleColor] REJECT reason={} node={} refs={} original_retained=true",plan.failure,plan.rejectedName,plan.rejectedRefs);return;
  }
  if(!plan.count)return;
  for(std::size_t i=0;i<plan.count;++i)if(!Configure(plan.entries[i],state.color)){logger::warn("[TOAM][CircleColor] Rejected material/neutral texture; original retained");return;}
  for(std::size_t i=0;i<plan.count;++i){auto& e=plan.entries[i];if(e.geometry->GetGeometryRuntimeData().shaderProperty.get()!=e.original.get())return;}
  for(std::size_t i=0;i<plan.count;++i){auto& e=plan.entries[i];auto* expected=e.replacement->GetBaseMaterial();
   e.geometry->GetGeometryRuntimeData().shaderProperty=e.replacement;++plan.assigned;
   if(!e.replacement->SetupGeometry(e.geometry)||e.geometry->GetGeometryRuntimeData().shaderProperty.get()!=e.replacement.get()||e.replacement->GetBaseMaterial()!=expected){logger::warn("[TOAM][CircleColor] Setup rejected; original properties restored");return;}
  }
  // Commit only after every detached branch has passed material/geometry setup.
  // SetAt maintains native parent pointers/refcounts and keeps engine root aliases.
  for(std::size_t i=0;i<lease->count;++i){auto& b=lease->branches[i];
   if(lease->root->GetChildren()[b.index].get()!=b.original.get())return;
  }
  for(std::size_t i=0;i<lease->count;++i){auto& b=lease->branches[i];lease->root->SetAt(b.index,b.replacement.get());
   if(lease->root->GetChildren()[b.index].get()!=b.replacement.get()||b.replacement->parent!=lease->root.get())return;
  }
  RE::NiUpdateData update{};root->Update(update);
  plan.committed=true;state.lease=std::move(lease);
  const auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
  logger::info("[TOAM][CircleColor] INSTANCE_APPLIED preset={} properties={} cpu_us={} private_branches=true property_and_material_unique=true alpha_gain_preserved=true pixels=UNVERIFIED",Presentation::colorKeys[std::size_t(state.color)],plan.count,us);
 }catch(...){logger::warn("[TOAM][CircleColor] Preparation failed; original retained");}
}
}
