// One detached heavy bolt sweeps a finite volume. Each hostile actor is affected once.
// Damage uses a dedicated targeted native Value Modifier spell, never raw Health subtraction.
struct SolarFlight {bool active=false,impactPlayed=false;int beam=-1,side=0;RE::NiPoint3 impactPoint{};RE::NiMatrix3 impactFacing{};std::uint64_t serial=0;float previous=0;std::array<std::uint32_t,64> hit{};unsigned hitCount=0;};
std::array<SolarFlight,4> solarFlights{};
void ClearSolarFlights(){solarFlights={};}
bool StartSolarFlight(int slot,RE::Actor* actor,RE::NiAVObject* root){
 for(int i=0;i<solarImpactSlots;i++)if(!solarFlights[i].active){
  auto& flight=solarFlights[i];flight=SolarFlight{};flight.active=true;flight.beam=slot;flight.side=slot/beamsPerHand;auto& beam=beamSlots[slot];flight.serial=beam.serial;
  if(beam.geometryHit){auto normal=beam.impactNormal,axis=beam.rotation*RE::NiPoint3{0,1,0};if(!Finite(normal)||Length(normal)<.001f)normal=axis*-1.f;normal=Unit(normal);
   flight.impactPoint=beam.origin+axis*beam.length+normal*8;flight.impactFacing=DirectionRotation(normal);ShowSolarImpact(i,flight.side,actor,root,flight.impactPoint,flight.impactFacing,true);
  }
  return true;
 }
 logger::warn("SOLAR_FLIGHT_POOL_FULL");return false;
}
void UpdateSolarFlights(RE::Actor* source){
 if(!gameReady||!forms.ready||!source||source->IsDead()||!source->Is3DLoaded())return;
 if(auto* ui=RE::UI::GetSingleton();ui&&ui->GameIsPaused())return;
 for(int flightIndex=0;flightIndex<solarImpactSlots;flightIndex++){
  auto& flight=solarFlights[flightIndex];if(!flight.active)continue;auto& beam=beamSlots[flight.beam];
  if(!beam.active||beam.mode!=SolarCrown||beam.serial!=flight.serial||beam.age>beam.length/BoltSpeed(SolarCrown)+BoltHold(SolarCrown)){ClearSolarImpactPreparation(flightIndex);flight=SolarFlight{};continue;}
  if(SolarTerrainImpactDue(beam.geometryHit,beam.age,beam.length,flight.impactPlayed)){
   flight.impactPlayed=true;if(auto* root=source->Get3D(false)){ShowSolarImpact(flightIndex,flight.side,source,root,flight.impactPoint,flight.impactFacing,false);logger::info("SOLAR_TERRAIN_VISUAL_REQUEST point=({},{},{}) distance={} extraDamage=0",flight.impactPoint.x,flight.impactPoint.y,flight.impactPoint.z,beam.length);}
  }
  const auto axis=beam.rotation*RE::NiPoint3{0,1,0};const auto segment=BeamPulseState(SolarCrown,beam.length,beam.age);
  std::array<RE::ActorHandle,64> contacts{};std::array<float,64> contactAlong{};unsigned contactCount=0;
  if(auto* list=RE::ProcessLists::GetSingleton())list->ForEachHighActor([&](RE::Actor* target){
   if(!target||target==source||target->IsDead()||!target->Is3DLoaded()||!target->IsHostileToActor(source)||flight.hitCount+contactCount>=flight.hit.size())return RE::BSContainer::ForEachResult::kContinue;
   for(unsigned i=0;i<flight.hitCount;i++)if(flight.hit[i]==target->GetFormID())return RE::BSContainer::ForEachResult::kContinue;
   auto center=Center(target),delta=center-beam.origin;float along=delta.x*axis.x+delta.y*axis.y+delta.z*axis.z;
   auto lateral=delta-axis*along;float radial=lateral.x*lateral.x+lateral.y*lateral.y+lateral.z*lateral.z;
   if(!SolarSweepContact(along,radial,flight.previous,segment.head,segment.tail))return RE::BSContainer::ForEachResult::kContinue;
   // Stop at the authored world/static hit and reject cover beside the thick beam.
   if(along>beam.length+24)return RE::BSContainer::ForEachResult::kContinue;
   auto nearest=beam.origin+axis*std::clamp(along,0.f,beam.length);auto cross=center-nearest;float crossLength=Length(cross);
   if(crossLength>1&&BeamLength(source,nearest,Unit(cross),true)+8<crossLength)return RE::BSContainer::ForEachResult::kContinue;
   auto handle=target->GetHandle();for(unsigned i=0;i<contactCount;i++)if(contacts[i]==handle)return RE::BSContainer::ForEachResult::kContinue;
   contacts[contactCount]=handle;contactAlong[contactCount++]=along;
   return RE::BSContainer::ForEachResult::kContinue;
  });
  // Finish process-list iteration before an engine spell can kill/move an actor.
  for(unsigned i=0;i<contactCount;i++){
   if(!flight.active||source->IsDead())break;auto target=contacts[i].get();if(!target||target->IsDead())continue;
   if(auto* caster=source->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)){
    flight.hit[flight.hitCount++]=target->GetFormID();int previousMode=currentDamageMode;currentDamageMode=SolarCrown;
    caster->CastSpellImmediate(forms.solarContact,true,target.get(),1,true,modes[SolarCrown].damage,source);currentDamageMode=previousMode;
    logger::info("SOLAR_CONTACT_REQUEST target={:08X} along={} head={} nominalDamage={} oncePerBolt=true",target->GetFormID(),contactAlong[i],segment.head,modes[SolarCrown].damage);
   }
  }
  flight.previous=segment.head;
 }
}
