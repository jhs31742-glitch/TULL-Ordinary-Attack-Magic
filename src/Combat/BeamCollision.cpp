#include "Combat/BeamCollision.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

namespace TOAM::Combat
{
    namespace
    {
        constexpr float
            kDirectionEpsilon{
                0.000001F
            };

        // This is not a gameplay range.
        // It only guarantees forward progress after a Havok Actor hit.
        constexpr float
            kWorldRayAdvanceEpsilon{
                1.0F
            };

        // Safety against pathological repeated Actor collision results.
        // Exceeding this produces an unresolved query rather than silently
        // turning an Actor into world-stop authority.
        constexpr std::size_t
            kMaximumActorPassThroughIterations{
                128
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


        [[nodiscard]]
        float Dot(
            const RE::NiPoint3& left,
            const RE::NiPoint3& right) noexcept
        {
            return
                left.x * right.x +
                left.y * right.y +
                left.z * right.z;
        }


        [[nodiscard]]
        float LengthSquared(
            const RE::NiPoint3& value) noexcept
        {
            return
                Dot(
                    value,
                    value
                );
        }


        [[nodiscard]]
        RE::NiPoint3 BeamPointAt(
            const NormalizedBeamSegment& beam,
            const float distance) noexcept
        {
            return
                beam.origin +
                beam.direction *
                distance;
        }


        void ResolveNoWorldStop(
            const NormalizedBeamSegment& beam,
            BeamWorldStop& result) noexcept
        {
            result.point =
                BeamPointAt(
                    beam,
                    beam.maxDistance
                );

            result.normal =
                RE::NiPoint3{
                    0.0F,
                    0.0F,
                    0.0F
                };

            result.distance =
                beam.maxDistance;

            result.blocked =
                false;

            result.resolved =
                true;
        }


        [[nodiscard]]
        bool IsValidHitFraction(
            const float fraction) noexcept
        {
            return
                std::isfinite(
                    fraction
                ) &&
                fraction >= 0.0F &&
                fraction <= 1.0F;
        }


        [[nodiscard]]
        RE::NiPoint3 ResolveHitNormal(
            const RE::bhkPickData& pick) noexcept
        {
            const RE::NiPoint3 normal{
                pick.rayOutput.normal.quad.m128_f32[0],
                pick.rayOutput.normal.quad.m128_f32[1],
                pick.rayOutput.normal.quad.m128_f32[2]
            };

            if (
                !IsFinitePoint(
                    normal
                )
            ) {
                return RE::NiPoint3{
                    0.0F,
                    0.0F,
                    0.0F
                };
            }

            return normal;
        }
    }


    bool NormalizeBeamSegment(
        const BeamSegmentQuery& query,
        NormalizedBeamSegment& result) noexcept
    {
        result =
            {};

        if (
            !IsFinitePoint(
                query.origin
            ) ||
            !IsFinitePoint(
                query.direction
            ) ||
            !std::isfinite(
                query.maxDistance
            ) ||
            query.maxDistance <=
                0.0F ||
            !std::isfinite(
                query.radius
            ) ||
            query.radius <
                0.0F
        ) {
            return false;
        }


        const float directionLengthSquared =
            LengthSquared(
                query.direction
            );


        if (
            !std::isfinite(
                directionLengthSquared
            ) ||
            directionLengthSquared <=
                kDirectionEpsilon
        ) {
            return false;
        }


        const float inverseDirectionLength =
            1.0F /
            std::sqrt(
                directionLengthSquared
            );


        result.origin =
            query.origin;

        result.direction =
            query.direction *
            inverseDirectionLength;

        result.maxDistance =
            query.maxDistance;

        result.radius =
            query.radius;

        result.valid =
            true;


        return
            IsFinitePoint(
                result.direction
            );
    }


    bool IntersectActorBounds(
        const NormalizedBeamSegment& beam,
        const ActorBounds& bounds,
        BeamActorIntersection& result) noexcept
    {
        result =
            {};

        if (
            !beam.valid ||
            !IsFinitePoint(
                beam.origin
            ) ||
            !IsFinitePoint(
                beam.direction
            ) ||
            !std::isfinite(
                beam.maxDistance
            ) ||
            beam.maxDistance <=
                0.0F ||
            !std::isfinite(
                beam.radius
            ) ||
            beam.radius <
                0.0F ||
            !IsUsableActorBounds(
                bounds
            )
        ) {
            return false;
        }


        const float combinedRadius =
            bounds.radius +
            beam.radius;


        if (
            !std::isfinite(
                combinedRadius
            ) ||
            combinedRadius <=
                0.0F
        ) {
            return false;
        }


        const RE::NiPoint3 offset =
            beam.origin -
            bounds.center;


        const float projection =
            Dot(
                offset,
                beam.direction
            );


        const float c =
            Dot(
                offset,
                offset
            ) -
            combinedRadius *
            combinedRadius;


        if (c <= 0.0F) {

            const float discriminant =
                projection *
                projection -
                c;


            if (
                !std::isfinite(
                    discriminant
                ) ||
                discriminant <
                    0.0F
            ) {
                return false;
            }


            float exitDistance =
                -projection +
                std::sqrt(
                    discriminant
                );


            if (
                !std::isfinite(
                    exitDistance
                ) ||
                exitDistance <
                    0.0F
            ) {
                return false;
            }


            if (
                exitDistance >
                beam.maxDistance
            ) {
                exitDistance =
                    beam.maxDistance;
            }


            result.entryPoint =
                beam.origin;

            result.entryDistance =
                0.0F;

            result.exitDistance =
                exitDistance;

            result.bounds =
                bounds;

            result.hit =
                true;

            return true;
        }


        if (projection > 0.0F) {
            return false;
        }


        const float discriminant =
            projection *
            projection -
            c;


        if (
            !std::isfinite(
                discriminant
            ) ||
            discriminant <
                0.0F
        ) {
            return false;
        }


        const float root =
            std::sqrt(
                discriminant
            );


        float entryDistance =
            -projection -
            root;

        float exitDistance =
            -projection +
            root;


        if (
            !std::isfinite(
                entryDistance
            ) ||
            !std::isfinite(
                exitDistance
            )
        ) {
            return false;
        }


        if (
            exitDistance <
            0.0F
        ) {
            return false;
        }


        if (
            entryDistance <
            0.0F
        ) {
            entryDistance =
                0.0F;
        }


        if (
            entryDistance >
            beam.maxDistance
        ) {
            return false;
        }


        if (
            exitDistance >
            beam.maxDistance
        ) {
            exitDistance =
                beam.maxDistance;
        }


        result.entryPoint =
            BeamPointAt(
                beam,
                entryDistance
            );

        result.entryDistance =
            entryDistance;

        result.exitDistance =
            exitDistance;

        result.bounds =
            bounds;

        result.hit =
            true;

        return true;
    }


    bool IntersectActor(
        const NormalizedBeamSegment& beam,
        RE::Actor* actor,
        BeamActorIntersection& result) noexcept
    {
        result =
            {};

        if (!actor) {
            return false;
        }


        ActorBounds bounds{};

        if (
            !ResolveActorBounds(
                actor,
                bounds
            )
        ) {
            return false;
        }


        return
            IntersectActorBounds(
                beam,
                bounds,
                result
            );
    }


    bool IsIntersectionBeforeStop(
        const BeamActorIntersection& intersection,
        const float stopDistance) noexcept
    {
        if (
            !intersection.hit ||
            !std::isfinite(
                intersection.entryDistance
            ) ||
            intersection.entryDistance <
                0.0F ||
            !std::isfinite(
                stopDistance
            ) ||
            stopDistance <
                0.0F
        ) {
            return false;
        }


        return
            intersection.entryDistance <=
            stopDistance;
    }


    bool ResolveBeamWorldStop(
        const NormalizedBeamSegment& beam,
        BeamWorldStop& result) noexcept
    {
        result =
            {};


        if (
            !beam.valid ||
            !IsFinitePoint(
                beam.origin
            ) ||
            !IsFinitePoint(
                beam.direction
            ) ||
            !std::isfinite(
                beam.maxDistance
            ) ||
            beam.maxDistance <=
                0.0F
        ) {
            return false;
        }


        auto* tes =
            RE::TES::
                GetSingleton();


        if (!tes) {
            return false;
        }


        const float havokScale =
            RE::bhkWorld::
                GetWorldScale();


        if (
            !std::isfinite(
                havokScale
            ) ||
            havokScale <=
                0.0F
        ) {
            return false;
        }


        float cursorDistance =
            0.0F;


        for (
            std::size_t iteration = 0;
            iteration <
                kMaximumActorPassThroughIterations;
            ++iteration
        ) {

            if (
                cursorDistance >=
                beam.maxDistance
            ) {

                ResolveNoWorldStop(
                    beam,
                    result
                );

                return true;
            }


            const float remainingDistance =
                beam.maxDistance -
                cursorDistance;


            if (
                !std::isfinite(
                    remainingDistance
                ) ||
                remainingDistance <=
                    0.0F
            ) {

                ResolveNoWorldStop(
                    beam,
                    result
                );

                return true;
            }


            const RE::NiPoint3 castStart =
                BeamPointAt(
                    beam,
                    cursorDistance
                );


            const RE::NiPoint3 castEnd =
                BeamPointAt(
                    beam,
                    beam.maxDistance
                );


            const RE::NiPoint3 castVector =
                castEnd -
                castStart;


            RE::bhkPickData pick{};


            pick.rayInput.from =
                RE::hkVector4{
                    castStart.x *
                        havokScale,

                    castStart.y *
                        havokScale,

                    castStart.z *
                        havokScale,

                    0.0F
                };


            pick.rayInput.to =
                RE::hkVector4{
                    castEnd.x *
                        havokScale,

                    castEnd.y *
                        havokScale,

                    castEnd.z *
                        havokScale,

                    0.0F
                };


            pick.rayInput.enableShapeCollectionFilter =
                false;


            pick.rayInput.filterInfo.
                SetCollisionLayer(
                    RE::COL_LAYER::kLOS
                );


            pick.ray =
                RE::hkVector4{
                    castVector.x *
                        havokScale,

                    castVector.y *
                        havokScale,

                    castVector.z *
                        havokScale,

                    0.0F
                };


            pick.rayOutput.Reset();


            RE::NiAVObject* pickedObject =
                tes->Pick(
                    pick
                );


            if (pick.pickFailed) {
                return false;
            }


            if (
                !pick.rayOutput.HasHit()
            ) {

                ResolveNoWorldStop(
                    beam,
                    result
                );

                return true;
            }


            const float fraction =
                pick.rayOutput.hitFraction;


            if (
                !IsValidHitFraction(
                    fraction
                )
            ) {
                return false;
            }


            const float localHitDistance =
                remainingDistance *
                fraction;


            const float globalHitDistance =
                cursorDistance +
                localHitDistance;


            if (
                !std::isfinite(
                    globalHitDistance
                ) ||
                globalHitDistance <
                    cursorDistance ||
                globalHitDistance >
                    beam.maxDistance
            ) {
                return false;
            }


            RE::TESObjectREFR* hitReference =
                pickedObject ?
                    pickedObject->GetUserData() :
                    nullptr;


            RE::Actor* hitActor =
                hitReference ?
                    hitReference->As<
                        RE::Actor>() :
                    nullptr;


            // Actor collision is explicitly NOT Beam-stop authority.
            if (hitActor) {

                BeamActorIntersection
                    actorIntersection{};


                if (
                    !IntersectActor(
                        beam,
                        hitActor,
                        actorIntersection
                    )
                ) {

                    // We know Havok reported an Actor but cannot prove
                    // a safe pass-through endpoint from runtime bounds.
                    // Fail unresolved instead of silently treating the
                    // Actor as terrain or jumping an arbitrary distance.
                    return false;
                }


                float nextDistance =
                    actorIntersection.
                        exitDistance +
                    kWorldRayAdvanceEpsilon;


                if (
                    nextDistance <=
                    globalHitDistance
                ) {

                    nextDistance =
                        globalHitDistance +
                        kWorldRayAdvanceEpsilon;
                }


                if (
                    !std::isfinite(
                        nextDistance
                    ) ||
                    nextDistance <=
                        cursorDistance
                ) {
                    return false;
                }


                if (
                    nextDistance >=
                    beam.maxDistance
                ) {

                    ResolveNoWorldStop(
                        beam,
                        result
                    );

                    return true;
                }


                cursorDistance =
                    nextDistance;

                continue;
            }


            // Any non-Actor Havok obstruction reached by the LOS query
            // is world-stop authority for this shared Beam resolver.
            // Bounded, near-stop-only diagnostics; collision policy unchanged.
            if (globalHitDistance <= 256.0F) {
                static std::atomic<std::uint32_t> nearStopLogs{0};
                if (nearStopLogs.fetch_add(1, std::memory_order_relaxed) < 32) {
                    logger::info(
                        "[TOAM][S04TRACE] WORLD_STOP_NEAR | Distance={:.3f} | Cursor={:.3f} | Fraction={:.5f} | Picked={} | HasRef={} | RefID={:08X}",
                        globalHitDistance,
                        cursorDistance,
                        fraction,
                        pickedObject != nullptr,
                        hitReference != nullptr,
                        hitReference ? hitReference->GetFormID() : 0U);
                }
            }
            result.point =
                BeamPointAt(
                    beam,
                    globalHitDistance
                );


            result.normal =
                ResolveHitNormal(
                    pick
                );


            result.distance =
                globalHitDistance;

            result.blocked =
                true;

            result.resolved =
                true;

            return true;
        }


        // Pathological repeated Actor collisions.
        // Fail closed as unresolved; never invent a wall or pass through
        // an unknown world state.
        return false;
    }


    bool CollectBeamActorHits(
        const NormalizedBeamSegment& beam,
        const float stopDistance,
        RE::Actor* sourceActor,
        std::vector<BeamActorHit>& result) noexcept
    {
        result.clear();


        if (
            !beam.valid ||
            !std::isfinite(
                stopDistance
            ) ||
            stopDistance <
                0.0F ||
            stopDistance >
                beam.maxDistance
        ) {
            return false;
        }


        auto* processLists =
            RE::ProcessLists::
                GetSingleton();


        if (!processLists) {
            return false;
        }


        processLists->
            ForAllActors(
                [&](RE::Actor* actor)
                {
                    if (
                        !actor ||
                        actor ==
                            sourceActor
                    ) {
                        return
                            RE::BSContainer::
                                ForEachResult::
                                    kContinue;
                    }


                    BeamActorIntersection
                        intersection{};


                    if (
                        !IntersectActor(
                            beam,
                            actor,
                            intersection
                        )
                    ) {
                        return
                            RE::BSContainer::
                                ForEachResult::
                                    kContinue;
                    }


                    if (
                        !IsIntersectionBeforeStop(
                            intersection,
                            stopDistance
                        )
                    ) {
                        return
                            RE::BSContainer::
                                ForEachResult::
                                    kContinue;
                    }


                    BeamActorHit hit{};

                    hit.actor =
                        actor->GetHandle();

                    hit.intersection =
                        intersection;


                    result.push_back(
                        hit
                    );


                    return
                        RE::BSContainer::
                            ForEachResult::
                                kContinue;
                }
            );


        std::sort(
            result.begin(),
            result.end(),
            [](
                const BeamActorHit& left,
                const BeamActorHit& right)
            {
                return
                    left.intersection.
                        entryDistance <
                    right.intersection.
                        entryDistance;
            }
        );


        return true;
    }


    bool ResolveBeamCollision(
        const BeamSegmentQuery& query,
        RE::Actor* sourceActor,
        BeamCollisionResult& result) noexcept
    {
        result =
            {};


        if (
            !NormalizeBeamSegment(
                query,
                result.beam
            )
        ) {
            return false;
        }


        if (
            !ResolveBeamWorldStop(
                result.beam,
                result.worldStop
            )
        ) {
            result =
                {};

            return false;
        }


        if (
            !result.worldStop.resolved
        ) {
            result =
                {};

            return false;
        }


        const float actorStopDistance =
            result.worldStop.distance;


        if (
            !CollectBeamActorHits(
                result.beam,
                actorStopDistance,
                sourceActor,
                result.actorHits
            )
        ) {
            result =
                {};

            return false;
        }


        result.resolved =
            true;

        return true;
    }
}