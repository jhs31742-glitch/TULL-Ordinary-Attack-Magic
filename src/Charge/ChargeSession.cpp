#include "Charge/ChargeSession.h"

#include <cmath>

namespace TOAM::Charge
{
    ChargeSession::ChargeSession(
        const ChargeProfile& profile) noexcept :
        profile_(&profile)
    {}

    const ChargeProfile&
    ChargeSession::Profile() const noexcept
    {
        return *profile_;
    }

    bool ChargeSession::BeginHand(
        const Hand hand,
        const float costMultiplier) noexcept
    {
        bool& held = HeldSlot(hand);

        if (held) {
            return false;
        }

        if (!active_ && HeldHandCount() == 0) {
            accumulatedCharge_ = 0.0F;
            // The first hand alone owns the immutable session multiplier.
            costMultiplier_ =
                std::isfinite(costMultiplier) &&
                costMultiplier >= 0.5F &&
                costMultiplier <= 1.0F ?
                    costMultiplier : 1.0F;
            active_ = true;
        }

        held = true;
        return true;
    }

    ReleaseResult ChargeSession::ReleaseHand(
        const Hand hand) noexcept
    {
        ReleaseResult result{};

        bool& held = HeldSlot(hand);

        if (!held) {
            return result;
        }

        result.wasHeld = true;

        held = false;

        result.tier = Tier();
        result.accumulatedCharge =
            accumulatedCharge_;
        result.costMultiplier = costMultiplier_;

        if (HeldHandCount() != 0) {
            return result;
        }

        result.finalRelease = true;
        result.shouldFire =
            result.tier > 0;

        active_ = false;

        return result;
    }

    void ChargeSession::Reset() noexcept
    {
        leftHeld_ = false;
        rightHeld_ = false;
        active_ = false;
        accumulatedCharge_ = 0.0F;
        costMultiplier_ = 1.0F;
    }

    bool ChargeSession::IsActive() const noexcept
    {
        return active_;
    }

    bool ChargeSession::IsHeld(
        const Hand hand) const noexcept
    {
        return HeldSlot(hand);
    }

    std::size_t
    ChargeSession::HeldHandCount() const noexcept
    {
        return
            static_cast<std::size_t>(leftHeld_) +
            static_cast<std::size_t>(rightHeld_);
    }

    float
    ChargeSession::DrainRatePerSecond() const noexcept
    {
        if (!active_) {
            return 0.0F;
        }

        if (HeldHandCount() == 0) {
            return 0.0F;
        }

        if (
            profile_->stopDrainAtStatCap &&
            accumulatedCharge_ >= profile_->chargeStatCap
        ) {
            return 0.0F;
        }

        return
            profile_->drainPerHandPerSecond *
            static_cast<float>(HeldHandCount());
    }

    float ChargeSession::RequestedDrain(
        const float deltaSeconds) const noexcept
    {
        if (!std::isfinite(deltaSeconds)) {
            return 0.0F;
        }

        if (deltaSeconds <= 0.0F) {
            return 0.0F;
        }

        float request =
            DrainRatePerSecond() *
            deltaSeconds;

        if (!std::isfinite(request)) {
            return 0.0F;
        }

        if (request <= 0.0F) {
            return 0.0F;
        }

        if (profile_->stopDrainAtStatCap) {
            // Nominal tier remaining becomes an actual Magicka budget.
            const float capacity =
                profile_->RemainingDrainCapacity(
                    accumulatedCharge_
                ) * costMultiplier_;

            if (request > capacity) {
                request = capacity;
            }
        } else if (
            profile_->id == ProfileID::kMagician &&
            costMultiplier_ < 1.0F
        ) {
            // Keep 500 MP/s before T8, discount only the overtime.
            // A frame crossing T8 splits both portions proportionally.
            const float remainingNominal =
                profile_->chargeStatCap - accumulatedCharge_;

            if (remainingNominal <= 0.0F) {
                request *= costMultiplier_;
            } else {
                const float realToCap =
                    remainingNominal * costMultiplier_;
                if (request > realToCap) {
                    request = realToCap +
                        (request - realToCap) * costMultiplier_;
                }
            }
        }

        return request > 0.0F ?
            request :
            0.0F;
    }

    void ChargeSession::ApplyConsumedMagicka(
        const float amount) noexcept
    {
        if (!active_) {
            return;
        }

        if (!std::isfinite(amount)) {
            return;
        }

        if (amount <= 0.0F) {
            return;
        }

        accumulatedCharge_ =
            profile_->ClampAccumulatedCharge(
                accumulatedCharge_ + amount / costMultiplier_
            );
    }

    float
    ChargeSession::AccumulatedCharge() const noexcept
    {
        return accumulatedCharge_;
    }

    float ChargeSession::CostMultiplier() const noexcept
    {
        return costMultiplier_;
    }

    std::size_t
    ChargeSession::Tier() const noexcept
    {
        return profile_->TierForCharge(
            accumulatedCharge_
        );
    }

    bool& ChargeSession::HeldSlot(
        const Hand hand) noexcept
    {
        return hand == Hand::kLeft ?
            leftHeld_ :
            rightHeld_;
    }

    const bool& ChargeSession::HeldSlot(
        const Hand hand) const noexcept
    {
        return hand == Hand::kLeft ?
            leftHeld_ :
            rightHeld_;
    }
}