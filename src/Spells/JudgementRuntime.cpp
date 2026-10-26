#include "Arcana/Integration.h"
#include "Audio/SpellAudio.h"

#include "Spells/JudgementRuntime.h"

#include "Charge/ChargeRuntime.h"
#include "Combat/FixedDamage.h"
#include "VFX/BeamRenderer.h"
#include "VFX/CircleRenderer.h"
#include "VFX/TransientVFXManager.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace TOAM::Spells::Judgement::Runtime
{
    namespace
    {
        constexpr float
            kDirectionEpsilon{
                0.000001F
            };


        // ----------------------------------------------------
        // Judgement visual-only starting tuning.
        //
        // These values do NOT alter gameplay beam radius,
        // explosion radius, formation radius, or damage.
        //
        // Final visual acceptance remains runtime-tunable.
        // ----------------------------------------------------

        constexpr float
            kMarkingVisualBeamWidth{
                0.35F
            };




        constexpr float
            kMarkingVisualLifetimeSeconds{
                0.08F
            };


        constexpr float
            kArtilleryVisualLifetimeSeconds{
                0.48F
            };


        constexpr float
            kArtilleryCircleVisualScale{
                0.10F
            };


        // Logical explosion gameplay remains:
        //
        //     50 fixed
        //     3 ft / 64 world units
        //
        // The values below affect presentation only.
        constexpr float
            kExplosionImpactVisualScale{
                0.75F
            };


        constexpr float
            kExplosionImpactVisualLifetimeSeconds{
                2.20F
            };


        [[nodiscard]]
        bool IsFinitePoint(
            const RE::NiPoint3& point) noexcept
        {
            return
                std::isfinite(
                    point.x
                ) &&
                std::isfinite(
                    point.y
                ) &&
                std::isfinite(
                    point.z
                );
        }


        [[nodiscard]]
        float LengthSquared(
            const RE::NiPoint3& value) noexcept
        {
            return
                value.x *
                    value.x +
                value.y *
                    value.y +
                value.z *
                    value.z;
        }


        [[nodiscard]]
        bool IsHostileToSource(
            RE::Actor* sourceActor,
            RE::Actor* targetActor) noexcept
        {
            return
                sourceActor &&
                targetActor &&
                sourceActor !=
                    targetActor &&
                !targetActor->IsDead() &&
                targetActor->
                    IsHostileToActor(
                        sourceActor
                    );
        }


        [[nodiscard]]
        bool IntersectsExplosionSphere(
            const RE::NiPoint3& center,
            const float explosionRadius,
            const Combat::ActorBounds& bounds) noexcept
        {
            if (
                !IsFinitePoint(
                    center
                ) ||
                !std::isfinite(
                    explosionRadius
                ) ||
                explosionRadius <=
                    0.0F ||
                !Combat::IsUsableActorBounds(
                    bounds
                )
            ) {
                return false;
            }


            const float combinedRadius =
                explosionRadius +
                bounds.radius;


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
                bounds.center -
                center;


            const float distanceSquared =
                LengthSquared(
                    offset
                );


            const float radiusSquared =
                combinedRadius *
                combinedRadius;


            return
                std::isfinite(
                    distanceSquared
                ) &&
                std::isfinite(
                    radiusSquared
                ) &&
                distanceSquared <=
                    radiusSquared;
        }


        [[nodiscard]]
        bool NormalizeDirection(
            const RE::NiPoint3& value,
            RE::NiPoint3& result) noexcept
        {
            const float lengthSquared =
                LengthSquared(
                    value
                );


            if (
                !std::isfinite(
                    lengthSquared
                ) ||
                lengthSquared <=
                    kDirectionEpsilon
            ) {
                return false;
            }


            const float inverseLength =
                1.0F /
                std::sqrt(
                    lengthSquared
                );


            result =
                value *
                inverseLength;


            return
                IsFinitePoint(
                    result
                );
        }


        [[nodiscard]]
        bool ResolveMarkingAim(
            RE::PlayerCharacter* player,
            RE::NiPoint3& origin,
            RE::NiPoint3& direction) noexcept
        {
            if (!player) {
                return false;
            }


            Combat::ActorBounds
                playerBounds{};


            if (
                !Combat::ResolveActorBounds(
                    player,
                    playerBounds
                )
            ) {
                return false;
            }


            // CP021: reuse v0.7.4's actual actor-center validity policy.
            // Long player-owned VFX bounds must not become the targeting origin.
            origin = Arcana::HostJudgementGuideAnchor(player, false);
            const auto rejected=playerBounds.center-origin;
            if(rejected.x*rejected.x+rejected.y*rejected.y+rejected.z*rejected.z>0.01F){
                static unsigned audits{};if(audits++<8)logger::warn(
                    "[TOAM][CP021GUIDE] ACTOR_BOUND_ORIGIN_REJECTED | Bound=({},{},{}) | Center=({},{},{}) | Policy=SHOULDER_GUIDE_AXIS",
                    playerBounds.center.x,playerBounds.center.y,playerBounds.center.z,origin.x,origin.y,origin.z);
            }


            if (
                !IsFinitePoint(
                    origin
                )
            ) {
                return false;
            }


            return Arcana::HostAimDirection(player, direction);
        }


        [[nodiscard]]
        bool BuildMarkingQuery(
            RE::PlayerCharacter* player,
            Combat::BeamSegmentQuery& result,
            RE::NiPoint3& forwardAxis) noexcept
        {
            result =
                {};


            RE::NiPoint3 origin{};
            RE::NiPoint3 direction{};


            if (
                !ResolveMarkingAim(
                    player,
                    origin,
                    direction
                )
            ) {
                return false;
            }


            result.origin =
                origin;


            result.direction =
                direction;


            result.maxDistance =
                kMarkingRangeWorldUnits;


            result.radius =
                kMarkingBeamRadiusWorldUnits;


            forwardAxis =
                direction;


            return true;
        }


        inline constexpr std::size_t
            kArtilleryScheduleCapacity{
                Combat::TargetFormation::
                    kCapacity
            };


        struct ArtilleryVisualState final
        {
            VFX::CircleRenderer
                circles{};

            VFX::BeamRenderer
                beam{};
        };


        struct RuntimeState final
        {
            std::array<
                ArtillerySchedule,
                kArtilleryScheduleCapacity
            > schedules{};

            std::array<
                ArtilleryVisualState,
                kArtilleryScheduleCapacity
            > visuals{};

            VFX::BeamRenderer
                markingBeam{};

            // Charge-only, damage-free guide. Never share markingBeam.
            VFX::BeamRenderer chargeGuide{};
            VFX::CircleRenderer chargeGuideCircle{};
            RE::NiNode* chargeGuideCircleRoot{};
            RE::NiNode* chargeGuideRoot{};
            bool chargeGuideWanted{};
            bool chargeGuideVisible{};
            RE::ActorHandle chargeGuideTarget{};
            float chargeGuideScanRemaining{};
            float chargeGuideLength{426.67F};
        };


        [[nodiscard]]
        RuntimeState& GetRuntimeState() noexcept
        {
            static RuntimeState
                state{};


            return state;
        }


        struct VisualBasis final
        {
            RE::NiPoint3 x{};
            RE::NiPoint3 y{};
            RE::NiPoint3 z{};
        };


        [[nodiscard]]
        bool IsFiniteTransform(
            const RE::NiTransform& transform) noexcept
        {
            if (
                !IsFinitePoint(
                    transform.translate
                ) ||
                !std::isfinite(
                    transform.scale
                ) ||
                transform.scale <=
                    0.0F
            ) {
                return false;
            }


            for (
                std::size_t row = 0;
                row < 3;
                ++row
            ) {
                for (
                    std::size_t column = 0;
                    column < 3;
                    ++column
                ) {
                    if (
                        !std::isfinite(
                            transform.rotate.
                                entry[row][column]
                        )
                    ) {
                        return false;
                    }
                }
            }


            return true;
        }


        [[nodiscard]]
        bool BuildVisualBasis(
            const RE::NiPoint3& forward,
            VisualBasis& result) noexcept
        {
            if (
                !NormalizeDirection(
                    forward,
                    result.y
                )
            ) {
                return false;
            }


            const RE::NiPoint3 reference =
                std::fabs(
                    result.y.z
                ) < 0.999F ?
                    RE::NiPoint3{
                        0.0F,
                        0.0F,
                        1.0F
                    } :
                    RE::NiPoint3{
                        1.0F,
                        0.0F,
                        0.0F
                    };


            if (
                !NormalizeDirection(
                    result.y.Cross(
                        reference
                    ),
                    result.x
                )
            ) {
                return false;
            }


            if (
                !NormalizeDirection(
                    result.x.Cross(
                        result.y
                    ),
                    result.z
                )
            ) {
                return false;
            }


            return true;
        }


        void ApplyVisualBasis(
            RE::NiTransform& transform,
            const VisualBasis& basis) noexcept
        {
            transform.rotate.entry[0][0] =
                basis.x.x;

            transform.rotate.entry[1][0] =
                basis.x.y;

            transform.rotate.entry[2][0] =
                basis.x.z;


            transform.rotate.entry[0][1] =
                basis.y.x;

            transform.rotate.entry[1][1] =
                basis.y.y;

            transform.rotate.entry[2][1] =
                basis.y.z;


            transform.rotate.entry[0][2] =
                basis.z.x;

            transform.rotate.entry[1][2] =
                basis.z.y;

            transform.rotate.entry[2][2] =
                basis.z.z;
        }


        [[nodiscard]]
        bool ResolveVisualRoot(
            RE::NiNode*& parent,
            RE::NiTransform& rootInverse) noexcept
        {
            auto* worldRoot =
                RE::Main::
                    WorldRootNode();


            if (
                !worldRoot ||
                !std::isfinite(
                    worldRoot->world.scale
                ) ||
                std::fabs(
                    worldRoot->world.scale
                ) <=
                    kDirectionEpsilon
            ) {
                return false;
            }


            rootInverse =
                worldRoot->world.
                    Invert();


            if (
                !IsFiniteTransform(
                    rootInverse
                )
            ) {
                return false;
            }


            parent =
                worldRoot;


            return true;
        }


        [[nodiscard]] float ArtilleryPresentationWidth(const RE::NiPoint3& origin,const RE::NiPoint3& aim) noexcept
        {
            const float candidate=0.10F*std::sqrt(LengthSquared(aim-origin))/99.84F;
            return candidate<0.35F ? candidate : 0.35F;
        }

        [[nodiscard]]
        bool ShowBeamVisual(
            VFX::BeamRenderer& renderer,
            const RE::NiPoint3& worldOrigin,
            const RE::NiPoint3& worldEnd,
            const float worldWidth,
            const float lifetimeSeconds,
            const bool wholeLens=false) noexcept
        {
            if (
                !IsFinitePoint(
                    worldOrigin
                ) ||
                !IsFinitePoint(
                    worldEnd
                ) ||
                !std::isfinite(
                    worldWidth
                ) ||
                worldWidth <=
                    0.0F ||
                !std::isfinite(
                    lifetimeSeconds
                ) ||
                lifetimeSeconds <=
                    0.0F
            ) {
                return false;
            }


            RE::NiNode* parent{};
            RE::NiTransform rootInverse{};


            if (
                !ResolveVisualRoot(
                    parent,
                    rootInverse
                )
            ) {
                return false;
            }


            const RE::NiPoint3
                localOrigin =
                    rootInverse *
                    worldOrigin;


            const RE::NiPoint3
                localEnd =
                    rootInverse *
                    worldEnd;


            const RE::NiPoint3
                localVector =
                    localEnd -
                    localOrigin;


            const float
                localLengthSquared =
                    LengthSquared(
                        localVector
                    );


            if (
                !std::isfinite(
                    localLengthSquared
                ) ||
                localLengthSquared <=
                    kDirectionEpsilon
            ) {
                return false;
            }


            const float
                localLength =
                    std::sqrt(
                        localLengthSquared
                    );


            RE::NiPoint3
                localDirection{};


            if (
                !NormalizeDirection(
                    localVector,
                    localDirection
                )
            ) {
                return false;
            }


            const float
                localWidth =
                    worldWidth *
                    std::fabs(
                        rootInverse.scale
                    );


            if (
                !std::isfinite(
                    localWidth
                ) ||
                localWidth <=
                    0.0F
            ) {
                return false;
            }


            VFX::BeamPose pose{};


            pose.origin =
                localOrigin;

            pose.direction =
                localDirection;

            pose.length =
                localLength;

            pose.widthScale =
                localWidth;

            pose.lifetimeSeconds =
                lifetimeSeconds;

            pose.visible =
                true;

            pose.asset =
                VFX::AssetID::
                    kBeamBodyUnitStatic;
            if(wholeLens) {pose.asset=VFX::AssetID::kBeamWholeLens;}


            const bool spawned=renderer.SetBeam(parent,pose);
            if(spawned && wholeLens) {renderer.Detach();}
            return spawned;
        }


        [[nodiscard]]
        bool ShowExplosionImpactVisual(
            const RE::NiPoint3& worldPosition) noexcept
        {
            if (
                !IsFinitePoint(
                    worldPosition
                )
            ) {
                return false;
            }


            RE::NiNode* parent{};
            RE::NiTransform rootInverse{};


            if (
                !ResolveVisualRoot(
                    parent,
                    rootInverse
                )
            ) {
                return false;
            }


            RE::NiTransform
                worldTransform{};


            worldTransform.translate =
                worldPosition;


            const RE::NiPoint3
                zeroRotation{
                    0.0F,
                    0.0F,
                    0.0F
                };


            worldTransform.rotate.
                SetEulerAnglesXYZ(
                    zeroRotation
                );


            worldTransform.scale =
                kExplosionImpactVisualScale;


            const RE::NiTransform
                localTransform =
                    rootInverse *
                    worldTransform;


            if (
                !IsFiniteTransform(
                    localTransform
                )
            ) {
                return false;
            }


            VFX::SpawnRequest
                request{};


            request.asset =
                VFX::AssetID::
                    kFlashCloud;


            request.parent =
                parent;


            request.local =
                localTransform;


            request.lifetimeSeconds =
                kExplosionImpactVisualLifetimeSeconds;


            request.visible =
                true;


            const auto handle =
                VFX::TransientVFXManager::
                    Get().
                    Spawn(
                        request
                    );


            return
                handle.IsValid();
        }


        [[nodiscard]]
        std::size_t SpawnArtilleryCircleVisuals(
            const std::size_t scheduleIndex,
            const WorldFixedArtilleryPlan& plan) noexcept
        {
            if (
                scheduleIndex >=
                    kArtilleryScheduleCapacity ||
                !plan.valid ||
                !plan.formation.valid
            ) {
                return 0;
            }


            RE::NiNode* parent{};
            RE::NiTransform rootInverse{};


            if (
                !ResolveVisualRoot(
                    parent,
                    rootInverse
                )
            ) {
                return 0;
            }


            auto& state =
                GetRuntimeState();


            auto& visual =
                state.visuals[
                    scheduleIndex
                ];


            visual.circles.
                ReleaseAll();


            std::size_t
                spawnedCount{};


            for (
                std::size_t index = 0;
                index <
                    plan.formation.count;
                ++index
            ) {
                const RE::NiPoint3
                    worldPosition =
                        plan.formation.
                            points[index];


                const RE::NiPoint3
                    facingVector =
                        plan.fixedAimPoint -
                        worldPosition;


                VisualBasis
                    basis{};


                if (
                    !BuildVisualBasis(
                        facingVector,
                        basis
                    )
                ) {
                    continue;
                }


                RE::NiTransform
                    worldTransform{};


                worldTransform.translate =
                    worldPosition;


                // Keep existing fixed origins and damage rays; scale only visible artillery.
                const float visualRatio = plan.formation.radius / 99.84F;
                const float boundedRatio = visualRatio < 1.0F ? 1.0F : (visualRatio > 10.0F ? 10.0F : visualRatio);
                worldTransform.scale = kArtilleryCircleVisualScale * boundedRatio;


                ApplyVisualBasis(
                    worldTransform,
                    basis
                );


                const RE::NiTransform
                    localTransform =
                        rootInverse *
                        worldTransform;


                if (
                    !IsFiniteTransform(
                        localTransform
                    )
                ) {
                    continue;
                }


                if (
                    visual.circles.
                        SpawnTransform(
                            index,
                            parent,
                            VFX::AssetID::
                                kCircleStatic,
                            localTransform,
                            true
                        )
                ) {
                    static_cast<void>(visual.circles.BeginGrowth(index,0.24F));
                    ++spawnedCount;
                }
            }


            return spawnedCount;
        }


        void ReleaseArtilleryCircleVisual(
            const std::size_t scheduleIndex,
            const std::size_t shotIndex) noexcept
        {
            if (
                scheduleIndex >=
                    kArtilleryScheduleCapacity ||
                shotIndex >=
                    VFX::CircleRenderer::
                        kCapacity
            ) {
                return;
            }


            static_cast<void>(
                GetRuntimeState().
                    visuals[
                        scheduleIndex
                    ].
                    circles.
                    Retire(
                        shotIndex,0.60F
                    )
            );
        }


        [[nodiscard]]
        ArtillerySchedule*
        AcquireScheduleSlot(
            std::size_t& resultIndex) noexcept
        {
            auto& state =
                GetRuntimeState();


            resultIndex =
                kArtilleryScheduleCapacity;


            for (
                std::size_t index = 0;
                index <
                    state.schedules.size();
                ++index
            ) {
                auto& schedule =
                    state.schedules[
                        index
                    ];


                if (
                    schedule.active
                ) {
                    continue;
                }


                // A previous short beam may already have expired in the
                // manager. Release is still the bounded ownership cleanup
                // boundary before this slot is reused.
                state.visuals[index].
                    circles.
                    ReleaseAll();


                state.visuals[index].
                    beam.
                    Release();


                schedule =
                    {};


                resultIndex =
                    index;


                return
                    std::addressof(
                        schedule
                    );
            }


            return nullptr;
        }


        [[nodiscard]]
        bool BuildStageSubset(
            const Combat::TargetFormation& fullFormation,
            const std::size_t requestedCount,
            Combat::TargetFormation& result) noexcept
        {
            result =
                {};


            if (
                !fullFormation.valid ||
                fullFormation.count !=
                    Combat::kUpperHemisphereSlotCount ||
                !Combat::IsSupportedFormationCount(
                    requestedCount
                )
            ) {
                return false;
            }


            std::array<std::size_t, Combat::kUpperHemisphereSlotCount> chosen{};
            if (!Combat::SelectDistributedUpperHemisphereSlots(
                    requestedCount, 0, chosen)) {
                return false;
            }
            for (std::size_t i = 0; i < requestedCount; ++i) {
                result.points[i] = fullFormation.points[chosen[i]];
            }


            result.center =
                fullFormation.center;


            result.radius =
                fullFormation.radius;


            result.count =
                requestedCount;


            result.valid =
                true;


            return true;
        }
    }


    bool BuildReleasePlan(
        const float effectiveCharge,
        const Combat::BeamCollisionResult&
            markingCollision,
        ReleasePlan& result) noexcept
    {
        result =
            {};


        const auto* profile =
            ResolveStage(
                effectiveCharge
            );


        if (
            !profile ||
            !markingCollision.resolved ||
            markingCollision.actorHits.empty()
        ) {
            return false;
        }


        // Collision hits are sorted by entry distance. Corpses and
        // malformed hits cannot become new Judgement marking targets.
        // Already-started artillery retains its last-known aim snapshot.
        const Combat::BeamActorHit* chosen{};
        for (const auto& candidate : markingCollision.actorHits) {
            auto target = candidate.actor.get();
            auto* player=RE::PlayerCharacter::GetSingleton();
            if (!target || target->IsDead() || !player ||
                !target->IsHostileToActor(player) ||
                !candidate.intersection.hit ||
                !IsFinitePoint(candidate.intersection.entryPoint) ||
                !Combat::IsUsableActorBounds(candidate.intersection.bounds)) {
                continue;
            }
            chosen = &candidate;
            break;
        }
        if (!chosen) {
            return false;
        }
        const auto& firstHit = *chosen;


        result.profile =
            profile;


        result.mark.target =
            firstHit.actor;


        result.mark.initialHitPoint =
            firstHit.intersection.
                entryPoint;


        result.mark.initialBounds =
            firstHit.intersection.
                bounds;


        result.mark.valid =
            true;


        result.valid =
            true;


        return true;
    }


    bool BuildWorldFixedArtilleryPlan(
        const ReleasePlan& releasePlan,
        const RE::NiPoint3& forwardAxis,
        const float radiusScale,
        WorldFixedArtilleryPlan& result) noexcept
    {
        result =
            {};


        if (
            !releasePlan.valid ||
            !releasePlan.profile ||
            !releasePlan.mark.valid ||
            !Combat::IsUsableActorBounds(
                releasePlan.mark.initialBounds
            ) ||
            !IsFinitePoint(
                forwardAxis
            ) ||
            !std::isfinite(
                radiusScale
            ) ||
            radiusScale <=
                0.0F
        ) {
            return false;
        }


        const std::size_t requestedCount =
            static_cast<std::size_t>(
                releasePlan.profile->
                    circleCount
            );


        if (
            !Combat::IsSupportedFormationCount(
                requestedCount
            )
        ) {
            return false;
        }


        const float formationRadius =
            releasePlan.mark.initialBounds.
                radius *
            radiusScale;


        if (
            !std::isfinite(
                formationRadius
            ) ||
            formationRadius <=
                0.0F
        ) {
            return false;
        }


        Combat::TargetFormation
            fullFormation{};


        // The original marked Actor bounds-center is the formation center.
        //
        // BuildUpperHemisphereReference produces all nine world-space
        // positions immediately. Once copied into the artillery plan these
        // positions do not depend on later Actor movement or survival.
        if (
            !Combat::BuildUpperHemisphereReference(
                releasePlan.mark.initialBounds.
                    center,
                forwardAxis,
                formationRadius,
                fullFormation
            )
        ) {
            return false;
        }


        Combat::TargetFormation
            selectedFormation{};


        if (
            !BuildStageSubset(
                fullFormation,
                requestedCount,
                selectedFormation
            )
        ) {
            return false;
        }


        result.profile =
            releasePlan.profile;


        result.target =
            releasePlan.mark.target;


        result.formation =
            selectedFormation;


        // Preserve the original target bounds center as the initial/fallback
        // aim snapshot.
        //
        // The artillery CIRCLE positions are world-fixed.
        //
        // The SHOT aim is different:
        // each due shot attempts to snapshot the current living target
        // position shortly before firing.
        //
        // If the target later dies or its handle becomes invalid, remaining
        // already-created artillery continues toward the last valid snapshot.
        result.fixedAimPoint =
            releasePlan.mark.initialBounds.
                center;


        result.formationRadius =
            formationRadius;


        result.valid =
            true;


        return true;
    }


    bool BuildDefaultWorldFixedArtilleryPlan(
        const ReleasePlan& releasePlan,
        const RE::NiPoint3& forwardAxis,
        WorldFixedArtilleryPlan& result) noexcept
    {
        return
            BuildWorldFixedArtilleryPlan(
                releasePlan,
                forwardAxis,
                kFormationRadiusScale,
                result
            );
    }


    bool BeginArtillerySchedule(
        const WorldFixedArtilleryPlan& plan,
        ArtillerySchedule& result) noexcept
    {
        result =
            {};


        if (
            !plan.valid ||
            !plan.profile ||
            !plan.formation.valid ||
            !Combat::IsSupportedFormationCount(
                plan.formation.count
            ) ||
            plan.formation.count !=
                static_cast<std::size_t>(
                    plan.profile->
                        circleCount
                ) ||
            !IsFinitePoint(
                plan.fixedAimPoint
            ) ||
            !std::isfinite(
                plan.formationRadius
            ) ||
            plan.formationRadius <=
                0.0F
        ) {
            return false;
        }


        result.plan =
            plan;


        result.lastAimPoint =
            plan.fixedAimPoint;


        result.elapsedSeconds =
            0.0F;


        result.nextShotTime =
            kFirstShotDelaySeconds;


        result.nextShotIndex =
            0;


        result.active =
            true;


        result.complete =
            false;


        return true;
    }


    bool AdvanceArtillerySchedule(
        const float deltaSeconds,
        ArtillerySchedule& schedule,
        ArtilleryScheduleAdvance& result) noexcept
    {
        result =
            {};


        if (
            !schedule.active ||
            schedule.complete ||
            !schedule.plan.valid ||
            !schedule.plan.profile ||
            !schedule.plan.formation.valid ||
            !Combat::IsSupportedFormationCount(
                schedule.plan.formation.count
            ) ||
            !std::isfinite(
                deltaSeconds
            ) ||
            deltaSeconds <
                0.0F
        ) {
            return false;
        }


        const float nextElapsed =
            schedule.elapsedSeconds +
            deltaSeconds;


        if (
            !std::isfinite(
                nextElapsed
            )
        ) {
            return false;
        }


        schedule.elapsedSeconds =
            nextElapsed;


        while (
            schedule.active &&
            schedule.nextShotIndex <
                schedule.plan.formation.count &&
            schedule.elapsedSeconds >=
                schedule.nextShotTime
        ) {

            if (
                result.dueCount >=
                result.dueShotIndices.size()
            ) {
                return false;
            }


            result.dueShotIndices[
                result.dueCount
            ] =
                schedule.nextShotIndex;


            ++result.dueCount;

            ++schedule.nextShotIndex;


            if (
                schedule.nextShotIndex >=
                schedule.plan.formation.count
            ) {

                schedule.active =
                    false;


                schedule.complete =
                    true;


                result.completedThisAdvance =
                    true;


                break;
            }


            // shot 0 = 0.45
            // shot 1 = 0.57
            // shot 2 = 0.69
            // ...
            //
            // Computing from the initial authority instead of repeatedly
            // adding 0.12 reduces cumulative timing drift.
            schedule.nextShotTime =
                kFirstShotDelaySeconds +
                kSubsequentShotIntervalSeconds *
                static_cast<float>(
                    schedule.nextShotIndex
                );
        }


        return true;
    }


    bool SnapshotScheduledShotAim(
        ArtillerySchedule& schedule,
        RE::NiPoint3& result) noexcept
    {
        if (
            !schedule.plan.valid ||
            !IsFinitePoint(
                schedule.lastAimPoint
            )
        ) {
            return false;
        }


        result =
            schedule.lastAimPoint;


        auto target =
            schedule.plan.target.get();


        if (
            !target ||
            target->IsDead()
        ) {
            // Target death/invalid handle does not cancel artillery.
            //
            // Continue using the last valid snapshot.
            return true;
        }


        Combat::ActorBounds
            currentBounds{};


        if (
            !Combat::ResolveActorBounds(
                target.get(),
                currentBounds
            ) ||
            !Combat::IsUsableActorBounds(
                currentBounds
            ) ||
            !IsFinitePoint(
                currentBounds.center
            )
        ) {
            // Runtime bounds temporarily unavailable:
            // preserve the last known valid snapshot.
            return true;
        }


        schedule.lastAimPoint =
            currentBounds.center;


        result =
            schedule.lastAimPoint;


        return true;
    }


    bool ResolveScheduledShotCollision(
        RE::Actor* collisionIgnoreActor,
        const WorldFixedArtilleryPlan& plan,
        const std::size_t shotIndex,
        const RE::NiPoint3& shotAimPoint,
        const float shotRadius,
        ScheduledShotCollision& result) noexcept
    {
        result =
            {};


        if (
            !collisionIgnoreActor ||
            !plan.valid ||
            !plan.profile ||
            !plan.formation.valid ||
            shotIndex >=
                plan.formation.count ||
            !IsFinitePoint(
                shotAimPoint
            ) ||
            !std::isfinite(
                shotRadius
            ) ||
            shotRadius <
                0.0F
        ) {
            return false;
        }


        Combat::BeamSegmentQuery
            query{};


        if (
            !BuildFixedShotQuery(
                plan.formation.points[
                    shotIndex
                ],
                shotAimPoint,
                shotRadius,
                query
            )
        ) {
            return false;
        }


        result.shotIndex =
            shotIndex;


        result.query =
            query;


        result.queryBuilt =
            true;


        if (
            !Combat::ResolveBeamCollision(
                query,
                collisionIgnoreActor,
                result.collision
            ) ||
            !result.collision.resolved
        ) {
            return false;
        }


        result.collisionResolved =
            true;


        return true;
    }


    bool ExecuteScheduledShotDamage(
        RE::Actor* sourceActor,
        const WorldFixedArtilleryPlan& plan,
        const std::size_t shotIndex,
        const RE::NiPoint3& shotAimPoint,
        const float shotRadius,
        const float damageMultiplier,
        ScheduledShotDamageResult& result) noexcept
    {
        result =
            {};


        if (
            !sourceActor ||
            !plan.valid ||
            !plan.profile ||
            !plan.formation.valid ||
            shotIndex >=
                plan.formation.count ||
            !std::isfinite(damageMultiplier) ||
            damageMultiplier < 0.5F ||
            damageMultiplier > 1.0F ||
            !std::isfinite(
                plan.profile->
                    beamDamage
            ) ||
            plan.profile->
                beamDamage <=
                    0.0F ||
            !std::isfinite(
                plan.profile->
                    explosionDamage
            ) ||
            plan.profile->
                explosionDamage <=
                    0.0F ||
            !std::isfinite(
                plan.profile->
                    explosionRadius
            ) ||
            plan.profile->
                explosionRadius <=
                    0.0F
        ) {
            return false;
        }


        auto* processLists =
            RE::ProcessLists::
                GetSingleton();


        if (!processLists) {
            return false;
        }


        if (
            !ResolveScheduledShotCollision(
                sourceActor,
                plan,
                shotIndex,
                shotAimPoint,
                shotRadius,
                result.shot
            ) ||
            !result.shot.collisionResolved ||
            !result.shot.collision.
                worldStop.resolved
        ) {
            return false;
        }


        result.explosionCenter =
            result.shot.collision.
                worldStop.point;


        if (
            !IsFinitePoint(
                result.explosionCenter
            )
        ) {
            return false;
        }


        // ----------------------------------------------------
        // Beam component
        //
        // Every hostile Actor intersecting the actual beam before
        // world/static stop receives full 150 fixed damage.
        //
        // Actor intersections do not stop the beam.
        // ----------------------------------------------------

        result.beamIntersections =
            result.shot.collision.
                actorHits.size();


        for (
            const auto& hit :
            result.shot.collision.
                actorHits
        ) {
            auto target =
                hit.actor.get();


            if (!target) {
                continue;
            }


            if (
                !IsHostileToSource(
                    sourceActor,
                    target.get()
                )
            ) {
                continue;
            }


            ++result.beamHostileIntersections;


            if (
                Combat::ApplyFixedHealthDamage(
                    sourceActor,
                    target.get(),
                    plan.profile->
                        beamDamage * damageMultiplier,
                    Combat::FirewallOrigin::
                        kJudgement
                )
            ) {
                ++result.beamActorsDamaged;
            }
        }


        // ----------------------------------------------------
        // Logical explosion component
        //
        // Center:
        // actual resolved beam endpoint.
        //
        // If world/static stopped the beam, this is that impact.
        // Otherwise BeamCollision resolves the endpoint at maxDistance,
        // which is the fixed artillery aim point.
        //
        // Explosion gameplay is a logical radius query.
        // It does not perform a second LOS/wall test.
        //
        // Beam + explosion are independent components, therefore
        // the same hostile Actor may receive both 150 + 50.
        // ----------------------------------------------------

        processLists->
            ForAllActors(
                [&](RE::Actor* actor)
                {
                    if (
                        !IsHostileToSource(
                            sourceActor,
                            actor
                        )
                    ) {
                        return
                            RE::BSContainer::
                                ForEachResult::
                                    kContinue;
                    }


                    Combat::ActorBounds
                        bounds{};


                    if (
                        !Combat::ResolveActorBounds(
                            actor,
                            bounds
                        )
                    ) {
                        return
                            RE::BSContainer::
                                ForEachResult::
                                    kContinue;
                    }


                    if (
                        !IntersectsExplosionSphere(
                            result.explosionCenter,
                            plan.profile->
                                explosionRadius,
                            bounds
                        )
                    ) {
                        return
                            RE::BSContainer::
                                ForEachResult::
                                    kContinue;
                    }


                    ++result.explosionActorsInRadius;


                    if (
                        Combat::ApplyFixedHealthDamage(
                            sourceActor,
                            actor,
                            plan.profile->
                                explosionDamage * damageMultiplier,
                            Combat::FirewallOrigin::
                                kJudgement
                        )
                    ) {
                        ++result.explosionActorsDamaged;
                    }


                    return
                        RE::BSContainer::
                            ForEachResult::
                                kContinue;
                }
            );


        result.explosionResolved =
            true;


        result.executed =
            true;


        return true;
    }


    FinalReleaseResult ExecuteFinalRelease(
        RE::PlayerCharacter* player,
        const float effectiveCharge,
        const float costMultiplier) noexcept
    {
        FinalReleaseResult
            result{};


        result.effectiveCharge =
            effectiveCharge;

        // Reuse the charging-session snapshot, never rescan equipment.
        const float damageMultiplier =
            std::isfinite(costMultiplier) &&
            costMultiplier >= 0.5F &&
            costMultiplier <= 1.0F ?
                costMultiplier : 1.0F;

        if (
            !player ||
            !std::isfinite(
                effectiveCharge
            ) ||
            effectiveCharge <=
                0.0F
        ) {
            return result;
        }


        result.attempted =
            true;


        const auto* profile =
            ResolveStage(
                effectiveCharge
            );


        if (!profile) {
            return result;
        }


        result.stage =
            profile->stage;


        Combat::BeamSegmentQuery
            markingQuery{};


        RE::NiPoint3
            forwardAxis{};


        if (
            !BuildMarkingQuery(
                player,
                markingQuery,
                forwardAxis
            )
        ) {
            return result;
        }


        result.aimResolved =
            true;


        Combat::BeamCollisionResult
            markingCollision{};


        if (
            !Combat::ResolveBeamCollision(
                markingQuery,
                player,
                markingCollision
            ) ||
            !markingCollision.resolved
        ) {
            return result;
        }


        result.markingCollisionResolved =
            true;


        const bool markingVisual =
            ShowBeamVisual(
                GetRuntimeState().
                    markingBeam,
                markingQuery.origin,
                markingCollision.
                    worldStop.point,
                kMarkingVisualBeamWidth,
                kMarkingVisualLifetimeSeconds
            );


        if (!markingVisual) {
            logger::warn(
                "[TOAM][S06VFX] MARKING_BEAM_VISUAL_FAILED"
            );
        }


        result.markingActorIntersections =
            markingCollision.
                actorHits.size();


        ReleasePlan
            releasePlan{};


        if (
            !BuildReleasePlan(
                effectiveCharge,
                markingCollision,
                releasePlan
            ) ||
            !releasePlan.valid ||
            !releasePlan.mark.valid
        ) {
            // No Actor acquired:
            // no artillery is created.
            return result;
        }


        result.targetAcquired =
            true;


        auto markedTarget =
            releasePlan.mark.target.get();


        if (markedTarget) {

            result.markingDamageApplied =
                Combat::ApplyFixedHealthDamage(
                    player,
                    markedTarget.get(),
                    kMarkingDamage * damageMultiplier,
                    Combat::FirewallOrigin::
                        kJudgement
                );
        }


        WorldFixedArtilleryPlan
            artilleryPlan{};


        if (
            !BuildDefaultWorldFixedArtilleryPlan(
                releasePlan,
                forwardAxis,
                artilleryPlan
            ) ||
            !artilleryPlan.valid
        ) {
            return result;
        }


        result.formationResolved =
            true;


        std::size_t
            scheduleIndex{};


        auto* scheduleSlot =
            AcquireScheduleSlot(
                scheduleIndex
            );


        if (!scheduleSlot) {

            logger::error(
                "[TOAM][S06] FINAL_RELEASE_REJECTED | Reason=SCHEDULE_CAPACITY | Stage={} | Charge={:.3f}",
                result.stage,
                effectiveCharge
            );


            return result;
        }


        if (
            !BeginArtillerySchedule(
                artilleryPlan,
                *scheduleSlot
            )
        ) {
            *scheduleSlot =
                {};


            return result;
        }


        // Preserve the original charge-start multiplier for delayed shots.
        scheduleSlot->damageMultiplier =
            damageMultiplier;

        result.scheduleStarted =
            true;


        const std::size_t
            visualCircleCount =
                SpawnArtilleryCircleVisuals(
                    scheduleIndex,
                    artilleryPlan
                );


        result.executed =
            true;


        logger::info(
            "[TOAM][S06] FINAL_RELEASE | Schedule={} | Stage={} | Charge={:.3f} | Mult={:.3f} | MarkHits={} | MarkDamage={} | Circles={} | VisualCircles={} | FirstDelay={:.3f} | Interval={:.3f}",
            scheduleIndex,
            result.stage,
            effectiveCharge,
            damageMultiplier,
            result.markingActorIntersections,
            result.markingDamageApplied,
            artilleryPlan.formation.count,
            visualCircleCount,
            kFirstShotDelaySeconds,
            kSubsequentShotIntervalSeconds
        );


        return result;
    }


    void UpdateArtillery(
        RE::PlayerCharacter* player,
        const float deltaSeconds) noexcept
    {
        if (
            !player ||
            !std::isfinite(
                deltaSeconds
            ) ||
            deltaSeconds <=
                0.0F
        ) {
            return;
        }


        auto& state =
            GetRuntimeState();


        for (
            std::size_t scheduleIndex = 0;
            scheduleIndex <
                state.schedules.size();
            ++scheduleIndex
        ) {
            auto& schedule =
                state.schedules[
                    scheduleIndex
                ];


            if (
                !schedule.active
            ) {
                continue;
            }


            ArtilleryScheduleAdvance
                advance{};


            if (
                !AdvanceArtillerySchedule(
                    deltaSeconds,
                    schedule,
                    advance
                )
            ) {
                logger::error(
                    "[TOAM][S06] SCHEDULE_ADVANCE_FAILED"
                );


                schedule =
                    {};


                continue;
            }


            for (
                std::size_t dueIndex = 0;
                dueIndex <
                    advance.dueCount;
                ++dueIndex
            ) {
                const std::size_t
                    shotIndex =
                        advance.
                            dueShotIndices[
                                dueIndex
                            ];


                RE::NiPoint3
                    shotAimPoint{};


                if (
                    !SnapshotScheduledShotAim(
                        schedule,
                        shotAimPoint
                    )
                ) {
                    logger::error(
                        "[TOAM][S06] SHOT_REJECTED | Reason=AIM_SNAPSHOT | Schedule={} | Index={}",
                        scheduleIndex,
                        shotIndex
                    );


                    continue;
                }


                ScheduledShotDamageResult
                    damageResult{};


                const bool executed =
                    ExecuteScheduledShotDamage(
                        player,
                        schedule.plan,
                        shotIndex,
                        shotAimPoint,
                        kArtilleryBeamRadiusWorldUnits,
                        schedule.damageMultiplier,
                        damageResult
                    );


                if(executed) { TOAM::Audio::JudgementShot(shotIndex,shotAimPoint); }

                bool beamVisual{};


                if (
                    executed &&
                    damageResult.shot.
                        collisionResolved
                ) {
                    beamVisual =
                        ShowBeamVisual(
                            state.visuals[
                                scheduleIndex
                            ].
                            beam,
                            schedule.plan.
                                formation.
                                points[
                                    shotIndex
                                ],
                            damageResult.shot.
                                collision.
                                worldStop.
                                point,
                            ArtilleryPresentationWidth(schedule.plan.formation.points[shotIndex],shotAimPoint),
                            kArtilleryVisualLifetimeSeconds,true
                        );
                }


                bool impactVisual{};


                if (
                    executed &&
                    damageResult.
                        explosionResolved
                ) {
                    impactVisual =
                        ShowExplosionImpactVisual(
                            damageResult.
                                explosionCenter
                        );
                }


                ReleaseArtilleryCircleVisual(
                    scheduleIndex,
                    shotIndex
                );


                logger::info(
                    "[TOAM][S06] SHOT | Schedule={} | Index={} | Executed={} | Mult={:.3f} | PlannedBeamPerHit={:.3f} | PlannedExplosionPerHit={:.3f} | BeamVisual={} | ImpactVisual={} | BeamHits={} | BeamHostile={} | BeamDamaged={} | ExplosionInRadius={} | ExplosionDamaged={} | Aim=({:.3f},{:.3f},{:.3f})",
                    scheduleIndex,
                    shotIndex,
                    executed,
                    schedule.damageMultiplier,
                    schedule.plan.profile ? schedule.plan.profile->beamDamage * schedule.damageMultiplier : 0.0F,
                    schedule.plan.profile ? schedule.plan.profile->explosionDamage * schedule.damageMultiplier : 0.0F,
                    beamVisual,
                    impactVisual,
                    damageResult.beamIntersections,
                    damageResult.beamHostileIntersections,
                    damageResult.beamActorsDamaged,
                    damageResult.explosionActorsInRadius,
                    damageResult.explosionActorsDamaged,
                    shotAimPoint.x,
                    shotAimPoint.y,
                    shotAimPoint.z
                );
            }


            if (
                schedule.complete
            ) {
                state.visuals[
                    scheduleIndex
                ].
                circles.
                ReleaseAll();


                schedule =
                    {};
            }
        }
    }


    void ReleaseChargeGuide() noexcept
    {
        auto& state = GetRuntimeState();
        if (state.chargeGuideVisible) {
            logger::info("[TOAM][S06GUIDE] RELEASE");
        }
        state.chargeGuide.Release();
        state.chargeGuideRoot = nullptr;
        state.chargeGuideCircle.ReleaseAll();
        state.chargeGuideCircleRoot = nullptr;
        state.chargeGuideVisible = false;
        state.chargeGuideWanted = false;
        state.chargeGuideTarget={};
        state.chargeGuideScanRemaining=0;
    }

    void SuspendChargeGuide() noexcept
    {
        auto& state = GetRuntimeState();
        if (!state.chargeGuide.IsActive() && !state.chargeGuideVisible) {
            return;
        }
        state.chargeGuide.Release();
        state.chargeGuideRoot = nullptr;
        state.chargeGuideCircle.ReleaseAll();
        state.chargeGuideCircleRoot = nullptr;
        state.chargeGuideVisible = false;
        // Preserve chargeGuideWanted only until the charge session ends.
    }

    void BeginChargeGuide() noexcept
    {
        ReleaseChargeGuide();
        GetRuntimeState().chargeGuideWanted = true;
        logger::info("[TOAM][S06GUIDE] BEGIN | Damage=false");
    }

    void UpdateChargeGuide(RE::PlayerCharacter* player) noexcept
    {
        auto& state = GetRuntimeState();
        if (!state.chargeGuideWanted) {
            return;
        }
        if (!TOAM::Charge::Runtime::IsJudgementChargeActive()) {
            ReleaseChargeGuide();
            return;
        }
        if (!player || player->IsDead() || !player->GetParentCell()) {
            SuspendChargeGuide();
            return;
        }

        Combat::BeamSegmentQuery query{};
        RE::NiPoint3 forwardAxis{};
        RE::NiNode* root{};
        RE::NiTransform inverse{};
        if (!BuildMarkingQuery(player, query, forwardAxis) ||
            !ResolveVisualRoot(root, inverse)) {
            SuspendChargeGuide();
            return;
        }

        const float dt=RE::Main::QFrameAnimTime();
        if(std::isfinite(dt)&&dt>0)state.chargeGuideScanRemaining-=dt;
        if(state.chargeGuideScanRemaining<=0) {
            state.chargeGuideScanRemaining=0.10F;
            Combat::BeamCollisionResult selection{};
            ReleasePlan selected{};
            auto previous=state.chargeGuideTarget;
            state.chargeGuideTarget={};state.chargeGuideLength=query.maxDistance;
            if(Combat::ResolveBeamCollision(query,player,selection)&&selection.resolved) {
                const auto offset=selection.worldStop.point-query.origin;
                state.chargeGuideLength=std::sqrt(LengthSquared(offset));
                // Selection only: BuildReleasePlan has no payment or damage side effects.
                if(BuildReleasePlan(300.0F,selection,selected)&&selected.mark.valid) {
                    state.chargeGuideTarget=selected.mark.target;
                    state.chargeGuideLength=std::sqrt(LengthSquared(selected.mark.initialHitPoint-query.origin));
                }
            }
            if(previous!=state.chargeGuideTarget) {
                auto selectedActor=state.chargeGuideTarget.get();
                logger::info("[TOAM][S06GUIDE] TARGET | Actor={:08X} | Damage=false",
                    selectedActor?selectedActor->GetFormID():0);
            }
        }
        // CP027 overrides the earlier hand-node presentation. The engine's aim owns
        // the logical ray and the small shoulder emitter's converging guide.
        const bool leftGuide = false; // One fixed right-shoulder emitter on both charge and release.
        const RE::NiPoint3 guideMuzzle = Arcana::HostJudgementGuideAnchor(player, leftGuide);
        if (!IsFinitePoint(guideMuzzle)) { SuspendChargeGuide(); return; }
        const RE::NiPoint3 localStart = inverse * guideMuzzle;
        const float guideLength=std::isfinite(state.chargeGuideLength)&&state.chargeGuideLength>0?
            state.chargeGuideLength:query.maxDistance;
        const RE::NiPoint3 localEnd = inverse *
            (query.origin + query.direction * guideLength);
        const RE::NiPoint3 localVector = localEnd - localStart;
        const float lengthSquared = LengthSquared(localVector);
        RE::NiPoint3 localDirection{};
        const float width = 0.06F * std::fabs(inverse.scale);
        if (!std::isfinite(lengthSquared) ||
            lengthSquared <= kDirectionEpsilon ||
            !NormalizeDirection(localVector, localDirection) ||
            !std::isfinite(width) || width <= 0.0F) {
            SuspendChargeGuide();
            return;
        }

        // One palm-sized charge guide circle, independent of the 3/6/9 artillery.
        // Measured AS_CirclePose child width is 400u; scale .03 gives a 12u outer diameter.
        VisualBasis guideBasis{};
        if (!BuildVisualBasis(query.origin + query.direction * guideLength - guideMuzzle, guideBasis)) {
            SuspendChargeGuide();
            return;
        }
        RE::NiTransform guideCircleWorld{};
        guideCircleWorld.translate = guideMuzzle;
        guideCircleWorld.scale = 0.03F;
        ApplyVisualBasis(guideCircleWorld, guideBasis);
        const auto guideCircleLocal = inverse * guideCircleWorld;
        if (state.chargeGuideCircleRoot != root) {
            state.chargeGuideCircle.ReleaseAll();
            state.chargeGuideCircleRoot = root;
        }
        const bool circleUpdated = state.chargeGuideCircle.IsActive(0) ?
            state.chargeGuideCircle.SetTransform(0, guideCircleLocal, true) :
            state.chargeGuideCircle.SpawnTransform(0, root, VFX::AssetID::kCircleStatic, guideCircleLocal, true);
        if (!circleUpdated) {
            SuspendChargeGuide();
            return;
        }
        VFX::BeamPose pose{};
        pose.origin = localStart;
        pose.direction = localDirection;
        pose.length = std::sqrt(lengthSquared);
        pose.widthScale = width;
        pose.lifetimeSeconds = -1.0F;
        pose.visible = true;
        pose.asset = VFX::AssetID::kBeamWholeLens;

        const bool updated = state.chargeGuide.IsActive() &&
            state.chargeGuideRoot == root ?
            state.chargeGuide.UpdateBeam(pose) :
            state.chargeGuide.SetBeam(root, pose);
        if (!updated) {
            SuspendChargeGuide();
            static std::uint32_t failures{};
            if (failures++ < 4) {
                logger::warn("[TOAM][S06GUIDE] VISUAL_RETRY_PENDING");
            }
            return;
        }
        state.chargeGuideRoot = root;
        if (!state.chargeGuideVisible) {
            logger::info("[TOAM][S06GUIDE] REQUESTED | Damage=false | Range={:.2f} | Pixels=UNVERIFIED",
                query.maxDistance);
            const auto feet=player->GetPosition();
            logger::info("[TOAM][CP021GUIDE] AIM_ORIGIN | World=({},{},{}) | Player=({},{},{}) | Damage=false",
                query.origin.x,query.origin.y,query.origin.z,feet.x,feet.y,feet.z);
            logger::info("[TOAM][CP023GUIDE] VISUAL_MUZZLE | World=({},{},{}) | Source=SHOULDER_FRONT_ENGINE_AIM | CircleDiameter=12 | Damage=false | Pixels=UNVERIFIED",
                guideMuzzle.x,guideMuzzle.y,guideMuzzle.z);
        }
        state.chargeGuideVisible = true;
    }

    void ResetTransientState() noexcept
    {
        auto& state =
            GetRuntimeState();


        ReleaseChargeGuide();
        state.markingBeam.
            Release();


        for (
            std::size_t index = 0;
            index <
                state.schedules.size();
            ++index
        ) {
            state.visuals[index].
                circles.
                ReleaseAll();


            state.visuals[index].
                beam.
                Release();


            state.schedules[index] =
                {};
        }
    }


    bool HasActiveArtillery() noexcept
    {
        const auto& state =
            GetRuntimeState();


        for (
            const auto& schedule :
            state.schedules
        ) {
            if (
                schedule.active
            ) {
                return true;
            }
        }


        return false;
    }


    bool BuildFixedShotQuery(
        const RE::NiPoint3& circleOrigin,
        const RE::NiPoint3& fixedAimPoint,
        const float shotRadius,
        Combat::BeamSegmentQuery& result) noexcept
    {
        result =
            {};


        if (
            !IsFinitePoint(
                circleOrigin
            ) ||
            !IsFinitePoint(
                fixedAimPoint
            ) ||
            !std::isfinite(
                shotRadius
            ) ||
            shotRadius <
                0.0F
        ) {
            return false;
        }


        const RE::NiPoint3 vector =
            fixedAimPoint -
            circleOrigin;


        const float lengthSquared =
            LengthSquared(
                vector
            );


        if (
            !std::isfinite(
                lengthSquared
            ) ||
            lengthSquared <=
                kDirectionEpsilon
        ) {
            return false;
        }


        const float distance =
            std::sqrt(
                lengthSquared
            );


        if (
            !std::isfinite(
                distance
            ) ||
            distance <=
                0.0F
        ) {
            return false;
        }


        const float inverseDistance =
            1.0F /
            distance;


        const RE::NiPoint3 direction =
            vector *
            inverseDistance;


        if (
            !IsFinitePoint(
                direction
            )
        ) {
            return false;
        }


        result.origin =
            circleOrigin;


        result.direction =
            direction;


        result.maxDistance =
            distance;


        result.radius =
            shotRadius;


        return true;
    }
}