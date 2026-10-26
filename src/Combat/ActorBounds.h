#pragma once

#include "RE/Skyrim.h"

namespace TOAM::Combat
{
    struct ActorBounds final
    {
        RE::NiPoint3 center{
            0.0F,
            0.0F,
            0.0F
        };

        float radius{};

        bool valid{};
    };

    [[nodiscard]]
    bool ResolveActorBounds(
        RE::Actor* actor,
        ActorBounds& result) noexcept;

    [[nodiscard]]
    bool IsUsableActorBounds(
        const ActorBounds& bounds) noexcept;

    [[nodiscard]]
    float InflateActorRadius(
        const ActorBounds& bounds,
        float extraRadius) noexcept;
}