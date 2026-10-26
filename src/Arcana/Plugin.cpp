#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <array>
#include <vector>
#include <memory>
#include <cmath>
#include <chrono>
#include <mutex>
#include <atomic>
#include <unordered_map>
#undef min
#undef max
#include "Design.h"
#include "PlacementPolicy.h"
#include "SentryPolicy.h"
#include "FlightPolicy.h"
#include "FlightCombatPolicy.h"
#include "FlightLegPoseData.h"
#include <cstring>
#include "Integration.h"
#include "Core/DamageSettings.h"
#include "Core/PresentationSettings.h"
#include "VFX/CircleTint.h"
#include "VFX/CircleMotion.h"
#include "IntegratedFormMap.h"
using namespace std::literals;
namespace logger=SKSE::log;
#ifndef ARCANA_EMBEDDED
SKSEPluginInfo(.Version=REL::Version{0,7,4,0},.Name="ArcanaStaffs"sv,.Author="Arcana Workshop"sv,
 .StructCompatibility=SKSE::StructCompatibility::Dependent,.RuntimeCompatibility=SKSE::VersionIndependence::AddressLibrary,
 .MinimumSKSEVersion=REL::Version{2,0,20,0})
#endif
namespace Arcana {
#ifdef ARCANA_EMBEDDED
std::string plugin="TULL Ordinary Attack Magic.esp";
#else
std::string plugin="ArcanaStaffs.esp";
#endif
std::uint32_t formOffset=0;
std::uint32_t ResolveSourceForm(std::uint32_t id) {
#ifdef ARCANA_EMBEDDED
 return IntegratedForms::Map(id);
#else
 return id+formOffset;
#endif
}
struct Forms {
 std::array<RE::SpellItem*,8> carrier{},payload{};
 std::array<RE::SpellItem*,2> hostAimCarrier{}; // Presentation admission only; never Arcana resource owners.
 std::array<RE::EnchantmentItem*,8> enchant{};
 std::array<RE::SpellItem*,2> adaptiveSpell{};std::array<RE::EnchantmentItem*,2> adaptiveEnchant{};
 std::array<RE::BGSPerk*,3> stance{};bool stancesReady=false;
 std::array<RE::BGSArtObject*,32> legacyCircle{};std::array<RE::BGSArtObject*,96> beam{};std::array<RE::BGSArtObject*,176> circle{};
 std::array<RE::BGSArtObject*,192> rainMuzzle{};std::array<RE::BGSArtObject*,28> burst{};std::array<RE::BGSArtObject*,12> sentry{};
 std::array<RE::BGSSoundDescriptorForm*,13> sound{};std::array<RE::BGSArtObject*,4> flightCircle{},flightAscent{};
 RE::SpellItem* flightPower{};RE::SpellItem* flightMarker{};RE::SpellItem* slow{};RE::SpellItem* solarContact{};RE::SpellItem* sentryContact{};bool ready=false;
} forms;
struct ArtSlot {
 TOAM::VFX::CircleTint::State tint{};
 RE::NiPointer<RE::ModelReferenceEffect> effect{};RE::BGSArtObject* form{};
 RE::NiPoint3 pos{};RE::NiMatrix3 rot{};float scale=.001f,alpha=0;bool requested=false;
 float motionAge=0,spinPhase=0;
 float presentationAge=0;bool firstPose=false; // Sun-only growth clock starts at actual 3D pose.
};
struct TrackLane {
 RE::ActorHandle target{};RE::NiPoint3 position{},previous{};
 float age=0,wait=0;bool active=false,fired=false,hasPrevious=false;
};
struct Hand {
 TOAM::Presentation::Color circleColor{TOAM::Presentation::Color::Default};
 RE::MagicItem* spell{};int mode=-1,family=0,bank=0;Cadence cadence{};FivefoldCadence fivefold{};
 std::array<ArtSlot,12> circles{};std::array<TrackLane,5> lanes{};std::vector<RE::ActorHandle> targets{};
 RE::NiPoint3 field{},sky{};bool fieldSet=false;
 bool adaptive=false,done=false,releasePlayed=false,soundStarted=false,parryArmed=false,denied=false,rainStarted=false,rainDetonated=false;
 float damageMultiplier=1; // Damage-only cast snapshot; never used by billing.
 float sessionCost=-1,nextShake=0,scanTimer=0,slowTimer=0,rainNext=rainFirstShotSeconds,rainDetonationAt=0;
 bool resourceStarted=false,engineUpdateSeen=false;float paidBase=0,paidResource=0;unsigned resourceAudits=0;
 RE::BSSoundHandle loop{};std::array<RE::BSSoundHandle,16> shots{};
 unsigned soundSlot=0,launched=0,failed=0,rainFired=0,rainQueued=0,rainWave=0,rng=1;
 bool rainFinalePrepared=false;bool worldAir=false;bool worldBlastStarted=false;float worldBlastAt=0,worldScan=0;int worldBurstsShown=0;std::vector<std::uint32_t> worldHitIDs{};
};
std::array<Hand,2> hands{};std::array<unsigned,2> generations{};std::atomic_bool gameReady{false};
std::array<RE::BSSoundHandle,4> endingSounds{};unsigned endingSlot=0;
REL::Relocation<void(*)(RE::ActorMagicCaster*,float)> original;
REL::Relocation<void(*)(RE::ActorMagicCaster*,bool)> originalInterrupt;
REL::Relocation<void(*)(RE::ActorMagicCaster*)> originalFinish,originalDeselect,originalRequest,originalStart;

double Clock(){return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}
RE::NiMatrix3 AimRotation(float pitch,float heading){
 const float cp=std::cos(pitch),sp=std::sin(pitch),ch=std::cos(heading),sh=std::sin(heading);RE::NiMatrix3 r;
 r.entry[0][0]=ch;r.entry[1][0]=-sh;r.entry[2][0]=0;
 r.entry[0][1]=sh*cp;r.entry[1][1]=ch*cp;r.entry[2][1]=-sp;
 r.entry[0][2]=sh*sp;r.entry[1][2]=ch*sp;r.entry[2][2]=cp;return r;
}
RE::NiMatrix3 DirectionRotation(const RE::NiPoint3& d){return AimRotation(-std::atan2(d.z,std::hypot(d.x,d.y)),std::atan2(d.x,d.y));}
void Sound(RE::BSSoundHandle& h,RE::BGSSoundDescriptorForm* descriptor,RE::NiAVObject* follow,float volume,TOAM::Presentation::Event event){
 if(h.IsValid())static_cast<void>(h.Stop());h=RE::BSSoundHandle{};
 const auto choice=TOAM::Presentation::Resolve(event,volume);
 const bool silentStart=event==TOAM::Presentation::Event::FoolHum&&volume==0&&TOAM::Presentation::Resolve(event,1).volume>0;
 if(choice.volume<=0&&!silentStart)return;
 if(auto* selected=TOAM::Presentation::Descriptor(choice.id))descriptor=selected;
 auto* mgr=RE::BSAudioManager::GetSingleton();
 if(!mgr||!descriptor||!follow||!RE::BSAudioManager::QPlatformInstance())return;if(!mgr->GetSoundHandle(h,descriptor))return;
 h.SetObjectToFollow(follow);if(!h.SetVolume(choice.volume)||!h.Play()){static_cast<void>(h.Stop());h=RE::BSSoundHandle{};}
}
struct CircleRequestTicket {RE::ActorHandle actor{};double deadline=0,reuseAfter=0;unsigned searches=0;bool acquired=false;};
std::unordered_map<RE::BGSArtObject*,CircleRequestTicket> circleRequests{};
bool RequestCircleArt(RE::Actor* actor,RE::BGSArtObject* art,RE::NiAVObject* node,float duration=2.f){
 if(!actor||!art||!node)return false;
 auto previous=circleRequests.find(art);
 if(previous!=circleRequests.end()&&!previous->second.acquired&&Clock()<previous->second.reuseAfter)return false;
 // SE1.5.97 ID22289 returns BOOL; acquire ref-counted objects only from ProcessLists.
 using Fn=bool(*)(RE::TESObjectREFR*,RE::BGSArtObject*,float,RE::TESObjectREFR*,bool,bool,RE::NiAVObject*,bool);
 static REL::Relocation<Fn> apply{REL::RelocationID(22289,22769)};
 const bool requested=apply(actor,art,std::min(duration,2.f),nullptr,false,false,node,false);
 if(requested)circleRequests[art]={actor->GetHandle(),Clock()+1.2,Clock()+2.2,0,false};
 return requested;
}
void MoveArt(RE::ModelReferenceEffect* e,const RE::NiPoint3& p,const RE::NiMatrix3& r,float scale){
 if(!e||e->finished)return;auto* root=e->Get3D();if(!root)return;
 static const RE::BSFixedString name{"AS_CirclePose"};auto* o=root->GetObjectByName(name);if(!o||!o->parent)return;
 const auto& t=o->parent->world;if(!std::isfinite(t.scale)||std::abs(t.scale)<.001f)return;
 e->lifetime=e->age+2.f; // An acquired, valid pose is renewed only while explicitly owned.
 auto inv=t.rotate.Transpose();o->local.translate=inv*(p-t.translate)/t.scale;o->local.rotate=inv*r;o->local.scale=scale/t.scale;
 RE::NiUpdateData u{};root->Update(u);
}
void Complete(Hand&,int,RE::Actor*);
#include "AudioPresentation.inl"
#include "CirclePresentation.inl"
#include "BeamPresentation.inl"
void Shake(RE::Actor*,float,float);
#include "BurstPresentation.inl"
#include "RainImpact.inl"
#include "Balance.inl"
#include "ArsenalFeatures.inl"
#include "Casting.inl"
void ConfigureFormSource(const char* name,std::uint32_t offset){
 if(forms.ready||gameReady){logger::error("Form source must be configured before DataLoaded");return;}
 #ifdef ARCANA_EMBEDDED
 if(!name||std::strcmp(name,"TULL Ordinary Attack Magic.esp")!=0||offset!=0){logger::critical("Invalid integrated form source");return;}
#endif
 if(name&&*name){plugin=name;formOffset=offset;}
}
bool OwnsHandMagic(RE::MagicItem* spell) noexcept {return forms.ready&&Identify(spell)>=0;}
// Actor::Update(float), pinned SE primary virtual slot0xAD. Sentries keep running after cast completion.
REL::Relocation<void(*)(RE::PlayerCharacter*,float)> originalPlayerFrame;
void BeforePlayerFrame(RE::PlayerCharacter* player){
 FlightRestoreRangedLegs(player);
 if(gameReady&&forms.ready){FlightGuardGravity(player);FlightGunUpdateMarkers(player);}
 if(gameReady&&forms.ready)FlightInstallNotifyGuard(player);
}
void AfterPlayerFrame(RE::PlayerCharacter* player,float dt){
 if(!gameReady||!forms.ready||!player)return;
 static RE::TESObjectCELL* observedCell=nullptr;
 auto* cell=player->GetParentCell();
 if(observedCell&&cell!=observedCell){Reset();PurgeArt();}
 observedCell=cell;
 FlightFrame(player,dt);FlightUpdateRangedLegs(player,dt);
 for(int side=0;side<2;side++)if(auto* caster=player->GetMagicCaster(side==0?RE::MagicSystem::CastingSource::kLeftHand:RE::MagicSystem::CastingSource::kRightHand)){
  UpdateSentries(side,caster,player,dt);UpdateBeams(side,player,dt);UpdateRetiredCircles(side,player,dt);UpdateRetiredCircles(side,player,dt,true);UpdateBursts(side,player,dt);
 }
 UpdateSolarFlights(player);UpdateRainImpacts(player);
}
void PlayerFrame(RE::PlayerCharacter* player,float dt){
 BeforePlayerFrame(player);originalPlayerFrame(player,dt);AfterPlayerFrame(player,dt);
}
RE::NiPoint3 HostActorCenter(RE::Actor* actor){return actor?Center(actor):RE::NiPoint3{};}
float HostDomeGrowth(std::size_t bank,float fraction){
 for(int j=0;j<12;++j)if(std::size_t(worldOrder[j])==bank)
  return std::max(.02f,WorldCircleGrowth(j,std::clamp(fraction,0.f,1.f)*worldChargeSeconds));
 return .02f;
}
float HostSunScale(std::size_t layer){return layer<8?UltimateScale(int(layer),ultimateChargeSeconds):0.f;}
bool HostAimDirection(RE::PlayerCharacter* player,RE::NiPoint3& direction) noexcept {
 if(!player)return false;const float pitch=player->GetAimAngle(),heading=player->GetAimHeading();
 if(!std::isfinite(pitch)||!std::isfinite(heading))return false;
 direction=AimRotation(pitch,heading)*RE::NiPoint3{0,1,0};return true;
}
RE::NiPoint3 HostSunAnchor(RE::PlayerCharacter* player){
 if(!player)return {};const auto& point=LayoutData::ultimate[0];
 const auto aim=AimRotation(player->GetAimAngle(),player->GetAimHeading());
 const auto flat=AimRotation(0,player->GetAimHeading());
 return FlightCastOffset(player->GetPosition(),flat,aim,RE::NiPoint3{point.x,point.y,point.z});
}
RE::NiPoint3 HostJudgementGuideAnchor(RE::PlayerCharacter* player,bool left){
 if(!player)return {};const auto aim=AimRotation(player->GetAimAngle(),player->GetAimHeading());
 const auto flat=AimRotation(0,player->GetAimHeading());
 // CP027: shoulder-height emitter, slightly forward; no hand animation or VFX bounds.
 return FlightCastOffset(player->GetPosition(),flat,aim,RE::NiPoint3{left?-34.f:34.f,22.f,126.f});
}
bool HostDomePose(std::size_t index,RE::PlayerCharacter* player,RE::NiPoint3& position,RE::NiPoint3& direction,float& scale,bool airborne){
 if(index>=12||!player)return false;const auto& p=LayoutData::hermit[index];const auto& n=LayoutData::hermitNormals[index];
 auto flat=AimRotation(0,player->GetAngleZ());
 if(airborne){
  int deployment=0;for(int j=0;j<12;++j)if(std::size_t(worldOrder[j])==index){deployment=j;break;}
  const auto air=AirWorldNormal(deployment);direction=flat*RE::NiPoint3{air.x,air.y,air.z};
  position=player->GetPosition()+RE::NiPoint3{0,0,78}+direction*112.f;
 }else{
  position=player->GetPosition()+flat*RE::NiPoint3{p.x*worldDomeScale,p.y*worldDomeScale,70+(p.z-70)*worldDomeScale};
  direction=flat*RE::NiPoint3{n.x,n.y,n.z};
 }
 scale=worldCircleScale;return true;
}
void BindHostRequest(HostRequestFn next){originalRequest=reinterpret_cast<std::uintptr_t>(next);}
void RequestFromHost(RE::ActorMagicCaster* caster){
 if(originalRequest.address()==0){logger::critical("HOST_REQUEST_UNBOUND");return;}Request(caster);
}
bool InstallHooks(){
 static bool installed=false;if(installed)return true;
 if(REL::Module::get().version()!=SKSE::RUNTIME_SSE_1_5_97)return false;
 REL::Relocation<std::uintptr_t> t{RE::VTABLE_ActorMagicCaster[0]};
 Arcana::original=t.write_vfunc(0x1D,Arcana::Update);
#ifndef ARCANA_EMBEDDED
 Arcana::originalRequest=t.write_vfunc(0x03,Arcana::Request);
#endif
 Arcana::originalStart=t.write_vfunc(0x06,Arcana::Start);
 Arcana::originalInterrupt=t.write_vfunc(0x08,Arcana::Interrupt);Arcana::originalFinish=t.write_vfunc(0x07,Arcana::Finish);Arcana::originalDeselect=t.write_vfunc(0x12,Arcana::Deselect);
#ifndef ARCANA_EMBEDDED
 REL::Relocation<std::uintptr_t> playerTable{RE::VTABLE_PlayerCharacter[0]};originalPlayerFrame=playerTable.write_vfunc(0xAD,PlayerFrame);
#endif
 InstallDomeCommitHook();InstallRainImpactHook();InstallFlightInputHooks();
 installed=true;logger::info("ArcanaStaffs 0.7.4 hooks installed; runtime ActorState accessor repaired; firearm flight HKX and six-leg pose; no Papyrus");return true;
}
}
#ifndef ARCANA_EMBEDDED
SKSEPluginLoad(const SKSE::LoadInterface* skse){
 SKSE::Init(skse);auto dir=logger::log_directory();if(!dir)return false;*dir/="ArcanaStaffs.log";
 auto log=std::make_shared<spdlog::logger>("ArcanaStaffs",std::make_shared<spdlog::sinks::basic_file_sink_mt>(dir->string(),true));
 spdlog::set_default_logger(log);spdlog::set_level(spdlog::level::info);spdlog::flush_on(spdlog::level::info);
 if(REL::Module::get().version()!=SKSE::RUNTIME_SSE_1_5_97){logger::error("SE 1.5.97 required");return false;}
 auto* m=SKSE::GetMessagingInterface();if(!m||!m->RegisterListener(Arcana::Message))return false;
 return Arcana::InstallHooks();

}

#endif
