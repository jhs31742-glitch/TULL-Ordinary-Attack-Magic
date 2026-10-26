float Length(const RE::NiPoint3& d){return std::sqrt(d.x*d.x+d.y*d.y+d.z*d.z);}
RE::NiPoint3 Unit(RE::NiPoint3 d){float len=Length(d);return len>.001f?d/len:RE::NiPoint3{0,1,0};}
bool Finite(const RE::NiPoint3& p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);}
RE::NiPoint3 Center(RE::Actor* a){
 auto p=a->GetPosition()+RE::NiPoint3{0,0,65};auto* root=a->Get3D(false);
 if(root&&Finite(root->worldBound.center)&&Length(root->worldBound.center-a->GetPosition())<180)p=root->worldBound.center;
 return p;
}
void Shake(RE::Actor* a,float strength,float duration){
 auto* c=RE::PlayerCamera::GetSingleton();if(!a||!c||!c->currentState||!c->cameraRoot||c->cameraRoot->GetChildren().empty())return;
 RE::ShakeCamera(strength,a->GetPosition(),duration);
}
#include "Flight.inl"
#include "SolarFlight.inl"
void Launch(Hand& h,int side,RE::MagicCaster* c,RE::Actor* actor,RE::NiAVObject* root,const RE::NiPoint3& p,const RE::NiMatrix3& r,int mode,float maxLength=4096,float width=0,float launchPhase=-1,float powerOverride=-1){
 if(mode==Rapid&&!PayResource(static_cast<RE::ActorMagicCaster*>(c),h,modes[Rapid].cost)){DenyCast(static_cast<RE::ActorMagicCaster*>(c),h);return;}
 if(mode==Fivefold&&!PayResource(static_cast<RE::ActorMagicCaster*>(c),h,modes[Fivefold].cost/5.f)){DenyCast(static_cast<RE::ActorMagicCaster*>(c),h);return;}
 if(mode==SolarCrown){int slot=ShowBeam(side,actor,root,p,r,mode,solarMaxRange,width);if(slot>=0&&StartSolarFlight(slot,actor,root))++h.launched;else{if(slot>=0)ReleaseBeam(beamSlots[slot]);++h.failed;}return;}
 auto d=r*RE::NiPoint3{0,1,0};RE::Projectile::ProjectileRot angles{-std::atan2(d.z,std::hypot(d.x,d.y)),std::atan2(d.x,d.y)};
 RE::Projectile::LaunchData data(actor,p,angles,forms.payload[mode]);data.useOrigin=true;data.autoAim=false;data.castingSource=c->GetCastingSource();
 // Star lanes retain their five independent native periods. Payload magnitude=1.
 if(mode==Tracking&&powerOverride>=0)data.power*=powerOverride;
 ShowBeam(side,actor,root,p,r,mode,maxLength,width);RE::ProjectileHandle result;
 auto previous=currentRainContext;currentRainContext=mode==Rain?RainContext{true,side,launchPhase>=0?launchPhase:h.cadence.elapsed,powerOverride>=0?powerOverride:RainShake(h.rainFired),d}:RainContext{};if(mode==Rain){currentRainContext.generation=generations[side];currentRainContext.finale=h.rainFired+1==rainShots;}
 RE::Projectile::Launch(&result,data);if(mode==Rain){auto projectile=result.get();if(projectile)RegisterRainProjectile(projectile.get(),currentRainContext);}currentRainContext=previous;
 if(result)++h.launched;else ++h.failed;
}
std::vector<RE::ActorHandle> Enemies(RE::Actor* source,const RE::NiPoint3& center,float radius,bool los){
 std::vector<std::pair<float,RE::ActorHandle>> ordered;
 if(auto* list=RE::ProcessLists::GetSingleton())list->ForEachHighActor([&](RE::Actor* a){
  if(!a||a==source||a->IsDead()||!a->Is3DLoaded()||!a->IsHostileToActor(source))return RE::BSContainer::ForEachResult::kContinue;
  float dist=Length(a->GetPosition()-center);if(dist>radius)return RE::BSContainer::ForEachResult::kContinue;
  bool detection=false;if(los&&!source->HasLineOfSight(a,detection))return RE::BSContainer::ForEachResult::kContinue;
  ordered.emplace_back(dist,a->GetHandle());return RE::BSContainer::ForEachResult::kContinue;
 });
 std::sort(ordered.begin(),ordered.end(),[](auto& a,auto& b){return a.first<b.first;});
 std::vector<RE::ActorHandle> out;for(auto& p:ordered){if(out.size()>=64)break;out.push_back(p.second);}return out;
}
float Random(Hand& h){h.rng=h.rng*1664525u+1013904223u;return float(h.rng>>8)/16777216.f;}
struct DomeWard {double expires=0;float factor=1;};std::array<DomeWard,2> domeWards{};
std::mutex guardMutex;double rainCooldown=0;bool domeCommitReady=false;
std::atomic_uint damageAuditCount{0};
void RefreshDomeWard(int side,float coverage){std::scoped_lock lock(guardMutex);domeWards[side]={Clock()+.25,DomeDamageFactor(coverage)};}
using CommitFn=void(*)(RE::Actor*,RE::ACTOR_VALUE_MODIFIER,RE::ActorValue,float,RE::Actor*);
CommitFn originalCommit=nullptr;SKSE::Trampoline domeTrampoline{"ArcanaDomeDefense"};
void ClearGuard(int side){std::scoped_lock lock(guardMutex);domeWards[side]={};}
void CommitActorValue(RE::Actor* target,RE::ACTOR_VALUE_MODIFIER modifier,RE::ActorValue av,float delta,RE::Actor* attacker){
 if(gameReady&&forms.ready&&target&&target->IsPlayerRef()&&attacker&&attacker!=target&&modifier==RE::ACTOR_VALUE_MODIFIER::kDamage&&av==RE::ActorValue::kHealth&&std::isfinite(delta)&&delta<0){
  float factor=1;{std::scoped_lock lock(guardMutex);double now=Clock();for(const auto& ward:domeWards)if(now<ward.expires)factor=std::min(factor,ward.factor);}delta*=factor;
 }
 if(rainDamage.active&&target&&attacker&&attacker->IsPlayerRef()&&target!=attacker&&modifier==RE::ACTOR_VALUE_MODIFIER::kDamage&&av==RE::ActorValue::kHealth&&std::isfinite(delta)&&delta<0)
  delta=LimitRainBase(target,delta); // Scale only Arcana Tower after existing resistance.
 const bool own=currentDamageMode>=0&&currentDamageMode<8;
 bool audit=gameReady&&target&&attacker&&attacker->IsPlayerRef()&&target!=attacker&&av==RE::ActorValue::kHealth&&modifier==RE::ACTOR_VALUE_MODIFIER::kDamage&&std::isfinite(delta)&&delta<0&&(own?ownHealthAudits[currentDamageMode].fetch_add(1)<120:damageAuditCount.fetch_add(1)<64);
 auto* owner=audit?target->AsActorValueOwner():nullptr;float before=owner?owner->GetActorValue(RE::ActorValue::kHealth):0;
 originalCommit(target,modifier,av,delta,attacker);
 if(owner&&own)logger::info("ARCANA_HEALTH_COMMIT mode={} target={:08X} requested={} before={} after={} [synchronous missile impact context]",currentDamageMode,target->GetFormID(),delta,before,owner->GetActorValue(RE::ActorValue::kHealth));
 else if(owner)logger::info("OUTGOING_HEALTH_COMMIT target={:08X} requested={} before={} after={} [all player damage, not solely Arcana]",target->GetFormID(),delta,before,owner->GetActorValue(RE::ActorValue::kHealth));
}
bool InstallDomeCommitHook(){
 if(domeCommitReady)return true;
 REL::Relocation<std::uintptr_t> entry{REL::ID(37523)};
 const auto address=entry.address();auto* code=reinterpret_cast<const std::uint8_t*>(address);
 constexpr std::array<std::uint8_t,5> prologue{{0x48,0x89,0x5C,0x24,0x10}};
 const bool pristine=std::memcmp(code,prologue.data(),5)==0;const bool chained=code[0]==0xE9;
 if(!pristine&&!chained){logger::error("DOME_PRECOMMIT_DISABLED: unrecognized SE37523 entry; no unsafe patch");return false;}
 domeTrampoline.create(128);
 if(pristine){
  auto* stub=static_cast<std::uint8_t*>(domeTrampoline.allocate(19));std::memcpy(stub,code,5);
  constexpr std::array<std::uint8_t,6> jump{{0xFF,0x25,0,0,0,0}};std::memcpy(stub+5,jump.data(),6);
  const auto resume=address+5;std::memcpy(stub+11,&resume,8);FlushInstructionCache(GetCurrentProcess(),stub,19);originalCommit=reinterpret_cast<CommitFn>(stub);
 }else{std::int32_t displacement{};std::memcpy(&displacement,code+1,4);originalCommit=reinterpret_cast<CommitFn>(address+5+displacement);}
 static_cast<void>(domeTrampoline.write_branch<5>(address,CommitActorValue));
 domeCommitReady=true;logger::info("DOME_PRECOMMIT_READY SE37523 chained={}",chained);return true;
}
#include "Sentry.inl"
#include "WorldUpdate.inl"
#include "Tracking.inl"
#include "Rain.inl"
void ResetFeatures(){std::scoped_lock lock(guardMutex);domeWards={};rainCooldown=0;slowRegistry={};}
