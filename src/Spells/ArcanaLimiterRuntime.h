#pragma once

namespace RE
{
    class PlayerCharacter;
}

namespace TOAM::Spells::ArcanaLimiter::Runtime
{
    // Optional ESP form. Failure must never disable the four core spells.
    void ResolveMarker() noexcept;

    // Called only when starting a charge session; result is snapshotted.
    [[nodiscard]] float SnapshotCostMultiplier(RE::PlayerCharacter* player) noexcept;
}
