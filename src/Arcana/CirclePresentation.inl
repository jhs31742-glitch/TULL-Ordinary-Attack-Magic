struct RetiredCircle {float growthRate=0,spinRate=0;ArtSlot slot{};int side=-1;float age=0,hold=circleTailHold,fade=circleTailFade,growth=0;bool pending=false,sentryOwned=false;};
std::array<RetiredCircle,96> retiredCircles{};std::array<unsigned,2> retiredCursor{};
void FinishCircle(ArtSlot& a){if(a.effect)a.effect->finished=true;a=ArtSlot{};}
void ClearRetiredCircles(){for(auto& r:retiredCircles){FinishCircle(r.slot);r=RetiredCircle{};}}
bool CircleReady(const ArtSlot& a){
 if(!a.effect||a.effect->finished)return false;auto* root=a.effect->Get3D();if(!root)return false;
 static const RE::BSFixedString name{"AS_CirclePose"};return root->GetObjectByName(name)!=nullptr;
}
void CircleAlpha(RE::NiAVObject* o,float a){
 if(!o)return;o->fadeAmount=a;if(auto* n=o->AsNode())for(auto& ch:n->GetChildren())if(ch)CircleAlpha(ch.get(),a);
}
RE::NiPointer<RE::ModelReferenceEffect> FindCircle(RE::Actor* actor,RE::BGSArtObject* art){
 RE::NiPointer<RE::ModelReferenceEffect> found;
 auto request=circleRequests.find(art);
 if(request==circleRequests.end()||request->second.actor.get().get()!=actor||
    Clock()>request->second.deadline||request->second.searches++>=120)return found;
 if(auto* lists=RE::ProcessLists::GetSingleton())lists->ForEachModelEffect([&](RE::ModelReferenceEffect* e){
  if(e->finished||e->artObject!=art||e->target.get().get()!=actor)return RE::BSContainer::ForEachResult::kContinue;
  for(auto& h:hands)for(auto& a:h.circles)if(a.effect.get()==e)return RE::BSContainer::ForEachResult::kContinue;
  for(auto& r:retiredCircles)if(r.slot.effect.get()==e)return RE::BSContainer::ForEachResult::kContinue;
  found.reset(e);request->second.acquired=true;return RE::BSContainer::ForEachResult::kStop;
 });return found;
}
RE::BGSArtObject* CircleForm(int side,int bank,int index,int kind=0){
 return forms.circle[kind==3?168+side*4+bank:(kind==2?160+side*4+bank:(kind==1?96+(side*4+bank)*8+index:(side*4+bank)*12+index))];
}
void EnsureCircle(ArtSlot& a,RE::BGSArtObject* form,RE::Actor* actor,RE::NiAVObject* root){
 if(a.effect&&a.effect->finished)a.effect.reset();
 if(!a.requested){
  for(auto& r:retiredCircles)if(r.side>=0&&r.slot.form==form){FinishCircle(r.slot);r=RetiredCircle{};}
  a.form=form;a.requested=RequestCircleArt(actor,form,root);
 }
 if(a.requested&&!a.effect)a.effect=FindCircle(actor,form);
}
// Refresh the attachment before converting the common world axis into parent space.
void RefreshCircleParent(ArtSlot& a){if(a.effect&&!a.effect->finished)if(auto* root=a.effect->Get3D()){RE::NiUpdateData u{};root->Update(u);}}
void AuditSunAxis(const ArtSlot& a,int side,int layer,const RE::NiPoint3& anchor,const RE::NiPoint3& axis){
 if(!CircleReady(a)){logger::warn("SUN_AXIS hand={} layer={} sceneReady=false",side,layer);return;}
 static const RE::BSFixedString name{"AS_CirclePose"};auto* node=a.effect->Get3D()->GetObjectByName(name);auto delta=node->world.translate-anchor;
 float along=delta.x*axis.x+delta.y*axis.y+delta.z*axis.z;auto off=delta-axis*along,err=node->world.translate-a.pos;
 logger::info("SUN_AXIS hand={} layer={} lateralError={} positionError={} scale={} [transform sample only]",side,layer,std::sqrt(off.x*off.x+off.y*off.y+off.z*off.z),std::sqrt(err.x*err.x+err.y*err.y+err.z*err.z),node->world.scale);
}
RE::NiMatrix3 AnimatedCircleRotation(ArtSlot& a,const RE::NiMatrix3& base,float dt,float rate,bool reveal=true){
 if(CircleReady(a)){a.spinPhase=TOAM::VFX::CircleMotion::Advance(a.spinPhase,reveal?TOAM::VFX::CircleMotion::RevealSpeed(a.motionAge,rate):rate,dt);a.motionAge+=dt;}
 RE::NiMatrix3 spin;spin.MakeYRotation(a.spinPhase);return base*spin;
}
void PoseCircle(ArtSlot& a,const RE::NiPoint3& p,const RE::NiMatrix3& r,float s,float alpha){
 a.pos=p;a.rot=r;a.scale=s;a.alpha=alpha;MoveArt(a.effect.get(),p,r,s);
 if(CircleReady(a)){TOAM::VFX::CircleTint::Apply(a.tint,a.effect->Get3D());CircleAlpha(a.effect->Get3D(),alpha);}
}
void RetireCircle(ArtSlot& a,int side,bool tail,RE::Actor* actor,float growth=0,float fade=circleTailFade,float growthRate=0,float hold=circleTailHold,float spinRate=0,bool sentryOwned=false){
 if(!a.requested&&!a.effect)return;if(!a.effect&&actor)a.effect=FindCircle(actor,a.form);
 if(!tail&&a.effect){FinishCircle(a);return;}
 auto& r=retiredCircles[side*48+retiredCursor[side]++%48];FinishCircle(r.slot);r=RetiredCircle{};
 r.side=side;r.growth=growth;r.fade=fade;r.growthRate=growthRate;r.hold=hold;r.spinRate=spinRate;r.sentryOwned=sentryOwned;r.pending=!a.effect||!CircleReady(a);r.slot=std::move(a);r.age=tail?0:circleTailHold+circleTailFade;a=ArtSlot{};
}
void UpdateRetiredCircles(int side,RE::Actor* actor,float dt,bool onlySentry=false){
 if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused())return;
 if(!std::isfinite(dt)||dt<=0)return;dt=std::min(dt,.1f);
 for(auto& r:retiredCircles){
  if(r.side!=side||r.sentryOwned!=onlySentry)continue;r.age+=dt;if(!r.slot.effect&&actor)r.slot.effect=FindCircle(actor,r.slot.form);
  if(r.pending){if(r.slot.effect){FinishCircle(r.slot);r=RetiredCircle{};}else if(r.age>(r.sentryOwned?10.f:1.2f))r=RetiredCircle{};continue;}
  if(r.age>=r.hold+r.fade||!CircleReady(r.slot)){FinishCircle(r.slot);r=RetiredCircle{};continue;}
  RE::NiMatrix3 spin;spin.MakeYRotation(r.spinRate*r.age);MoveArt(r.slot.effect.get(),r.slot.pos,r.slot.rot*spin,r.slot.scale*(1+r.growth*Smooth(r.age/(r.hold+r.fade))+r.growthRate*r.age));CircleAlpha(r.slot.effect->Get3D(),r.slot.alpha*(r.age<r.hold?1:1-Smooth((r.age-r.hold)/r.fade)));
 }
}

void RetireHandCircle(Hand& h,ArtSlot& art,int index,int side,bool tail,RE::Actor* actor){
 if(h.mode==Rain&&tail)RetireCircle(art,side,true,actor,0,rainDissolveSeconds,index==1?rainRadiusSpeed/RainFootprint(h.cadence.elapsed):0.f,0.f,index==1?RainAngularSpeed(h.cadence.elapsed):0.f);
 else RetireCircle(art,side,tail,actor);
}
