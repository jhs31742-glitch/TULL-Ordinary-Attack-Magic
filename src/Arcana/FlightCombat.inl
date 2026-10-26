// Scoped player-only post-pose correction and attack admission.
// All six local rotations are borrowed for one frame and restored before the next original update.
struct FlightBorrowedLeg {RE::NiPointer<RE::NiAVObject> node{};RE::NiMatrix3 before{},applied{};};
RE::NiPointer<RE::NiAVObject> flightLegRoot{};
std::array<FlightBorrowedLeg,6> flightBorrowedLegs{};
float flightLegBlend=0;bool flightLegPending=false;
std::array<bool,3> flightAttackLatch{};unsigned flightMeleeLogs=0,flightLegLogs=0;
bool FlightSameRotation(const RE::NiMatrix3& a,const RE::NiMatrix3& b){
 for(int i=0;i<3;++i)for(int j=0;j<3;++j)if(std::abs(a.entry[i][j]-b.entry[i][j])>.00001f)return false;return true;
}
void FlightRefreshLegTrees(){
 RE::NiUpdateData u{};u.flags.set(RE::NiUpdateData::Flag::kDirty);
 if(flightBorrowedLegs[0].node)flightBorrowedLegs[0].node->Update(u);
 if(flightBorrowedLegs[3].node)flightBorrowedLegs[3].node->Update(u);
}
void FlightRestoreRangedLegs(RE::PlayerCharacter* p){
 auto* current=p?p->Get3D(false):nullptr;bool changed=false;
 if(flightLegPending&&current==flightLegRoot.get()){
  for(std::size_t i=0;i<flightBorrowedLegs.size();++i){
   auto& b=flightBorrowedLegs[i];
   if(b.node&&current->GetObjectByName(RE::BSFixedString(flightLegCurves[i].bone))==b.node.get()&&FlightSameRotation(b.node->local.rotate,b.applied)){
    b.node->local.rotate=b.before;changed=true;
   }
  }
  if(changed)FlightRefreshLegTrees();
 }
 flightLegPending=false;for(auto& b:flightBorrowedLegs)b={};flightLegRoot.reset();
}
FlightHandKind FlightEquippedKind(RE::PlayerCharacter* p,bool left){
 auto* f=p?p->GetEquippedObject(left):nullptr;if(!f)return FlightHandKind::Bare;
 if(auto* w=f->As<RE::TESObjectWEAP>()){
  if(w->IsBow()||w->IsCrossbow())return FlightHandKind::Ranged;
  if(w->IsStaff())return FlightHandKind::Staff;if(w->IsMelee())return FlightHandKind::Melee;
 }
 if(f->As<RE::SpellItem>())return FlightHandKind::Spell;return FlightHandKind::Other;
}
// Bounded changes in the existing log, not automatic runtime acceptance.
void FlightPoseStatus(RE::PlayerCharacter* p,const char* stage,const char* missing=""){
 static const char* previous=nullptr;static unsigned logs=0;
 if(previous==stage)return;previous=stage;if(logs++>=32)return;
 auto* state=p?p->AsActorState():nullptr;
 auto* left=p?p->GetEquippedObject(true):nullptr;auto* right=p?p->GetEquippedObject(false):nullptr;
 logger::info("FLIGHT_RANGED_POSE_STAGE stage={} weaponState={} stateOffset={} left={:08X} right={:08X} gunProfile={} missing={} [execution only; pixels unverified]",
 stage,state?int(state->GetWeaponState()):-1,state?std::uintptr_t(state)-std::uintptr_t(p):0,
 left?left->GetFormID():0,right?right->GetFormID():0,flightGunAuthoredActive,missing);
}
void FlightUpdateRangedLegs(RE::PlayerCharacter* p,float dt){
 const auto left=FlightEquippedKind(p,true),right=FlightEquippedKind(p,false);
 if(!FlightActive()){flightLegBlend=0;return;}
 if(!flightCombatSettings.rangedLegPose){flightLegBlend=0;FlightPoseStatus(p,"disabled");return;}
 if(!FlightEligible(p)){flightLegBlend=0;FlightPoseStatus(p,"ineligible");return;}
 auto* state=p->AsActorState();
 if(!state->IsWeaponDrawn()){flightLegBlend=0;FlightPoseStatus(p,"sheathed");return;}
 if(left!=FlightHandKind::Ranged&&right!=FlightHandKind::Ranged){flightLegBlend=0;FlightPoseStatus(p,"no_ranged_weapon");return;}
 auto* root=p->Get3D(false);if(!root){flightLegBlend=0;FlightPoseStatus(p,"no_third_person_root");return;}
 std::array<RE::NiAVObject*,6> nodes{};
 for(std::size_t i=0;i<nodes.size();++i){
  nodes[i]=root->GetObjectByName(RE::BSFixedString(flightLegCurves[i].bone));
  if(!nodes[i]||!nodes[i]->parent){flightLegBlend=0;FlightPoseStatus(p,"missing_leg_node",flightLegCurves[i].bone);return;}
 }
 if(std::isfinite(dt)&&dt>0)flightLegBlend=std::min(1.f,flightLegBlend+std::min(dt,.05f)/.20f);
 if(flightLegBlend<=0)return;
 flightLegRoot=RE::NiPointer<RE::NiAVObject>(root);
 const float phase=flightGunAuthoredActive?.60f:std::fmod(flightAge+.60f,flightLegSeconds);
 for(std::size_t i=0;i<nodes.size();++i){
  auto* node=nodes[i];auto& b=flightBorrowedLegs[i];b.node=RE::NiPointer<RE::NiAVObject>(node);b.before=node->local.rotate;
  RE::NiQuaternion from(b.before);const auto& c=flightLegCurves[i];auto target=FlightSampleLeg(c.keys,c.count,phase);
  auto q=FlightNlerp({from.w,from.x,from.y,from.z},target,flightLegBlend);
  RE::NiQuaternion(q.w,q.x,q.y,q.z).ToRotation(b.applied);node->local.rotate=b.applied;
 }
 flightLegPending=true;FlightRefreshLegTrees();FlightPoseStatus(p,"six_leg_rotations_written");
 if(flightLegLogs++==0)logger::info("FLIGHT_RANGED_LEGS_WRITTEN joints=6 upperBody=false rootPelvis=false [pose write only; visible result unverified]");
}
FlightAttackSide FlightButtonSide(const RE::ButtonEvent* b){
 auto* e=RE::UserEvents::GetSingleton();if(!e||!b)return FlightAttackSide::None;
 const auto& s=b->QUserEvent();if(s==e->leftAttack)return FlightAttackSide::Left;
 if(s==e->rightAttack)return FlightAttackSide::Right;if(s==e->dualAttack)return FlightAttackSide::Dual;return FlightAttackSide::None;
}
bool FlightSuppressMeleeInput(RE::AttackBlockHandler* handler,RE::InputEvent* event){
 auto* b=event?event->AsButtonEvent():nullptr;auto side=FlightButtonSide(b);
 if(side==FlightAttackSide::None)return false;
 auto index=side==FlightAttackSide::Left?0u:(side==FlightAttackSide::Right?1u:2u);
 if(!FlightGameplay()){flightAttackLatch[index]=false;return false;}
 auto* p=RE::PlayerCharacter::GetSingleton();
 bool airborne=flightCombatSettings.blockMelee&&FlightActive()&&
  FlightMeleeButton(FlightEquippedKind(p,true),FlightEquippedKind(p,false),side);
 bool block=airborne||flightAttackLatch[index];if(!block)return false;
 bool first=!flightAttackLatch[index]&&b->IsPressed();flightAttackLatch[index]=b->IsPressed();
 handler->SetHeldStateActive(false);handler->triggerReleaseEvent=false;
 if(first&&flightMeleeLogs++<12)logger::info("FLIGHT_MELEE_INPUT_BLOCKED side={} [input admission only]",index);
 return true;
}
REL::Relocation<bool(*)(RE::AttackBlockHandler*,RE::InputEvent*)> flightOriginalAttackCan;
REL::Relocation<void(*)(RE::AttackBlockHandler*,RE::ButtonEvent*,RE::PlayerControlsData*)> flightOriginalAttackButton;
bool FlightAttackCanProcess(RE::AttackBlockHandler* h,RE::InputEvent* e){
 return FlightSuppressMeleeInput(h,e)?false:flightOriginalAttackCan(h,e);
}
void FlightAttackProcessButton(RE::AttackBlockHandler* h,RE::ButtonEvent* b,RE::PlayerControlsData* d){
 if(!FlightSuppressMeleeInput(h,b))flightOriginalAttackButton(h,b,d);
}
REL::Relocation<bool(*)(RE::IAnimationGraphManagerHolder*,const RE::BSFixedString&)> flightOriginalNotify;
bool flightNotifyInstalled=false;
bool FlightCombatNotify(RE::IAnimationGraphManagerHolder* holder,const RE::BSFixedString& event){
 auto* p=RE::PlayerCharacter::GetSingleton();
 if(p&&holder==static_cast<RE::IAnimationGraphManagerHolder*>(p)&&flightCombatSettings.blockMelee&&FlightActive()&&FlightGameplay()&&
    event.c_str()&&FlightMeleeGraphEvent(event.c_str(),FlightEquippedKind(p,true),FlightEquippedKind(p,false))){
  if(flightMeleeLogs++<12)logger::info("FLIGHT_MELEE_EVENT_BLOCKED tag={} [actor Notify route]",event.c_str());
  return false;
 }
 return flightOriginalNotify(holder,event);
}
void FlightInstallNotifyGuard(RE::PlayerCharacter* p){
 if(flightNotifyInstalled||!p||!p->Is3DLoaded())return;
 auto* holder=static_cast<RE::IAnimationGraphManagerHolder*>(p);
 auto table=*reinterpret_cast<std::uintptr_t*>(holder);
 REL::Relocation<std::uintptr_t> vtable{table};
 flightOriginalNotify=vtable.write_vfunc(0x01,FlightCombatNotify);flightNotifyInstalled=true;
 logger::info("FLIGHT_MELEE_NOTIFY_GUARD_READY [runtime interception unverified]");
}
void InstallFlightCombatInputHooks(){
 REL::Relocation<std::uintptr_t> table{RE::VTABLE_AttackBlockHandler[0]};
 flightOriginalAttackCan=table.write_vfunc(0x01,FlightAttackCanProcess);
 flightOriginalAttackButton=table.write_vfunc(0x04,FlightAttackProcessButton);
}
