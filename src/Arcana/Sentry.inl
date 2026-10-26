// Independent installed emitters: fixed world position, smooth aim, owned held beam.
struct SentrySlot {ArtSlot circle{};SentryLife life{};RE::NiPoint3 position{};RE::NiMatrix3 facing{};RE::TESObjectCELL* cell=nullptr;RE::TESWorldSpace* world=nullptr;RE::ActorHandle target{};RE::BSSoundHandle loop{};int beam=-1;std::uint64_t beamSerial=0;unsigned audits=0,contacts=0;float next=0,scan=0,wait=0;int side=0;};
std::array<SentrySlot,sentryLimit> sentries{};std::array<unsigned,sentryLimit> sentryBanks{};std::uint64_t sentrySerial=0;
void StopSentryBeam(SentrySlot& s,bool tail=true){
 if(s.beam>=0&&s.beam<static_cast<int>(beamSlots.size())){auto& b=beamSlots[s.beam];if(b.active&&b.mode==Sentry&&b.serial==s.beamSerial){
  if(tail&&b.started){b.retiring=true;b.continuous=false;b.age=0;b.life=beamReleaseHold+beamReleaseFade;}else ReleaseBeam(b);
 }}
 s.beam=-1;s.beamSerial=0;if(s.loop.IsValid()){if(tail)FadeSustain(s.loop);else static_cast<void>(s.loop.Stop());s.loop=RE::BSSoundHandle{};}
}
void RemoveSentry(SentrySlot& s){StopSentryBeam(s);RetireCircle(s.circle,s.side,false,RE::PlayerCharacter::GetSingleton(),0,circleTailFade,0,circleTailHold,0,true);s=SentrySlot{};}
void ClearSentries(){for(auto& s:sentries){StopSentryBeam(s,false);RemoveSentry(s);}}
void PlaceSentry(int side,RE::Actor* actor,RE::NiAVObject* root,const RE::NiPoint3& position,const RE::NiMatrix3& facing){
 std::array<SentryLife,3> life{};for(int i=0;i<3;i++)life[i]=sentries[i].life;int index=SentryReplacement(life);auto& s=sentries[index];bool replaced=s.life.active;RemoveSentry(s);
 s.life={true,++sentrySerial,0};s.position=position;s.facing=facing;s.side=side;s.cell=actor->GetParentCell();s.world=actor->GetWorldspace();s.next=.24f;
 auto* art=forms.sentry[index*4+(++sentryBanks[index])%4];EnsureCircle(s.circle,art,actor,root);
 logger::info("SENTRY_PLACED slot={} serial={} replacedOldest={} seconds=3 heldBeam=true position=({},{},{})",index,s.life.serial,replaced,position.x,position.y,position.z);
}
RE::Actor* SentryTarget(SentrySlot& s,RE::Actor* actor,RE::ActorHandle& handle){
 auto target=handle.get();if(!target||target.get()==actor||target->IsDead()||!target->Is3DLoaded()||!target->IsHostileToActor(actor))return nullptr;
 auto delta=Center(target.get())-s.position;float distance=Length(delta);
 if(distance<1||distance>sentryRadius||BeamLength(actor,s.position,Unit(delta),true,distance+48)+24<distance)return nullptr;
 return target.get();
}
void UpdateSentries(int side,RE::MagicCaster*,RE::Actor* actor,float dt){
 if(!gameReady||!forms.ready||!actor)return;if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused())return;if(!std::isfinite(dt)||dt<=0)return;dt=std::min(dt,.1f);
 for(auto& s:sentries){if(!s.life.active||s.side!=side)continue;s.life.age+=dt;s.scan-=dt;
  auto* cell=actor->GetParentCell();bool switched=cell!=s.cell&&(!cell||!s.cell||cell->IsInteriorCell()||s.cell->IsInteriorCell()||actor->GetWorldspace()!=s.world);
  if(SentryExpired(s.life.age)||actor->IsDead()||!actor->Is3DLoaded()||switched||Length(actor->GetPosition()-s.position)>4096){logger::info("SENTRY_REMOVED serial={} age={}",s.life.serial,s.life.age);RemoveSentry(s);continue;}
  auto* root=actor->Get3D(false);if(!root)continue;EnsureCircle(s.circle,s.circle.form,actor,root);
  if(!CircleReady(s.circle)){s.wait+=dt;if(s.wait>1.5f){logger::warn("SENTRY_MODEL_TIMEOUT serial={}",s.life.serial);RemoveSentry(s);}continue;}
  auto* target=SentryTarget(s,actor,s.target);
  if(!target&&s.scan<=0){s.scan=sentryScanSeconds;s.target={};auto candidates=Enemies(actor,s.position,sentryRadius,false);
   for(auto& handle:candidates)if(SentryTarget(s,actor,handle)){s.target=handle;break;}target=SentryTarget(s,actor,s.target);
  }
  float alpha=s.life.age<sentrySeconds-.25f?1:1-Smooth((s.life.age-sentrySeconds+.25f)/.25f);
  auto retainedTarget=s.target.get(); // Retain ownership without bypassing eligibility.
  if(target){
   auto current=s.facing*RE::NiPoint3{0,1,0},delta=Center(target)-s.position,desired=Unit(delta);
   float yaw=SentryTurnAngle(std::atan2(current.x,current.y),std::atan2(desired.x,desired.y),dt);
   float pitch=SentryTurnAngle(-std::atan2(current.z,std::hypot(current.x,current.y)),-std::atan2(desired.z,std::hypot(desired.x,desired.y)),dt);
   s.facing=AimRotation(pitch,yaw);
   if(s.beam>=0&&(!beamSlots[s.beam].active||beamSlots[s.beam].serial!=s.beamSerial))s.beam=-1;
   s.beam=ShowBeam(side,actor,root,s.position,s.facing,Sentry,sentryRadius,0,s.beam);
   if(s.beam>=0){auto& beam=beamSlots[s.beam];s.beamSerial=beam.serial;if(!s.loop.IsValid())Sound(s.loop,forms.sound[1],root,.42f);
    auto direction=s.facing*RE::NiPoint3{0,1,0};float dot=direction.x*desired.x+direction.y*desired.y+direction.z*desired.z;
    bool clear=beam.length+64>=Length(delta);
    // CP029: continue110 contacts at the original .20s interval for the original3s lifetime.
    if(s.life.age+.00001f>=s.next&&SentryDamageReady(dot,clear,beam.started)){
     s.next=s.life.age+sentryInterval;
     if(auto* caster=actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)){
      ++s.contacts;const int previous=currentDamageMode;currentDamageMode=Sentry;
      caster->CastSpellImmediate(forms.sentryContact,true,target,1,true,modes[Sentry].damage,actor);currentDamageMode=previous;
      if(s.audits++<16)logger::info("SENTRY_CONTACT_REQUEST serial={} target={:08X} magnitude={} interval={}",s.life.serial,target->GetFormID(),modes[Sentry].damage,sentryInterval);
     }
    }
   }
  }else{StopSentryBeam(s);s.target={};}
  RefreshCircleParent(s.circle);PoseCircle(s.circle,s.position,s.facing,.25f,alpha);
 }
}
