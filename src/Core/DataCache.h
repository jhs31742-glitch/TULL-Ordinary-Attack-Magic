#pragma once

#include "RE/Skyrim.h"

namespace TOAM::Core::DataCache
{
    enum class SpellID : std::uint8_t
    {
        kMagician = 0,
        kJudgement,
        kTheWorld,
        kTemperance,
        kCount
    };

    [[nodiscard]] bool ResolveAll() noexcept;
    void Reset() noexcept;

    [[nodiscard]] bool IsReady() noexcept;

    [[nodiscard]] RE::SpellItem* GetSpell(
        SpellID id) noexcept;

    [[nodiscard]] RE::SpellItem* GetMagician() noexcept;
    [[nodiscard]] RE::SpellItem* GetJudgement() noexcept;
    [[nodiscard]] RE::SpellItem* GetTheWorld() noexcept;
    [[nodiscard]] RE::SpellItem* GetTemperance() noexcept;
}