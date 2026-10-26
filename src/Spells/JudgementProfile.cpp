#include "Spells/JudgementProfile.h"

#include <array>
#include <cmath>

namespace TOAM::Spells::Judgement
{
    namespace
    {
        constexpr std::array<
            StageProfile,
            kStageCount
        > kStages{{
            {
                1,
                300.0F,
                3,
                kArtilleryBeamDamage,
                kArtilleryExplosionDamage,
                kArtilleryExplosionRadiusWorldUnits,
                600.0F
            },

            {
                2,
                900.0F,
                6,
                kArtilleryBeamDamage,
                kArtilleryExplosionDamage,
                kArtilleryExplosionRadiusWorldUnits,
                1200.0F
            },

            {
                3,
                1800.0F,
                9,
                kArtilleryBeamDamage,
                kArtilleryExplosionDamage,
                kArtilleryExplosionRadiusWorldUnits,
                1800.0F
            }
        }};


        [[nodiscard]]
        constexpr bool ValidateStages() noexcept
        {
            if (
                kStages.size() !=
                    kStageCount
            ) {
                return false;
            }


            for (
                std::size_t index = 0;
                index < kStages.size();
                ++index
            ) {
                const auto& stage =
                    kStages[index];


                if (
                    stage.stage !=
                        index + 1 ||
                    stage.chargeThreshold <=
                        0.0F ||
                    stage.circleCount ==
                        0 ||
                    stage.beamDamage !=
                        kArtilleryBeamDamage ||
                    stage.explosionDamage !=
                        kArtilleryExplosionDamage ||
                    stage.explosionRadius !=
                        kArtilleryExplosionRadiusWorldUnits ||
                    stage.totalDamage !=
                        (
                            (
                                stage.beamDamage +
                                stage.explosionDamage
                            ) *
                            static_cast<float>(
                                stage.circleCount
                            )
                        )
                ) {
                    return false;
                }


                if (
                    index > 0 &&
                    stage.chargeThreshold <=
                        kStages[
                            index - 1
                        ].
                        chargeThreshold
                ) {
                    return false;
                }
            }


            return
                kStages[0].
                    chargeThreshold ==
                        300.0F &&

                kStages[0].
                    circleCount ==
                        3 &&

                kStages[0].
                    totalDamage ==
                        600.0F &&


                kStages[1].
                    chargeThreshold ==
                        900.0F &&

                kStages[1].
                    circleCount ==
                        6 &&

                kStages[1].
                    totalDamage ==
                        1200.0F &&


                kStages[2].
                    chargeThreshold ==
                        1800.0F &&

                kStages[2].
                    circleCount ==
                        9 &&

                kStages[2].
                    totalDamage ==
                        1800.0F;
        }


        static_assert(
            ValidateStages(),
            "Judgement stage table changed"
        );
    }


    const StageProfile* GetStageProfile(
        const std::uint8_t stage) noexcept
    {
        if (
            stage == 0 ||
            stage >
                kStageCount
        ) {
            return nullptr;
        }


        return
            &kStages[
                static_cast<
                    std::size_t>(
                        stage - 1
                    )
            ];
    }


    const StageProfile* ResolveStage(
        const float effectiveCharge) noexcept
    {
        if (
            !std::isfinite(
                effectiveCharge
            ) ||
            effectiveCharge <
                kStages.front().
                    chargeThreshold
        ) {
            return nullptr;
        }


        const StageProfile*
            resolved{
                nullptr
            };


        for (
            const auto& stage :
            kStages
        ) {
            if (
                effectiveCharge <
                stage.
                    chargeThreshold
            ) {
                break;
            }


            resolved =
                &stage;
        }


        return resolved;
    }


    std::uint8_t ResolveStageNumber(
        const float effectiveCharge) noexcept
    {
        const auto* stage =
            ResolveStage(
                effectiveCharge
            );


        return
            stage ?
                stage->stage :
                0;
    }
}