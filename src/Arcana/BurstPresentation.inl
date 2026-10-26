struct BurstSlot {ArtSlot art{};RE::NiPoint3 pos{};RE::NiMatrix3 rotation{};float age=0,wait=0,size=64;int side=-1;BurstKind kind=BurstKind::World;bool active=false,started=false,prepared=false;};
std::array<BurstSlot,baseBurstSlots+solarImpactSlots> bursts{};
struct BurstVoice {RE::BSSoundHandle handle{};float age=0;};
std::array<BurstVoice,8> burstVoices{};
void ClearBursts(){for(auto& b:bursts){FinishCircle(b.art);b=BurstSlot{};}for(auto& v:burstVoices){if(v.handle.IsValid())static_cast<void>(v.handle.Stop());v=BurstVoice{};}}
void ShowBurstSlot(int absolute,int side,RE::Actor* actor,RE::NiAVObject* root,const RE::NiPoint3& p,const RE::NiMatrix3& r,float size,bool prepare,BurstKind kind){
 auto& b=bursts[absolute];bool warm=b.prepared&&b.art.form==forms.burst[absolute]&&b.kind==kind;
 if(!warm){FinishCircle(b.art);b=BurstSlot{};}b.age=0;b.wait=0;b.started=false;b.active=true;b.prepared=prepare;b.pos=p;b.rotation=r;b.size=size;b.side=side;b.kind=kind;
 EnsureCircle(b.art,forms.burst[absolute],actor,root);
}
void ShowBurst(int side,int index,RE::Actor* actor,RE::NiAVObject* root,const RE::NiPoint3& p,const RE::NiMatrix3& r,float size,bool prepare=false,BurstKind kind=BurstKind::World){
 ShowBurstSlot(side*burstsPerHand+index,side,actor,root,p,r,size,prepare,kind);
}
void ShowSolarImpact(int flight,int side,RE::Actor* actor,RE::NiAVObject* root,const RE::NiPoint3& point,const RE::NiMatrix3& facing,bool prepare){
 ShowBurstSlot(baseBurstSlots+flight,side,actor,root,point,facing,solarImpactSize,prepare,BurstKind::SolarImpact);
}
void ClearSolarImpactPreparation(int flight){auto& b=bursts[baseBurstSlots+flight];if(b.prepared){FinishCircle(b.art);b=BurstSlot{};}}
void ClearPreparedBursts(int side){for(int i=side*burstsPerHand;i<(side+1)*burstsPerHand;i++)if(bursts[i].prepared){FinishCircle(bursts[i].art);bursts[i]=BurstSlot{};}}
void UpdateBursts(int side,RE::Actor* actor,float dt){
 if(!actor||actor->IsDead())return;if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused())return;
 if(!std::isfinite(dt)||dt<=0)return;dt=std::min(dt,.1f);auto* root=actor->Get3D(false);
 for(int v=side*4;v<(side+1)*4;v++){auto& voice=burstVoices[v];voice.age+=dt;if(voice.age>burstVoiceSeconds&&voice.handle.IsValid()){static_cast<void>(voice.handle.Stop());voice=BurstVoice{};}}
 for(int i=0;i<static_cast<int>(bursts.size());i++){
  auto& b=bursts[i];if(!b.active||b.side!=side)continue;b.wait+=dt;if(!root||(!b.prepared&&b.wait>2.8f)){FinishCircle(b.art);b=BurstSlot{};continue;}
  EnsureCircle(b.art,forms.burst[i],actor,root);
  if(b.prepared){if(CircleReady(b.art)){b.wait=0;PoseCircle(b.art,b.pos,b.rotation,b.size/100.f,0);}else if(b.wait>2.8f){logger::warn("BURST_PREPARE_TIMEOUT slot={} kind={}",i,static_cast<int>(b.kind));FinishCircle(b.art);b=BurstSlot{};}continue;}
  if(!CircleReady(b.art))continue;
  if(!b.started){
   b.started=true;int local=i<baseBurstSlots?i-side*burstsPerHand:-1;
   if(BurstSoundDue(b.kind,local)){auto& voice=burstVoices[side*4];voice.age=0;Sound(voice.handle,forms.sound[12],root,b.kind==BurstKind::Rain?1.f:.94f,b.kind==BurstKind::Rain?TOAM::Presentation::Event::TowerExplosion:TOAM::Presentation::Event::HermitExplosion);logger::info("VANILLA_EXPLOSION_REQUEST hand={} kind={} source=private_firestorm fullTail=true",side,static_cast<int>(b.kind));}
   if(b.kind==BurstKind::Rain&&local==0)Shake(actor,1.f,.75f);
   else if(b.kind==BurstKind::World&&local%3==0){int group=local/3;Shake(actor,group?.88f:1.f,group?.40f:.80f);}
  }
  b.age+=dt;if(b.age>=worldBurstSeconds){FinishCircle(b.art);b=BurstSlot{};continue;}
  const float fade=1-Smooth((b.age-(worldBurstSeconds-.30f))/.30f);PoseCircle(b.art,b.pos,b.rotation,b.size/100.f,fade);
  auto* model=b.art.effect->Get3D();RE::NiUpdateData animation{};animation.time=b.age;animation.flags.set(RE::NiUpdateData::Flag::kDirty);model->Update(animation);
 }
}
