#pragma once

#include "RE/Skyrim.h"
#include "Spells/WorldFormationData.h"

#include <array>
#include <cstddef>

namespace TOAM::Spells::World
{
    inline constexpr std::size_t
        kWorldSlotCount{
            12
        };

    using LocalDirection = FormationData::Direction;

    struct LocalFormation final
    {
        std::array<
            LocalDirection,
            kWorldSlotCount
        > directions{};

        std::array<std::size_t,kWorldSlotCount> bankSlots{};
        std::size_t count{};
        bool valid{};
    };

    [[nodiscard]]
    bool ResolveLocalDirection(
        std::size_t slotIndex,
        LocalDirection& result) noexcept;

    [[nodiscard]]
    bool BuildLocalFormation(
        std::size_t stage,
        LocalFormation& result) noexcept;
}