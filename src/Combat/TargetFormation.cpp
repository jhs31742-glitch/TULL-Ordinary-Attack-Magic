#include "Combat/TargetFormation.h"

#include <array>
#include <cmath>
#include <utility>
#include <chrono>
#include <cstdint>

namespace TOAM::Combat
{
    namespace
    {
        constexpr float
            kTau{
                6.28318530717958647692F
            };

        constexpr float
            kVectorEpsilon{
                0.000001F
            };


        struct HemisphereCoordinate final
        {
            float right{};
            float forward{};
            float up{};
        };


        // Starting implementation mapping derived from the approved
        // Judgement / Temperance 9-point visual authority.
        //
        // Coordinate order:
        // 1 apex
        // 2 front upper
        // 3 rear upper
        // 4 left upper
        // 5 right upper
        // 6 front mid-upper
        // 7 rear mid-upper
        // 8 left mid-upper
        // 9 right mid-upper
        //
        // These are normalized direction coordinates only.
        // Gameplay radius / scale remains separate and tunable.
        constexpr std::array<
            HemisphereCoordinate,
            kUpperHemisphereSlotCount
        > kUpperHemisphereCoordinates{{
            {
                0.0F,
                0.0F,
                1.0F
            },

            {
                0.0F,
                0.55F,
                0.83516465F
            },

            {
                0.0F,
                -0.55F,
                0.83516465F
            },

            {
                -0.55F,
                0.0F,
                0.83516465F
            },

            {
                0.55F,
                0.0F,
                0.83516465F
            },

            {
                0.0F,
                0.82F,
                0.57236352F
            },

            {
                0.0F,
                -0.82F,
                0.57236352F
            },

            {
                -0.82F,
                0.0F,
                0.57236352F
            },

            {
                0.82F,
                0.0F,
                0.57236352F
            }
        }};


        struct HemisphereBasis final
        {
            RE::NiPoint3 right{};
            RE::NiPoint3 forward{};
            RE::NiPoint3 up{
                0.0F,
                0.0F,
                1.0F
            };
        };


        [[nodiscard]]
        bool IsFinitePoint(
            const RE::NiPoint3& point) noexcept
        {
            return
                std::isfinite(point.x) &&
                std::isfinite(point.y) &&
                std::isfinite(point.z);
        }


        [[nodiscard]]
        float Dot(
            const RE::NiPoint3& left,
            const RE::NiPoint3& right) noexcept
        {
            return
                left.x * right.x +
                left.y * right.y +
                left.z * right.z;
        }


        [[nodiscard]]
        RE::NiPoint3 Cross(
            const RE::NiPoint3& left,
            const RE::NiPoint3& right) noexcept
        {
            return RE::NiPoint3{
                left.y * right.z -
                    left.z * right.y,

                left.z * right.x -
                    left.x * right.z,

                left.x * right.y -
                    left.y * right.x
            };
        }


        [[nodiscard]]
        float LengthSquared(
            const RE::NiPoint3& value) noexcept
        {
            return
                Dot(
                    value,
                    value
                );
        }


        [[nodiscard]]
        bool Normalize(
            const RE::NiPoint3& value,
            RE::NiPoint3& result) noexcept
        {
            const float lengthSquared =
                LengthSquared(
                    value
                );

            if (
                !std::isfinite(
                    lengthSquared
                ) ||
                lengthSquared <=
                    kVectorEpsilon
            ) {
                return false;
            }


            const float inverseLength =
                1.0F /
                std::sqrt(
                    lengthSquared
                );


            result =
                value *
                inverseLength;


            return
                IsFinitePoint(
                    result
                );
        }


        [[nodiscard]]
        bool BuildPlaneBasis(
            const RE::NiPoint3& planeNormal,
            const RE::NiPoint3& referenceAxis,
            RE::NiPoint3& radial,
            RE::NiPoint3& tangent) noexcept
        {
            RE::NiPoint3 normal{};

            if (
                !Normalize(
                    planeNormal,
                    normal
                )
            ) {
                return false;
            }


            RE::NiPoint3 projected =
                referenceAxis -
                normal *
                Dot(
                    referenceAxis,
                    normal
                );


            if (
                !Normalize(
                    projected,
                    radial
                )
            ) {

                const RE::NiPoint3 fallback =
                    std::fabs(
                        normal.z
                    ) < 0.9F ?
                        RE::NiPoint3{
                            0.0F,
                            0.0F,
                            1.0F
                        } :
                        RE::NiPoint3{
                            1.0F,
                            0.0F,
                            0.0F
                        };


                projected =
                    fallback -
                    normal *
                    Dot(
                        fallback,
                        normal
                    );


                if (
                    !Normalize(
                        projected,
                        radial
                    )
                ) {
                    return false;
                }
            }


            if (
                !Normalize(
                    Cross(
                        normal,
                        radial
                    ),
                    tangent
                )
            ) {
                return false;
            }


            return true;
        }


        [[nodiscard]]
        bool BuildHemisphereBasis(
            const RE::NiPoint3& forwardAxis,
            HemisphereBasis& result) noexcept
        {
            result =
                {};


            result.up =
                RE::NiPoint3{
                    0.0F,
                    0.0F,
                    1.0F
                };


            RE::NiPoint3 horizontalForward{
                forwardAxis.x,
                forwardAxis.y,
                0.0F
            };


            if (
                !Normalize(
                    horizontalForward,
                    result.forward
                )
            ) {
                result.forward =
                    RE::NiPoint3{
                        0.0F,
                        1.0F,
                        0.0F
                    };
            }


            if (
                !Normalize(
                    Cross(
                        result.forward,
                        result.up
                    ),
                    result.right
                )
            ) {
                return false;
            }


            return true;
        }


        [[nodiscard]]
        bool ResolveHemisphereDirection(
            const HemisphereBasis& basis,
            const HemisphereCoordinate& coordinate,
            RE::NiPoint3& result) noexcept
        {
            const RE::NiPoint3 direction =
                basis.right *
                    coordinate.right +
                basis.forward *
                    coordinate.forward +
                basis.up *
                    coordinate.up;


            return
                Normalize(
                    direction,
                    result
                );
        }
    }


    static_assert(
        kUpperHemisphereSlotCount ==
            TargetFormation::kCapacity,
        "Upper hemisphere slot count must match TargetFormation capacity"
    );


    bool SelectDistributedUpperHemisphereSlots(
        const std::size_t requestedCount,
        const std::uint16_t avoidMask,
        std::array<std::size_t, kUpperHemisphereSlotCount>& result) noexcept
    {
        result = {};
        if (requestedCount == 0 || requestedCount > kUpperHemisphereSlotCount) {
            return false;
        }
        std::array<std::size_t, kUpperHemisphereSlotCount> choices{};
        std::size_t available = 0;
        for (std::size_t i = 0; i < kUpperHemisphereSlotCount; ++i) {
            if ((avoidMask & (std::uint16_t{1} << i)) == 0) {
                choices[available++] = i;
            }
        }
        if (available < requestedCount) {
            return false;
        }

        static thread_local std::uint64_t entropy =
            static_cast<std::uint64_t>(
                std::chrono::steady_clock::now().time_since_epoch().count()) |
            std::uint64_t{1};
        const auto nextRandom = [&]() noexcept -> std::uint64_t {
            entropy ^= entropy >> 12;
            entropy ^= entropy << 25;
            entropy ^= entropy >> 27;
            return entropy * UINT64_C(2685821657736338717);
        };
        for (std::size_t n = available; n > 1; --n) {
            std::swap(choices[n - 1], choices[nextRandom() % n]);
        }

        std::uint16_t used{};
        for (std::size_t pick = 0; pick < requestedCount; ++pick) {
            std::size_t best = kUpperHemisphereSlotCount;
            float score = -1.0F;
            for (std::size_t i = 0; i < available; ++i) {
                const auto slot = choices[i];
                if ((used & (std::uint16_t{1} << slot)) != 0) {
                    continue;
                }
                float minimum = 100.0F;
                for (std::size_t prev = 0; prev < pick; ++prev) {
                    const auto& a = kUpperHemisphereCoordinates[slot];
                    const auto& b = kUpperHemisphereCoordinates[result[prev]];
                    const auto x = a.right - b.right;
                    const auto y = a.forward - b.forward;
                    const auto z = a.up - b.up;
                    const auto distance = x*x + y*y + z*z;
                    minimum = minimum < distance ? minimum : distance;
                }
                const auto candidateScore = pick == 0 ? 0.0F : minimum;
                if (best == kUpperHemisphereSlotCount || candidateScore > score) {
                    best = slot;
                    score = candidateScore;
                }
            }
            if (best == kUpperHemisphereSlotCount) {
                return false;
            }
            result[pick] = best;
            used |= std::uint16_t{1} << best;
        }
        return true;
    }


    bool IsSupportedFormationCount(
        const std::size_t count) noexcept
    {
        return
            count == 3 ||
            count == 6 ||
            count == 9;
    }


    bool BuildRingFormation(
        const RE::NiPoint3& center,
        const RE::NiPoint3& planeNormal,
        const RE::NiPoint3& referenceAxis,
        const float radius,
        const std::size_t count,
        const float phaseRadians,
        TargetFormation& result) noexcept
    {
        result =
            {};


        if (
            !IsFinitePoint(
                center
            ) ||
            !IsFinitePoint(
                planeNormal
            ) ||
            !IsFinitePoint(
                referenceAxis
            ) ||
            !std::isfinite(
                radius
            ) ||
            radius <=
                0.0F ||
            !std::isfinite(
                phaseRadians
            ) ||
            !IsSupportedFormationCount(
                count
            )
        ) {
            return false;
        }


        RE::NiPoint3 radial{};
        RE::NiPoint3 tangent{};


        if (
            !BuildPlaneBasis(
                planeNormal,
                referenceAxis,
                radial,
                tangent
            )
        ) {
            return false;
        }


        const float angularStep =
            kTau /
            static_cast<float>(
                count
            );


        for (
            std::size_t index = 0;
            index < count;
            ++index
        ) {

            const float angle =
                phaseRadians +
                angularStep *
                static_cast<float>(
                    index
                );


            const float cosine =
                std::cos(
                    angle
                );


            const float sine =
                std::sin(
                    angle
                );


            result.points[index] =
                center +
                radial *
                    (
                        cosine *
                        radius
                    ) +
                tangent *
                    (
                        sine *
                        radius
                    );
        }


        result.center =
            center;

        result.radius =
            radius;

        result.count =
            count;

        result.valid =
            true;


        return true;
    }


    bool BuildActorRingFormation(
        RE::Actor* actor,
        const RE::NiPoint3& planeNormal,
        const RE::NiPoint3& referenceAxis,
        const float radiusScale,
        const std::size_t count,
        const float phaseRadians,
        TargetFormation& result) noexcept
    {
        result =
            {};


        if (
            !std::isfinite(
                radiusScale
            ) ||
            radiusScale <=
                0.0F
        ) {
            return false;
        }


        ActorBounds bounds{};


        if (
            !ResolveActorBounds(
                actor,
                bounds
            )
        ) {
            return false;
        }


        const float formationRadius =
            bounds.radius *
            radiusScale;


        if (
            !std::isfinite(
                formationRadius
            ) ||
            formationRadius <=
                0.0F
        ) {
            return false;
        }


        return
            BuildRingFormation(
                bounds.center,
                planeNormal,
                referenceAxis,
                formationRadius,
                count,
                phaseRadians,
                result
            );
    }


    bool ResolveUpperHemisphereDirection(
        const RE::NiPoint3& forwardAxis,
        const UpperHemisphereSlot slot,
        RE::NiPoint3& result) noexcept
    {
        result =
            {};


        const auto index =
            static_cast<std::size_t>(
                slot
            );


        if (
            index >=
            kUpperHemisphereCoordinates.size()
        ) {
            return false;
        }


        HemisphereBasis basis{};


        if (
            !BuildHemisphereBasis(
                forwardAxis,
                basis
            )
        ) {
            return false;
        }


        return
            ResolveHemisphereDirection(
                basis,
                kUpperHemisphereCoordinates[index],
                result
            );
    }


    bool BuildUpperHemisphereReference(
        const RE::NiPoint3& center,
        const RE::NiPoint3& forwardAxis,
        const float radius,
        TargetFormation& result) noexcept
    {
        result =
            {};


        if (
            !IsFinitePoint(
                center
            ) ||
            !std::isfinite(
                radius
            ) ||
            radius <=
                0.0F
        ) {
            return false;
        }


        HemisphereBasis basis{};


        if (
            !BuildHemisphereBasis(
                forwardAxis,
                basis
            )
        ) {
            return false;
        }


        for (
            std::size_t index = 0;
            index <
                kUpperHemisphereCoordinates.size();
            ++index
        ) {

            RE::NiPoint3 direction{};


            if (
                !ResolveHemisphereDirection(
                    basis,
                    kUpperHemisphereCoordinates[index],
                    direction
                )
            ) {
                result =
                    {};

                return false;
            }


            result.points[index] =
                center +
                direction *
                radius;
        }


        result.center =
            center;

        result.radius =
            radius;

        result.count =
            kUpperHemisphereCoordinates.size();

        result.valid =
            true;


        return true;
    }
}