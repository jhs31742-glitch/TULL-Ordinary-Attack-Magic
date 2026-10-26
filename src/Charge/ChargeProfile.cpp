#include "Charge/ChargeProfile.h"

#include <limits>

namespace TOAM::Charge
{
    namespace
    {
        constexpr ChargeProfile kMagicianProfile{
            .id = ProfileID::kMagician,
            .name = "Magician",
            .drainPerHandPerSecond = 500.0F,
            .chargeStatCap = 5000.0F,
            .stopDrainAtStatCap = false,
            .thresholds = {
                100.0F,
                300.0F,
                600.0F,
                1200.0F,
                2000.0F,
                3000.0F,
                4000.0F,
                5000.0F
            },
            .thresholdCount = 8
        };

        constexpr ChargeProfile kJudgementProfile{
            .id = ProfileID::kJudgement,
            .name = "Judgement",
            .drainPerHandPerSecond = 500.0F,
            .chargeStatCap = 1800.0F,
            .stopDrainAtStatCap = true,
            .thresholds = {
                300.0F,
                900.0F,
                1800.0F,
                0.0F,
                0.0F,
                0.0F,
                0.0F,
                0.0F
            },
            .thresholdCount = 3
        };
    }

    std::size_t ChargeProfile::TierForCharge(
        const float charge) const noexcept
    {
        if (charge <= 0.0F) {
            return 0;
        }

        std::size_t tier = 0;

        for (
            std::size_t i = 0;
            i < thresholdCount && i < thresholds.size();
            ++i
        ) {
            if (charge >= thresholds[i]) {
                tier = i + 1;
            } else {
                break;
            }
        }

        return tier;
    }

    float ChargeProfile::ClampAccumulatedCharge(
        const float charge) const noexcept
    {
        const float nonNegative =
            charge > 0.0F ? charge : 0.0F;

        if (chargeStatCap <= 0.0F) {
            return nonNegative;
        }

        return nonNegative < chargeStatCap ?
            nonNegative :
            chargeStatCap;
    }

    float ChargeProfile::RemainingDrainCapacity(
        const float charge) const noexcept
    {
        if (!stopDrainAtStatCap) {
            return std::numeric_limits<float>::infinity();
        }

        const float remaining =
            chargeStatCap - ClampAccumulatedCharge(charge);

        return remaining > 0.0F ?
            remaining :
            0.0F;
    }

    const ChargeProfile& MagicianProfile() noexcept
    {
        return kMagicianProfile;
    }

    const ChargeProfile& JudgementProfile() noexcept
    {
        return kJudgementProfile;
    }
}