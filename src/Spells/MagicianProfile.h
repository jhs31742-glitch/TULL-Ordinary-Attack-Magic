#pragma once

#include "Combat/BeamCollision.h"

#include "RE/Skyrim.h"

#include <cstddef>
#include <cstdint>

namespace TOAM::Spells::Magician
{
    struct TierProfile final
    {
        std::uint8_t tier{};

        float chargeThreshold{};

        float beamDamage{};
        float beamRange{};
        float beamRadius{};

        float explosionDamage{};
        float explosionRadius{};

        std::uint8_t circleCount{};
    };


    struct FirePlan final
    {
        const TierProfile* profile{};

        Combat::BeamSegmentQuery
            beam{};

        bool valid{};
    };


    inline constexpr
        std::size_t kTierCount{
            8
        };


    [[nodiscard]]
    const TierProfile* GetTierProfile(
        std::uint8_t tier) noexcept;


    [[nodiscard]]
    const TierProfile* ResolveTier(
        float effectiveCharge) noexcept;


    [[nodiscard]]
    std::uint8_t ResolveTierNumber(
        float effectiveCharge) noexcept;


    [[nodiscard]]
    bool BuildFirePlan(
        float effectiveCharge,
        const RE::NiPoint3& origin,
        const RE::NiPoint3& direction,
        FirePlan& result) noexcept;
}