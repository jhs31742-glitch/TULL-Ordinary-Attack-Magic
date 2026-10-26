#pragma once

#include "RE/Skyrim.h"

#include <cstddef>
#include <cstdint>

namespace TOAM::Spells::Magician::Runtime
{
    struct FireResult final
    {
        float effectiveCharge{};
        float beamDamage{};
        float resolvedDistance{};

        std::size_t actorIntersections{};
        std::size_t actorsDamaged{};

        std::uint8_t tier{};

        bool attempted{};
        bool aimResolved{};
        bool firePlanResolved{};
        bool collisionResolved{};
        bool worldBlocked{};
        bool executed{};
    };


    [[nodiscard]]
    bool BeginChargeVisual(
        RE::PlayerCharacter* player) noexcept;

    [[nodiscard]]
    bool UpdateChargeVisual(
        RE::PlayerCharacter* player,
        float effectiveCharge,
        float deltaSeconds) noexcept;

    void ReleaseChargeVisual() noexcept;

    void ResetVisualState() noexcept;

    [[nodiscard]]
    FireResult ExecuteFinalRelease(
        RE::PlayerCharacter* player,
        float effectiveCharge,
        float costMultiplier) noexcept;
}