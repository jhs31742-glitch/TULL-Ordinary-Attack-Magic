void PlayEnd(){
 auto* p=RE::PlayerCharacter::GetSingleton();if(!gameReady||!p||p->IsDead())return;
 if(auto* root=p->Get3D(false))Sound(endingSounds[endingSlot++%4],forms.sound[7],root,.50f,TOAM::Presentation::Event::SunCancel);
}
void Stop(Hand& h,bool tail=true){
 if(h.mode<0)return;int side=&h==&hands[0]?0:1;
 bool ending=tail&&!h.done&&h.mode==SolarCrown&&h.loop.IsValid();
 ClearPreparedBursts(side);ClearGuard(side);ClearSlows(side);StopBeams(side,tail);StopRainShots(side,tail,RE::PlayerCharacter::GetSingleton());
 auto* player=RE::PlayerCharacter::GetSingleton();for(int i=0;i<12;i++)RetireHandCircle(h,h.circles[i],i,side,tail,player);
 if(h.loop.IsValid()){if(tail&&h.mode==Sustain)FadeSustain(h.loop);else static_cast<void>(h.loop.Stop());}
 for(auto& s:h.shots)if(s.IsValid())static_cast<void>(s.Stop());
 logger::info("END mode={} projectiles={} launchFailures={} resourceBase={} resourcePaid={}",h.mode,h.launched,h.failed,h.paidBase,h.paidResource);h=Hand{};if(ending)PlayEnd();
}
void Reset(bool resetFlight=true){
 if(resetFlight)FlightReset();ClearSolarFlights();ClearSentries();ClearUltimateAudio();damageAuditCount=0;ClearFadingAudio();ClearBeams();ClearBursts();ClearRainImpactHistory();for(auto& h:hands)Stop(h,false);ClearRetiredCircles();circleRequests.clear();ResetFeatures();
 for(auto& s:endingSounds)if(s.IsValid())static_cast<void>(s.Stop());
}
void PurgeArt(){
 if(auto* lists=RE::ProcessLists::GetSingleton())lists->ForEachModelEffect([](RE::ModelReferenceEffect* e){
  for(auto* a:forms.flightCircle)if(a&&e->artObject==a){e->finished=true;break;}
  for(auto* a:forms.flightAscent)if(a&&e->artObject==a){e->finished=true;break;}
  for(auto* a:forms.circle)if(a&&e->artObject==a){e->finished=true;break;}
  for(auto* a:forms.legacyCircle)if(a&&e->artObject==a){e->finished=true;break;}
  for(auto* a:forms.rainMuzzle)if(a&&e->artObject==a){e->finished=true;break;}
  for(auto* a:forms.burst)if(a&&e->artObject==a){e->finished=true;break;}
  for(auto* a:forms.sentry)if(a&&e->artObject==a){e->finished=true;break;}
  for(auto* a:forms.beam)if(a&&e->artObject==a){e->finished=true;break;}
  return RE::BSContainer::ForEachResult::kContinue;
 });
}
struct DeathSink final:RE::BSTEventSink<RE::TESDeathEvent>{
 RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* e,RE::BSTEventSource<RE::TESDeathEvent>*)override{
  if(e&&e->actorDying&&e->actorDying->IsPlayerRef()){Reset();PurgeArt();}return RE::BSEventNotifyControl::kContinue;
 }
} deathSink;
Hand* GetHand(RE::ActorMagicCaster* c){
 if(!c||!c->actor||!c->actor->IsPlayerRef())return nullptr;
 auto s=c->GetCastingSource();return s==RE::MagicSystem::CastingSource::kLeftHand?&hands[0]:(s==RE::MagicSystem::CastingSource::kRightHand?&hands[1]:nullptr);
}
void StopCasterHand(RE::ActorMagicCaster* c){if(auto* h=GetHand(c))Stop(*h);}
void Interrupt(RE::ActorMagicCaster* c,bool d){originalInterrupt(c,d);StopCasterHand(c);}
void Finish(RE::ActorMagicCaster* c){originalFinish(c);StopCasterHand(c);}
void Deselect(RE::ActorMagicCaster* c){originalDeselect(c);StopCasterHand(c);}
int Identify(RE::MagicItem* spell){
 if(!spell)return -1;for(int i=0;i<8;i++)if(spell==forms.carrier[i]||spell==forms.enchant[i])return i;
 for(int i=0;i<2;i++)if(spell==forms.adaptiveSpell[i]||spell==forms.adaptiveEnchant[i])return 8+i;return -1;
}
int ReadStance(RE::Actor* a){return forms.stancesReady?StanceMode(a->HasPerk(forms.stance[0]),a->HasPerk(forms.stance[1]),a->HasPerk(forms.stance[2])):0;}
bool Begin(RE::ActorMagicCaster* c,Hand& h,int id){
 Stop(h);int side=&h==&hands[0]?0:1;h.spell=c->currentSpell;h.adaptive=id>=8;h.family=h.adaptive?id-8:id/4;
 h.mode=h.adaptive?SelectMode(h.family,ReadStance(c->actor)):id;
 h.damageMultiplier=TOAM::Core::DamageSettings::GetMultiplier();
 h.circleColor=TOAM::Presentation::Snapshot(TOAM::Presentation::modeSpell[h.mode]);for(auto& art:h.circles)art.tint.color=h.circleColor;
 const auto& other=hands[1-side];
 auto* otherCaster=c->actor->GetMagicCaster(side==0?RE::MagicSystem::CastingSource::kRightHand:RE::MagicSystem::CastingSource::kLeftHand);
 const bool otherActive=otherCaster&&OwnsHandMagic(otherCaster->currentSpell)&&other.spell==otherCaster->currentSpell&&!other.done;
 if(!CanBeginMode(h.mode,other.mode,otherActive)){
  h.done=true;h.denied=true;logger::info("ULTIMATE_DENIED mode={} hand={} owner={}",h.mode,side,1-side);return false;
 }
 h.worldAir=FlightActive();h.bank=++generations[side]%4;h.rng=0x932a5f1u+generations[side]*7237u+side*91823u;
 logger::info("BEGIN mode={} adaptive={} hand={} bank={} spell={:08X} staff={} stance={}",h.mode,h.adaptive,side,h.bank,h.spell?h.spell->GetFormID():0,IsStaff(h.spell),h.adaptive?ReadStance(c->actor):-1);return true;
}
bool IsStaff(const RE::MagicItem* item){
 if(!item)return false;for(auto* p:forms.enchant)if(item==p)return true;for(auto* p:forms.adaptiveEnchant)if(item==p)return true;return false;
}
void SessionCost(RE::ActorMagicCaster* c,Hand& h){
 if(!OwnsHandMagic(c->currentSpell)||h.mode<0||h.spell!=c->currentSpell)return;
 if(IsStaff(c->currentSpell)||h.done||h.mode!=Hermit){c->currentSpellCost=0;return;}
 // Hermit remains engine-billed. Apply the same80% reduction ceiling before
 // the original engine update, without adding a second Native Magicka debit.
 const float factor=ResourceFactor(c->actor,Hermit);
 if(!std::isfinite(factor)||factor<0){DenyCast(c,h);return;}
 const float cappedCost=float(forms.carrier[Hermit]->data.costOverride)*factor;
 if(!h.adaptive){c->currentSpellCost=cappedCost;return;}
 if(h.sessionCost<0)h.sessionCost=cappedCost;
 if(std::isfinite(h.sessionCost)&&h.sessionCost>=0)c->currentSpellCost=h.sessionCost;
}
bool CheckFromHost(HostCheckFn next,RE::ActorMagicCaster* c,RE::MagicItem* spell,bool dual,float* strength,RE::MagicSystem::CannotCastReason* reason,bool baseCost){
 RE::MagicSystem::CannotCastReason observed=RE::MagicSystem::CannotCastReason::kOK;
 const bool originalOK=next&&next(c,spell,dual,strength,&observed,baseCost);
 const int id=Identify(spell);
 if(!forms.ready||!gameReady||!c||!c->actor||!c->actor->IsPlayerRef()||id<0){if(reason)*reason=observed;return originalOK;}
 // Never override water, shout, power, multi-cast or other engine restrictions.
 if(!originalOK&&observed!=RE::MagicSystem::CannotCastReason::kMagicka&&observed!=RE::MagicSystem::CannotCastReason::kItemCharge){if(reason)*reason=observed;return false;}
 const auto* live=GetHand(c);
 const int mode=live&&live->spell==spell&&!live->done&&c->state==RE::MagicCaster::State::kCasting?
  live->mode:(id>=8?SelectMode(id-8,ReadStance(c->actor)):id);
 // Preserve vanilla Hermit denial, then add capped-cost admission even when
 // vanilla CalculateMagickaCost returns0 from100% equipment reduction.
 if(mode==Hermit&&!IsStaff(spell)&&!originalOK){if(reason)*reason=observed;return false;}
 // Admission only: exact billing is committed in the owning Hand at its trigger.
 const bool staff=IsStaff(spell);float available=0;
 if(!staff&&RE::PlayerCharacter::IsGodMode()){if(reason)*reason=RE::MagicSystem::CannotCastReason::kOK;return true;}
 if(staff){
  if(spell!=c->currentSpell){if(reason)*reason=observed;return originalOK;}
  const auto resource=StaffInstance(c);auto* list=resource.instance;auto* weapon=resource.weapon;
  if(!list||!weapon){if(reason)*reason=RE::MagicSystem::CannotCastReason::kItemCharge;return false;}
  auto* charge=list->GetByType<RE::ExtraCharge>();available=charge?charge->charge:float(weapon->amountofEnchantment);
 }else available=c->actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagicka);
 const float factor=ResourceFactor(c->actor,mode);
 const float first=mode==Rapid?modes[Rapid].cost:mode==Fivefold?modes[Fivefold].cost/5.f:mode==SolarCrown?modes[SolarCrown].cost/ultimateChargeSeconds*.01f:mode==Sustain?3.f:mode==Sentry?modes[Sentry].cost:mode==Tracking?modes[Tracking].cost:mode==Rain?modes[Rain].cost:modes[Hermit].cost*.01f;
 const bool prepaid=live&&live->spell==spell&&!live->done&&c->state==RE::MagicCaster::State::kCasting&&
  ((live->mode==Rain&&live->resourceStarted)||(live->mode==SolarCrown&&live->paidBase>=modes[SolarCrown].cost-.001f));
 const bool ok=std::isfinite(factor)&&factor>=0&&std::isfinite(available)&&(!staff||available>0||prepaid)&&
  (prepaid||available+.0001f>=first*factor);
 if(reason)*reason=ok?RE::MagicSystem::CannotCastReason::kOK:(staff?RE::MagicSystem::CannotCastReason::kItemCharge:RE::MagicSystem::CannotCastReason::kMagicka);
 return ok;
}
void DenyCast(RE::ActorMagicCaster* c,Hand& h){
 const int mode=h.mode;auto* spell=c->currentSpell;c->currentSpellCost=0;
 c->InterruptCast(false); // Whole caster lifecycle, rather than only slot08's callback.
 h.spell=spell;h.mode=mode;h.done=true;h.denied=true;c->currentSpellCost=0;
}
void Request(RE::ActorMagicCaster* c){
 if(auto* h=GetHand(c)){int id=Identify(c->currentSpell);if(gameReady&&forms.ready&&id>=0){if(!Begin(c,*h,id)){DenyCast(c,*h);return;}SessionCost(c,*h);}}
 originalRequest(c);if(auto* h=GetHand(c))SessionCost(c,*h);
}
void Start(RE::ActorMagicCaster* c){
 if(auto* h=GetHand(c)){int id=Identify(c->currentSpell);if(gameReady&&forms.ready&&id>=0){
  if((h->denied&&h->spell==c->currentSpell)||(h->spell!=c->currentSpell&&!Begin(c,*h,id))){DenyCast(c,*h);return;}
  SessionCost(c,*h);
 }}
 originalStart(c);if(auto* h=GetHand(c))SessionCost(c,*h);
}
void Complete(Hand& h,int side,RE::Actor* actor){
 for(int i=0;i<12;i++)RetireHandCircle(h,h.circles[i],i,side,true,actor);if(h.loop.IsValid())static_cast<void>(h.loop.Stop());
 ClearPreparedBursts(side);StopBeams(side,true);StopRainShots(side,true,actor);ClearGuard(side);ClearSlows(side);h.done=true;
}
#include "HandUpdate.inl"
template<class T>T* Get(std::uint32_t id){
 const auto mapped=ResolveSourceForm(id);auto* dh=RE::TESDataHandler::GetSingleton();
 return mapped&&dh?dh->LookupForm<T>(mapped,plugin):nullptr;
}
void LoadForms(){
 forms.ready=true;
 for(int i=0;i<2;i++){forms.hostAimCarrier[i]=RE::TESDataHandler::GetSingleton()->LookupForm<RE::SpellItem>(0x800+i,plugin);forms.ready&=forms.hostAimCarrier[i]!=nullptr;}
 for(int i=0;i<8;i++){forms.carrier[i]=Get<RE::SpellItem>(0x800+i);forms.enchant[i]=Get<RE::EnchantmentItem>(0x810+i);forms.payload[i]=Get<RE::SpellItem>(0x820+i);forms.ready&=forms.carrier[i]&&forms.enchant[i]&&forms.payload[i];}
 for(int i=0;i<32;i++){forms.legacyCircle[i]=Get<RE::BGSArtObject>(i<16?0x850+i:0x8E0+i-16);}
 for(int i=0;i<96;i++){forms.beam[i]=Get<RE::BGSArtObject>(0x900+i);forms.ready&=forms.beam[i]!=nullptr;}
 for(int i=0;i<168;i++){int id=i<96?0x980+i:(i<160?0xA00+i-96:0xA50+i-160);forms.circle[i]=Get<RE::BGSArtObject>(id);forms.ready&=forms.circle[i]!=nullptr;}
 // Tower sky uses separate bank-owned ARTOs; shared circles retain their original rim depth.
 for(int i=0;i<8;i++){forms.circle[168+i]=RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSArtObject>(0xDC6+i,plugin);forms.ready&=forms.circle[168+i]!=nullptr;}
 for(int i=0;i<192;i++){forms.rainMuzzle[i]=Get<RE::BGSArtObject>(0xB00+i);forms.ready&=forms.rainMuzzle[i]!=nullptr;}
 for(int i=0;i<24;i++){forms.burst[i]=Get<RE::BGSArtObject>(0xC00+i);forms.ready&=forms.burst[i]!=nullptr;}
 for(int i=0;i<4;i++){forms.burst[24+i]=Get<RE::BGSArtObject>(0xC44+i);forms.ready&=forms.burst[24+i]!=nullptr;}
 for(int i=0;i<12;i++){forms.sentry[i]=Get<RE::BGSArtObject>(0xC30+i);forms.ready&=forms.sentry[i]!=nullptr;}
 for(int i=0;i<11;i++){forms.sound[i]=Get<RE::BGSSoundDescriptorForm>(i<9?0x860+i:0xC20+i-9);forms.ready&=forms.sound[i]!=nullptr;}
 forms.sound[11]=Get<RE::BGSSoundDescriptorForm>(0xC40);forms.ready&=forms.sound[11]!=nullptr;
 forms.sound[12]=Get<RE::BGSSoundDescriptorForm>(0xC43);forms.ready&=forms.sound[12]!=nullptr;
 for(int i=0;i<4;i++){forms.flightCircle[i]=Get<RE::BGSArtObject>(0xC4D+i);forms.ready&=forms.flightCircle[i]!=nullptr;}
 for(int i=0;i<4;i++){forms.flightAscent[i]=RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSArtObject>(0xDC1+i,plugin);forms.ready&=forms.flightAscent[i]!=nullptr;}
 forms.flightPower=Get<RE::SpellItem>(0xC4B);forms.flightMarker=Get<RE::SpellItem>(0xC4C);forms.ready&=forms.flightPower&&forms.flightMarker;FlightDataLoaded();
 forms.slow=Get<RE::SpellItem>(0x960);forms.ready&=forms.slow!=nullptr;
 forms.solarContact=Get<RE::SpellItem>(0xC42);forms.ready&=forms.solarContact!=nullptr;
 forms.sentryContact=Get<RE::SpellItem>(0xC49);forms.ready&=forms.sentryContact!=nullptr;
 auto* dh=RE::TESDataHandler::GetSingleton();
 for(int i=0;i<2;i++){forms.adaptiveSpell[i]=dh->LookupForm<RE::SpellItem>(0x800+i,"TULL Ordinary Attack Magic - Stances Addon.esp");forms.adaptiveEnchant[i]=dh->LookupForm<RE::EnchantmentItem>(0x810+i,"TULL Ordinary Attack Magic - Stances Addon.esp");}
 for(int i=0;i<3;i++)forms.stance[i]=dh->LookupForm<RE::BGSPerk>(0x4251A-i,"Stances - Dynamic Weapon Movesets SE.esp");
 forms.stancesReady=forms.stance[0]&&forms.stance[1]&&forms.stance[2];
 if(auto* e=RE::ScriptEventSourceHolder::GetSingleton())e->AddEventSink<RE::TESDeathEvent>(&deathSink);
 auto count=[](const auto& list){std::size_t n=0;for(auto* p:list)if(p)++n;return n;};
 logger::info("ARCANA_FORM_GROUPS plugin={} carrier={}/8 enchant={}/8 payload={}/8 beam={}/96 circle={}/176 muzzle={}/192 burst={}/28 sentry={}/12 sound={}/13 flight={}/4",
  plugin,count(forms.carrier),count(forms.enchant),count(forms.payload),count(forms.beam),count(forms.circle),count(forms.rainMuzzle),count(forms.burst),count(forms.sentry),count(forms.sound),count(forms.flightCircle));
 logger::info("FORMS ready={} addonFirstClass={} addonMillennial={} stances={}",forms.ready,forms.adaptiveSpell[0]!=nullptr,forms.adaptiveSpell[1]!=nullptr,forms.stancesReady);
}
void Message(SKSE::MessagingInterface::Message* m){
 if(!m)return;switch(m->type){case SKSE::MessagingInterface::kDataLoaded:{static bool loaded=false;if(!loaded){loaded=true;LoadForms();}}break;
 case SKSE::MessagingInterface::kPreLoadGame:gameReady=false;Reset();PurgeArt();break;
 case SKSE::MessagingInterface::kNewGame:Reset();PurgeArt();gameReady=true;FlightScheduleReady();break;
 case SKSE::MessagingInterface::kPostLoadGame:Reset(false);PurgeArt();gameReady=m->data!=nullptr;FlightScheduleReady();break;default:break;}
}
