#pragma once

#include "Combat/ActorBounds.h"

#include "RE/Skyrim.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace TOAM::Combat
{
    struct TargetFormation final
    {
        static constexpr
            std::size_t kCapacity{
                9
            };

        std::array<
            RE::NiPoint3,
            kCapacity
        > points{};

        RE::NiPoint3 center{
            0.0F,
            0.0F,
            0.0F
        };

        float radius{};

        std::size_t count{};

        bool valid{};
    };


    enum class UpperHemisphereSlot :
        std::uint8_t
    {
        kApex = 0,

        kFrontUpper,
        kRearUpper,
        kLeftUpper,
        kRightUpper,

        kFrontMidUpper,
        kRearMidUpper,
        kLeftMidUpper,
        kRightMidUpper,

        kCount
    };


    inline constexpr
        std::size_t
        kUpperHemisphereSlotCount{
            static_cast<std::size_t>(
                UpperHemisphereSlot::kCount
            )
        };


    [[nodiscard]]
    bool IsSupportedFormationCount(
        std::size_t count) noexcept;


    [[nodiscard]]
    bool BuildRingFormation(
        const RE::NiPoint3& center,
        const RE::NiPoint3& planeNormal,
        const RE::NiPoint3& referenceAxis,
        float radius,
        std::size_t count,
        float phaseRadians,
        TargetFormation& result) noexcept;


    [[nodiscard]]
    bool BuildActorRingFormation(
        RE::Actor* actor,
        const RE::NiPoint3& planeNormal,
        const RE::NiPoint3& referenceAxis,
        float radiusScale,
        std::size_t count,
        float phaseRadians,
        TargetFormation& result) noexcept;


    [[nodiscard]]
    bool ResolveUpperHemisphereDirection(
        const RE::NiPoint3& forwardAxis,
        UpperHemisphereSlot slot,
        RE::NiPoint3& result) noexcept;


    [[nodiscard]]
    bool BuildUpperHemisphereReference(
        const RE::NiPoint3& center,
        const RE::NiPoint3& forwardAxis,
        float radius,
        TargetFormation& result) noexcept;


    // Shared nine-position selector. Caller owns selection history.
    [[nodiscard]]
    bool SelectDistributedUpperHemisphereSlots(
        std::size_t requestedCount,
        std::uint16_t avoidMask,
        std::array<std::size_t, kUpperHemisphereSlotCount>& result) noexcept;
}