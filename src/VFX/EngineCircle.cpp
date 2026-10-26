#include "VFX/EngineCircle.h"
#include "VFX/CircleMotion.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <unordered_map>
namespace TOAM::VFX::EngineCircle {
namespace {
constexpr std::size_t capacity=193; // Last slot is the independent Temperance halo.
constexpr float requestLifetime=2.0F, acquireLimit=1.8F, quarantineSeconds=2.2F;
struct Slot {
 CircleTint::State tint{};
 RE::NiPointer<RE::ModelReferenceEffect> effect{};
 RE::BGSArtObject* form{};
 RE::NiTransform world{};
 std::uint16_t generation{};
 float requestAge{},readyAge{},growthSeconds{},retireAge{},retireSeconds{},spinPhase{};
 Style style{Style::Original};
 bool occupied{},cancelled{},posed{},visible{true},retiring{};unsigned phase{};
};
std::array<Slot,capacity> slots{};
std::array<RE::BGSArtObject*,capacity> forms{};
bool formsResolved{},formsReady{};std::array<bool,capacity> resources{};
float Smooth(float x){x=x<0?0:(x>1?1:x);return x*x*(3-2*x);}
bool Finite(const RE::NiTransform& t){
 if(!std::isfinite(t.scale)||t.scale<=0||!std::isfinite(t.translate.x)||!std::isfinite(t.translate.y)||!std::isfinite(t.translate.z))return false;
 for(int i=0;i<3;++i)for(int j=0;j<3;++j)if(!std::isfinite(t.rotate.entry[i][j]))return false;
 return true;
}
void Finish(Slot& s){
 if(s.effect)s.effect->finished=true;
 const auto generation=s.generation;s=Slot{};s.generation=generation;
}
void Cancel(Slot& s){
 if(s.effect){Finish(s);return;}
 // A pending request owns its form until the finite engine TTL has elapsed.
 // A late returned effect is acquired and finished, never posed or made visible.
 s.cancelled=true;s.visible=false;s.retiring=false;
}
Slot* Resolve(VFXHandle h){
 if(!h.IsValid()||h.slot>=capacity)return nullptr;
 auto& s=slots[h.slot];return s.occupied&&s.generation==h.generation&&!s.cancelled?&s:nullptr;
}
void ResolveForms(){
 if(formsResolved)return;
 auto* data=RE::TESDataHandler::GetSingleton();if(!data)return;
 formsResolved=true;formsReady=true;
 for(std::size_t i=0;i<capacity;++i){
  forms[i]=data->LookupForm<RE::BGSArtObject>(i==192?0xDC5:0xD00+static_cast<std::uint32_t>(i),"TULL Ordinary Attack Magic.esp");
  formsReady=formsReady&&forms[i]!=nullptr;
 }
 std::unordered_map<std::string,bool> checked;
 for(std::size_t i=0;i<capacity;++i){
  const char* model=forms[i]?forms[i]->GetModel():nullptr;
  if(!model||!*model){logger::error("[TOAM][CP021C] MODEL_PATH_MISSING | Slot={}",i);continue;}
  const std::string path=std::string("meshes\\")+model;
  auto found=checked.find(path);
  if(found!=checked.end()){resources[i]=found->second;continue;}
  RE::NiPointer<RE::NiNode> prototype;RE::BSModelDB::DBTraits::ArgsType args{};
  const auto result=RE::BSModelDB::Demand(path.c_str(),prototype,args);
  static const RE::BSFixedString name{"AS_CirclePose"};
  const bool ready=result==RE::BSResource::ErrorCode::kNone&&prototype&&prototype->GetObjectByName(name);
  checked.emplace(path,ready);resources[i]=ready;
  logger::info("[TOAM][CP021C] MODEL_PROBE | Path={} | Error={} | Pose={} | Pixels=UNVERIFIED",
   path,int(result),ready);
 }
 logger::info("[TOAM][CP019C] FORMS | Ready={} | Slots={} | SharedWithArcana=false",formsReady,capacity);
}
void Alpha(RE::NiAVObject* object,float a){
 if(!object)return;object->fadeAmount=a;object->SetAppCulled(a<=0);
 if(auto* node=object->AsNode())for(auto& child:node->GetChildren())if(child)Alpha(child.get(),a);
}
bool Pose(Slot& s,float alpha,float growth){
 if(!s.effect||s.effect->finished)return false;
 auto* root=s.effect->Get3D();if(!root)return false;
 CircleTint::Apply(s.tint,root);
 RE::NiUpdateData update{};root->Update(update);
 static const RE::BSFixedString poseName{"AS_CirclePose"};
 auto* node=root->GetObjectByName(poseName);if(!node||!node->parent)return false;
 const auto& parent=node->parent->world;
 if(!std::isfinite(parent.scale)||std::fabs(parent.scale)<0.001F)return false;
 const auto inverse=parent.rotate.Transpose();
 node->local.translate=inverse*(s.world.translate-parent.translate)/parent.scale;
 RE::NiMatrix3 spin;spin.MakeYRotation(s.spinPhase);
 node->local.rotate=inverse*s.world.rotate*spin;
 node->local.scale=s.world.scale*growth/parent.scale;
 root->Update(update);Alpha(root,s.visible?alpha:0);
 return true;
}
}
VFXHandle Spawn(RE::NiNode* parent,const RE::NiTransform& local,bool visible,Style style,Presentation::Color color) noexcept {
 ResolveForms();auto* player=RE::PlayerCharacter::GetSingleton();
 if(!formsReady||!player||!player->Get3D(false)||!parent||!Finite(local)){
  static unsigned rejects{};if(rejects++<8)logger::warn("[TOAM][CP021C] REQUEST_REJECTED | Forms={} | Player={} | Parent={} | Transform={}",formsReady,player!=nullptr,parent!=nullptr,Finite(local));return {};
 }
 const auto world=parent->world*local;if(!Finite(world))return {};
 std::size_t begin=0,end=128;
 if(style==Style::SunFull){begin=128;end=144;}else if(style==Style::SunOuter){begin=144;end=160;}else if(style==Style::Dome){begin=160;end=192;}else if(style==Style::Halo){begin=192;end=193;}
 bool anyResource=false;for(std::size_t i=begin;i<end;++i)anyResource|=resources[i];
 if(!anyResource){static std::array<bool,5> logged{};const auto k=std::size_t(style);if(k<logged.size()&&!logged[k]){logged[k]=true;logger::error("[TOAM][CP021C] STYLE_RESOURCE_UNAVAILABLE | Style={}",int(style));}return {};}
 for(std::size_t i=begin;i<end;++i){
  auto& s=slots[i];if(s.occupied||!resources[i])continue;
  auto generation=static_cast<std::uint16_t>(s.generation+1);if(!generation)generation=1;
  s=Slot{};s.generation=generation;s.occupied=true;s.form=forms[i];s.world=world;s.visible=visible;s.style=style;s.tint.color=color;
  // Arcana SE97 adapter: ApplyArtObject ID22289 returns BOOL, not an effect pointer.
  using Apply=bool(*)(RE::TESObjectREFR*,RE::BGSArtObject*,float,RE::TESObjectREFR*,bool,bool,RE::NiAVObject*,bool);
  static REL::Relocation<Apply> apply{REL::RelocationID(22289,22769)};
  if(!apply(player,s.form,requestLifetime,nullptr,false,false,player->Get3D(false),false)){
   logger::warn("[TOAM][CP021C] REQUEST_FAILED | Slot={} | Gen={} | Form={:08X}",i,generation,s.form->GetFormID());Finish(s);return {};}
  logger::info("[TOAM][CP021C] REQUEST_ACCEPTED | Slot={} | Gen={} | Form={:08X} | Owner={:08X} | Pixels=UNVERIFIED",i,generation,s.form->GetFormID(),player->GetFormID());
  return {static_cast<std::uint16_t>(i),generation};
 }
 static unsigned fullLogs{};if(fullLogs++<8)logger::warn("[TOAM][CP019C] POOL_FULL | Style={}",int(style));
 return {};
}
bool Set(VFXHandle h,const RE::NiTransform& local,bool visible) noexcept {
 auto* s=Resolve(h);if(!s||!Finite(local))return false;
 // CircleRenderer converts parent-local authoring into world coordinates first.
 s->world=local;s->visible=visible;return true;
}
bool Visible(VFXHandle h,bool visible) noexcept {auto* s=Resolve(h);if(!s)return false;s->visible=visible;return true;}
bool Growth(VFXHandle h,float seconds) noexcept {auto* s=Resolve(h);if(!s||!std::isfinite(seconds)||seconds<=0)return false;s->growthSeconds=seconds;return true;}
bool Retire(VFXHandle h,float seconds) noexcept {
 auto* s=Resolve(h);if(!s||!std::isfinite(seconds)||seconds<=0)return false;
 s->retiring=true;s->retireSeconds=seconds;s->retireAge=0;return true;
}
bool Release(VFXHandle h) noexcept {auto* s=Resolve(h);if(!s)return false;Cancel(*s);return true;}
bool Alive(VFXHandle h) noexcept {return Resolve(h)!=nullptr;}
bool Ready(VFXHandle h) noexcept {auto* s=Resolve(h);return s&&s->posed&&s->effect&&!s->effect->finished&&s->effect->Get3D();}
RE::NiAVObject* Node(VFXHandle h) noexcept {auto* s=Resolve(h);return Ready(h)?s->effect->Get3D():nullptr;}
void Update(float dt) noexcept {
 if(!std::isfinite(dt)||dt<=0)return;auto* ui=RE::UI::GetSingleton();if(ui&&ui->GameIsPaused())return;
 dt=dt>0.1F?0.1F:dt;
 auto* player=RE::PlayerCharacter::GetSingleton();bool pending=false;
 for(auto& s:slots)if(s.occupied){s.requestAge+=dt;if(!s.effect&&s.requestAge<quarantineSeconds)pending=true;}
 // At most one ProcessLists pass per frame, only for outstanding finite requests.
 if(pending)if(auto* lists=RE::ProcessLists::GetSingleton())lists->ForEachModelEffect([&](RE::ModelReferenceEffect* e){
  if(e->finished||!player||e->target.get().get()!=player)return RE::BSContainer::ForEachResult::kContinue;
  for(auto& s:slots)if(s.occupied&&!s.effect&&s.form==e->artObject){
   s.effect.reset(e);
   logger::info("[TOAM][CP021C] EFFECT_ACQUIRED | Slot={} | Gen={} | Cancelled={} | Has3D={}",std::size_t(&s-slots.data()),s.generation,s.cancelled,e->Get3D()!=nullptr);
   if(s.cancelled)e->finished=true;break;
  }
  return RE::BSContainer::ForEachResult::kContinue;
 });
 for(std::size_t i=0;i<slots.size();++i){
  auto& s=slots[i];if(!s.occupied)continue;
  if(s.cancelled){if(s.effect||s.requestAge>=quarantineSeconds)Finish(s);continue;}
  if(s.effect&&s.effect->finished){Finish(s);continue;}
  if(!s.effect||!s.effect->Get3D()){
   if(s.requestAge>=acquireLimit){logger::warn("[TOAM][CP021C] {} | Slot={} | Gen={} | Retired={}",s.effect?"MODEL_3D_TIMEOUT":"EFFECT_ACQUIRE_TIMEOUT",i,s.generation,s.retiring);Cancel(s);}
   continue;
  }
  if(s.phase<1){s.phase=1;logger::info("[TOAM][CP021C] MODEL_3D_READY | Slot={} | Gen={} | Pixels=UNVERIFIED",i,s.generation);}
  // BSTempEffect lifetime/age are pinned SE fields; keep alive only while explicitly owned.
  s.effect->lifetime=s.effect->age+requestLifetime;
  float growth=s.growthSeconds>0?0.02F+0.98F*Smooth(s.readyAge/s.growthSeconds):1.0F;
  // Telegraph time begins only after first valid pose; gameplay timers never wait for this.
  const float hold=s.retireSeconds*0.30F;
  float alpha=s.retiring&&s.retireAge>hold?1-Smooth((s.retireAge-hold)/(s.retireSeconds-hold)):1;
  // Original/Dome owners gain reveal spin. Sun and halo retain their own phase owners.
  if(s.style==Style::Original||s.style==Style::Dome){
   const float rate=(i%2?-.70F:.85F)+float(i%5)*.04F*(i%2?-1.F:1.F);
   s.spinPhase=CircleMotion::Advance(s.spinPhase,CircleMotion::RevealSpeed(s.readyAge,rate),dt);
  }
  if(!Pose(s,alpha,growth)){
   if(s.requestAge>=acquireLimit){logger::warn("[TOAM][CP021C] POSE_INVALID_TIMEOUT | Slot={} | Gen={} | HasNode={} | Pixels=UNVERIFIED",i,s.generation,s.effect->Get3D()->GetObjectByName(RE::BSFixedString{"AS_CirclePose"})!=nullptr);Cancel(s);}continue;
  }
  if(!s.posed){s.posed=true;logger::info("[TOAM][CP019C] FIRST_POSE | Slot={} | Wait={:.3f} | Pixels=UNVERIFIED",i,s.requestAge);}
  s.readyAge+=dt;
  // Keep a complete growth/hold before a retired, late-ready circle starts fading.
  if(s.retiring&&s.readyAge>=s.growthSeconds+0.10F){s.retireAge+=dt;if(s.retireAge>=s.retireSeconds)Finish(s);}
 }
}
void Reset() noexcept {
 ResolveForms();
 for(auto& s:slots)if(s.occupied)Cancel(s);
 // Cold loads can restore engine effects before any in-memory renderer owns a handle.
 // Purge TOAM's original beam/flash forms as well, with one bounded event-time scan.
 static std::array<RE::BGSArtObject*,512> legacy{};
 static bool legacyResolved=false;
 if(!legacyResolved&&RE::TESDataHandler::GetSingleton()){
  legacyResolved=true;
  constexpr std::array<const char*,4> prefixes{{"TOAM_ART_Beam_","TOAM_ART_BeamTint_","TOAM_ART_Flash_","TOAM_ART_FlashTint_"}};
  for(std::size_t k=0;k<4;++k)for(std::size_t i=0;i<128;++i){
   std::array<char,40> id{};std::snprintf(id.data(),id.size(),"%s%03zu",prefixes[k],i);
   legacy[k*128+i]=RE::TESForm::LookupByEditorID<RE::BGSArtObject>(id.data());
  }
 }
 // Lifecycle purge is restricted to this module's own forms. Never finish Arcana's effects.
 if(auto* lists=RE::ProcessLists::GetSingleton())lists->ForEachModelEffect([](RE::ModelReferenceEffect* e){
  for(auto* form:forms)if(form&&e->artObject==form){e->finished=true;break;}
  if(!e->finished)for(auto* form:legacy)if(form&&e->artObject==form){e->finished=true;break;}
  return RE::BSContainer::ForEachResult::kContinue;
 });
}
}