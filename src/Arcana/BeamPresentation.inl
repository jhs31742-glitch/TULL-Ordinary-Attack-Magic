// Engine-owned ARTO instances provide non-empty geometry independently of damage projectiles.
// The private effect-only axis carries longitudinal stretch. No shared material is edited.
struct BeamSlot {
 RE::NiPointer<RE::ModelReferenceEffect> effect{};
 RE::NiPoint3 origin{};RE::NiMatrix3 rotation{};
 double reuseAfter=0;
 float age=0,visibleAge=0,waiting=0,life=beamPulseSeconds,width=4.5f,length=4096;
 RE::NiPoint3 impactNormal{0,0,1};bool geometryHit=false;std::uint64_t serial=0;int mode=-1;bool active=false,continuous=false,retiring=false,started=false;
};
std::uint64_t beamSerial=0;std::array<BeamSlot,96> beamSlots{};std::array<unsigned,2> beamReadyLogs{};std::array<unsigned,2> beamCursor{};
void ReleaseBeam(BeamSlot& b){
 double reuse=b.reuseAfter;if(b.active&&!b.effect)reuse=Clock()+1.6;
 if(b.effect)b.effect->finished=true;b=BeamSlot{};b.reuseAfter=reuse;
}
void ClearBeams(){for(auto& b:beamSlots)ReleaseBeam(b);}
void StopBeams(int side,bool tail){for(int i=side*beamsPerHand;i<(side+1)*beamsPerHand;i++){auto& b=beamSlots[i];if(!b.active||b.mode==Sentry)continue;if(tail&&!b.continuous&&!b.retiring)continue;if(tail&&b.started){b.retiring=true;b.continuous=false;b.age=0;b.life=beamReleaseHold+beamReleaseFade;}else ReleaseBeam(b);}}
float BeamLength(RE::Actor* actor,const RE::NiPoint3& origin,const RE::NiPoint3& direction,bool ignoreActors=false,float maxDistance=4096,bool* geometryHit=nullptr,RE::NiPoint3* hitNormal=nullptr){
 if(geometryHit)*geometryHit=false;if(hitNormal)*hitNormal=RE::NiPoint3{0,0,1};
 const float range=std::clamp(maxDistance,1.f,solarMaxRange);
 auto* tes=RE::TES::GetSingleton();const float hs=RE::bhkWorld::GetWorldScale();if(!tes||!std::isfinite(hs)||hs<=0)return range;
 float offset=0;
 for(int step=0;step<(ignoreActors?512:5);step++){
  const auto a=origin+direction*offset;const auto z=origin+direction*range;const auto ray=z-a;
  RE::bhkPickData pick{};pick.rayInput.from=RE::hkVector4{a.x*hs,a.y*hs,a.z*hs,0};pick.rayInput.to=RE::hkVector4{z.x*hs,z.y*hs,z.z*hs,0};
  pick.rayInput.enableShapeCollectionFilter=false;pick.rayInput.filterInfo.SetCollisionLayer(RE::COL_LAYER::kLOS);pick.ray=RE::hkVector4{ray.x*hs,ray.y*hs,ray.z*hs,0};pick.rayOutput.Reset();auto* object=tes->Pick(pick);
  if(pick.pickFailed||!pick.rayOutput.HasHit())return range;
  const float f=pick.rayOutput.hitFraction;if(!std::isfinite(f)||f<0||f>1)return range;
  const float distance=offset+(range-offset)*f;
  auto* ref=object?object->GetUserData():nullptr;
  if((ref==actor&&distance<160)||(ignoreActors&&ref&&ref->IsActor())){offset=distance+(ignoreActors?8.f:24.f);if(offset>=range)return range;continue;}
  const bool geometry=!ref||!ref->IsActor();if(geometryHit)*geometryHit=geometry;
  if(hitNormal&&geometry){float normal[4];_mm_storeu_ps(normal,pick.rayOutput.normal.quad);*hitNormal=RE::NiPoint3{normal[0],normal[1],normal[2]};}
  return std::clamp(distance,1.f,range);
 }
 return ignoreActors?std::min(offset,range):range;
}
void BeamBounds(RE::NiAVObject* object,const RE::NiPoint3& center,float radius,float opacity){
 if(!object)return;object->fadeAmount=opacity;object->worldBound.center=center;object->worldBound.radius=radius;
 if(auto* node=object->AsNode())for(auto& child:node->GetChildren())if(child)BeamBounds(child.get(),center,radius,opacity);
}
void PoseBeam(BeamSlot& b,float envelope,float opacity=1.f,float offset=0,float visibleLength=-1){
 if(!b.effect||b.effect->finished)return;auto* root=b.effect->Get3D();if(!root)return;
 static const RE::BSFixedString name{"AS_BeamAxis"};auto* axis=root->GetObjectByName(name);if(!axis||!axis->parent)return;
 const auto& parent=axis->parent->world;if(!std::isfinite(parent.scale)||std::abs(parent.scale)<.001f)return;
 b.effect->lifetime=b.effect->age+1.5f;
 const auto direction=b.rotation*RE::NiPoint3{0,1,0};auto visualOrigin=b.origin+direction*offset;const float segment=visibleLength<0?b.length:std::max(.001f,visibleLength);
 const auto inverse=parent.rotate.Transpose();axis->local.translate=inverse*(visualOrigin-parent.translate)/parent.scale;
 auto stretch=b.rotation;const float transverse=b.width*envelope;
 for(int row=0;row<3;row++){stretch.entry[row][0]*=transverse;stretch.entry[row][1]*=segment/512.f;stretch.entry[row][2]*=transverse;}
 axis->local.rotate=inverse*stretch;axis->local.scale=1.f/parent.scale;
 RE::NiUpdateData update{};root->Update(update);
 const auto center=visualOrigin+direction*(segment*.5f);
 BeamBounds(root,center,segment*.5f+b.width*3+8,opacity);
}
void UpdateBeams(int side,RE::Actor* actor,float dt,int onlyMode=-1){
 if(!actor||actor->IsDead()){StopBeams(side,false);return;}auto* ui=RE::UI::GetSingleton();if(ui&&ui->GameIsPaused())return;
 if(!std::isfinite(dt)||dt<=0)return;dt=std::clamp(dt,0.f,.10f);
 for(int i=side*beamsPerHand;i<(side+1)*beamsPerHand;i++){
  auto& b=beamSlots[i];if(!b.active||(onlyMode>=0&&b.mode!=onlyMode))continue;
  if(b.effect&&b.effect->finished){ReleaseBeam(b);continue;}
  b.age+=dt;if(b.age>=b.life){ReleaseBeam(b);continue;}
  if(!b.effect){
   b.waiting+=dt;if(auto* lists=RE::ProcessLists::GetSingleton())lists->ForEachModelEffect([&](RE::ModelReferenceEffect* e){
    if(e->finished||e->artObject!=forms.beam[i]||e->target.get().get()!=actor)return RE::BSContainer::ForEachResult::kContinue;
    b.effect.reset(e);return RE::BSContainer::ForEachResult::kStop;
   });
   if(b.waiting>1.f){ReleaseBeam(b);logger::warn("BEAM_ART_TIMEOUT slot={}",i);continue;}
  }
  if(!b.started){
   auto* model=b.effect?b.effect->Get3D():nullptr;
   if(!model)continue;static const RE::BSFixedString name{"AS_BeamAxis"};if(!model->GetObjectByName(name))continue;
   b.started=true;if(beamReadyLogs[side]++<16)logger::info("BEAM_MODEL_READY slot={} length={:.1f} actual={:X}",i,b.length,reinterpret_cast<std::uintptr_t>(model));
  }
  b.visibleAge+=dt;
  if(!b.continuous&&!b.retiring){auto pulse=BeamPulseState(b.mode,b.length,b.age);PoseBeam(b,pulse.width,pulse.alpha,pulse.tail,pulse.head-pulse.tail);continue;}
  const float envelope=b.retiring?ReleaseOpacity(b.age):(b.continuous?Smooth(b.visibleAge/.02f):PulseWidth(b.age));
  const float opacity=b.retiring?ReleaseOpacity(b.age):(b.continuous?Smooth(b.visibleAge/.02f):PulseOpacity(b.age));
  PoseBeam(b,envelope<.001f?.001f:envelope,opacity);
 }
}
int ShowBeam(int side,RE::Actor* actor,RE::NiAVObject* root,const RE::NiPoint3& origin,const RE::NiMatrix3& rotation,int mode,float maxLength=4096,float width=0,int ownedSlot=-1){
 const bool continuous=BeamStyleFor(mode)==BeamStyle::Held;int slot=side*beamsPerHand;
 if(continuous&&mode==Sentry){slot=ownedSlot;if(slot<=side*beamsPerHand||slot>=(side+1)*beamsPerHand||!beamSlots[slot].active||beamSlots[slot].mode!=Sentry||beamSlots[slot].retiring)slot=-1;}
 if(!continuous||slot<0){slot=-1;for(int n=0;n<beamsPerHand-1;n++){int k=side*beamsPerHand+1+(beamCursor[side]+n)%(beamsPerHand-1);if(!beamSlots[k].active&&Clock()>=beamSlots[k].reuseAfter){slot=k;beamCursor[side]=(k-side*beamsPerHand)%(beamsPerHand-1);break;}}if(slot<0){logger::warn("BEAM_POOL_FULL hand={}",side);return -1;}}
 auto& b=beamSlots[slot];
 if(!continuous||b.retiring||!b.active){
  if(Clock()<b.reuseAfter)return -1;
  ReleaseBeam(b);b.active=true;b.continuous=continuous;b.serial=++beamSerial;
  using Fn=bool(*)(RE::TESObjectREFR*,RE::BGSArtObject*,float,RE::TESObjectREFR*,bool,bool,RE::NiAVObject*,bool);static REL::Relocation<Fn> apply{REL::RelocationID(22289,22769)};
  if(!apply(actor,forms.beam[slot],1.5f,nullptr,false,false,root,false)){ReleaseBeam(b);return -1;}
 }
 b.mode=mode;b.origin=origin;b.rotation=rotation;b.width=width>0?width:BeamRadius(mode);b.length=std::min(maxLength,BeamLength(actor,origin,rotation*RE::NiPoint3{0,1,0},(mode==Tracking||mode==SolarCrown),maxLength,&b.geometryHit,&b.impactNormal));b.life=BeamVisualLifetime(mode,b.length);
 if(continuous){b.age=0;if(b.started)PoseBeam(b,Smooth(b.visibleAge/.02f),Smooth(b.visibleAge/.02f));}
 return slot;
}

bool RainBeamBusy(int side){for(int i=side*beamsPerHand;i<(side+1)*beamsPerHand;i++)if(beamSlots[i].active&&beamSlots[i].mode==Rain)return true;return false;}

int FreePulseSlots(int side){int free=0;for(int i=side*beamsPerHand+1;i<(side+1)*beamsPerHand;i++)if(!beamSlots[i].active&&Clock()>=beamSlots[i].reuseAfter)free++;return free;}
