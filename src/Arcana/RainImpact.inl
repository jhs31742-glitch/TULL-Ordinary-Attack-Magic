// SE1.5.97 BeamProjectile::AddImpact slot BD, chained using the pinned declaration.
struct RainContext {bool active=false;int side=0;float phase=0,power=0;RE::NiPoint3 direction{};unsigned generation=0;bool finale=false;};
thread_local RainContext currentRainContext{};
struct RainDamageScope {bool active=false;int side=0;unsigned generation=0;float nominal=0;};
thread_local RainDamageScope rainDamage{};
struct RainBudgetEntry {std::uint32_t target=0;float base=0;};
struct RainBudget {unsigned generation=0;std::array<RainBudgetEntry,64> targets{};};
std::array<RainBudget,2> rainBudgets{};
float LimitRainBase(RE::Actor* target,float delta){
 if(!rainDamage.active||rainDamage.side<0||rainDamage.side>=2||rainDamage.nominal<=0||!target)return delta;
 auto& budget=rainBudgets[rainDamage.side];
 if(budget.generation!=rainDamage.generation){budget=RainBudget{};budget.generation=rainDamage.generation;}
 RainBudgetEntry* entry=nullptr;
 for(auto& e:budget.targets)if(e.target==target->GetFormID()){entry=&e;break;}
 if(!entry)for(auto& e:budget.targets)if(!e.target){entry=&e;e.target=target->GetFormID();break;}
 if(!entry)return 0; // Bounded ledger fails closed rather than losing the cap.
 const float allow=std::min(rainDamage.nominal,std::max(0.f,rainTargetBaseCap-entry->base));
 entry->base+=allow;return delta*(allow/rainDamage.nominal);
}
thread_local int currentDamageMode=-1;std::array<std::atomic_uint,8> ownHealthAudits{};
using MissileProcessFn=bool(*)(RE::MissileProjectile*);MissileProcessFn originalMissileProcess=nullptr;
bool MissileProcessImpacts(RE::MissileProjectile* p);
struct RainTag {std::uintptr_t object=0;std::uint32_t id=0;RainContext context{};double stamp=0;bool queued=false,finalQueued=false;};
struct GroundEvent {RE::NiPoint3 position{};RainContext context{};float normalZ=0,error=100;};
std::array<RainTag,256> rainTags{};unsigned rainTagCursor=0;std::array<GroundEvent,128> groundEvents{};unsigned groundEventCount=0;std::mutex impactMutex;
struct FinaleContact {bool valid=false;unsigned generation=0;RE::NiPoint3 position{};};
std::array<FinaleContact,2> rainFinalContacts{};
bool ConsumeRainFinalContact(int side,unsigned generation,RE::NiPoint3& position){
 std::scoped_lock lock(impactMutex);auto& contact=rainFinalContacts[side];
 if(!contact.valid||contact.generation!=generation)return false;position=contact.position;contact=FinaleContact{};return true;
}
using RainImpactFn=void(*)(RE::BeamProjectile*,RE::TESObjectREFR*,const RE::NiPoint3&,const RE::NiPoint3&,RE::hkpCollidable*,std::int32_t,std::uint32_t);
RainImpactFn originalRainImpact=nullptr;
using MissileImpactFn=void(*)(RE::MissileProjectile*,RE::TESObjectREFR*,const RE::NiPoint3&,const RE::NiPoint3&,RE::hkpCollidable*,std::int32_t,std::uint32_t);
MissileImpactFn originalMissileImpact=nullptr;
RainTag* FindRainTag(RE::Projectile* p){for(auto& tag:rainTags)if(tag.object==reinterpret_cast<std::uintptr_t>(p)&&tag.id==p->GetFormID()&&Clock()-tag.stamp<4)return &tag;return nullptr;}
bool MissileProcessImpacts(RE::MissileProjectile* p){
 if(!gameReady||!forms.ready)return originalMissileProcess(p);
 const int previous=currentDamageMode;const auto priorRain=rainDamage;currentDamageMode=-1;rainDamage={};
 auto* spell=p->GetProjectileRuntimeData().spell;for(int i=0;i<8;i++)if(spell==forms.payload[i])currentDamageMode=i;
 if(currentDamageMode==Rain){std::scoped_lock lock(impactMutex);if(auto* tag=FindRainTag(p))rainDamage={true,tag->context.side,tag->context.generation,modes[Rain].damage};}
 const bool result=originalMissileProcess(p);currentDamageMode=previous;rainDamage=priorRain;return result;
}
void RegisterRainProjectile(RE::Projectile* p,RainContext context){if(!p||!context.active)return;std::scoped_lock lock(impactMutex);if(FindRainTag(p))return;rainTags[rainTagCursor++%rainTags.size()]={reinterpret_cast<std::uintptr_t>(p),p->GetFormID(),context,Clock(),false};}
bool ActualGroundAt(RE::Projectile* projectile,const RE::NiPoint3& position,const RainContext& context,const RE::hkpCollidable* directBody,float& normalZ,float& error){
 if(!projectile||!directBody)return false;auto* cell=projectile->GetParentCell();auto* bhk=cell?cell->GetbhkWorld():nullptr;auto* world=bhk?bhk->GetWorld1():nullptr;
 const float scale=RE::bhkWorld::GetWorldScale();if(!world||!std::isfinite(scale)||scale<=0)return false;
 const auto& direction=context.direction;float len=std::sqrt(direction.x*direction.x+direction.y*direction.y+direction.z*direction.z);if(!std::isfinite(len)||len<.001f)return false;auto unit=direction/len;
 auto a=position-unit*16,z=position+unit*16;RE::hkpWorldRayCastInput input{};input.from=RE::hkVector4{a.x*scale,a.y*scale,a.z*scale,0};input.to=RE::hkVector4{z.x*scale,z.y*scale,z.z*scale,0};
 const auto* base=projectile->GetProjectileBase();if(!base)return false;auto layer=base->data.collisionLayer?base->data.collisionLayer->collisionIdx:static_cast<std::uint32_t>(RE::COL_LAYER::kSpell);if(layer>0x7F)return false;
 input.filterInfo.filter=(static_cast<std::uint32_t>(projectile->GetCollisionGroup())<<16)|layer;
 RE::hkpWorldRayCastOutput output{};output.Reset();world->CastRay(input,output);
 if(!output.HasHit()||output.rootCollidable!=directBody||!std::isfinite(output.hitFraction)||output.hitFraction<0||output.hitFraction>1)return false;
 float normal[4];_mm_storeu_ps(normal,output.normal.quad);normalZ=normal[2];auto delta=a+(z-a)*output.hitFraction-position;error=std::sqrt(delta.x*delta.x+delta.y*delta.y+delta.z*delta.z);return true;
}
std::atomic_uint impactAuditCount{0};
void ObserveArcanaImpact(RE::Projectile* projectile,RE::TESObjectREFR* target,const RE::NiPoint3& position,const RE::NiPoint3& velocity,RE::hkpCollidable* body,std::int32_t arg6,std::uint32_t arg7){
 if(!projectile||!gameReady||!forms.ready)return;
 auto* spell=projectile->GetProjectileRuntimeData().spell;int mode=-1;for(int i=0;i<8;i++)if(spell==forms.payload[i])mode=i;
 if(mode>=0&&impactAuditCount.fetch_add(1)<240)logger::info("ARCANA_PROJECTILE_IMPACT mode={} target={:08X} actor={} nominalDamage={} power={}",mode,target?target->GetFormID():0,target&&target->IsActor(),modes[mode].damage,projectile->GetProjectileRuntimeData().power);
 if(mode!=Rain||!body||!std::isfinite(position.x)||!std::isfinite(position.y)||!std::isfinite(position.z))return;
 auto shooter=projectile->GetProjectileRuntimeData().shooter.get();if(!shooter||!shooter->IsPlayerRef())return;
 std::scoped_lock lock(impactMutex);auto* tag=FindRainTag(projectile);
 if(!tag&&currentRainContext.active){auto& slot=rainTags[rainTagCursor++%rainTags.size()];slot={reinterpret_cast<std::uintptr_t>(projectile),projectile->GetFormID(),currentRainContext,Clock(),false,false};tag=&slot;}
 if(!tag)return;
 if(tag->context.finale&&!tag->finalQueued&&tag->context.side>=0&&tag->context.side<2){tag->finalQueued=true;rainFinalContacts[tag->context.side]={true,tag->context.generation,position};}
 if((target&&target->IsActor())||tag->queued||tag->context.phase<rainImpactStart)return;
 tag->queued=true;float normal=0,error=100;bool ground=ActualGroundAt(projectile,position,tag->context,body,normal,error);
 if(GroundImpactEligible(tag->context.phase,false,ground,normal,error)&&groundEventCount<groundEvents.size())groundEvents[groundEventCount++]={position,tag->context,normal,error};
}
void RainAddImpact(RE::BeamProjectile* p,RE::TESObjectREFR* t,const RE::NiPoint3& at,const RE::NiPoint3& v,RE::hkpCollidable* b,std::int32_t a6,std::uint32_t a7){ObserveArcanaImpact(p,t,at,v,b,a6,a7);originalRainImpact(p,t,at,v,b,a6,a7);}
void MissileAddImpact(RE::MissileProjectile* p,RE::TESObjectREFR* t,const RE::NiPoint3& at,const RE::NiPoint3& v,RE::hkpCollidable* b,std::int32_t a6,std::uint32_t a7){ObserveArcanaImpact(p,t,at,v,b,a6,a7);originalMissileImpact(p,t,at,v,b,a6,a7);}

void UpdateRainImpacts(RE::Actor* actor){
 if(!gameReady||!forms.ready||!actor||actor->IsDead()||!actor->Is3DLoaded())return;if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused())return;
 std::array<GroundEvent,128> pending{};unsigned count=0;{std::scoped_lock lock(impactMutex);count=groundEventCount;std::copy_n(groundEvents.begin(),count,pending.begin());groundEventCount=0;}
 float energy=0;unsigned accepted=0;
 for(unsigned i=0;i<count;i++){const auto& event=pending[i];if(GroundImpactEligible(event.context.phase,false,true,event.normalZ,event.error)){float impulse=1.10f*event.context.power;energy+=impulse*impulse;accepted++;}}
 if(accepted){Shake(actor,GroundShakePower(energy),std::min(.40f,.18f+.02f*accepted));logger::info("RAIN_GROUND_IMPACT count={} strength={:.3f} actualAddImpact=true",accepted,GroundShakePower(energy));}
}
void ClearRainImpactHistory(){std::scoped_lock lock(impactMutex);rainTags={};rainFinalContacts={};rainBudgets={};rainDamage={};groundEventCount=0;currentRainContext={};impactAuditCount=0;currentDamageMode=-1;for(auto& count:ownHealthAudits)count=0;}
void InstallRainImpactHook(){REL::Relocation<std::uintptr_t> table{RE::BeamProjectile::VTABLE[0]};originalRainImpact=reinterpret_cast<RainImpactFn>(table.write_vfunc(0xBD,RainAddImpact));REL::Relocation<std::uintptr_t> missiles{RE::MissileProjectile::VTABLE[0]};originalMissileImpact=reinterpret_cast<MissileImpactFn>(missiles.write_vfunc(0xBD,MissileAddImpact));originalMissileProcess=reinterpret_cast<MissileProcessFn>(missiles.write_vfunc(0xAC,MissileProcessImpacts));logger::info("RAIN_IMPACT_READY BeamProjectile and MissileProjectile slot=BD; trajectory ray and exact collided-root validation; queued shake on caster update");}
