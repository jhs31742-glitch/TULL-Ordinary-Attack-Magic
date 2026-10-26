#pragma once

#include "Charge/ChargeProfile.h"

#include <cstddef>
#include <cstdint>

namespace TOAM::Charge
{
    enum class Hand : std::uint8_t
    {
        kLeft = 0,
        kRight = 1
    };

    struct ReleaseResult
    {
        bool wasHeld{ false };
        bool finalRelease{ false };
        bool shouldFire{ false };

        std::size_t tier{ 0 };
        float accumulatedCharge{ 0.0F };
        float costMultiplier{ 1.0F };
    };

    class ChargeSession
    {
    public:
        explicit ChargeSession(
            const ChargeProfile& profile) noexcept;

        [[nodiscard]]
        const ChargeProfile& Profile() const noexcept;

        bool BeginHand(Hand hand, float costMultiplier = 1.0F) noexcept;

        [[nodiscard]]
        ReleaseResult ReleaseHand(Hand hand) noexcept;

        void Reset() noexcept;

        [[nodiscard]]
        bool IsActive() const noexcept;

        [[nodiscard]]
        bool IsHeld(Hand hand) const noexcept;

        [[nodiscard]]
        std::size_t HeldHandCount() const noexcept;

        [[nodiscard]]
        float DrainRatePerSecond() const noexcept;

        [[nodiscard]]
        float RequestedDrain(float deltaSeconds) const noexcept;

        void ApplyConsumedMagicka(float amount) noexcept;

        [[nodiscard]]
        float AccumulatedCharge() const noexcept;

        [[nodiscard]]
        float CostMultiplier() const noexcept;

        [[nodiscard]]
        std::size_t Tier() const noexcept;

    private:
        [[nodiscard]]
        bool& HeldSlot(Hand hand) noexcept;

        [[nodiscard]]
        const bool& HeldSlot(Hand hand) const noexcept;

        const ChargeProfile* profile_{ nullptr };

        bool leftHeld_{ false };
        bool rightHeld_{ false };
        bool active_{ false };

        float accumulatedCharge_{ 0.0F };
        float costMultiplier_{ 1.0F };
    };
}