#include "Combat/ActorBounds.h"

#include <cmath>

namespace TOAM::Combat
{
    namespace
    {
        constexpr float
            kBoundsEpsilon{
                0.000001F
            };

        [[nodiscard]]
        bool IsFinitePoint(
            const RE::NiPoint3& point) noexcept
        {
            return
                std::isfinite(point.x) &&
                std::isfinite(point.y) &&
                std::isfinite(point.z);
        }
    }


    bool IsUsableActorBounds(
        const ActorBounds& bounds) noexcept
    {
        return
            bounds.valid &&
            IsFinitePoint(
                bounds.center
            ) &&
            std::isfinite(
                bounds.radius
            ) &&
            bounds.radius >
                kBoundsEpsilon;
    }


    bool ResolveActorBounds(
        RE::Actor* actor,
        ActorBounds& result) noexcept
    {
        result =
            {};

        if (!actor) {
            return false;
        }

        auto* actor3D =
            actor->Get3D();

        if (!actor3D) {
            return false;
        }

        const auto& worldBound =
            actor3D->worldBound;

        if (
            !IsFinitePoint(
                worldBound.center
            ) ||
            !std::isfinite(
                worldBound.radius
            ) ||
            worldBound.radius <=
                kBoundsEpsilon
        ) {
            return false;
        }

        result.center =
            worldBound.center;

        result.radius =
            worldBound.radius;

        result.valid =
            true;

        return true;
    }


    float InflateActorRadius(
        const ActorBounds& bounds,
        const float extraRadius) noexcept
    {
        if (
            !IsUsableActorBounds(
                bounds
            )
        ) {
            return 0.0F;
        }

        if (
            !std::isfinite(
                extraRadius
            ) ||
            extraRadius <= 0.0F
        ) {
            return
                bounds.radius;
        }

        const float inflated =
            bounds.radius +
            extraRadius;

        if (
            !std::isfinite(
                inflated
            ) ||
            inflated <= 0.0F
        ) {
            return 0.0F;
        }

        return inflated;
    }
}