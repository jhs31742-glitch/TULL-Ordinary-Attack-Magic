#include "Spells/WorldFormation.h"

#include "Spells/WorldProfile.h"

#include <array>

namespace TOAM::Spells::World
{
    namespace
    {
        constexpr auto kWorldDirections = FormationData::kDirections;
    }


    static_assert(
        kWorldDirections.size() ==
            kWorldSlotCount
    );


    bool ResolveLocalDirection(
        const std::size_t slotIndex,
        LocalDirection& result) noexcept
    {
        result = {};

        if (
            slotIndex >=
            kWorldDirections.size()
        ) {
            return false;
        }

        result =
            kWorldDirections[
                slotIndex
            ];

        return true;
    }


    bool BuildLocalFormation(
        const std::size_t stage,
        LocalFormation& result) noexcept
    {
        result = {};

        const std::size_t count =
            CircleCountForStage(
                stage
            );

        if (
            count == 0 ||
            count >
                kWorldDirections.size()
        ) {
            return false;
        }

        for (
            std::size_t index = 0;
            index < count;
            ++index
        ) {
            const auto bankSlot=FormationData::ActiveSlot(stage,index);
            result.bankSlots[index]=bankSlot;
            result.directions[index]=kWorldDirections[bankSlot];
        }

        result.count =
            count;

        result.valid =
            true;

        return true;
    }
}