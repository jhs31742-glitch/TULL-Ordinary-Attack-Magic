#include "Spells/ArcanaLimiterRuntime.h"

#include "Spells/TemperanceRuntime.h"

#include "RE/Skyrim.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace TOAM::Spells::ArcanaLimiter::Runtime
{
    namespace
    {
        // These three EditorIDs will be owned by the TOAM ESP.
        // Their effects grant Magicka +10/+20/+30 respectively.
        // They do not modify vanilla Destruction spell costs.
        constexpr std::array<const char*, 3> kTierEditorIDs{
            "TOAM_ArcanaLimiter_Magicka10_MGEF",
            "TOAM_ArcanaLimiter_Magicka20_MGEF",
            "TOAM_ArcanaLimiter_Magicka30_MGEF"
        };

        std::array<RE::EffectSetting*, 3> g_tierEffects{};

        [[nodiscard]]
        bool TierFormsReady() noexcept
        {
            for (const auto* form : g_tierEffects) {
                if (!form) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]]
        std::uint8_t ActiveTierMask(
            RE::PlayerCharacter* player) noexcept
        {
            if (!player || !TierFormsReady()) {
                return 0;
            }

            // Use the runtime-correct MagicTarget interface (SE offset 0x98).
            auto* magicTarget = player->AsMagicTarget();
            if (!magicTarget) {
                return 0;
            }
            auto* effects = magicTarget->GetActiveEffectList();
            if (!effects) {
                return 0;
            }

            std::uint8_t mask = 0;

            for (auto* active : *effects) {
                if (!active) {
                    continue;
                }

                if (active->flags.any(
                        RE::ActiveEffect::Flag::kInactive,
                        RE::ActiveEffect::Flag::kDispelled)) {
                    continue;
                }

                if (active->conditionStatus ==
                    RE::ActiveEffect::ConditionStatus::kFalse) {
                    continue;
                }

                const auto* base = active->GetBaseObject();
                if (!base) {
                    continue;
                }

                for (std::size_t i = 0;
                     i < g_tierEffects.size();
                     ++i) {
                    if (base == g_tierEffects[i]) {
                        mask |= static_cast<std::uint8_t>(
                            1u << i);
                    }
                }
            }

            return mask;
        }
    }

    void ResolveMarker() noexcept
    {
        std::size_t resolved = 0;

        for (std::size_t i = 0;
             i < kTierEditorIDs.size();
             ++i) {
            g_tierEffects[i] =
                RE::TESForm::LookupByEditorID<RE::EffectSetting>(
                    kTierEditorIDs[i]);

            if (g_tierEffects[i]) {
                ++resolved;
            }
        }

        logger::info(
            "[TOAM][ArcanaLimiter] Magicka tier markers | Resolved={}/3 | Ready={}",
            resolved, TierFormsReady());
    }

    float SnapshotCostMultiplier(
        RE::PlayerCharacter* player) noexcept
    {
        if (!player) {
            return 1.0F;
        }

        const std::uint8_t activeMask =
            ActiveTierMask(player);

        float limiterDiscount = 0.0F;
        int limiterTier = 0;

        switch (activeMask) {
        case 0x01:
            limiterTier = 10;
            limiterDiscount = 0.10F;
            break;
        case 0x02:
            limiterTier = 20;
            limiterDiscount = 0.20F;
            break;
        case 0x04:
            limiterTier = 30;
            limiterDiscount = 0.30F;
            break;
        default:
            // No tier or conflicting tiers: fail closed.
            // Never sum multiple Limiter enchantment tiers.
            if (activeMask != 0) {
                logger::warn(
                    "[TOAM][ArcanaLimiter] Conflicting active tier effects | Mask={}",
                    static_cast<unsigned>(activeMask));
            }
            break;
        }

        const bool temperanceEnabled =
            TOAM::Spells::Temperance::Runtime::IsEnabled();

        const float temperanceDiscount =
            temperanceEnabled ? 0.20F : 0.0F;

        const float totalDiscount =
            limiterDiscount + temperanceDiscount;

        const float multiplier = 1.0F - totalDiscount;

        logger::info(
            "[TOAM][C3] COST_SNAPSHOT | LimiterWorn={} | LimiterTier={} | ActiveMask={} | MarkersReady={} | Temperance={} | Discount={:.3f} | Mult={:.3f}",
            limiterTier != 0,
            limiterTier,
            static_cast<unsigned>(activeMask),
            TierFormsReady(),
            temperanceEnabled,
            totalDiscount,
            multiplier);

        return multiplier;
    }
}
