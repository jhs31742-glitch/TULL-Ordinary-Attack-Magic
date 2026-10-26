#pragma once

#include "RE/Skyrim.h"

namespace TOAM::Combat::WeaponHitSource
{
    using Callback = void (*)(
        RE::Actor* target,
        RE::TESObjectWEAP* weapon,
        const RE::HitData& hitData,
        bool powerAttack) noexcept;

    void SetCallback(Callback callback) noexcept;

    [[nodiscard]] bool IsInstalled() noexcept;

    // S01: compiled but intentionally not called.
    // Installation is deferred until Temperance implementation.
    void Install();
}