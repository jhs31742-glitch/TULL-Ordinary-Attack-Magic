#include "Spells/MagicianProfile.h"

#include <array>
#include <cmath>

namespace TOAM::Spells::Magician
{
    namespace
    {
        constexpr std::array<
            TierProfile,
            kTierCount
        > kProfiles{{
            {
                1,
                100.0F,
                100.0F,
                1024.0F,
                8.0F,
                20.0F,
                64.0F,
                1
            },

            {
                2,
                300.0F,
                300.0F,
                1536.0F,
                12.0F,
                60.0F,
                80.0F,
                2
            },

            {
                3,
                600.0F,
                600.0F,
                2048.0F,
                16.0F,
                150.0F,
                96.0F,
                3
            },

            {
                4,
                1200.0F,
                1200.0F,
                3072.0F,
                24.0F,
                300.0F,
                128.0F,
                4
            },

            {
                5,
                2000.0F,
                2000.0F,
                4096.0F,
                32.0F,
                600.0F,
                160.0F,
                5
            },

            {
                6,
                3000.0F,
                3000.0F,
                5120.0F,
                40.0F,
                900.0F,
                192.0F,
                6
            },

            {
                7,
                4000.0F,
                4000.0F,
                6144.0F,
                48.0F,
                1400.0F,
                256.0F,
                7
            },

            {
                8,
                5000.0F,
                5000.0F,
                8192.0F,
                56.0F,
                2000.0F,
                320.0F,
                8
            }
        }};


        [[nodiscard]]
        constexpr bool ValidateLockedTable() noexcept
        {
            if (
                kProfiles.size() !=
                kTierCount
            ) {
                return false;
            }


            for (
                std::size_t index = 0;
                index < kProfiles.size();
                ++index
            ) {

                const auto& profile =
                    kProfiles[index];


                if (
                    profile.tier !=
                        index + 1 ||
                    profile.circleCount !=
                        index + 1 ||
                    profile.chargeThreshold <=
                        0.0F ||
                    profile.beamDamage !=
                        profile.chargeThreshold ||
                    profile.beamRange <=
                        0.0F ||
                    profile.beamRadius <=
                        0.0F ||
                    profile.explosionDamage <=
                        0.0F ||
                    profile.explosionRadius <=
                        0.0F
                ) {
                    return false;
                }


                if (
                    index > 0 &&
                    profile.chargeThreshold <=
                        kProfiles[index - 1].
                            chargeThreshold
                ) {
                    return false;
                }
            }


            return
                kProfiles[0].
                    chargeThreshold ==
                        100.0F &&

                kProfiles[1].
                    chargeThreshold ==
                        300.0F &&

                kProfiles[2].
                    chargeThreshold ==
                        600.0F &&

                kProfiles[3].
                    chargeThreshold ==
                        1200.0F &&

                kProfiles[4].
                    chargeThreshold ==
                        2000.0F &&

                kProfiles[5].
                    chargeThreshold ==
                        3000.0F &&

                kProfiles[6].
                    chargeThreshold ==
                        4000.0F &&

                kProfiles[7].
                    chargeThreshold ==
                        5000.0F &&


                kProfiles[0].
                    beamRange ==
                        1024.0F &&

                kProfiles[1].
                    beamRange ==
                        1536.0F &&

                kProfiles[2].
                    beamRange ==
                        2048.0F &&

                kProfiles[3].
                    beamRange ==
                        3072.0F &&

                kProfiles[4].
                    beamRange ==
                        4096.0F &&

                kProfiles[5].
                    beamRange ==
                        5120.0F &&

                kProfiles[6].
                    beamRange ==
                        6144.0F &&

                kProfiles[7].
                    beamRange ==
                        8192.0F &&


                kProfiles[0].
                    beamRadius ==
                        8.0F &&

                kProfiles[1].
                    beamRadius ==
                        12.0F &&

                kProfiles[2].
                    beamRadius ==
                        16.0F &&

                kProfiles[3].
                    beamRadius ==
                        24.0F &&

                kProfiles[4].
                    beamRadius ==
                        32.0F &&

                kProfiles[5].
                    beamRadius ==
                        40.0F &&

                kProfiles[6].
                    beamRadius ==
                        48.0F &&

                kProfiles[7].
                    beamRadius ==
                        56.0F &&


                kProfiles[0].
                    explosionDamage ==
                        20.0F &&

                kProfiles[1].
                    explosionDamage ==
                        60.0F &&

                kProfiles[2].
                    explosionDamage ==
                        150.0F &&

                kProfiles[3].
                    explosionDamage ==
                        300.0F &&

                kProfiles[4].
                    explosionDamage ==
                        600.0F &&

                kProfiles[5].
                    explosionDamage ==
                        900.0F &&

                kProfiles[6].
                    explosionDamage ==
                        1400.0F &&

                kProfiles[7].
                    explosionDamage ==
                        2000.0F &&


                kProfiles[0].
                    explosionRadius ==
                        64.0F &&

                kProfiles[1].
                    explosionRadius ==
                        80.0F &&

                kProfiles[2].
                    explosionRadius ==
                        96.0F &&

                kProfiles[3].
                    explosionRadius ==
                        128.0F &&

                kProfiles[4].
                    explosionRadius ==
                        160.0F &&

                kProfiles[5].
                    explosionRadius ==
                        192.0F &&

                kProfiles[6].
                    explosionRadius ==
                        256.0F &&

                kProfiles[7].
                    explosionRadius ==
                        320.0F &&


                (
                    kProfiles[7].
                        beamDamage +
                    kProfiles[7].
                        explosionDamage
                ) ==
                    7000.0F;
        }


        static_assert(
            ValidateLockedTable(),
            "Locked Magician tier table changed"
        );
    }


    const TierProfile* GetTierProfile(
        const std::uint8_t tier) noexcept
    {
        if (
            tier == 0 ||
            tier >
                kTierCount
        ) {
            return nullptr;
        }


        return
            std::addressof(
                kProfiles[
                    static_cast<
                        std::size_t>(
                            tier - 1
                        )
                ]
            );
    }


    const TierProfile* ResolveTier(
        const float effectiveCharge) noexcept
    {
        if (
            !std::isfinite(
                effectiveCharge
            ) ||
            effectiveCharge <
                kProfiles.front().
                    chargeThreshold
        ) {
            return nullptr;
        }


        const TierProfile*
            resolved =
                nullptr;


        for (
            const auto& profile :
            kProfiles
        ) {

            if (
                effectiveCharge <
                profile.
                    chargeThreshold
            ) {
                break;
            }


            resolved =
                std::addressof(
                    profile
                );
        }


        return resolved;
    }


    std::uint8_t ResolveTierNumber(
        const float effectiveCharge) noexcept
    {
        const auto* profile =
            ResolveTier(
                effectiveCharge
            );


        return
            profile ?
                profile->tier :
                0;
    }


    bool BuildFirePlan(
        const float effectiveCharge,
        const RE::NiPoint3& origin,
        const RE::NiPoint3& direction,
        FirePlan& result) noexcept
    {
        result =
            {};


        const auto* profile =
            ResolveTier(
                effectiveCharge
            );


        if (!profile) {
            return false;
        }


        if (
            !std::isfinite(
                origin.x
            ) ||
            !std::isfinite(
                origin.y
            ) ||
            !std::isfinite(
                origin.z
            ) ||
            !std::isfinite(
                direction.x
            ) ||
            !std::isfinite(
                direction.y
            ) ||
            !std::isfinite(
                direction.z
            )
        ) {
            return false;
        }


        result.profile =
            profile;


        result.beam.origin =
            origin;


        result.beam.direction =
            direction;


        result.beam.maxDistance =
            profile->beamRange;


        result.beam.radius =
            profile->beamRadius;


        result.valid =
            true;


        return true;
    }
}