#include "Arcana/Integration.h"
#include "VFX/EngineCircle.h"
#include "Charge/ChargeRuntime.h"
#include "VFX/VFXRuntime.h"

#include "VFX/AssetCatalog.h"
#include "VFX/TransientVFXManager.h"
#include "Spells/TemperanceRuntime.h"
#include "Spells/JudgementRuntime.h"

#include "RE/Skyrim.h"

#include <cmath>
#include <cstdint>

namespace TOAM::VFX::Runtime
{
    namespace
    {
        bool g_installed{};

        bool g_assetsReady{};

        bool g_cellObserved{};
        std::uint32_t g_lastCellFormID{};

        bool g_combatObserved{};
        bool g_lastInCombat{};

        bool g_deathObserved{};
        bool g_lastDead{};

        void ReleaseForBoundary(
            const char* reason) noexcept
        {
            auto& manager =
                TransientVFXManager::Get();

            const auto active =
                manager.ActiveCount();

            if (active == 0) {
                return;
            }

            manager.ReleaseAll();

            logger::info(
                "[TOAM][VFXRuntime] Cleanup | Reason={} | Released={}",
                reason,
                active);
        }

        void UpdateLifetime() noexcept
        {
            const float deltaSeconds =
                RE::Main::QFrameAnimTime();

            if (
                !std::isfinite(
                    deltaSeconds
                ) ||
                deltaSeconds <=
                    0.0F
            ) {
                return;
            }

            TransientVFXManager::
                Get().
                Update(
                    deltaSeconds
                );
        }

        void UpdatePlayerBoundaries() noexcept
        {
            auto* player =
                RE::PlayerCharacter::
                    GetSingleton();

            if (!player) {
                return;
            }

            auto* currentCell =
                player->GetParentCell();

            const std::uint32_t
                currentCellFormID =
                    currentCell ?
                        currentCell->GetFormID() :
                        0;

            if (
                g_cellObserved &&
                currentCellFormID !=
                    g_lastCellFormID
            ) {

                TOAM::Charge::Runtime::ResetTransientState();
                EngineCircle::Reset();
                ReleaseForBoundary("CELL_TRANSITION");
            }

            g_lastCellFormID =
                currentCellFormID;

            g_cellObserved =
                true;


            const bool inCombat =
                player->IsInCombat();

            if (
                g_combatObserved &&
                g_lastInCombat &&
                !inCombat
            ) {

                const auto releasedUnbounded =
                    TransientVFXManager::Get().ReleaseUnbounded();

                logger::info(
                    "[TOAM][VFXRuntime] CombatEnd | ReleasedUnbounded={} | RetainedTimed={}",
                    releasedUnbounded,
                    TransientVFXManager::Get().ActiveCount());
            }

            g_lastInCombat =
                inCombat;

            g_combatObserved =
                true;


            const bool dead =
                player->IsDead();

            if (
                g_deathObserved &&
                !g_lastDead &&
                dead
            ) {

                TOAM::Charge::Runtime::ResetTransientState();
                EngineCircle::Reset();
                ReleaseForBoundary("PLAYER_DEATH");
            }

            g_lastDead =
                dead;

            g_deathObserved =
                true;
        }

        void Tick() noexcept
        {
            UpdateLifetime();
            UpdatePlayerBoundaries();
        }

        REL::Relocation<void (*)(RE::PlayerCharacter*, float)> g_previousPlayerUpdate{};
        bool g_activeFrameSession{};

        void PlayerFrameUpdate(RE::PlayerCharacter* player, float deltaSeconds)
        {
            Arcana::BeforePlayerFrame(player);
            g_previousPlayerUpdate(player, deltaSeconds);
            Arcana::AfterPlayerFrame(player, deltaSeconds);
            if (!g_installed || !player) { return; }

            auto* ui = RE::UI::GetSingleton();
            if (ui && ui->GameIsPaused()) {
                TOAM::Spells::Judgement::Runtime::SuspendChargeGuide();
                return;
            }
            // The already-installed PlayerCharacterUpdate path owns visual
            // guide movement; no new hook, Actor scan or damage event.
            TOAM::Spells::Judgement::Runtime::UpdateChargeGuide(player);
            TOAM::Spells::Temperance::Runtime::UpdateHalo();
            const float procDelta = RE::Main::QFrameAnimTime();
            if (TOAM::Spells::Temperance::Runtime::HasPending() &&
                std::isfinite(procDelta) && procDelta > 0.0F) {
                TOAM::Spells::Temperance::Runtime::UpdatePending(procDelta);
            }

            EngineCircle::Update(deltaSeconds);
            UpdatePlayerBoundaries();
            auto& manager = TransientVFXManager::Get();
            if (manager.ActiveCount() == 0u) {
                g_activeFrameSession = false;
                return;
            }
            if (!g_activeFrameSession) {
                // Boundary history is continuous across empty visual pools.
                g_activeFrameSession = true;
            }

            Tick();
        }

    }


    void Install() noexcept
    {
        if (g_installed) {
            return;
        }

        if (REL::Module::get().version() != SKSE::RUNTIME_SSE_1_5_97) {
            logger::error("[TOAM][VFXRuntime] Install requires SE 1.5.97");
            return;
        }
        REL::Relocation<std::uintptr_t> playerVtable{RE::VTABLE_PlayerCharacter[0]};
        g_previousPlayerUpdate = playerVtable.write_vfunc(0xAD, PlayerFrameUpdate);
        g_installed = true;
        ResetTracking();
        logger::info("[TOAM][VFXRuntime] Installed | FrameSource=PlayerCharacterUpdate");
    }


    void ResetTracking() noexcept
    {
        g_cellObserved =
            false;

        g_lastCellFormID =
            0;

        g_combatObserved =
            false;

        g_lastInCombat =
            false;

        g_deathObserved =
            false;

        g_lastDead =
            false;
    }


    bool EnsureCoreAssetsReady() noexcept
    {
        if (g_assetsReady) {
            return true;
        }

        g_assetsReady =
            ProbeCoreAssets();

        logger::info(
            "[TOAM][VFXRuntime] CoreAssets | Ready={}",
            g_assetsReady);

        return g_assetsReady;
    }


    bool AreCoreAssetsReady() noexcept
    {
        return g_assetsReady;
    }


    bool IsInstalled() noexcept
    {
        return g_installed;
    }
}