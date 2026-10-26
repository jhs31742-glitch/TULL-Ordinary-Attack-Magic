#include "Audio/SpellAudio.h"
#include "Audio/AudioPolicy.h"
#include "Core/PresentationSettings.h"
#include "REL/Module.h"
#include "REL/Relocation.h"
#include "RE/B/BGSDefaultObjectManager.h"
#include "RE/B/BGSSoundOutput.h"
#include <mutex>
#include "RE/B/BSAudioManager.h"
#include "RE/B/BSSoundHandle.h"
#include "RE/I/ID.h"
#include <array>
#include <chrono>
#include <cmath>
#include <optional>

// Windows GDI defines the object-like GetObject macro after the CommonLib PCH.
// Scope this correction to this translation unit; leave shared build policy alone.
#pragma push_macro("GetObject")
#undef GetObject

namespace TOAM::Audio
{
    namespace
    {
        enum class Group : std::size_t { Magician, World, Judgement, Temperance, Count };
        using Clock = std::chrono::steady_clock;
        struct FileContract { std::uint32_t flags; std::uint32_t priority; };
        [[nodiscard]] std::optional<FileContract> VerifiedFileContract() noexcept
        {
            // Exact SE1.5.97 positioned-file precedent, RVA 0x5E9128..0x5E918E.
            // ID constructor RVA0xC4A420 zeroes ID then calls GenerateFromPath.
            // See S03-013/NATIVE_FILE_CONTRACT_PROOF.json; these are not guessed values.
            const auto version=REL::Module::get().version();
            if(version.major()!=1 || version.minor()!=5 || version.patch()!=97 || version.build()!=0) {
                return std::nullopt; // AE is compile compatibility only.
            }
            return FileContract{0x10u,0x80u};
        }
        // Pinned SetPosition(NiPoint3) has an ABI mismatch on SE1.5.97.
        // The native function at SE ID66370 takes three scalar floats.
        // See NATIVE_AUDIO_UNPACKED_AUDIT.json and DEV_LESSONS S03-R11-AUDIO-01.
        [[nodiscard]] bool SetPositionSE97(RE::BSSoundHandle& sound,const RE::NiPoint3& p) noexcept
        {
            using Function=bool (*)(RE::BSSoundHandle*,float,float,float);
            static REL::Relocation<Function> function{REL::ID{66370}};
            return function(std::addressof(sound),p.x,p.y,p.z);
        }
        struct Voice
        {
            RE::BSSoundHandle handle{};
            Clock::time_point expires{};
            bool owned{};
        };
        struct Channel
        {
            std::array<Voice, 3> active{};
            Voice retiring{};
            Clock::time_point lastPlayed{};
            bool played{};
        };
        std::array<Channel, static_cast<std::size_t>(Group::Count)> channels{};
        std::mutex audioMutex{};
        void Stop(Voice& voice) noexcept
        {
            if (voice.owned) {
                auto* manager=RE::BSAudioManager::GetSingleton();
                if(manager && RE::BSAudioManager::QPlatformInstance() &&
                   !manager->flags.all(RE::BSAudioManager::Flags::ShuttingDown)) {
                    static_cast<void>(voice.handle.Stop());
                }
            }
            voice = {};
        }
        [[nodiscard]] bool Finite(const RE::NiPoint3& p) noexcept
        { return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z); }
        void Play(Group group, const char* path, float volume, float seconds,
                  std::size_t cap, std::uint16_t fadeMS, const RE::NiPoint3& p) noexcept
        {
            const std::scoped_lock lock(audioMutex);
            if (!Finite(p)) { logger::warn("[TOAM][Audio] Invalid position; gameplay unchanged"); return; }
            auto& c=channels[static_cast<std::size_t>(group)];
            const auto now=Clock::now();
            const auto sincePreviousMs = c.played ?
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - c.lastPlayed).count() : -1LL;
            // Catch-up artillery shots in one dispatch must not create an audio burst.
            if (group==Group::Judgement && c.played && now-c.lastPlayed<std::chrono::milliseconds(80)) { return; }
            const auto contract=VerifiedFileContract();
            if (!contract) {
                logger::warn("[TOAM][Audio] UNSUPPORTED_RUNTIME | File={} | Volume={} | Cap={} | GameplayUnchanged=true", path, volume, cap);
                return;
            }
            auto* manager=RE::BSAudioManager::GetSingleton();
            if (!manager || !RE::BSAudioManager::QPlatformInstance() ||
                !manager->flags.all(RE::BSAudioManager::Flags::ManagerInitialized) ||
                !manager->flags.all(RE::BSAudioManager::Flags::PlatformInitialized) ||
                manager->flags.all(RE::BSAudioManager::Flags::ShuttingDown)) {
                logger::warn("[TOAM][Audio] Backend not ready; gameplay unchanged");
                return;
            }
            auto* defaults=RE::BGSDefaultObjectManager::GetSingleton();
            // F2-G3: keep every playback in the audible 2D route.
            // The original 3D output is retained only when 2D is unavailable.
            auto* output2D = defaults ?
                defaults->GetObject<RE::BGSSoundOutput>(
                    RE::DEFAULT_OBJECT::kDialogueOutputModel2D) : nullptr;
            const bool use2DControl = output2D != nullptr;
            auto* output = use2DControl ? output2D :
                (defaults ? defaults->GetObject<RE::BGSSoundOutput>(
                    RE::DEFAULT_OBJECT::kDialogueOutputModel3D) : nullptr);
            const char* outputMode =
                use2DControl ? "2D_ALWAYS" : "3D_FALLBACK";
            if (!use2DControl) {
                logger::warn("[TOAM][Audio] F2_2D_FALLBACK | Group={} | File={} | Selected={}",
                    static_cast<std::size_t>(group), path, outputMode);
            }
            if (!output || (!use2DControl &&
                !output->data.flags.all(RE::BGSSoundOutput::Data::Flag::kAttenuatesWithDistance))) {
                logger::warn("[TOAM][Audio] OUTPUT_MODEL_UNAVAILABLE | File={} | Mode={}", path, outputMode);
                return;
            }
            RE::BSResource::ID resource{};
            resource.GenerateFromPath(path);
            RE::BSSoundHandle sound{};
            manager->GetSoundHandleByFile(sound,resource,contract->flags,contract->priority);
            // Do not query asynchronous backend validity before opening completes.
            if (sound.soundID==RE::BSSoundHandle::kInvalidID) {
                logger::warn("[TOAM][Audio] File handle allocation failed | File={}",path);return;
            }
            sound.SetOutputModel(output);
            if ((!use2DControl && !SetPositionSE97(sound,p)) || !sound.SetVolume(volume)) {
                logger::warn("[TOAM][Audio] Position/volume request failed | File={} | Mode={}",path,outputMode);
                static_cast<void>(sound.Stop()); return;
            }
            // Prune only on actual audio events; never poll all voices every frame.
            for (auto& v:c.active) { if (v.owned && v.expires<=now) { Stop(v); } }
            if (c.retiring.owned && c.retiring.expires<=now) { Stop(c.retiring); }
            bool capReplaced = false;
            bool fadeRequested = false;
            std::size_t immediateStops = 0;
            std::size_t slot=cap;
            for (std::size_t i=0;i<cap;++i) { if (!c.active[i].owned) { slot=i; break; } }
            if (slot==cap) {
                capReplaced = true;
                slot=0;
                for(std::size_t i=1;i<cap;++i) { if(c.active[i].expires<c.active[slot].expires) {slot=i;} }
                if (c.retiring.owned) { ++immediateStops; }
                Stop(c.retiring);
                c.retiring=c.active[slot];
                // FadeOutAndRelease invalidates the passed handle. Retain a copy
                // of the original ID so reset/cap replacement can still stop its tail.
                auto releaseRequest=c.retiring.handle;
                fadeRequested = releaseRequest.FadeOutAndRelease(fadeMS);
                if (!fadeRequested) { ++immediateStops; Stop(c.retiring); }
                else { c.retiring.expires=now+std::chrono::milliseconds(fadeMS); }
                c.active[slot]={};
            }
            if(c.retiring.owned) {
                std::size_t live=1;
                for(std::size_t i=0;i<cap;++i) {if(i!=slot && c.active[i].owned) {++live;}}
                if(live>=cap) {
                    for(std::size_t i=0;i<cap;++i) {if(i!=slot && c.active[i].owned) {++immediateStops;Stop(c.active[i]);break;}}
                }
            }
            if (!sound.Play()) {
                logger::warn("[TOAM][Audio] PLAY_REQUEST_FAILED | File={} | Reason=PLAY_RETURN_FALSE", path);
                static_cast<void>(sound.Stop());
                return;
            }
            c.active[slot]={sound,now+std::chrono::milliseconds(static_cast<int>(seconds*1000.0F)+100),true};
            c.lastPlayed=now;c.played=true;
            std::size_t owned=c.retiring.owned ? 1 : 0;
            for(const auto& v:c.active) {if(v.owned) {++owned;}}
            // Request-side evidence only: player distance is a proxy,
            // not proof of the actual listener or audible mixing.
            float playerDistanceProxy = -1.0F;
            if (const auto* player = RE::PlayerCharacter::GetSingleton()) {
                const auto playerPos = player->GetPosition();
                if (Finite(playerPos)) {
                    const float dx = p.x - playerPos.x;
                    const float dy = p.y - playerPos.y;
                    const float dz = p.z - playerPos.z;
                    playerDistanceProxy = std::sqrt(dx*dx + dy*dy + dz*dz);
                }
            }
            logger::info(
                "[TOAM][Audio] PLAY_REQUEST | File={} | Volume={} | Owned={} | Cap={} | "
                "Source=({:.1f},{:.1f},{:.1f}) | PlayerDistanceProxy={:.1f} | "
                "SincePreviousMs={} | CapReplaced={} | FadeRequested={} | ImmediateStops={} | Mode={}",
                path, volume, owned, cap, p.x, p.y, p.z,
                playerDistanceProxy, sincePreviousMs, capReplaced, fadeRequested, immediateStops, outputMode);
        }
    }
    void MagicianFinal(std::size_t tier,const RE::NiPoint3& p) noexcept
    {

        if(tier<1 || tier>8) {return;}
        const auto choice=Presentation::Resolve(tier<=4?Presentation::Event::MagicianLaunch:Presentation::Event::MagicianHeavyLaunch,Policy::kMagicianGain[tier-1]);
        if(choice.volume>0)Play(Group::Magician,Presentation::sources[choice.id-1].path,choice.volume,tier<=4 ? 1.161F : 1.719F,3,60,p);
    }
    void WorldFinal(std::size_t stage,const RE::NiPoint3& p) noexcept
    {

        if(stage<1 || stage>3) {return;}
        const auto choice=Presentation::Resolve(Presentation::Event::WorldRelease,Policy::kWorldGain[stage-1]);
        if(choice.volume>0)Play(Group::World,Presentation::sources[choice.id-1].path,choice.volume,1.719F,3,60,p);
    }
    void JudgementShot(std::size_t shotIndex,const RE::NiPoint3& p) noexcept
    {
        if(!Policy::JudgementAudibleShot(shotIndex)) {return;}
        const auto choice=Presentation::Resolve(Presentation::Event::JudgementShot,.95F);
        if(choice.volume>0)Play(Group::Judgement,Presentation::sources[choice.id-1].path,choice.volume,1.394F,Policy::kJudgementVoices,60,p);
    }
    void TemperanceProc(const RE::NiPoint3& p) noexcept
    {
        const auto choice=Presentation::Resolve(Presentation::Event::TemperanceProc,.95F);
        if(choice.volume>0)Play(Group::Temperance,Presentation::sources[choice.id-1].path,choice.volume,1.533F,Policy::kTemperanceVoices,100,p);
    }
    void Reset() noexcept
    {
        const std::scoped_lock lock(audioMutex);
        for(auto& c:channels) { for(auto& v:c.active) {Stop(v);} Stop(c.retiring);c.played=false; }
    }
}
#pragma pop_macro("GetObject")
