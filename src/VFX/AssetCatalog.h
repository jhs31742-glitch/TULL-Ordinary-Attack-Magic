#pragma once

#include <cstdint>

namespace TOAM::VFX
{
    enum class AssetID : std::uint8_t
    {
        kCircleStatic = 0,
        kCircleCoreStatic,
        kCircleAnimated,
        kCircleCoreAnimated,
        kCircleSmall,

        kBeamBodyUnit,
        kBeamBodyUnitStatic,
        kBeamMuzzle,

        kImpactLow,
        kImpactMid,
        kImpactHigh,
        kImpactHighCompact,

        // S05 V6 reusable Magician circle part.
        kCircleOuterBandStatic,

        kBeamWholeLens,
        kBeamWholeLensTintable,
        kFlashCloud,
        kFlashCloudTintable,

        kCount
    };

    [[nodiscard]]
    const char* AssetName(
        AssetID id) noexcept;

    [[nodiscard]]
    const char* AssetPath(
        AssetID id) noexcept;

    [[nodiscard]]
    bool ProbeAsset(
        AssetID id) noexcept;

    [[nodiscard]]
    bool ProbeCoreAssets() noexcept;
}