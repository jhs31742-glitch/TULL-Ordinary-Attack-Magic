#include "Spells/WorldProfile.h"

namespace TOAM::Spells::World
{
    namespace
    {
        constexpr TOAM::Charge::ChargeProfile
            kWorldChargeProfile{
                .id =
                    TOAM::Charge::ProfileID::kWorld,

                .name =
                    "TheWorld",

                .drainPerHandPerSecond =
                    500.0F,

                .chargeStatCap =
                    kStage3Charge,

                .stopDrainAtStatCap =
                    true,

                .thresholds = {
                    kStage1Charge,
                    kStage2Charge,
                    kStage3Charge,
                    0.0F,
                    0.0F,
                    0.0F,
                    0.0F,
                    0.0F
                },

                .thresholdCount =
                    3
            };
    }


    const TOAM::Charge::ChargeProfile&
        GetChargeProfile() noexcept
    {
        return kWorldChargeProfile;
    }


    std::size_t ResolveStage(
        const float accumulatedCharge) noexcept
    {
        return
            kWorldChargeProfile.
                TierForCharge(
                    accumulatedCharge
                );
    }


    std::size_t CircleCountForStage(
        const std::size_t stage) noexcept
    {
        switch (stage) {

        case 1:
            return kStage1CircleCount;

        case 2:
            return kStage2CircleCount;

        case 3:
            return kStage3CircleCount;

        default:
            return 0;
        }
    }
}