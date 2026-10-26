void UpdateWorld(RE::ActorMagicCaster* c,Hand& h,int side,RE::Actor* a,RE::NiAVObject* root,const RE::NiPoint3& feet,const RE::NiMatrix3& flat,float dt,float t){
 int count=WorldCount(t);bool all=true;float coverage=0;
 for(int j=0;j<count;j++){
  int i=worldOrder[j];const auto& point=LayoutData::hermit[i];const auto& normal=LayoutData::hermitNormals[i];auto n=h.worldAir?AirWorldNormal(j):FlightVector{normal.x,normal.y,normal.z};auto direction=flat*RE::NiPoint3{n.x,n.y,n.z};
  auto p=feet+flat*RE::NiPoint3{point.x*worldDomeScale,point.y*worldDomeScale,70+(point.z-70)*worldDomeScale};if(h.worldAir)p=feet+RE::NiPoint3{0,0,78}+direction*112.f;auto r=DirectionRotation(direction);
  EnsureCircle(h.circles[j],CircleForm(side,h.bank,j),a,root);float growth=WorldCircleGrowth(j,t);float alpha=Smooth((t-WorldDeployTime(j))/.20f);
  PoseCircle(h.circles[j],p,r,worldCircleScale*growth,alpha);bool ready=CircleReady(h.circles[j]);all&=ready;if(ready)coverage+=growth/12;
 }
 RefreshDomeWard(side,coverage);
 if(!h.worldBlastStarted&&t>=worldChargeSeconds&&all){h.worldBlastStarted=true;h.worldBlastAt=t;h.worldScan=0;logger::info("DOME_BURST_BEGIN hand={} defenseFactor={:.2f}",side,DomeDamageFactor(coverage));}
 if(h.worldBlastStarted){
  float age=t-h.worldBlastAt;
  while(h.worldBurstsShown<12&&age>=(h.worldBurstsShown/3)*.12f){
   int j=h.worldBurstsShown++;auto direction=h.circles[j].rot*RE::NiPoint3{0,1,0};auto spread=WorldBurstDisplacement(direction.x,direction.y,direction.z);auto p=h.circles[j].pos+(h.worldAir?direction*worldBurstSpread:RE::NiPoint3{spread.x,spread.y,spread.z});
   ShowBurst(side,j,a,root,p,h.circles[j].rot,worldBurstSize);
  }
  h.worldScan-=dt;if(h.worldScan<=0){h.worldScan=.10f;auto center=feet+RE::NiPoint3{0,0,h.worldAir?78.f:70.f};auto targets=Enemies(a,center,worldDamageRadius,true);
   for(auto& handle:targets){auto target=handle.get();if(!target)continue;auto id=target->GetFormID();if(std::find(h.worldHitIDs.begin(),h.worldHitIDs.end(),id)!=h.worldHitIDs.end())continue;
    h.worldHitIDs.push_back(id);if(auto* caster=a->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant))caster->CastSpellImmediate(forms.payload[Hermit],true,target.get(),1,true,modes[Hermit].damage,a);
   }
  }
  if(age>=worldReleaseSeconds){Complete(h,side,a);c->currentSpellCost=0;}
 }else if(t>worldChargeSeconds+2.5f){Complete(h,side,a);c->currentSpellCost=0;}
}
