void Update(RE::ActorMagicCaster* c,float dt){
 auto* hp=GetHand(c);int side=hp==&hands[0]?0:1;
 if(gameReady&&forms.ready&&c->currentSpell&&(c->currentSpell==forms.hostAimCarrier[0]||c->currentSpell==forms.hostAimCarrier[1])){
  // Same engine aimed-concentration presentation as Sun, without a second projectile/payment owner.
  c->projectileTimer=100000.f;c->currentSpellCost=0;
 }
 if(forms.ready&&hp&&Identify(c->currentSpell)>=0){c->projectileTimer=100000.f;SessionCost(c,*hp);}
 if(hp&&gameReady&&forms.ready&&Identify(c->currentSpell)>=0&&c->state==RE::MagicCaster::State::kCasting&&hp->spell!=c->currentSpell){if(!Begin(c,*hp,Identify(c->currentSpell))){DenyCast(c,*hp);return;}}
 if(hp&&gameReady&&forms.ready&&Identify(c->currentSpell)>=0)SessionCost(c,*hp); // Include Begin fallback before the engine update.
 const bool firstUpdate=hp&&hp->spell==c->currentSpell&&hp->mode>=0&&!hp->engineUpdateSeen;
 if(firstUpdate){hp->engineUpdateSeen=true;logger::info("ARCANA_ENGINE_UPDATE_ENTER hand={} mode={} staff={} state={}",side,hp->mode,IsStaff(hp->spell),c->state.underlying());}
 original(c,dt);
 if(firstUpdate)logger::info("ARCANA_ENGINE_UPDATE_RETURN hand={} state={}",side,c->state.underlying());
 if(!hp)return;auto& h=*hp;auto* actor=c->actor;int id=Identify(c->currentSpell);
 if(!gameReady||!forms.ready||id<0||actor->IsDead()||!actor->Is3DLoaded()){Stop(h);return;}
 if(c->state!=RE::MagicCaster::State::kCasting)return;
 if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused())return;
 if(!std::isfinite(dt)||dt<=0)return;dt=std::clamp(dt,0.f,.1f);
 if(h.spell!=c->currentSpell&&!Begin(c,h,id)){DenyCast(c,h);return;}if(h.done)return;
 const float priorTime=h.cadence.elapsed;
 bool fire=h.cadence.Tick(dt,modes[h.mode].interval);float t=h.cadence.elapsed;
 float fee=0;
 if(h.mode==Sustain)fee=modes[Sustain].cost*dt;
 else if(h.mode==Tracking){
  unsigned deployed=0;for(int i=0;i<5;i++)if(h.lanes[i].hasPrevious&&CircleReady(h.circles[i]))++deployed;
  fee=trackingUpkeepPerLane*float(deployed)*dt+(h.resourceStarted?0.f:modes[Tracking].cost);
 }
 else if(h.mode==SolarCrown)fee=(modes[SolarCrown].cost/ultimateChargeSeconds)*
  std::max(0.f,std::min(t,ultimateChargeSeconds)-std::min(priorTime,ultimateChargeSeconds));
 else if(h.mode==Hermit&&IsStaff(h.spell))fee=modes[Hermit].cost*dt;
 if(fee>0||h.mode==Tracking){
  if(!PayResource(c,h,fee)){DenyCast(c,h);return;}
  if(h.mode==Tracking)h.resourceStarted=true;
 }
 auto* root=actor->Get3D(false);if(!root)return;
 auto rot=AimRotation(actor->GetAimAngle(),actor->GetAimHeading()),flat=AimRotation(0,actor->GetAimHeading());
 auto feet=actor->GetPosition(),base=feet+RE::NiPoint3{0,0,98};
 bool matching=hands[1-side].spell&&hands[1-side].mode==h.mode&&!hands[1-side].done;
 bool secondary=SecondaryStack(side,matching);
 auto off=FrontOffset(h.mode,side,matching);auto front=FlightCastOffset(feet,flat,rot,RE::NiPoint3{off.x,off.y,off.z});
 if(h.mode==Hermit){UpdateWorld(c,h,side,actor,root,feet,flat,dt,t);return;}
 if(h.mode==Sentry){if(t>=.24f){
  if(!StartResource(c,h,modes[Sentry].cost)){DenyCast(c,h);return;}
  PlaceSentry(side,actor,root,front,FlightActive()?rot:flat,h.damageMultiplier);Complete(h,side,actor);c->currentSpellCost=0;}return;}
 if(h.mode==Tracking){UpdateTracking(c,h,side,actor,root,front,rot,dt,t);return;}
 if(h.mode==Rain){UpdateRain(c,h,side,actor,root,feet,flat,dt,t);return;}
 if(h.mode==SolarCrown&&UltimateDone(t)){logger::warn("SUN_READINESS_TIMEOUT charge={} spawned={}",t,h.launched);Complete(h,side,actor);c->currentSpellCost=0;return;}
 if(!h.soundStarted){h.soundStarted=true;if(h.mode==Sustain)Sound(h.shots[h.soundSlot++%16],forms.sound[6],root,.65f);}
 bool ult=h.mode==SolarCrown&&UltimateFiring(t);
 bool auditSun=ult&&!h.releasePlayed;
 if(h.mode==Sustain&&!h.loop.IsValid())Sound(h.loop,forms.sound[1],root,.78f);
 int count=CircleCount(h.mode,t);float fit=1,lift=0;
 if(h.mode==SolarCrown&&!FlightActive()){
  for(int i=0;i<8;i++){auto axis=UltimateAxisOffset(i,t);auto p=front+rot*RE::NiPoint3{axis.x,axis.y,axis.z};lift=std::max(lift,feetClearance+UltimateScale(i,t)*fit*circleGroundRadius-(p.z-feet.z));}
 }
 for(int i=0;i<count;i++){
  auto p=front;float s=off.scale*CircleGrowth(0,t);
  if(h.mode==SolarCrown){auto axis=UltimateAxisOffset(i,t);p+=rot*RE::NiPoint3{axis.x,axis.y,axis.z};p.z+=lift;s=UltimateScale(i,t)*fit*(secondary?.72f:1.f);}
  else if(h.mode==Fivefold){auto f=FivefoldOffset(i,t,secondary);p=FlightCastOffset(feet,flat,rot,RE::NiPoint3{f.x,f.y,f.z});s=f.scale*CircleGrowth(i,t);}
  RE::NiMatrix3 spin;spin.MakeYRotation(i*.47f+t*(i%2?-.21f:.25f));
  auto& art=h.circles[i];EnsureCircle(art,CircleForm(side,h.bank,i,h.mode==SolarCrown?1:0),actor,root);
  float alpha=1.f;
  if(h.mode==SolarCrown){
   RefreshCircleParent(art);
   if(CircleReady(art)){if(art.firstPose)art.presentationAge+=dt;else {art.firstPose=true;art.presentationAge=0;}}
   const float growth=Smooth(art.presentationAge/.35f);s*=.04f+.96f*growth;alpha=Smooth(art.presentationAge/.12f);
  }
  PoseCircle(art,p,rot*spin,std::max(.001f,s),alpha);
 }
 if(auditSun){
  auto anchor=front+RE::NiPoint3{0,0,lift},axis=rot*RE::NiPoint3{0,1,0};
  for(int i=0;i<8;i++)AuditSunAxis(h.circles[i],side,i,anchor,axis);
 }
 bool sunReady=true;if(h.mode==SolarCrown)for(int i=0;i<8;i++)sunReady&=CircleReady(h.circles[i])&&h.circles[i].firstPose&&h.circles[i].presentationAge>=.35f;
 unsigned mask=h.mode==SolarCrown?(SunReleaseDue(t,h.releasePlayed,sunReady)?1u:0u):(h.mode==Fivefold?h.fivefold.Tick(t,count):(fire?1u:0u));
 if(!mask||(h.mode==SolarCrown&&!ult))return;
 const auto launchesBefore=h.launched;
 for(int i=0;i<(h.mode==Fivefold?count:1);i++){
  if(!(mask&(1u<<i))||!CircleReady(h.circles[i]))continue;
  auto p=h.circles[i].pos;if(h.mode==SolarCrown)p=front+rot*RE::NiPoint3{0,UltimateBeamDepth(t),0}+RE::NiPoint3{0,0,lift};
  auto fireRot=h.mode==Sustain?DirectionRotation(base+rot*RE::NiPoint3{0,3000,0}-p):rot; Launch(h,side,c,actor,root,p,fireRot,h.mode);
  if(h.done)return;
 }
 if(h.mode==SolarCrown){h.releasePlayed=true;Sound(ultimateVoices[ultimateVoiceCursor++%ultimateVoices.size()],forms.sound[11],root,.90f);Shake(actor,ultimateKickStrength,ultimateKickSeconds);logger::info("SUN_BOLT_RELEASE charge={} sweptBolts={} failures={}",t,h.launched,h.failed);Complete(h,side,actor);c->currentSpellCost=0;return;}
 if(h.launched>launchesBefore&&(h.mode==Rapid||h.mode==Fivefold))Sound(h.shots[h.soundSlot++%16],forms.sound[0],root,h.mode==Rapid?.68f:.54f);
}
