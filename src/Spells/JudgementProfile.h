#pragma once

#include <cstddef>
#include <cstdint>

namespace TOAM::Spells::Judgement
{
    // --------------------------------------------------------
    // JUDGEMENT ARTILLERY DAMAGE TUNING
    //
    // Current gameplay authority:
    //
    // beam       = 150 fixed
    // explosion  = 50 fixed
    //
    // One Actor may receive both components from one shot.
    //
    // Explosion design radius:
    // 3 ft
    //
    // Skyrim world scale reference:
    // 128 world units ~= 6 ft
    // therefore 3 ft ~= 64 world units.
    //
    // The world-unit radius remains BALANCE-TUNABLE.
    // --------------------------------------------------------

    inline constexpr float
        kArtilleryBeamDamage{
            150.0F
        };

    inline constexpr float
        kArtilleryExplosionDamage{
            50.0F
        };

    inline constexpr float
        kArtilleryExplosionRadiusDesignFeet{
            3.0F
        };

    inline constexpr float
        kArtilleryExplosionRadiusWorldUnits{
            64.0F
        };


    // Skyrim level-design scale already adopted by B1E1:
    // 128 world units ~= 6 ft.
    inline constexpr float
        kSkyrimWorldUnitsPerFoot{
            128.0F / 6.0F
        };


    inline constexpr float
        kMarkingDamage{
            1.0F
        };


    inline constexpr float
        kMarkingRangeDesignFeet{
            20.0F
        };


    inline constexpr float
        kMarkingRangeWorldUnits{
            kMarkingRangeDesignFeet *
            kSkyrimWorldUnitsPerFoot
        };


    // No additional gameplay beam thickness has been approved.
    //
    // Zero here means collision uses the ray against ActorBounds
    // without inventing an extra padded beam radius.
    inline constexpr float
        kMarkingBeamRadiusWorldUnits{
            0.0F
        };


    inline constexpr float
        kArtilleryBeamRadiusWorldUnits{
            0.0F
        };


    struct StageProfile final
    {
        std::uint8_t stage{};

        float chargeThreshold{};

        std::uint8_t circleCount{};

        float beamDamage{};

        float explosionDamage{};

        float explosionRadius{};

        // Maximum direct total when one target receives
        // both beam and explosion from every artillery shot.
        float totalDamage{};
    };


    inline constexpr
        std::size_t kStageCount{
            3
        };


    [[nodiscard]]
    const StageProfile* GetStageProfile(
        std::uint8_t stage) noexcept;


    [[nodiscard]]
    const StageProfile* ResolveStage(
        float effectiveCharge) noexcept;


    [[nodiscard]]
    std::uint8_t ResolveStageNumber(
        float effectiveCharge) noexcept;
}