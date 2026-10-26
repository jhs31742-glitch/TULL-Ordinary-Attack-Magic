#pragma once

#include "Charge/ChargeProfile.h"

#include <cstddef>

namespace TOAM::Spells::World
{
    inline constexpr float kStage1Charge{
        350.0F
    };

    inline constexpr float kStage2Charge{
        700.0F
    };

    inline constexpr float kStage3Charge{
        1050.0F
    };

    inline constexpr std::size_t kStage1CircleCount{
        4
    };

    inline constexpr std::size_t kStage2CircleCount{
        8
    };

    inline constexpr std::size_t kStage3CircleCount{
        12
    };

    inline constexpr float kShellRadiusWorldUnits{
        128.0F
    };

    inline constexpr float kVisualBeamLengthWorldUnits{
        64.0F
    };

    inline constexpr float kExplosionRadiusWorldUnits{
        85.333333F
    };

    inline constexpr float kExplosionDamage{
        100.0F
    };

    [[nodiscard]]
    const TOAM::Charge::ChargeProfile&
        GetChargeProfile() noexcept;

    [[nodiscard]]
    std::size_t ResolveStage(
        float accumulatedCharge) noexcept;

    [[nodiscard]]
    std::size_t CircleCountForStage(
        std::size_t stage) noexcept;
}