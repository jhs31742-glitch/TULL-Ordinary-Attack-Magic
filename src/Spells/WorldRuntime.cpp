#include "Arcana/Integration.h"
#include "Core/ExclusiveCast.h"
#include "Audio/SpellAudio.h"

#include "Spells/WorldRuntime.h"
#include "Core/DamageSettings.h"

#include "Charge/ChargeSession.h"
#include "Charge/MagickaChargeArbiter.h"
#include "Combat/ActorBounds.h"
#include "Combat/FixedDamage.h"
#include "Core/DataCache.h"
#include "Spells/ArcanaLimiterRuntime.h"
#include "Spells/WorldFormation.h"
#include "Spells/WorldProfile.h"
#include "VFX/AssetCatalog.h"
#include "VFX/BeamRenderer.h"
#include "VFX/CircleRenderer.h"
#include "VFX/TransientVFXManager.h"

#include "REL/Relocation.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>


namespace TOAM::Spells::World::Runtime
{
    namespace
    {
        constexpr float
            kDirectionEpsilon{
                0.000001F
            };


        // Presentation-only starting values.
        //
        // CP021 spatial authority: Arcana HostDomePose. Ground remains the
        // CP019 Hermit layout; flight uses the original Arcana air sphere
        // (feet+Z78, radius112). Outward beam64, explosion radius85.333333.
        // The explosion center is the same frozen visual beam endpoint.
        //
        // These VFX scale/width/lifetime values remain runtime-tunable.


        // R11 presentation width; gameplay radii remain independent.
        constexpr float
            kWorldBeamVisualWidth{
                0.06F
            };


        constexpr float
            kWorldBeamVisualLifetimeSeconds{
                0.48F
            };


        constexpr float
            kWorldImpactLifetimeSeconds{
                2.20F
            };


        constexpr float
            kWorldImpactScale{
                2.40F
            };


        // ChargeSession's shared input unit is named Hand.
        //
        // The World owns one Shout/Power input channel; kRight is only
        // an internal reusable ChargeSession channel identifier.
        // It is not hand-cast gameplay.
        inline constexpr
            TOAM::Charge::Hand
            kWorldChargeChannel{
                TOAM::Charge::Hand::kRight
            };


        struct State final
        {
            TOAM::Charge::ChargeSession
                session{
                    TOAM::Spells::World::
                        GetChargeProfile()
                };

            bool buttonHeld{};
            bool releasePending{};
            TOAM::Presentation::Color circleColor{TOAM::Presentation::Color::Default};
            bool airborneVisual{}; // Snapshot at Begin, as Arcana Hermit does.

            double requested{};
            double allocated{};
        };


        struct VisualBasis final
        {
            RE::NiPoint3 x{};
            RE::NiPoint3 y{};
            RE::NiPoint3 z{};
        };


        struct SlotGeometry final
        {
            RE::NiPoint3
                worldDirection{};

            RE::NiPoint3
                circleWorld{};

            RE::NiPoint3
                endpointWorld{};

            std::size_t bankSlot{};
            float circleScale{0.40F};
            bool valid{};
        };


        struct ReleaseGeometry final
        {
            std::array<
                SlotGeometry,
                TOAM::Spells::World::
                    kWorldSlotCount
            > slots{};

            std::size_t count{};
            bool valid{};
        };


        struct ReleaseExecutionResult final
        {
            std::size_t
                beamsSpawned{};

            std::size_t
                impactsSpawned{};

            std::size_t
                hostileActorsInAnyRadius{};

            std::size_t
                qualifyingExplosionHits{};

            std::size_t
                damageHitsApplied{};

            bool geometryResolved{};
            bool damageScanResolved{};
            bool executed{};
        };


        struct VisualState final
        {
            TOAM::VFX::CircleRenderer
                circles{};

            std::array<
                TOAM::VFX::BeamRenderer,
                TOAM::Spells::World::
                    kWorldSlotCount
            > beams{};

            std::array<
                TOAM::VFX::VFXHandle,
                TOAM::Spells::World::
                    kWorldSlotCount
            > impacts{};
        };


        [[nodiscard]]
        State& GetState() noexcept
        {
            static State state{};
            return state;
        }


        [[nodiscard]]
        VisualState& GetVisualState() noexcept
        {
            static VisualState state{};
            return state;
        }


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
        bool Normalize(
            const RE::NiPoint3& value,
            RE::NiPoint3& result) noexcept
        {
            result =
                {};


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
                    kDirectionEpsilon
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
            const RE::NiPoint3& direction,
            VisualBasis& result) noexcept
        {
            result =
                {};


            // Shared authored beam/circle facing precedent:
            // local +Y points outward.
            if (
                !Normalize(
                    direction,
                    result.y
                )
            ) {
                return false;
            }


            const RE::NiPoint3 reference =
                std::fabs(
                    result.y.z
                ) <
                    0.999F ?
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
                !Normalize(
                    result.y.Cross(
                        reference
                    ),
                    result.x
                )
            ) {
                return false;
            }


            if (
                !Normalize(
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
            parent =
                nullptr;

            rootInverse =
                {};


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
                worldRoot->
                    world.
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


        [[nodiscard]]
        float AllocatedAmount(
            const TOAM::Charge::CommitResult& result,
            const TOAM::Charge::ChargeSession*
                session) noexcept
        {
            for (
                std::size_t index = 0;
                index < result.plan.count;
                ++index
            ) {
                const auto& entry =
                    result.plan.entries[
                        index
                    ];


                if (
                    entry.session ==
                    session
                ) {
                    return
                        entry.allocated;
                }
            }


            return 0.0F;
        }


        [[nodiscard]]
        bool IsHostileToSource(
            RE::Actor* sourceActor,
            RE::Actor* targetActor) noexcept
        {
            // Reuse current Judgement hostility authority exactly.
            return
                sourceActor &&
                targetActor &&
                sourceActor !=
                    targetActor &&
                targetActor->
                    IsHostileToActor(
                        sourceActor
                    );
        }


        [[nodiscard]]
        bool IntersectsExplosionSphere(
            const RE::NiPoint3& center,
            const float explosionRadius,
            const TOAM::Combat::
                ActorBounds& bounds) noexcept
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
                !TOAM::Combat::
                    IsUsableActorBounds(
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


            const RE::NiPoint3 offset{
                bounds.center.x -
                    center.x,
                bounds.center.y -
                    center.y,
                bounds.center.z -
                    center.z
            };


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
        bool ResolvePlayerFrame(
            RE::PlayerCharacter* player,
            RE::NiPoint3& center,
            RE::NiTransform& actorWorld) noexcept
        {
            center =
                {};

            actorWorld =
                {};


            if (!player) {
                return false;
            }


            TOAM::Combat::ActorBounds
                bounds{};


            if (
                !TOAM::Combat::
                    ResolveActorBounds(
                        player,
                        bounds
                    )
            ) {
                return false;
            }


            auto* player3D =
                player->Get3D();


            if (!player3D) {
                return false;
            }


            center =
                bounds.center;

            actorWorld =
                player3D->world;


            return
                IsFinitePoint(
                    center
                ) &&
                IsFiniteTransform(
                    actorWorld
                );
        }


        [[nodiscard]]
        bool TransformLocalDirection(
            const RE::NiTransform& actorWorld,
            const TOAM::Spells::World::
                LocalDirection& local,
            RE::NiPoint3& worldDirection) noexcept
        {
            const RE::NiPoint3
                localOrigin{
                    0.0F,
                    0.0F,
                    0.0F
                };


            const RE::NiPoint3
                localPoint{
                    local.right,
                    local.forward,
                    local.up
                };


            const RE::NiPoint3
                worldOrigin =
                    actorWorld *
                    localOrigin;


            const RE::NiPoint3
                worldPoint =
                    actorWorld *
                    localPoint;


            const RE::NiPoint3 vector{
                worldPoint.x -
                    worldOrigin.x,
                worldPoint.y -
                    worldOrigin.y,
                worldPoint.z -
                    worldOrigin.z
            };


            return
                Normalize(
                    vector,
                    worldDirection
                );
        }


        [[nodiscard]]
        bool BuildReleaseGeometry(
            RE::PlayerCharacter* player,
            const std::size_t stage,
            ReleaseGeometry& result) noexcept
        {
            result =
                {};


            TOAM::Spells::World::
                LocalFormation
                localFormation{};


            if (
                !TOAM::Spells::World::
                    BuildLocalFormation(
                        stage,
                        localFormation
                    ) ||
                !localFormation.valid ||
                localFormation.count == 0 ||
                localFormation.count >
                    TOAM::Spells::World::
                        kWorldSlotCount
            ) {
                return false;
            }


            RE::NiPoint3 center{};
            RE::NiTransform actorWorld{};


            if (
                !ResolvePlayerFrame(
                    player,
                    center,
                    actorWorld
                )
            ) {
                return false;
            }


            for (
                std::size_t index = 0;
                index <
                    localFormation.count;
                ++index
            ) {
                RE::NiPoint3
                    worldDirection{};


                if (
                    !TransformLocalDirection(
                        actorWorld,
                        localFormation.
                            directions[index],
                        worldDirection
                    )
                ) {
                    return false;
                }


                auto& slot =
                    result.slots[
                        index
                    ];


                const auto bankSlot=localFormation.bankSlots[index];
                slot.bankSlot=bankSlot;
                // CP019: Arcana Hermit owns position, normal and visual scale.

                slot.worldDirection =
                    worldDirection;


                if(!Arcana::HostDomePose(bankSlot,player,slot.circleWorld,
                    slot.worldDirection,slot.circleScale,GetState().airborneVisual)) {return false;}
                worldDirection=slot.worldDirection;


                slot.endpointWorld =
                    slot.circleWorld +
                    worldDirection *
                    TOAM::Spells::World::
                        kVisualBeamLengthWorldUnits;


                if (
                    !IsFinitePoint(
                        slot.circleWorld
                    ) ||
                    !IsFinitePoint(
                        slot.endpointWorld
                    )
                ) {
                    return false;
                }


                slot.valid =
                    true;
            }


            result.count =
                localFormation.count;

            result.valid =
                true;


            return true;
        }


        [[nodiscard]]
        bool BuildCircleLocalTransform(
            const SlotGeometry& slot,
            const RE::NiTransform& rootInverse,
            RE::NiTransform& result) noexcept
        {
            result =
                {};


            if (
                !slot.valid ||
                !IsFinitePoint(
                    slot.circleWorld
                )
            ) {
                return false;
            }


            VisualBasis basis{};


            if (
                !BuildVisualBasis(
                    slot.worldDirection,
                    basis
                )
            ) {
                return false;
            }


            RE::NiTransform
                worldTransform{};


            worldTransform.translate =
                slot.circleWorld;

            worldTransform.scale =
                slot.circleScale;


            ApplyVisualBasis(
                worldTransform,
                basis
            );


            result =
                rootInverse *
                worldTransform;


            return
                IsFiniteTransform(
                    result
                );
        }


        void ReleaseCircleVisuals() noexcept
        {
            GetVisualState().
                circles.
                ReleaseAll();
        }


        void ReleaseBeamVisuals() noexcept
        {
            auto& visual =
                GetVisualState();


            for (
                auto& beam :
                visual.beams
            ) {
                beam.Release();
            }
        }


        void ReleaseImpactVisuals() noexcept
        {
            auto& visual =
                GetVisualState();

            auto& manager =
                TOAM::VFX::
                    TransientVFXManager::
                    Get();


            for (
                auto& handle :
                visual.impacts
            ) {
                if (
                    handle.IsValid()
                ) {
                    static_cast<void>(
                        manager.Release(
                            handle
                        )
                    );
                }


                handle =
                    {};
            }
        }


        void ReleaseAllWorldVisuals() noexcept
        {
            ReleaseCircleVisuals();
            ReleaseBeamVisuals();
            ReleaseImpactVisuals();
        }


        [[nodiscard]]
        bool UpdateFormationVisual(
            RE::PlayerCharacter* player,
            const std::size_t stage) noexcept
        {
            auto& visual =
                GetVisualState();


            if (stage == 0) {

                visual.circles.
                    ReleaseAll();

                return true;
            }


            ReleaseGeometry
                geometry{};


            if (
                !BuildReleaseGeometry(
                    player,
                    stage,
                    geometry
                ) ||
                !geometry.valid
            ) {
                return false;
            }


            RE::NiNode* parent{};
            RE::NiTransform
                rootInverse{};


            if (
                !ResolveVisualRoot(
                    parent,
                    rootInverse
                )
            ) {
                return false;
            }


            visual.circles.SetColor(GetState().circleColor);
            visual.circles.SetStyle(TOAM::VFX::EngineCircle::Style::Dome);
            bool allReady =
                true;


            for (
                std::size_t index = 0;
                index <
                    TOAM::Spells::World::
                        kWorldSlotCount;
                ++index
            ) {
                bool selected=false;
                for(std::size_t j=0;j<geometry.count;++j) {selected=selected || geometry.slots[j].bankSlot==index;}
                if(selected) {continue;}
                if (
                    visual.circles.
                        IsActive(
                            index
                        )
                ) {
                    static_cast<void>(
                        visual.circles.
                            Release(
                                index
                            )
                    );
                }
            }


            for (
                std::size_t index = 0;
                index <
                    geometry.count;
                ++index
            ) {
                const auto rendererSlot=geometry.slots[index].bankSlot;
                RE::NiTransform
                    localTransform{};


                if (
                    !BuildCircleLocalTransform(
                        geometry.slots[
                            index
                        ],
                        rootInverse,
                        localTransform
                    )
                ) {
                    allReady =
                        false;

                    continue;
                }


                // CP021: TOAM has three discrete groups, not Hermit's continuous
                // twelve-node deployment. Per-instance growth starts at first pose.
                bool ready =
                    false;


                if (
                    visual.circles.
                        IsActive(
                            rendererSlot
                        )
                ) {
                    ready =
                        visual.circles.
                            SetTransform(
                                rendererSlot,
                                localTransform,
                                true
                            );
                }


                if (!ready) {

                    ready =
                        visual.circles.
                            SpawnTransform(
                                rendererSlot,
                                parent,
                                TOAM::VFX::
                                    AssetID::
                                        kCircleStatic,
                                localTransform,
                                true
                            );
                    if(ready) {static_cast<void>(visual.circles.BeginGrowth(rendererSlot,0.12F));}
                }

                if (!ready) {
                    allReady =
                        false;
                }
            }


            return allReady;
        }


        [[nodiscard]]
        bool BuildLocalBeamPose(
            const SlotGeometry& slot,
            const RE::NiTransform& rootInverse,
            TOAM::VFX::BeamPose& pose) noexcept
        {
            pose =
                {};


            if (
                !slot.valid ||
                !IsFinitePoint(
                    slot.circleWorld
                ) ||
                !IsFinitePoint(
                    slot.endpointWorld
                )
            ) {
                return false;
            }


            const RE::NiPoint3
                localOrigin =
                    rootInverse *
                    slot.circleWorld;


            const RE::NiPoint3
                localEnd =
                    rootInverse *
                    slot.endpointWorld;


            const RE::NiPoint3
                localVector{
                    localEnd.x -
                        localOrigin.x,
                    localEnd.y -
                        localOrigin.y,
                    localEnd.z -
                        localOrigin.z
                };


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


            const float localLength =
                std::sqrt(
                    localLengthSquared
                );


            RE::NiPoint3
                localDirection{};


            if (
                !Normalize(
                    localVector,
                    localDirection
                )
            ) {
                return false;
            }


            const float localWidth =
                kWorldBeamVisualWidth *
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


            pose.origin =
                localOrigin;

            pose.direction =
                localDirection;

            pose.length =
                localLength;

            pose.widthScale =
                localWidth;

            pose.lifetimeSeconds =
                kWorldBeamVisualLifetimeSeconds;

            pose.visible =
                true;

            pose.asset =
                TOAM::VFX::
                    AssetID::
                        kBeamWholeLens;


            return true;
        }


        [[nodiscard]]
        bool SpawnEndpointImpact(
            const std::size_t slotIndex,
            RE::NiNode* parent,
            const RE::NiTransform& rootInverse,
            const RE::NiPoint3& endpointWorld) noexcept
        {
            if (
                slotIndex >=
                    TOAM::Spells::World::
                        kWorldSlotCount ||
                !parent ||
                !IsFinitePoint(
                    endpointWorld
                )
            ) {
                return false;
            }


            auto& visual =
                GetVisualState();

            auto& manager =
                TOAM::VFX::
                    TransientVFXManager::
                    Get();


            auto& previous =
                visual.impacts[
                    slotIndex
                ];


            if (
                previous.IsValid()
            ) {
                static_cast<void>(
                    manager.Release(
                        previous
                    )
                );

                previous =
                    {};
            }


            RE::NiTransform
                impactWorld{};


            impactWorld.translate =
                endpointWorld;

            impactWorld.rotate.
                SetEulerAnglesXYZ(
                    RE::NiPoint3{
                        0.0F,
                        0.0F,
                        0.0F
                    }
                );

            impactWorld.scale =
                kWorldImpactScale;


            const RE::NiTransform
                impactLocal =
                    rootInverse *
                    impactWorld;


            if (
                !IsFiniteTransform(
                    impactLocal
                )
            ) {
                return false;
            }


            TOAM::VFX::SpawnRequest
                request{};


            request.asset =
                TOAM::VFX::
                    AssetID::
                        kFlashCloud;

            request.parent =
                parent;

            request.local =
                impactLocal;

            request.lifetimeSeconds =
                kWorldImpactLifetimeSeconds;

            request.visible =
                true;


            const auto handle =
                manager.Spawn(
                    request
                );


            if (
                !handle.IsValid()
            ) {
                return false;
            }


            // Positive-lifetime cloud is owned only by the transient manager.
            previous = {};


            return true;
        }


        void ShowReleaseVFX(
            const ReleaseGeometry& geometry,
            ReleaseExecutionResult& result) noexcept
        {
            if (
                !geometry.valid ||
                geometry.count == 0
            ) {
                return;
            }


            RE::NiNode* parent{};
            RE::NiTransform
                rootInverse{};


            if (
                !ResolveVisualRoot(
                    parent,
                    rootInverse
                )
            ) {
                return;
            }


            auto& visual =
                GetVisualState();


            for (
                std::size_t index = 0;
                index <
                    geometry.count;
                ++index
            ) {
                const auto& slot =
                    geometry.slots[
                        index
                    ];


                TOAM::VFX::BeamPose
                    pose{};


                if (
                    BuildLocalBeamPose(
                        slot,
                        rootInverse,
                        pose
                    ) &&
                    visual.beams[
                        index
                    ].SetBeam(
                        parent,
                        pose
                    )
                ) {
                    visual.beams[index].Detach();
                    ++result.beamsSpawned;
                }


                if (
                    SpawnEndpointImpact(
                        index,
                        parent,
                        rootInverse,
                        slot.endpointWorld
                    )
                ) {
                    ++result.impactsSpawned;
                }
            }
        }


        [[nodiscard]]
        bool ApplyReleaseDamage(
            RE::PlayerCharacter* player,
            const ReleaseGeometry& geometry,
            const float explosionDamage,
            ReleaseExecutionResult& result) noexcept
        {
            if (
                !player ||
                !geometry.valid ||
                geometry.count == 0
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
                            !IsHostileToSource(
                                player,
                                actor
                            )
                        ) {
                            return
                                RE::BSContainer::
                                    ForEachResult::
                                        kContinue;
                        }


                        TOAM::Combat::ActorBounds
                            bounds{};


                        if (
                            !TOAM::Combat::
                                ResolveActorBounds(
                                    actor,
                                    bounds
                                )
                        ) {
                            return
                                RE::BSContainer::
                                    ForEachResult::
                                        kContinue;
                        }


                        bool actorQualified =
                            false;


                        for (
                            std::size_t index = 0;
                            index <
                                geometry.count;
                            ++index
                        ) {
                            const auto& slot =
                                geometry.slots[
                                    index
                                ];


                            if (
                                !slot.valid ||
                                !IntersectsExplosionSphere(
                                    slot.endpointWorld,
                                    TOAM::Spells::World::
                                        kExplosionRadiusWorldUnits,
                                    bounds
                                )
                            ) {
                                continue;
                            }


                            actorQualified =
                                true;


                            ++result.
                                qualifyingExplosionHits;


                            if (
                                TOAM::Combat::
                                    ApplyFixedHealthDamage(
                                        player,
                                        actor,
                                        explosionDamage,
                                        TOAM::Combat::
                                            FirewallOrigin::
                                                kWorld
                                    )
                            ) {
                                ++result.
                                    damageHitsApplied;
                            }
                        }


                        if (actorQualified) {

                            ++result.
                                hostileActorsInAnyRadius;
                        }


                        return
                            RE::BSContainer::
                                ForEachResult::
                                    kContinue;
                    }
                );


            return true;
        }


        [[nodiscard]]
        bool ExecuteWorldRelease(
            RE::PlayerCharacter* player,
            const std::size_t stage,
            const float explosionDamage,
            ReleaseExecutionResult& result) noexcept
        {
            result =
                {};


            ReleaseGeometry
                geometry{};


            if (
                !BuildReleaseGeometry(
                    player,
                    stage,
                    geometry
                ) ||
                !geometry.valid
            ) {
                return false;
            }


            result.geometryResolved =
                true;


            // VFX and gameplay damage deliberately share endpoint geometry,
            // but the visual beam has no collision/gameplay authority.
            ShowReleaseVFX(
                geometry,
                result
            );


            result.damageScanResolved =
                ApplyReleaseDamage(
                    player,
                    geometry,
                    explosionDamage,
                    result
                );


            result.executed =
                true;
            TOAM::Audio::WorldFinal(stage,geometry.slots[0].circleWorld-geometry.slots[0].worldDirection*TOAM::Spells::World::kShellRadiusWorldUnits);


            return true;
        }


        using ShoutProcessButton =
            void (*)(
                RE::ShoutHandler*,
                RE::ButtonEvent*,
                RE::PlayerControlsData*);


        std::uintptr_t
            g_originalShoutProcessButton{};


        bool
            g_inputHookInstalled{};


        void HookedShoutProcessButton(
            RE::ShoutHandler* handler,
            RE::ButtonEvent* event,
            RE::PlayerControlsData* data)
        {
            auto* player =
                RE::PlayerCharacter::
                    GetSingleton();


            const auto disposition =
                HandleShoutButton(
                    player,
                    event
                );


            // Bounded proof of actual ShoutHandler dispatch. No input mutation.
            if (event && (event->IsDown() || event->IsUp())) {
                static std::uint32_t hookTraceCount{};
                if (hookTraceCount < 32) {
                    ++hookTraceCount;
                    logger::info(
                        "[TOAM][S07TRACE] HANDLER | Down={} | Up={} | Selected={} | Consumed={}",
                        event->IsDown(),
                        event->IsUp(),
                        IsWorldSelected(player),
                        disposition == InputDisposition::kConsume);
                }
            }


            if (
                disposition ==
                InputDisposition::
                    kConsume
            ) {
                return;
            }


            const auto original =
                reinterpret_cast<
                    ShoutProcessButton
                >(
                    g_originalShoutProcessButton
                );


            if (original) {

                original(
                    handler,
                    event,
                    data
                );
            }
        }
    }


    bool IsWorldSelected(
        RE::PlayerCharacter* player) noexcept
    {
        if (
            !player ||
            !TOAM::Core::DataCache::
                IsReady()
        ) {
            return false;
        }


        auto* world =
            TOAM::Core::DataCache::
                GetTheWorld();


        if (!world) {
            return false;
        }


        // E3-C1: consult the equipped Power/Shout slot first.
        // Preserve the existing engine predicate as fallback.
        if (player->GetActorRuntimeData().selectedPower == world) {
            return true;
        }

        return player->IsCurrentShout(world);
    }


    bool IsCharging() noexcept
    {
        const auto& state =
            GetState();


        return
            state.buttonHeld &&
            state.session.IsActive();
    }


    bool BeginCharge(
        RE::PlayerCharacter* player) noexcept
    {
        if (!player || player->IsDead() || !player->Is3DLoaded()) return false;
        if (
            !IsWorldSelected(
                player
            )
        ) {
            return false;
        }


        auto& state =
            GetState();


        if (
            state.buttonHeld ||
            state.releasePending ||
            state.session.IsActive()
        ) {
            return false;
        }


        // R1-C2C: reject a new Power charge before touching visuals/state.
        // This is an admission check, not a Magicka reservation.
        const float admissionMultiplier =
            TOAM::Spells::ArcanaLimiter::Runtime::
                SnapshotCostMultiplier(player);
        auto* av = player ? player->AsActorValueOwner() : nullptr;
        const float firstTierCost =
            TOAM::Spells::World::kStage1Charge * admissionMultiplier;
        const float currentMagicka = av ?
            av->GetActorValue(RE::ActorValue::kMagicka) : -1.0F;
        if (!std::isfinite(currentMagicka) ||
            !std::isfinite(admissionMultiplier) ||
            admissionMultiplier < 0.5F || admissionMultiplier > 1.0F ||
            currentMagicka < firstTierCost) {
            logger::info("[TOAM][R1C2C] World Begin blocked: below T1 MP");
            return false;
        }

        if (!TOAM::Core::ExclusiveCast::Acquire(TOAM::Core::ExclusiveCast::Owner::World)) {
            logger::info("[TOAM][CP034] EXCLUSIVE_DENY owner={} requested=WORLD phase=WORLD_BEGIN",int(TOAM::Core::ExclusiveCast::Current()));
            return false;
        }

        // Clear any expired/stale post-release handles before a new charge.
        ReleaseAllWorldVisuals();


        state.releasePending =
            false;

        state.requested =
            0.0;

        state.allocated =
            0.0;


        if (
            !state.session.BeginHand(
                kWorldChargeChannel,
                admissionMultiplier
            )
        ) {
            TOAM::Core::ExclusiveCast::Release(TOAM::Core::ExclusiveCast::Owner::World);
            return false;
        }


        state.circleColor=TOAM::Presentation::Snapshot(TOAM::Presentation::Spell::World);
        state.airborneVisual=Arcana::FlightActive();
        logger::info("[TOAM][CP021WORLD] FORMATION_MODE | Air={} | GroupGrowth=0.12 | Beam=64 | AirEndpointRadius=176 | DamagePolicy=UNCHANGED",state.airborneVisual);
        state.buttonHeld =
            true;


        logger::info(
            "[TOAM][S07C3] WORLD_CHARGE_BEGIN | Charge={:.3f} | Stage={} | CostMult={:.3f}",
            state.session.
                AccumulatedCharge(),
            state.session.
                Tier(),
            state.session.CostMultiplier());


        return true;
    }


    void UpdateCharge(
        RE::PlayerCharacter* player,
        const float deltaSeconds) noexcept
    {
        auto& state =
            GetState();


        if (
            !player ||
            !state.buttonHeld ||
            !state.session.IsActive()
        ) {
            return;
        }


        if (player->IsDead() || !player->Is3DLoaded() ||
            !IsWorldSelected(player)) {
            CancelCharge();
            return;
        }


        if (
            !std::isfinite(
                deltaSeconds
            ) ||
            deltaSeconds <=
                0.0F
        ) {
            return;
        }


        const auto stageBefore =
            state.session.Tier();


        const float requested =
            state.session.
                RequestedDrain(
                    deltaSeconds
                );


        if (requested > 0.0F) {

            std::array<
                TOAM::Charge::DrainRequest,
                1
            > requests{{
                TOAM::Charge::
                    DrainRequest{
                        &state.session,
                        requested
                    }
            }};


            const auto result =
                TOAM::Charge::
                    CommitChargeDrain(
                        player,
                        std::span<
                            const TOAM::Charge::
                                DrainRequest>{
                            requests.data(),
                            requests.size()
                        }
                    );


            state.requested +=
                static_cast<double>(
                    requested
                );


            const float allocated =
                AllocatedAmount(
                    result,
                    &state.session
                );


            if (result.committed) {

                state.allocated +=
                    static_cast<double>(
                        allocated
                    );
            }
        }


        const auto stageAfter =
            state.session.Tier();


        // This remains active even while held at Stage III cap where
        // RequestedDrain() becomes zero. The formation therefore
        // continues to follow the player at full charge.
        if (
            !UpdateFormationVisual(
                player,
                stageAfter
            )
        ) {
            logger::warn(
                "[TOAM][S07C3] WORLD_FORMATION_VISUAL_UPDATE_FAILED | Stage={} | Charge={:.3f}",
                stageAfter,
                state.session.
                    AccumulatedCharge());
        }


        if (
            stageAfter !=
            stageBefore
        ) {
            logger::info(
                "[TOAM][S07C3] WORLD_STAGE | {}->{} | Charge={:.3f} | Circles={}",
                stageBefore,
                stageAfter,
                state.session.
                    AccumulatedCharge(),
                TOAM::Spells::World::
                    CircleCountForStage(
                        stageAfter
                    ));
        }
    }


    ReleaseState ProcessPendingRelease(
        RE::PlayerCharacter* player) noexcept
    {
        auto& state =
            GetState();


        if (
            !state.releasePending
        ) {
            return {};
        }


        state.releasePending =
            false;


        return
            ReleaseCharge(
                player
            );
    }


    ReleaseState ReleaseCharge(
        RE::PlayerCharacter* player) noexcept
    {
        ReleaseState output{};


        auto& state =
            GetState();


        state.releasePending =
            false;


        if (
            !state.buttonHeld ||
            !state.session.IsActive()
        ) {
            ReleaseCircleVisuals();
            TOAM::Core::ExclusiveCast::Release(TOAM::Core::ExclusiveCast::Owner::World);
            return output;
        }


        const auto release =
            state.session.ReleaseHand(
                kWorldChargeChannel
            );


        state.buttonHeld =
            false;


        if (!release.wasHeld) {

            state.session.Reset();

            state.requested =
                0.0;

            state.allocated =
                0.0;

            ReleaseCircleVisuals();
            TOAM::Core::ExclusiveCast::Release(TOAM::Core::ExclusiveCast::Owner::World);
            return output;
        }


        output.wasCharging =
            true;


        output.shouldFire =
            release.finalRelease &&
            release.shouldFire;


        output.stage =
            release.tier;


        output.circleCount =
            TOAM::Spells::World::
                CircleCountForStage(
                    release.tier
                );


        output.accumulatedCharge =
            release.accumulatedCharge;


        // Charge-session snapshot is authoritative even if equipment
        // or Temperance changes after charging starts.
        const float damageMultiplier =
            std::isfinite(release.costMultiplier) &&
            release.costMultiplier >= 0.5F &&
            release.costMultiplier <= 1.0F ?
                release.costMultiplier : 1.0F;

        const float explosionDamage =
            TOAM::Spells::World::kExplosionDamage *
                damageMultiplier * Core::DamageSettings::GetMultiplier();


        ReleaseExecutionResult
            execution{};


        bool worldExecuted =
            false;


        if (
            output.shouldFire &&
            output.stage > 0 &&
            output.circleCount > 0
        ) {
            worldExecuted =
                ExecuteWorldRelease(
                    player,
                    output.stage,
                    explosionDamage,
                    execution
                );
        }


        // Transfer fired circles out of charge ownership. Alpha fade remains blocked.
        if(worldExecuted) {GetVisualState().circles.RetireAll(0.60F);}
        else {ReleaseCircleVisuals();}


        logger::info(
            "[TOAM][S07C3] WORLD_CHARGE_RELEASE | Fire={} | Executed={} | Stage={} | Circles={} | Charge={:.3f} | Requested={:.3f} | Allocated={:.3f} | Beams={} | Impacts={} | Hostiles={} | QualifyingExplosionHits={} | DamageHits={}",
            output.shouldFire,
            worldExecuted,
            output.stage,
            output.circleCount,
            output.accumulatedCharge,
            state.requested,
            state.allocated,
            execution.beamsSpawned,
            execution.impactsSpawned,
            execution.hostileActorsInAnyRadius,
            execution.qualifyingExplosionHits,
            execution.damageHitsApplied);

        // One diagnostic record per completed World release, not per hit.
        if (output.shouldFire) {
            logger::info(
                "[TOAM][QA] ARCANA_DAMAGE | Spell=World | Stage={} | Circles={} | Charge={:.3f} | Mult={:.3f} | BaseExplosion={:.3f} | EffectiveExplosion={:.3f} | QualifyingExplosionHits={} | DamageHits={} | Executed={}",
                output.stage,
                output.circleCount,
                output.accumulatedCharge,
                damageMultiplier,
                TOAM::Spells::World::kExplosionDamage,
                explosionDamage,
                execution.qualifyingExplosionHits,
                execution.damageHitsApplied,
                worldExecuted);
        }


        state.requested =
            0.0;

        state.allocated =
            0.0;


        TOAM::Core::ExclusiveCast::Release(TOAM::Core::ExclusiveCast::Owner::World);
        return output;
    }


    void CancelCharge() noexcept
    {
        auto& state =
            GetState();


        const bool hadState =
            state.buttonHeld ||
            state.releasePending ||
            state.session.IsActive();


        state.session.Reset();

        state.buttonHeld =
            false;

        state.releasePending =
            false;

        state.requested =
            0.0;

        state.allocated =
            0.0;


        ReleaseAllWorldVisuals();
        TOAM::Core::ExclusiveCast::Release(TOAM::Core::ExclusiveCast::Owner::World);


        if (hadState) {

            logger::info(
                "[TOAM][S07C3] WORLD_CHARGE_CANCEL");
        }
    }


    InputDisposition HandleShoutButton(
        RE::PlayerCharacter* player,
        RE::ButtonEvent* event) noexcept
    {
        if (
            !player ||
            !event
        ) {
            return
                InputDisposition::
                    kForwardVanilla;
        }


        auto& state =
            GetState();


        const bool worldSelected =
            IsWorldSelected(
                player
            );


        const bool ownsWorldCharge =
            state.buttonHeld ||
            state.releasePending ||
            state.session.IsActive();


        if (
            !worldSelected &&
            !ownsWorldCharge
        ) {
            return
                InputDisposition::
                    kForwardVanilla;
        }


        if (!worldSelected) {

            CancelCharge();


            // The transition belonged to the World charge previously
            // acquired by TOAM. Do not pass this same transition into
            // a newly selected vanilla Shout/Power.
            return
                InputDisposition::
                    kConsume;
        }


        if (
            event->IsDown()
        ) {
            static_cast<void>(
                BeginCharge(
                    player
                )
            );
        }
        else if (
            event->IsUp()
        ) {
            if (
                state.buttonHeld &&
                state.session.IsActive()
            ) {
                // C2 ordering authority:
                //
                // queue release here; the shared recurring InputEvent
                // tick performs the final UpdateCharge before calling
                // ProcessPendingRelease.
                state.releasePending =
                    true;


                logger::info(
                    "[TOAM][S07C3] WORLD_RELEASE_PENDING | Charge={:.3f} | Stage={}",
                    state.session.
                        AccumulatedCharge(),
                    state.session.
                        Tier());
            }
        }


        return
            InputDisposition::
                kConsume;
    }


    bool InstallInputHook() noexcept
    {
        if (
            g_inputHookInstalled
        ) {
            return true;
        }


        REL::Relocation<
            std::uintptr_t
        > vtable{
            RE::ShoutHandler::
                VTABLE[0]
        };


        const std::uintptr_t original =
            vtable.write_vfunc(
                0x04,
                HookedShoutProcessButton
            );


        if (original == 0) {

            logger::error(
                "[TOAM][S07C3] ShoutHandler ProcessButton original address is null");


            return false;
        }


        g_originalShoutProcessButton =
            original;


        g_inputHookInstalled =
            true;


        logger::info(
            "[TOAM][S07C3] ShoutHandler ProcessButton hook installed | Slot=04");


        return true;
    }


    bool IsInputHookInstalled() noexcept
    {
        return
            g_inputHookInstalled;
    }


    void ResetTransientState() noexcept
    {
        CancelCharge();
    }
}