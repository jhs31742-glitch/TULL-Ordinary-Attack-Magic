#pragma once
#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>
#include <cstdint>
namespace Arcana {
// Host integration: compile Plugin.cpp with ARCANA_EMBEDDED=1.
// Configure before DataLoaded. Offsets rebase this module's documented local FormIDs.
void ConfigureFormSource(const char* pluginName,std::uint32_t localIDOffset=0);
bool InstallHooks();
// Explicit host dispatch: embedded mode never writes the shared 03/AD slots.
using HostRequestFn = void (*)(RE::ActorMagicCaster*);
void BindHostRequest(HostRequestFn next);
void RequestFromHost(RE::ActorMagicCaster* caster);
// Exact resolved carrier/enchantment ownership; names and foreign forms never grant admission.
bool OwnsHandMagic(RE::MagicItem* spell) noexcept;
using HostCheckFn=bool(*)(RE::ActorMagicCaster*,RE::MagicItem*,bool,float*,RE::MagicSystem::CannotCastReason*,bool);
bool CheckFromHost(HostCheckFn next,RE::ActorMagicCaster*,RE::MagicItem*,bool,float*,RE::MagicSystem::CannotCastReason*,bool);
void BeforePlayerFrame(RE::PlayerCharacter* player);
void AfterPlayerFrame(RE::PlayerCharacter* player,float dt);
bool FlightActive();
// Menu thread only queues; existing game frame owns reload and flight state.
void QueueFlightSettingsReload() noexcept;
RE::NiPoint3 HostActorCenter(RE::Actor* actor);
float HostSunScale(std::size_t layer);
float HostDomeGrowth(std::size_t bankSlot,float fraction);
RE::NiPoint3 HostSunAnchor(RE::PlayerCharacter* player);
bool HostAimDirection(RE::PlayerCharacter* player,RE::NiPoint3& direction) noexcept;
RE::NiPoint3 HostJudgementGuideAnchor(RE::PlayerCharacter* player,bool left);
bool HostDomePose(std::size_t bankSlot,RE::PlayerCharacter* player,RE::NiPoint3& position,RE::NiPoint3& direction,float& scale,bool airborne=false);
void Message(SKSE::MessagingInterface::Message* message);
}
