#include "VFX/CircleTint.h"
#include <array>
#include <chrono>
#include <cstring>
#include <vector>
namespace TOAM::VFX::CircleTint {
namespace {
constexpr std::array<const char*,4> originalPaths{
 "textures\\TOAM\\VFX\\TOAM_Circle_Face.dds","textures\\TOAM\\VFX\\TOAM_Circle_Rings.dds","textures\\TOAM\\VFX\\TOAM_Glow_Soft.dds","textures\\TOAM\\VFX\\TOAM_Hebrew_Band.dds"};
constexpr std::array<const char*,4> neutralPaths{
 "textures\\TOAM\\VFX\\Arcana_Circle_Face.dds","textures\\TOAM\\VFX\\Arcana_Circle_Rings.dds","textures\\TOAM\\VFX\\Arcana_Glow_Soft.dds","textures\\TOAM\\VFX\\Arcana_Hebrew_Band.dds"};
std::array<RE::NiPointer<RE::NiSourceTexture>,4> neutral{};
std::array<RE::NiColor,5> palette{{{1,1,1},{.30F,.60F,1.F},{1.F,.16F,.12F},{1.F,.67F,.18F},{.68F,.25F,1.F}}};
struct Entry {RE::BSGeometry* geometry{};RE::NiPointer<RE::BSShaderProperty> original{},replacement{};};
struct Plan {
 std::array<Entry,32> entries{};std::size_t count{},nodes{},assigned{};bool committed=false;
 ~Plan(){if(!committed)for(std::size_t i=0;i<assigned;++i){auto& e=entries[i];e.geometry->GetGeometryRuntimeData().shaderProperty=e.original;}}
};
bool PrepareGeometry(RE::NiAVObject* o,Plan& plan,unsigned depth){
 if(!o||depth>24||++plan.nodes>256)return false;
 if(depth&&o->AsNode()&&o->GetRefCount()!=1)return false;
 if(auto* g=o->AsGeometry()){
  // One parent reference is required. Shared cached geometry fails closed.
  if(g->GetRefCount()!=1||plan.count==plan.entries.size())return false;
  auto* property=g->GetGeometryRuntimeData().shaderProperty.get();
  if(!property||!property->GetRTTI()||std::strcmp(property->GetRTTI()->GetName(),"BSEffectShaderProperty"))return false;
  auto* p=static_cast<RE::BSEffectShaderProperty*>(property);auto* m=p->GetMaterial();
  if(!m||m->GetType()!=RE::BSShaderMaterial::Type::kEffect||!m->sourceTexture)return false;
  RE::NiPointer<RE::NiObject> copy(p->Clone());
  if(!copy||copy.get()==p||!copy->GetRTTI()||std::strcmp(copy->GetRTTI()->GetName(),"BSEffectShaderProperty"))return false;
  auto* c=static_cast<RE::BSEffectShaderProperty*>(copy.get());
  // Native SE98897 -> material manager Create/CopyMembers with a fresh unique seed.
  c->SetMaterial(m,true);auto* cm=c->GetMaterial();
  if(!cm||cm==m||!cm->DoIsCopy(m)||c->flags!=p->flags||c->alpha!=p->alpha||cm->baseColor.alpha!=m->baseColor.alpha||cm->baseColorScale!=m->baseColorScale)return false;
  // Alpha-controller clones must also be private and target the cloned property.
  auto* oc=p->controllers.get();auto* cc=c->controllers.get();unsigned controls=0;
  while(oc&&cc){
   if(++controls>16||oc==cc||cc->target!=c||!cc->GetRTTI()||std::strcmp(cc->GetRTTI()->GetName(),"BSEffectShaderPropertyFloatController")||cc->flags!=oc->flags||cc->frequency!=oc->frequency||cc->phase!=oc->phase||cc->loKeyTime!=oc->loKeyTime||cc->hiKeyTime!=oc->hiKeyTime)return false;
   oc=oc->GetNext();cc=cc->GetNext();
  }
  if(oc||cc)return false;
  for(std::size_t i=0;i<plan.count;++i)if(plan.entries[i].geometry==g||plan.entries[i].replacement.get()==c||plan.entries[i].replacement->GetBaseMaterial()==cm)return false;
  plan.entries[plan.count++]={g,RE::NiPointer<RE::BSShaderProperty>(p),RE::NiPointer<RE::BSShaderProperty>(c)};
 }
 if(auto* n=o->AsNode())for(auto& ch:n->GetChildren())if(ch&&!PrepareGeometry(ch.get(),plan,depth+1))return false;
 return true;
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
 const auto rgb=palette[std::size_t(color)];const float high=(std::max)({before->baseColor.red,before->baseColor.green,before->baseColor.blue});
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
void Apply(State& state,RE::NiAVObject* root) noexcept {
 if(!root||state.color==Presentation::Color::Default||state.color>=Presentation::Color::Count||root==state.attemptedRoot)return;
 state.attemptedRoot=root;const auto start=std::chrono::steady_clock::now();
 if(REL::Module::get().version()!=SKSE::RUNTIME_SSE_1_5_97||!root->GetObjectByName(RE::BSFixedString{"AS_CirclePose"}))return;
 // ModelReferenceEffect::artObject3D owns one NiPointer (pinned SE +C8).
 // An attached parent must own exactly one additional child reference.
 unsigned parentReferences=0;
 if(root->parent){const auto& children=root->parent->GetChildren();if(children.size()>256)return;for(const auto& child:children)if(child.get()==root)++parentReferences;if(parentReferences!=1)return;}
 if(root->GetRefCount()!=1U+parentReferences){logger::warn("[TOAM][CircleColor] Shared root ownership rejected; original retained");return;}

 try{
  Plan plan;if(!PrepareGeometry(root,plan,0)||!plan.count){logger::warn("[TOAM][CircleColor] Rejected property/geometry ownership; original retained");return;}
  for(std::size_t i=0;i<plan.count;++i)if(!Configure(plan.entries[i],state.color)){logger::warn("[TOAM][CircleColor] Rejected material/neutral texture; original retained");return;}
  for(std::size_t i=0;i<plan.count;++i){auto& e=plan.entries[i];if(e.geometry->GetGeometryRuntimeData().shaderProperty.get()!=e.original.get())return;}
  for(std::size_t i=0;i<plan.count;++i){auto& e=plan.entries[i];auto* expected=e.replacement->GetBaseMaterial();
   e.geometry->GetGeometryRuntimeData().shaderProperty=e.replacement;++plan.assigned;
   if(!e.replacement->SetupGeometry(e.geometry)||e.geometry->GetGeometryRuntimeData().shaderProperty.get()!=e.replacement.get()||e.replacement->GetBaseMaterial()!=expected){logger::warn("[TOAM][CircleColor] Setup rejected; original properties restored");return;}
  }
  plan.committed=true;
  const auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
  logger::info("[TOAM][CircleColor] INSTANCE_APPLIED preset={} properties={} cpu_us={} property_and_material_unique=true alpha_gain_preserved=true pixels=UNVERIFIED",Presentation::colorKeys[std::size_t(state.color)],plan.count,us);
 }catch(...){logger::warn("[TOAM][CircleColor] Preparation failed; original retained");}
}
}
