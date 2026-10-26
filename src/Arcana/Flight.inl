// Arcana-owned velocity controller. No Flight Framework code or binary is linked.
// Pinned SDK: bhkCharacterController vfunc07, context.currentState, fallTime/fallStartHeight.
FlightSettings flightSettings{};FlightCombatSettings flightCombatSettings{};FlightPhase flightPhase=FlightPhase::Off;
RE::NiPoint3 flightVelocity{},flightLastPosition{};RE::TESObjectCELL* flightCell=nullptr;RE::TESWorldSpace* flightWorld=nullptr;
std::atomic_bool flightToggleRequested{false};float flightAge=0;
struct FlightSigil {ArtSlot art{};RE::NiPoint3 position{};float age=0,wait=0;bool active=false,cancelled=false;};
std::array<FlightSigil,4> flightSigils{};unsigned flightSigilCursor=0;bool flightUp=false,flightDown=false,flightBoost=false,flightRecoverPending=false,flightReadyPending=false;
bool FlightActive(){return flightPhase==FlightPhase::Flying;}
bool FlightOwnsMovement(){return flightPhase!=FlightPhase::Off;}
bool FlightGameplay(){auto* ui=RE::UI::GetSingleton();auto* controls=RE::PlayerControls::GetSingleton();return gameReady&&forms.ready&&(!ui||!ui->GameIsPaused())&&controls&&!controls->blockPlayerInput&&!controls->data.remapMode;}
void FlightFallReset(RE::Actor* actor){if(auto* c=actor?actor->GetCharController():nullptr){c->fallTime=0;c->fallStartHeight=actor->GetPosition().z;}}
void FlightMarker(RE::Actor* actor,bool on){if(!actor||!forms.flightMarker)return;if(on){if(!actor->HasSpell(forms.flightMarker))actor->AddSpell(forms.flightMarker);}else if(actor->HasSpell(forms.flightMarker))actor->RemoveSpell(forms.flightMarker);}
void FlightClearSigils(){for(auto& s:flightSigils){FinishCircle(s.art);s=FlightSigil{};}}
void FlightEnsureSigil(FlightSigil& s,RE::BGSArtObject* form,RE::PlayerCharacter* p){
 if(!s.art.requested){s.art.form=form;s.art.requested=RequestCircleArt(p,form,p->Get3D(false),1.5f);}
 if(s.art.requested&&!s.art.effect)s.art.effect=FindCircle(p,form);
}
void FlightShowSigil(RE::PlayerCharacter* p){
 int index=flightSigilCursor++%4;auto& s=flightSigils[index];FinishCircle(s.art);s=FlightSigil{};s.active=true;s.position=p->GetPosition()+RE::NiPoint3{0,0,3};
 FlightEnsureSigil(s,forms.flightCircle[index],p);
}
void FlightUpdateSigils(RE::PlayerCharacter* p,float dt){
 if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused())return;if(!std::isfinite(dt)||dt<=0)return;dt=std::min(dt,.1f);
 for(auto& s:flightSigils){if(!s.active)continue;s.wait+=dt;FlightEnsureSigil(s,s.art.form,p);
  if(!CircleReady(s.art)){if(s.wait>1.2f){FinishCircle(s.art);s=FlightSigil{};}continue;}
  s.age+=dt;if(s.age>=flightSigilSeconds){FinishCircle(s.art);s=FlightSigil{};continue;}
  auto face=DirectionRotation(RE::NiPoint3{0,0,1});RE::NiMatrix3 spin;spin.MakeYRotation(s.age*.7f);
  RefreshCircleParent(s.art);PoseCircle(s.art,s.position,face*spin,.34f*Smooth(s.age/.16f),FlightSigilAlpha(s.age));
 }
}
// CP024: independent halo-band trail during held ascent. Snapshot world positions;
// no actor scan, new frame hook, damage, charge, or mutation of Temperance's halo.
std::array<FlightSigil,4> ascentBands{};float ascentInterval=0;bool ascentWasHeld=false;
void FlightClearAscent(){
 for(auto& s:ascentBands){FinishCircle(s.art);s=FlightSigil{};}
 ascentInterval=0;ascentWasHeld=false;
}
void FlightAscentFrame(RE::PlayerCharacter* p,float dt,bool ascending){
 if(!std::isfinite(dt)||dt<=0)return;dt=std::min(dt,.1f);
 // Release stops emission; already-visible world snapshots finish their own fade.
 // Menu/off/cell/load/death still use FlightClearAscent through FlightReset.
 if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused()){FlightClearAscent();return;}
 if(!ascending)ascentInterval=0;else ascentInterval-=dt;
 if(ascending&&(!ascentWasHeld||ascentInterval<=0)){
  ascentInterval=.26f;
  for(std::size_t i=0;i<ascentBands.size();++i){
   auto& s=ascentBands[i];if(s.active)continue;
   auto old=circleRequests.find(forms.flightAscent[i]);
   if(old!=circleRequests.end()&&!old->second.acquired&&Clock()<old->second.reuseAfter)continue;
   s=FlightSigil{};s.active=true;s.position=p->GetPosition()+RE::NiPoint3{0,0,-4};
   FlightEnsureSigil(s,forms.flightAscent[i],p);break;
  }
 }
 ascentWasHeld=ascending;
 for(auto& s:ascentBands){
  if(!s.active)continue;s.wait+=dt;FlightEnsureSigil(s,s.art.form,p);
  if(!ascending&&s.age==0)s.cancelled=true;
  if(s.cancelled){
   // Keep the request quarantined until its late return can be finished.
   if(s.art.effect||s.wait>1.2f){FinishCircle(s.art);s=FlightSigil{};}continue;
  }
  if(!CircleReady(s.art)){
   if(s.wait>1.2f){FinishCircle(s.art);s=FlightSigil{};}continue;
  }
  s.age+=dt;
  if(s.age>=.68f){FinishCircle(s.art);s=FlightSigil{};continue;}
  const float u=std::clamp(s.age/.68f,0.f,1.f);
  auto position=s.position;position.z-=22.f*u;
  RE::NiMatrix3 spin;spin.MakeYRotation(.6f*s.age);
  RefreshCircleParent(s.art);
  PoseCircle(s.art,position,DirectionRotation(RE::NiPoint3{0,0,1})*spin,
    (.15f+.16f*Smooth(u))*Smooth(s.age/.09f),1.f-Smooth(u));
 }
}
#include "GunFlight.inl"
RE::NiPointer<RE::bhkCharacterController> flightGravityOwner{};
float flightSavedGravity=0,flightHeightTarget=0;bool flightHeightValid=false;
RE::BSSoundHandle flightHum{};RE::BGSSoundDescriptorForm* flightHumForm=nullptr;
bool flightHumEnabled=true;float flightHumVolume=.35f;bool flightHumRequested=false;
void FlightRestoreGravity(){
 if(flightGravityOwner&&flightGravityOwner->gravity==0.f)flightGravityOwner->gravity=flightSavedGravity;
 flightGravityOwner.reset();flightHeightValid=false;
}
void FlightGuardGravity(RE::PlayerCharacter* p){
 if(!p||!FlightActive())return;auto* c=p->GetCharController();if(!c)return;
 if(c!=flightGravityOwner.get()){FlightRestoreGravity();flightGravityOwner=RE::NiPointer<RE::bhkCharacterController>(c);flightSavedGravity=c->gravity;flightHeightTarget=p->GetPosition().z;flightHeightValid=true;}
 c->gravity=0.f;
}
void FlightStopHum(){
 if(flightHum.IsValid())FadeSustain(flightHum);flightHum=RE::BSSoundHandle{};flightHumRequested=false;
}
void FlightUpdateHum(RE::PlayerCharacter* p){
 if(!FlightActive()||!flightHumEnabled||flightHumVolume<=0||!flightHumForm)return;auto* root=p?p->Get3D(false):nullptr;if(!root)return;
 if(!flightHumRequested){Sound(flightHum,flightHumForm,root,0.f);flightHumRequested=flightHum.IsValid();logger::info("FLIGHT_HUM_REQUEST valid={} [audibility unverified]",flightHumRequested);}
 if(flightHum.IsValid()){flightHum.SetObjectToFollow(root);static_cast<void>(flightHum.SetVolume(flightHumVolume*std::clamp(flightAge/.30f,0.f,1.f)));}
}

void FlightReset(bool clearMarker=true,bool clearSigils=true){
 auto* actor=RE::PlayerCharacter::GetSingleton();FlightRestoreGravity();FlightStopHum();FlightGunClear(actor);if(FlightOwnsMovement())FlightFallReset(actor);
 if(clearMarker)FlightMarker(actor,false);if(clearSigils)FlightClearSigils();FlightClearAscent();
 flightPhase=FlightPhase::Off;flightVelocity={};flightAge=0;flightCell=nullptr;flightWorld=nullptr;flightToggleRequested=false;flightUp=flightDown=flightBoost=false;
}
float FlightIniNumber(const wchar_t* key,float fallback,float lo,float hi){
 wchar_t text[96]{};GetPrivateProfileStringW(L"Flight",key,L"",text,96,L".\\Data\\SKSE\\Plugins\\TOAM.ini");if(!*text)return fallback;
 try{std::size_t end=0;float value=std::stof(text,&end);if(text[end]||!std::isfinite(value))return fallback;return std::clamp(value,lo,hi);}catch(...){return fallback;}
}
void FlightLoadSettings(){
 flightSettings.enabled=FlightIniNumber(L"Enabled",1,0,1)>.5f;
 flightGunAnimations=FlightIniNumber(L"GunAnimations",1,0,1)>.5f;flightHumEnabled=FlightIniNumber(L"HumEnabled",1,0,1)>.5f;flightHumVolume=FlightIniNumber(L"HumVolume",.35f,0,1);
 flightCombatSettings.rangedLegPose=FlightIniNumber(L"RangedLegPose",1,0,1)>.5f;
 flightCombatSettings.blockMelee=FlightIniNumber(L"BlockMeleeWhileFlying",1,0,1)>.5f;
 flightSettings.speed=FlightIniNumber(L"Speed",300,60,900);flightSettings.boost=FlightIniNumber(L"BoostSpeed",650,60,1200);
 flightSettings.vertical=FlightIniNumber(L"VerticalSpeed",220,40,600);flightSettings.takeoffSpeed=FlightIniNumber(L"TakeoffSpeed",160,60,320);flightSettings.takeoffSeconds=FlightIniNumber(L"TakeoffSeconds",.55f,.2f,1.2f);
 flightSettings.response=FlightIniNumber(L"Response",7,2,16);
 logger::info("FLIGHT_SETTINGS enabled={} speed={} boost={} vertical={} takeoff={} [game units/sec]",flightSettings.enabled,flightSettings.speed,flightSettings.boost,flightSettings.vertical,flightSettings.takeoffSpeed);
}
bool FlightEligible(RE::PlayerCharacter* p){
 // Multi-target SDK static ActorState=+0xA0, but SE1.5.97 needs +0xB8.
 auto* state=p?p->AsActorState():nullptr;
 return p&&state&&p->Is3DLoaded()&&!p->IsDead()&&!state->IsSwimming()&&!p->IsOnMount()&&!p->IsInKillMove()&&state->GetKnockState()==RE::KNOCK_STATE_ENUM::kNormal&&state->GetSitSleepState()==RE::SIT_SLEEP_STATE::kNormal&&p->GetCharController();
}
RE::NiPoint3 FlightCastOffset(const RE::NiPoint3& feet,const RE::NiMatrix3& flat,const RE::NiMatrix3& aim,const RE::NiPoint3& offset){
 if(!FlightActive())return feet+flat*offset;
 auto local=AirCircleOffset(offset.x,offset.y,offset.z);return feet+RE::NiPoint3{0,0,98}+aim*RE::NiPoint3{local.x,local.y,local.z};
}
RE::NiPoint3 FlightFieldPoint(RE::Actor* actor,const RE::NiPoint3& feet,const RE::NiMatrix3& flat){
 if(!FlightActive())return feet+flat*RE::NiPoint3{0,rainForward,0};
 auto origin=feet+RE::NiPoint3{0,0,98};auto direction=AimRotation(actor->GetAimAngle(),actor->GetAimHeading())*RE::NiPoint3{0,1,0};bool hit=false;
 float length=BeamLength(actor,origin,direction,true,4096,&hit);auto target=origin+direction*length;
 if(!hit){target=origin+direction*rainForward;bool ground=false;float down=BeamLength(actor,target,RE::NiPoint3{0,0,-1},true,8192,&ground);if(ground)target.z-=down;else target.z=feet.z;}
 return target;
}
#include "FlightCombat.inl"
void FlightScheduleReady(){flightReadyPending=true;flightRecoverPending=true;}
void FlightFrame(RE::PlayerCharacter* p,float dt){
 if(!p||!gameReady||!forms.ready)return;
 if(flightReadyPending&&p->Is3DLoaded()){
  flightReadyPending=false; // Learned from its tome; retain load recovery.
  if(flightRecoverPending){flightRecoverPending=false;bool saved=forms.flightMarker&&p->HasSpell(forms.flightMarker);FlightReset();
   if(saved)logger::info("FLIGHT_LOAD_RESET normalState=true");
  }
 }
 if(!p->Is3DLoaded()||p->IsDead()){FlightReset();return;}FlightUpdateSigils(p,dt);
 const bool gameplay=FlightGameplay();
 if(!gameplay){FlightClearAscent();flightUp=flightDown=flightBoost=false;flightToggleRequested=false;if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused())return;}
 if(!FlightEligible(p)||!flightSettings.enabled){FlightReset();return;}
 if(gameplay&&flightToggleRequested.exchange(false)){
  if(flightPhase==FlightPhase::Flying){FlightReset(true,false);logger::info("FLIGHT_TOGGLE_OFF immediateNormal=true noTransition=true");}
  else{flightPhase=FlightPhase::Flying;flightAge=0;flightVelocity={0,0,flightSettings.takeoffSpeed};flightCell=p->GetParentCell();flightWorld=p->GetWorldspace();flightLastPosition=p->GetPosition();FlightMarker(p,true);FlightShowSigil(p);if(p->IsInJumpState()){bool accepted=p->NotifyAnimationGraph("JumpLandEnd");p->SetGraphVariableBool("bInJumpState",false);logger::info("FLIGHT_AIR_TRANSITION jumpLandEndAccepted={} [not animation visibility proof]",accepted);}
   logger::info("FLIGHT_TOGGLE_ON animationMarker={:08X}",forms.flightMarker->GetFormID());}
 }
 if(!FlightOwnsMovement())return;if(!std::isfinite(dt)||dt<=0)return;dt=std::min(dt,.05f);
 auto position=p->GetPosition();auto* cell=p->GetParentCell();bool cellTransition=cell!=flightCell&&(!cell||!flightCell||cell->IsInteriorCell()||flightCell->IsInteriorCell()||p->GetWorldspace()!=flightWorld);
 if(cellTransition||Length(position-flightLastPosition)>2048){FlightReset();return;}flightLastPosition=position;
 FlightAscentFrame(p,dt,gameplay&&FlightActive()&&flightUp&&!flightDown);
 FlightGuardGravity(p);flightAge+=dt;FlightUpdateHum(p);auto* controls=RE::PlayerControls::GetSingleton();auto input=gameplay?controls->data.moveInputVec:RE::NiPoint2{};
 auto desired=FlightDesired(input.x,input.y,float(flightUp)-float(flightDown),p->GetAimHeading(),flightBoost,flightPhase,flightSettings,p->GetAimAngle());
 desired.z=FlightTakeoffVertical(desired.z,flightAge,flightDown,flightSettings);
 flightVelocity.x=FlightApproach(flightVelocity.x,desired.x,dt,flightSettings.response);flightVelocity.y=FlightApproach(flightVelocity.y,desired.y,dt,flightSettings.response);flightVelocity.z=FlightApproach(flightVelocity.z,desired.z,dt,flightSettings.response);
 if(!flightHeightValid){flightHeightTarget=position.z;flightHeightValid=true;}
 flightHeightTarget=std::clamp(flightHeightTarget+flightVelocity.z*dt,position.z-48.f,position.z+48.f);
 auto velocity=flightVelocity;velocity.z+=FlightHeightCorrection(flightHeightTarget-position.z);float speed=Length(velocity);
 if(speed>.01f){bool wall=false;RE::NiPoint3 normal{};auto direction=velocity/speed;float ray=BeamLength(p,position+RE::NiPoint3{0,0,64},direction,true,speed*dt+32,&wall,&normal);
  if(wall&&ray<speed*dt+30){flightHeightTarget=position.z;float into=velocity.x*normal.x+velocity.y*normal.y+velocity.z*normal.z;if(into<0)velocity-=normal*into;}
 }
 auto* controller=p->GetCharController();float hs=RE::bhkWorld::GetWorldScale();if(!controller||!std::isfinite(hs)||hs<=0){FlightReset();return;}
 controller->context.currentState=RE::hkpCharacterStateTypes::kInAir;FlightFallReset(p);
 controller->SetLinearVelocityImpl(RE::hkVector4{velocity.x*hs,velocity.y*hs,velocity.z*hs,0});
}
struct FlightSpellSink final:RE::BSTEventSink<RE::TESSpellCastEvent>{
 RE::BSEventNotifyControl ProcessEvent(const RE::TESSpellCastEvent* event,RE::BSTEventSource<RE::TESSpellCastEvent>*)override{
  if(event&&event->object&&event->object->IsPlayerRef()&&forms.flightPower&&event->spell==forms.flightPower->GetFormID()&&FlightGameplay())flightToggleRequested=true;
  return RE::BSEventNotifyControl::kContinue;
 }
} flightSpellSink;
struct FlightInputSink final:RE::BSTEventSink<RE::InputEvent*>{
 RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* events,RE::BSTEventSource<RE::InputEvent*>*)override{
  if(!events)return RE::BSEventNotifyControl::kContinue;auto* names=RE::UserEvents::GetSingleton();if(!names)return RE::BSEventNotifyControl::kContinue;
  for(auto* e=*events;e;e=e->next)if(auto* b=e->AsButtonEvent()){
   const auto& name=b->QUserEvent();const bool held=b->IsPressed()&&FlightGameplay();
   if(name==names->jump)flightUp=held;else if(name==names->sneak)flightDown=held;else if(name==names->sprint)flightBoost=held;
  }return RE::BSEventNotifyControl::kContinue;
 }
} flightInputSink;
REL::Relocation<bool(*)(RE::JumpHandler*,RE::InputEvent*)> flightOriginalJump;
REL::Relocation<bool(*)(RE::SneakHandler*,RE::InputEvent*)> flightOriginalSneak;
bool FlightJumpCanProcess(RE::JumpHandler* self,RE::InputEvent* event){return FlightOwnsMovement()&&FlightGameplay()?false:flightOriginalJump(self,event);}
bool FlightSneakCanProcess(RE::SneakHandler* self,RE::InputEvent* event){return FlightOwnsMovement()&&FlightGameplay()?false:flightOriginalSneak(self,event);}
void InstallFlightInputHooks(){
 InstallFlightCombatInputHooks();
 REL::Relocation<std::uintptr_t> jump{RE::VTABLE_JumpHandler[0]},sneak{RE::VTABLE_SneakHandler[0]};
 flightOriginalJump=jump.write_vfunc(0x01,FlightJumpCanProcess);flightOriginalSneak=sneak.write_vfunc(0x01,FlightSneakCanProcess);
}
void FlightDataLoaded(){
 FlightLoadSettings();FlightGunLoad();flightHumForm=RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSSoundDescriptorForm>(ResolveSourceForm(0xC64),plugin);if(auto* source=RE::ScriptEventSourceHolder::GetSingleton())source->AddEventSink<RE::TESSpellCastEvent>(&flightSpellSink);
 if(auto* input=RE::BSInputDeviceManager::GetSingleton())input->AddEventSink(&flightInputSink);
}
