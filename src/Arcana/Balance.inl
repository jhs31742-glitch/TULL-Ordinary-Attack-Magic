// CP021: this file owns Arcana billing only. Never call TOAM charge/limiter APIs.
// The original concentration update receives currentSpellCost=0 for manually
// billed modes. Hermit's ordinary spell keeps its original engine payment path.
bool IsStaff(const RE::MagicItem*);
void DenyCast(RE::ActorMagicCaster*,Hand&);
float ResourceFactor(RE::Actor* actor,int mode){
 if(!actor||mode<0||mode>=8||!forms.carrier[mode])return -1;
 auto* probe=forms.carrier[mode];const float base=float(probe->data.costOverride);
 const float final=probe->CalculateMagickaCost(actor);
 return base>0&&std::isfinite(final)&&final>=0?CapArcanaCostFactor(final/base):-1;
}
struct StaffResource {RE::TESObjectWEAP* weapon{};RE::ExtraDataList* instance{};bool left{};};
StaffResource StaffInstance(RE::ActorMagicCaster* c){
 if(!c||!c->actor||!IsStaff(c->currentSpell))return {};
 const auto hand=c->GetCastingSource();
 if(hand!=RE::MagicSystem::CastingSource::kLeftHand&&hand!=RE::MagicSystem::CastingSource::kRightHand)return {};
 const bool left=hand==RE::MagicSystem::CastingSource::kLeftHand;
 auto* entry=c->actor->GetEquippedEntryData(left);
 auto* weapon=entry&&entry->object?entry->object->As<RE::TESObjectWEAP>():nullptr;
 if(!weapon||!weapon->IsStaff()||weapon->formEnchanting!=c->currentSpell||!entry->extraLists)return {};
 for(auto* list:*entry->extraLists)if(list&&(left?list->HasType<RE::ExtraWornLeft>():list->HasType<RE::ExtraWorn>()))
  return {weapon,list,left};
 return {}; // One verified entry snapshot: never bill another copy or hand.
}
void RefreshStaffCharge(RE::Actor* actor,const StaffResource& staff,float charge){
 // SE37803 performs both charge display synchronization AND UpdateWeaponAbility.
 // Payment is not equip/unequip: rebuilding abilities inside caster Update can
 // reenter the same casting/animation lifecycle. Match the native charge AV
 // selector (RVA556780: left=82, right=64) and raw ExtraCharge units instead.
 if(auto* owner=actor?actor->AsActorValueOwner():nullptr)
  owner->SetActorValue(staff.left?RE::ActorValue::kLeftItemCharge:RE::ActorValue::kRightItemCharge,charge);
}
bool PayResource(RE::ActorMagicCaster* c,Hand& h,float base){
 if(!c||!c->actor||h.spell!=c->currentSpell||!std::isfinite(base)||base<0)return false;
 const bool staff=IsStaff(h.spell);float factor=ResourceFactor(c->actor,h.mode);
 if(!std::isfinite(factor)||factor<0)return false;
 const float amount=base*factor;float before=0,after=0,committed=amount;
 const bool engineGodMode=!staff&&c->actor->IsPlayerRef()&&RE::PlayerCharacter::IsGodMode();
 if(staff){
  const auto staffResource=StaffInstance(c);auto* list=staffResource.instance;auto* weapon=staffResource.weapon;
  if(!list||!weapon){logger::warn("ARCANA_RESOURCE_FAIL hand={} reason=STAFF_INSTANCE",int(&h==&hands[1]));return false;}
  auto* charge=list->GetByType<RE::ExtraCharge>();
  before=charge?charge->charge:float(weapon->amountofEnchantment);
  // An empty staff always needs a soul gem, including at a zero discount cost.
  if(!std::isfinite(before)||before<=0||before+0.0001f<amount)return false;
  if(amount>0){
   if(!charge){charge=new RE::ExtraCharge();charge->charge=before;list->Add(charge);
    if(list->GetByType<RE::ExtraCharge>()!=charge)return false;}
   charge->charge=std::max(0.f,before-amount);
   c->actor->AddChange(RE::TESObjectREFR::ChangeFlags::kInventory);
   RefreshStaffCharge(c->actor,staffResource,charge->charge);
  }
  after=amount>0?charge->charge:before;
 }else{
  auto* owner=c->actor->AsActorValueOwner();if(!owner)return false;
  before=owner->GetActorValue(RE::ActorValue::kMagicka);
  if(!std::isfinite(before)||(!engineGodMode&&before+0.0001f<amount))return false;
  // Compare the actual damage modifier as well as the displayed/cached total.
  // Never retry a debit or force-set Magicka: either can double-bill other hands.
  const float modifierBefore=c->actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kDamage,RE::ActorValue::kMagicka);
  const bool godMode=engineGodMode;if(godMode)committed=0;
  if(amount>0&&!godMode)owner->DamageActorValue(RE::ActorValue::kMagicka,amount);
  after=owner->GetActorValue(RE::ActorValue::kMagicka);
  const float modifierAfter=c->actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kDamage,RE::ActorValue::kMagicka);
  if(!godMode&&std::abs((before-after)-amount)>.05f&&std::abs((modifierBefore-modifierAfter)-amount)>.05f){
   logger::error("ARCANA_RESOURCE_READBACK_FAIL hand={} requested={} before={} after={} damageBefore={} damageAfter={} godMode={}",
     int(&h==&hands[1]),amount,before,after,modifierBefore,modifierAfter,godMode);return false;
  }
  if(godMode&&h.resourceAudits==0)logger::info("ARCANA_RESOURCE_GOD_MODE hand={} engineProtected=true",int(&h==&hands[1]));
 }
 h.paidBase+=base;h.paidResource+=committed;
 if(h.resourceAudits++<4)logger::info("ARCANA_RESOURCE hand={} mode={} staff={} base={} factor={} paid={} before={} after={} engineCost={} MPForStaff=0",
  int(&h==&hands[1]),h.mode,staff,base,factor,amount,before,after,c->currentSpellCost);
 return true;
}
bool StartResource(RE::ActorMagicCaster* c,Hand& h,float base){
 if(h.resourceStarted)return true;
 if(!PayResource(c,h,base))return false;h.resourceStarted=true;return true;
}
