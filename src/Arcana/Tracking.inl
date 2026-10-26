bool TrackingSpot(Hand& h,int lane,RE::Actor* source,RE::Actor* target,RE::NiPoint3& out){
 auto center=Center(target);auto& state=h.lanes[lane];
 for(int tries=0;tries<24;tries++){
  float angle=(Random(h)*6.2831853f)+lane*1.256637f;
  float radius=trackingDomeMin+(trackingDomeMax-trackingDomeMin)*Random(h);
  float elevation=.12f+.70f*Random(h);
  RE::NiPoint3 p=center+RE::NiPoint3{std::cos(angle)*radius,std::sin(angle)*radius,radius*elevation};
  auto delta=p-center;float distance=Length(delta),wall=BeamLength(target,center,Unit(delta));
  if(wall<distance+30)continue;
  float down=BeamLength(source,p,RE::NiPoint3{0,0,-1});if(down<45)continue;
  if(state.hasPrevious&&Length(p-state.previous)<trackingRepeatDistance)continue;
  bool crowded=false;for(int i=0;i<5;i++)if(i!=lane&&h.lanes[i].active&&Length(p-h.lanes[i].position)<trackingLaneClearance){crowded=true;break;}
  if(crowded)continue;out=p;return true;
 }return false;
}
void UpdateTracking(RE::ActorMagicCaster* c,Hand& h,int side,RE::Actor* a,RE::NiAVObject* root,const RE::NiPoint3& front,const RE::NiMatrix3& rot,float dt,float t){
 int count=TrackingCount(t);
 if(!count)return; // No caster charge circle: holding acquires enemies automatically.
 h.scanTimer-=dt;if(h.scanTimer<=0){h.scanTimer=.30f;h.targets=Enemies(a,a->GetPosition(),trackingRadius,true);if(h.targets.size()>5)h.targets.resize(5);}
 bool sound=false;
 for(int i=0;i<count;i++){
  auto& lane=h.lanes[i];auto& art=h.circles[i];
  if(!lane.active){
   lane.wait-=dt;if(lane.wait>0||h.targets.empty())continue;
   auto target=h.targets[i%h.targets.size()].get();
   if(!target||target->IsDead()||!target->Is3DLoaded())continue;
   RE::NiPoint3 pos;if(!TrackingSpot(h,i,a,target.get(),pos)){lane.wait=.15f;continue;}
   lane.target=target->GetHandle();lane.position=pos;lane.previous=pos;lane.hasPrevious=true;lane.age=0;lane.fired=false;lane.active=true;
  }
  auto target=lane.target.get();
  if(!target||target->IsDead()||!target->Is3DLoaded()||Length(target->GetPosition()-a->GetPosition())>trackingRadius+100){
   lane.active=false;lane.wait=.10f;PoseCircle(art,lane.position,rot,.001f,0);continue;
  }
  auto delta=Center(target.get())-lane.position;auto facing=DirectionRotation(delta);
  EnsureCircle(art,CircleForm(side,h.bank,i),a,root);
  if(!CircleReady(art))continue;lane.age+=dt;
  float after=lane.age-trackingBuild;
  float alpha=after<=trackingHold?1:1-Smooth((after-trackingHold)/trackingFade);
  PoseCircle(art,lane.position,facing,trackingCircleScale*Smooth(lane.age/trackingBuild),alpha);
  if(!lane.fired&&lane.age>=trackingBuild){
   // Do not damage an actor that has since crossed cover; the engine still owns collision.
   float length=Length(delta),hit=BeamLength(a,lane.position,Unit(delta));
   if(hit+40>=length){Launch(h,side,c,a,root,lane.position,facing,Tracking,4096.f,0,-1,100.f*trackingPeriods[i]/float(count));sound=true;}
   lane.fired=true;
  }
  if(lane.age>=trackingBuild+trackingHold+trackingFade){
   lane.active=false;lane.wait=std::max(.02f,trackingPeriods[i]-lane.age);PoseCircle(art,lane.position,facing,.001f,0);
  }
 }
 if(sound)Sound(h.shots[h.soundSlot++%16],forms.sound[0],root,.47f);
}
