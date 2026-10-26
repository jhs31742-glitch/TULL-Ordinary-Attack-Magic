#pragma once

#include "RE/Skyrim.h"

namespace TOAM::Core
{
    struct MagickaSpendResult final
    {
        bool paid{};
        float before{};
        float after{};
        float requested{};
    };

    [[nodiscard]] MagickaSpendResult TrySpendMagicka(
        RE::Actor* actor,
        float amount) noexcept;
}