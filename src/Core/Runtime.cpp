#include "Audio/SpellAudio.h"

#include "Core/Runtime.h"

#include "Charge/ChargeRuntime.h"
#include "Combat/TriggerFirewall.h"
#include "Core/DataCache.h"
#include "Spells/TemperanceRuntime.h"
#include "Spells/ArcanaLimiterRuntime.h"
#include "Spells/WorldRuntime.h"
#include "VFX/TransientVFXManager.h"
#include "VFX/VFXRuntime.h"

namespace TOAM::Core::Runtime
{
    namespace
    {
        GameState g_state{
            GameState::kCold
        };

        void ResetOOMTransientState() noexcept
        {
            TOAM::Audio::Reset();
            TOAM::Spells::Temperance::Runtime::
                ResetAllState();


            TOAM::Charge::Runtime::
                ResetTransientState();

            TOAM::VFX::Runtime::
                ResetTracking();

            TOAM::VFX::TransientVFXManager::
                Get().
                ReleaseAll();

            Combat::ResetTransientState();
        }

        void PrepareChargeRuntime() noexcept
        {
            TOAM::Spells::Temperance::Runtime::
                Install();


            TOAM::Charge::Runtime::
                ResetTransientState();

            TOAM::Charge::Runtime::
                Install();

            const bool worldInputReady =
                TOAM::Spells::World::Runtime::
                    InstallInputHook();

            if (!worldInputReady) {
                logger::error(
                    "[TOAM][S07C2] World input hook installation failed");
            }
        }

        [[nodiscard]]
        bool PrepareVFXRuntime() noexcept
        {
            TOAM::VFX::Runtime::
                ResetTracking();

            TOAM::VFX::Runtime::
                Install();

            return
                TOAM::VFX::Runtime::
                    IsInstalled() &&
                TOAM::VFX::Runtime::
                    EnsureCoreAssetsReady();
        }
    }


    void OnDataLoaded() noexcept
    {
        const bool formsReady =
            DataCache::ResolveAll();

        TOAM::Spells::ArcanaLimiter::Runtime::ResolveMarker();

        const bool vfxReady =
            PrepareVFXRuntime();

        g_state =
            formsReady ?
                GameState::kDataLoaded :
                GameState::kCold;

        if (formsReady) {
            PrepareChargeRuntime();
        }

        logger::info(
            "[TOAM][Runtime] DataLoaded | DataCache={} | VFXRuntime={} | VFXAssets={}",
            formsReady ?
                "READY" :
                "NOT_READY",
            TOAM::VFX::Runtime::
                IsInstalled() ?
                "READY" :
                "NOT_READY",
            vfxReady ?
                "READY" :
                "NOT_READY");
    }


    void OnPreLoadGame() noexcept
    {
        ResetOOMTransientState();

        g_state =
            DataCache::IsReady() ?
                GameState::kDataLoaded :
                GameState::kCold;

        logger::info(
            "[TOAM][Runtime] PreLoadGame | TransientState=RESET");
    }


    void OnPostLoadGame() noexcept
    {
        TOAM::Audio::Reset();
        TOAM::VFX::TransientVFXManager::
            Get().
            ReleaseAll();

        const bool vfxReady =
            PrepareVFXRuntime();

        const bool formsReady =
            DataCache::IsReady() ||
            DataCache::ResolveAll();

        if (formsReady) {

            PrepareChargeRuntime();
        }
        else {

            TOAM::Charge::Runtime::
                ResetTransientState();
        }

        g_state =
            formsReady ?
                GameState::kGameReady :
                GameState::kCold;

        logger::info(
            "[TOAM][Runtime] PostLoadGame | GameReady={} | VFXRuntime={} | VFXAssets={}",
            g_state ==
                GameState::kGameReady,
            TOAM::VFX::Runtime::
                IsInstalled(),
            vfxReady);
    }


    void OnNewGame() noexcept
    {
        ResetOOMTransientState();

        const bool vfxReady =
            PrepareVFXRuntime();

        const bool formsReady =
            DataCache::IsReady() ||
            DataCache::ResolveAll();

        if (formsReady) {
            PrepareChargeRuntime();
        }

        g_state =
            formsReady ?
                GameState::kGameReady :
                GameState::kCold;

        logger::info(
            "[TOAM][Runtime] NewGame | GameReady={} | VFXRuntime={} | VFXAssets={}",
            g_state ==
                GameState::kGameReady,
            TOAM::VFX::Runtime::
                IsInstalled(),
            vfxReady);
    }


    GameState GetState() noexcept
    {
        return g_state;
    }


    bool IsGameReady() noexcept
    {
        return
            g_state ==
            GameState::kGameReady;
    }
}