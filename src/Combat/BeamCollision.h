#pragma once

#include "Combat/ActorBounds.h"

#include "RE/Skyrim.h"

#include <vector>

namespace TOAM::Combat
{
    struct BeamSegmentQuery final
    {
        RE::NiPoint3 origin{
            0.0F,
            0.0F,
            0.0F
        };

        RE::NiPoint3 direction{
            0.0F,
            1.0F,
            0.0F
        };

        float maxDistance{};
        float radius{};
    };


    struct NormalizedBeamSegment final
    {
        RE::NiPoint3 origin{
            0.0F,
            0.0F,
            0.0F
        };

        RE::NiPoint3 direction{
            0.0F,
            1.0F,
            0.0F
        };

        float maxDistance{};
        float radius{};

        bool valid{};
    };


    struct BeamActorIntersection final
    {
        RE::NiPoint3 entryPoint{
            0.0F,
            0.0F,
            0.0F
        };

        float entryDistance{};
        float exitDistance{};

        ActorBounds bounds{};

        bool hit{};
    };


    struct BeamWorldStop final
    {
        RE::NiPoint3 point{
            0.0F,
            0.0F,
            0.0F
        };

        RE::NiPoint3 normal{
            0.0F,
            0.0F,
            0.0F
        };

        float distance{};

        bool blocked{};
        bool resolved{};
    };


    struct BeamActorHit final
    {
        RE::ActorHandle actor{};
        BeamActorIntersection intersection{};
    };


    struct BeamCollisionResult final
    {
        NormalizedBeamSegment beam{};

        BeamWorldStop worldStop{};

        std::vector<BeamActorHit>
            actorHits{};

        bool resolved{};
    };


    [[nodiscard]]
    bool NormalizeBeamSegment(
        const BeamSegmentQuery& query,
        NormalizedBeamSegment& result) noexcept;


    [[nodiscard]]
    bool IntersectActorBounds(
        const NormalizedBeamSegment& beam,
        const ActorBounds& bounds,
        BeamActorIntersection& result) noexcept;


    [[nodiscard]]
    bool IntersectActor(
        const NormalizedBeamSegment& beam,
        RE::Actor* actor,
        BeamActorIntersection& result) noexcept;


    [[nodiscard]]
    bool IsIntersectionBeforeStop(
        const BeamActorIntersection& intersection,
        float stopDistance) noexcept;


    [[nodiscard]]
    bool ResolveBeamWorldStop(
        const NormalizedBeamSegment& beam,
        BeamWorldStop& result) noexcept;


    [[nodiscard]]
    bool CollectBeamActorHits(
        const NormalizedBeamSegment& beam,
        float stopDistance,
        RE::Actor* sourceActor,
        std::vector<BeamActorHit>& result) noexcept;


    [[nodiscard]]
    bool ResolveBeamCollision(
        const BeamSegmentQuery& query,
        RE::Actor* sourceActor,
        BeamCollisionResult& result) noexcept;
}