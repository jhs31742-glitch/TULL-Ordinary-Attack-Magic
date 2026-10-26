#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace TOAM::Charge
{
    enum class ProfileID : std::uint8_t
    {
        kMagician = 0,
        kJudgement = 1,
        kWorld = 2
    };

    struct ChargeProfile
    {
        ProfileID id{};
        std::string_view name{};

        float drainPerHandPerSecond{ 0.0F };

        float chargeStatCap{ 0.0F };

        bool stopDrainAtStatCap{ false };

        std::array<float, 8> thresholds{};
        std::size_t thresholdCount{ 0 };

        [[nodiscard]]
        std::size_t TierForCharge(float charge) const noexcept;

        [[nodiscard]]
        float ClampAccumulatedCharge(float charge) const noexcept;

        [[nodiscard]]
        float RemainingDrainCapacity(float charge) const noexcept;
    };

    [[nodiscard]]
    const ChargeProfile& MagicianProfile() noexcept;

    [[nodiscard]]
    const ChargeProfile& JudgementProfile() noexcept;
}