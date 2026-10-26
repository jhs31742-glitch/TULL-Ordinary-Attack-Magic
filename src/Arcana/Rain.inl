#include "RainMuzzles.inl"
struct SlowEntry {RE::ActorHandle actor{};unsigned owners=0;};
std::array<SlowEntry,64> slowRegistry{};
void RemoveSlow(RE::Actor* target){
 auto* player=RE::PlayerCharacter::GetSingleton();if(!target||!player||!forms.slow)return;
 auto handle=player->GetHandle();if(auto* magic=target->AsMagicTarget())static_cast<void>(magic->DispelEffect(forms.slow,handle));
}
void ClearSlows(int side){
 for(auto& e:slowRegistry)if(e.owners&(1u<<side)){
  e.owners&=~(1u<<side);if(!e.owners){auto a=e.actor.get();if(a)RemoveSlow(a.get());e=SlowEntry{};}
 }
}
void ApplySlow(RE::Actor* source,RE::Actor* target,int side){
 SlowEntry* entry=nullptr;
 for(auto& e:slowRegistry)if(e.owners&&e.actor.get().get()==target){entry=&e;break;}
 if(!entry)for(auto& e:slowRegistry)if(!e.owners){entry=&e;e.actor=target->GetHandle();break;}
 if(!entry)return;entry->owners|=1u<<side;
 // Renew only this caster's finite, recoverable native active effect; no permanent AV edit.
 if(auto* magic=target->AsMagicTarget())if(auto* list=magic->GetActiveEffectList())for(auto* effect:*list){
  if(effect&&effect->spell==forms.slow&&effect->caster.get().get()==source&&!effect->flags.any(RE::ActiveEffect::Flag::kDispelled,RE::ActiveEffect::Flag::kInactive)){
   effect->elapsedSeconds=0;return;
  }
 }
 if(auto* caster=source->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant))caster->CastSpellImmediate(forms.slow,true,target,1,true,90,source);
}
void RefreshField(Hand& h,int side,RE::Actor* a){
 h.targets=Enemies(a,h.field,rainRadius,false);
 for(auto& e:slowRegistry)if(e.owners&(1u<<side)){
  bool present=false;for(auto& t:h.targets)if(t==e.actor){present=true;break;}
  if(!present){e.owners&=~(1u<<side);if(!e.owners){auto target=e.actor.get();if(target)RemoveSlow(target.get());e=SlowEntry{};}}
 }
 for(auto& t:h.targets){auto target=t.get();if(target)ApplySlow(a,target.get(),side);}
}
void RainFinaleArt(Hand& h,int side,RE::Actor* actor,RE::NiAVObject* root,bool prepare){
 auto horizontal=DirectionRotation(RE::NiPoint3{0,0,1});
 for(int i=0;i<rainFinalePlumes;i++){auto p=RainFinaleOffset(i);RE::NiMatrix3 spin;spin.MakeYRotation(i*.71f);ShowBurst(side,i,actor,root,h.field+RE::NiPoint3{p.x,p.y,p.z},horizontal*spin,p.scale,prepare,BurstKind::Rain);}
}
void UpdateRain(RE::ActorMagicCaster* c,Hand& h,int side,RE::Actor* a,RE::NiAVObject* root,const RE::NiPoint3& feet,const RE::NiMatrix3& flat,float dt,float t){
 if(!h.fieldSet){
  if(Clock()<rainCooldown){Complete(h,side,a);c->currentSpellCost=0;return;}
  if(!StartResource(c,h,modes[Rain].cost)){DenyCast(c,h);return;}
  h.fieldSet=true;h.field=FlightFieldPoint(a,feet,flat);if(!FlightActive())h.field.z=feet.z;
  float ceiling=BeamLength(a,h.field+RE::NiPoint3{0,0,90},RE::NiPoint3{0,0,1},true);
  h.sky=h.field+RE::NiPoint3{0,0,std::max(80.f,std::min(rainSkyHeight,ceiling+40.f))};
 }
 h.slowTimer-=dt;if(h.slowTimer<=0){h.slowTimer=.6f;RefreshField(h,side,a);}
 auto horizontal=DirectionRotation(RE::NiPoint3{0,0,1});
 EnsureCircle(h.circles[0],CircleForm(side,h.bank,0,2),a,root);
 PoseCircle(h.circles[0],h.field+RE::NiPoint3{0,0,35},horizontal,rainRadius/100.f,1);
 EnsureCircle(h.circles[1],CircleForm(side,h.bank,1,3),a,root);
 auto facingDown=DirectionRotation(RE::NiPoint3{0,0,-1});
 RE::NiMatrix3 skySpin;skySpin.MakeYRotation(RainRotation(t));
 PoseCircle(h.circles[1],h.sky,facingDown*skySpin,RainFootprint(t)/200.f,1);
 // Every actual beam owns its solar shot sound; no unrelated 8-second sting.
 if(t>=rainFirstShotSeconds&&CircleReady(h.circles[1])){
  int queuedThisUpdate=0;
  while(h.rainQueued<rainShots&&t>=RainShotTime(h.rainQueued)&&FreeMuzzleSlots(side)>0&&queuedThisUpdate<2){
   if(!h.rainStarted){h.rainStarted=true;rainCooldown=Clock()+rainCooldownSeconds;}
   float az=Random(h)*6.2831853f,r=std::sqrt(Random(h))*RainBombardmentRadius(t)*.88f;
   auto origin=h.sky+RE::NiPoint3{std::cos(az)*r,std::sin(az)*r,-rainBeamDrop};
   RE::NiPoint3 target;bool enemy=false;
   if(!h.targets.empty()&&h.rainQueued%3!=2){
    auto actor=h.targets[h.rainQueued%h.targets.size()].get();
    if(actor&&!actor->IsDead()&&actor->Is3DLoaded()&&Length(actor->GetPosition()-h.field)<=RainBombardmentRadius(t)*.90f){target=Center(actor.get());enemy=true;}
   }
   if(!enemy){float angle=Random(h)*6.2831853f,span=std::sqrt(Random(h))*RainBombardmentRadius(t)*.88f;target=h.field+RE::NiPoint3{std::cos(angle)*span,std::sin(angle)*span,4};}
   auto delta=target-origin;
   QueueRainShot(h,side,a,root,origin,DirectionRotation(delta),std::min(4096.f,Length(delta)+60),RainWidth(h.rainQueued));h.rainQueued++;queuedThisUpdate++;
  }
 }
 if(!h.rainFinalePrepared&&t>=6){RainFinaleArt(h,side,a,root,true);h.rainFinalePrepared=true;}
 UpdateRainShots(c,h,side,a,root,dt);
 RE::NiPoint3 lastContact{};bool lastContactReady=ConsumeRainFinalContact(side,generations[side],lastContact);
 if(RainFinaleReady(h.rainFired,lastContactReady,h.rainDetonated)){
  h.rainDetonated=true;h.rainDetonationAt=t;
  RainFinaleArt(h,side,a,root,false);
  const auto priorRain=rainDamage;const int priorMode=currentDamageMode;
  rainDamage={true,side,generations[side],900.f};currentDamageMode=Rain;
  for(auto& handle:Enemies(a,h.field,rainRadius,true)){auto enemy=handle.get();if(enemy)if(auto* instant=a->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant))instant->CastSpellImmediate(forms.payload[Hermit],true,enemy.get(),1,true,900*h.damageMultiplier,a);}
  rainDamage=priorRain;currentDamageMode=priorMode;
  logger::info("RAIN_FINAL_DETONATION fired={} radius={} plumes=9 nominalDamage=900 shake=1 actualLastContact=true hit=({},{},{})",h.rainFired,rainRadius,lastContact.x,lastContact.y,lastContact.z);
 }
 if((h.rainDetonated&&t>=h.rainDetonationAt+.25f)||t>RainShotTime(rainShots-1)+5.f){if(!h.rainDetonated)logger::warn("RAIN_FINALE_CONTACT_TIMEOUT fired={} queued={} [no fabricated impact]",h.rainFired,h.rainQueued);Complete(h,side,a);c->currentSpellCost=0;}
}
