#include "Arcana/Integration.h"
#include "Audio/SpellAudio.h"

#include "Spells/MagicianRuntime.h"

#include "Combat/ActorBounds.h"
#include "Combat/BeamCollision.h"
#include "Combat/FixedDamage.h"
#include "Combat/TriggerFirewall.h"
#include "Spells/MagicianChargeVisual.h"
#include "Spells/MagicianProfile.h"
#include "VFX/BeamRenderer.h"
#include "VFX/TransientVFXManager.h"

#include "RE/Skyrim.h"

#include <array>
#include <cmath>

namespace TOAM::Spells::Magician::Runtime
{
    namespace
    {
        constexpr float
            kDirectionEpsilon{
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


        [[nodiscard]]
        float LengthSquared(
            const RE::NiPoint3& value) noexcept
        {
            return
                value.x * value.x +
                value.y * value.y +
                value.z * value.z;
        }


        [[nodiscard]]
        bool Normalize(
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
        bool IsHostileToSource(
            RE::Actor* source,
            RE::Actor* target) noexcept
        {
            return source && target && source != target &&
                target->IsHostileToActor(source);
        }

        [[nodiscard]]
        bool IntersectsExplosionSphere(
            const RE::NiPoint3& center,
            float radius,
            const Combat::ActorBounds& bounds) noexcept
        {
            if (!IsFinitePoint(center) ||
                !std::isfinite(radius) || radius <= 0.0F ||
                !Combat::IsUsableActorBounds(bounds)) {
                return false;
            }

            const float combined = radius + bounds.radius;
            if (!std::isfinite(combined) || combined <= 0.0F) {
                return false;
            }

            const RE::NiPoint3 offset = bounds.center - center;
            const float distanceSquared = LengthSquared(offset);

            return std::isfinite(distanceSquared) &&
                distanceSquared <= combined * combined;
        }

        constexpr float
            kChargeVisualForwardOffset{
                96.0F
            };


        constexpr float
            kFinalBeamLifetimeSeconds{
                0.48F
            };


        constexpr std::array<
            float,
            kTierCount + 1
        > kVisualBeamWidthByTier{{
            0.0F,
            0.75F,
            0.90F,
            1.10F,
            1.35F,
            1.65F,
            2.00F,
            2.40F,
            2.90F
        }};


        struct VisualBasis final
        {
            RE::NiPoint3 x{};
            RE::NiPoint3 y{};
            RE::NiPoint3 z{};
        };


        struct VisualState final
        {
            ChargeVisual chargeVisual{};
            VFX::BeamRenderer beamRenderer{};

            RE::NiNode* parent{};
            bool chargeBegun{};
        };


        [[nodiscard]]
        VisualState& GetVisualState() noexcept
        {
            static VisualState state{};
            return state;
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
            const RE::NiPoint3& direction,
            VisualBasis& result) noexcept
        {
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
                worldRoot->world.Invert();


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
        bool ResolveGameplayOrigin(
            RE::PlayerCharacter* player,
            RE::NiPoint3& origin) noexcept
        {
            if (!player || !player->Is3DLoaded()) { return false; }
            origin = Arcana::HostSunAnchor(player); // CP027: visual and collision share one Sun-style axis.
            return IsFinitePoint(origin);
        }


        [[nodiscard]]
        bool ResolveAim(
            RE::PlayerCharacter* player,
            RE::NiPoint3& origin,
            RE::NiPoint3& direction) noexcept
        {
            if (!player) {
                return false;
            }


            if (
                !ResolveGameplayOrigin(
                    player,
                    origin
                )
            ) {
                return false;
            }


            if (
                !Arcana::HostAimDirection(player, direction)
            ) {
                return false;
            }


            return true;
        }


        [[nodiscard]]
        bool ResolveChargeAnchor(
            RE::PlayerCharacter* player,
            RE::NiNode*& parent,
            RE::NiTransform& anchorLocal) noexcept
        {
            RE::NiPoint3 origin{};
            RE::NiPoint3 direction{};


            if (
                !ResolveAim(
                    player,
                    origin,
                    direction
                )
            ) {
                return false;
            }


            VisualBasis basis{};


            if (
                !BuildVisualBasis(
                    direction,
                    basis
                )
            ) {
                return false;
            }


            RE::NiTransform rootInverse{};


            if (
                !ResolveVisualRoot(
                    parent,
                    rootInverse
                )
            ) {
                return false;
            }


            RE::NiTransform anchorWorld{};

            // Same engine aim and actor-front/flight anchor as Sun. No aim-point placement.
            anchorWorld.translate = Arcana::HostSunAnchor(player);

            anchorWorld.scale =
                1.0F;


            ApplyVisualBasis(
                anchorWorld,
                basis
            );


            anchorLocal =
                rootInverse *
                anchorWorld;


            return
                IsFiniteTransform(
                    anchorLocal
                );
        }


        [[nodiscard]]
        float VisualBeamWidth(
            const std::uint8_t tier) noexcept
        {
            if (
                tier == 0 ||
                tier >
                    kTierCount
            ) {
                return 0.0F;
            }


            return
                kVisualBeamWidthByTier[
                    static_cast<
                        std::size_t>(
                            tier
                        )
                ];
        }


        [[nodiscard]]
        bool ApplyChargeVisualState(
            VisualState& state,
            RE::NiNode* parent,
            const RE::NiTransform& anchorLocal,
            const float effectiveCharge,
            const float deltaSeconds) noexcept
        {
            if (
                !state.chargeBegun ||
                state.parent !=
                    parent
            ) {
                state.chargeVisual.
                    ReleaseAll();

                state.parent =
                    nullptr;

                state.chargeBegun =
                    false;


                if (
                    !state.chargeVisual.
                        Begin(
                            parent
                        )
                ) {
                    return false;
                }


                state.parent =
                    parent;

                state.chargeBegun =
                    true;
            }


            if (
                !state.chargeVisual.
                    SetAnchorTransform(
                        anchorLocal
                    )
            ) {
                return false;
            }


            if (
                !state.chargeVisual.
                    SetEffectiveCharge(
                        effectiveCharge
                    )
            ) {
                return false;
            }


            return
                state.chargeVisual.
                    Update(
                        deltaSeconds
                    );
        }


        // Presentation only. Does not create an explosion or modify damage.
        [[nodiscard]]
        bool ShowImpactExplosionVisual(
            const RE::NiPoint3& worldPosition,
            const float gameplayRadius) noexcept
        {
            if (!IsFinitePoint(worldPosition) ||
                !std::isfinite(gameplayRadius) ||
                gameplayRadius <= 0.0F) {
                return false;
            }

            RE::NiNode* parent{};
            RE::NiTransform rootInverse{};
            if (!ResolveVisualRoot(parent, rootInverse)) {
                return false;
            }

            RE::NiTransform worldTransform{};
            worldTransform.translate = worldPosition;
            worldTransform.rotate.SetEulerAnglesXYZ(
                RE::NiPoint3{0.0F, 0.0F, 0.0F});

            // Visual radius grows with the existing tier explosion radius;
            // no changes to gameplay radius, damage or collision rules.
            worldTransform.scale = 0.75F * (gameplayRadius / 64.0F);
            const auto localTransform = rootInverse * worldTransform;
            if (!IsFiniteTransform(localTransform)) {
                return false;
            }

            VFX::SpawnRequest request{};
            request.asset = VFX::AssetID::kFlashCloud;
            request.parent = parent;
            request.local = localTransform;
            request.lifetimeSeconds = 2.20F;
            request.visible = true;
            return VFX::TransientVFXManager::Get().Spawn(request).IsValid();
        }


        [[nodiscard]]
        bool ShowFinalBeam(
            const RE::NiPoint3& origin,
            const RE::NiPoint3& direction,
            const float resolvedDistance,
            const std::uint8_t tier,const float effectiveCharge) noexcept
        {
            if (
                !IsFinitePoint(
                    origin
                ) ||
                !IsFinitePoint(
                    direction
                ) ||
                !std::isfinite(
                    resolvedDistance
                ) ||
                resolvedDistance <=
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


            const float startOffset =
                resolvedDistance >
                    kChargeVisualForwardOffset ?
                    kChargeVisualForwardOffset :
                    0.0F;


            RE::NiPoint3 visualOriginWorld = Arcana::HostSunAnchor(RE::PlayerCharacter::GetSingleton())+
                direction*ChargeVisual::TuningBeamDepth(effectiveCharge);
            const auto muzzleOffset=visualOriginWorld-origin;
            if(muzzleOffset.x*direction.x+muzzleOffset.y*direction.y+muzzleOffset.z*direction.z>=resolvedDistance)
                visualOriginWorld=origin; // A wall before the stack still uses the exact collision endpoint.


            const RE::NiPoint3
                visualEndWorld =
                    origin +
                    direction *
                        resolvedDistance;


            const RE::NiPoint3
                localOrigin =
                    rootInverse *
                    visualOriginWorld;


            const RE::NiPoint3
                localEnd =
                    rootInverse *
                    visualEndWorld;


            const RE::NiPoint3 localVector{
                localEnd.x - localOrigin.x,
                localEnd.y - localOrigin.y,
                localEnd.z - localOrigin.z
            };


            const float localLengthSquared =
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


            RE::NiPoint3 localDirection{};


            if (
                !Normalize(
                    localVector,
                    localDirection
                )
            ) {
                return false;
            }


            const float widthWorld =
                VisualBeamWidth(
                    tier
                );


            const float widthLocal =
                widthWorld *
                std::fabs(
                    rootInverse.scale
                );


            if (
                !std::isfinite(
                    widthLocal
                ) ||
                widthLocal <=
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
                widthLocal;

            pose.lifetimeSeconds =
                kFinalBeamLifetimeSeconds;

            pose.visible =
                true;

            pose.asset =
                VFX::AssetID::
                    kBeamWholeLens;


            auto& state =
                GetVisualState();


            const bool spawned =
                state.beamRenderer.
                    SetBeam(
                        parent,
                        pose
                    );


            if (spawned) {
                state.beamRenderer.Detach();
                logger::info(
                    "[TOAM][S05C2] BEAM_VISUAL | Tier={} | Distance={:.3f} | Width={:.3f} | Lifetime={:.3f}",
                    tier,
                    resolvedDistance,
                    widthWorld,
                    kFinalBeamLifetimeSeconds);
            }


            return spawned;
        }
    }


    bool BeginChargeVisual(
        RE::PlayerCharacter* player) noexcept
    {
        if (!player) {
            return false;
        }


        auto& state =
            GetVisualState();


        state.chargeVisual.
            ReleaseAll();

        state.parent =
            nullptr;

        state.chargeBegun =
            false;


        RE::NiNode* parent{};
        RE::NiTransform anchorLocal{};


        if (
            !ResolveChargeAnchor(
                player,
                parent,
                anchorLocal
            )
        ) {
            return false;
        }


        if (
            !state.chargeVisual.
                Begin(
                    parent
                ) ||
            !state.chargeVisual.
                SetAnchorTransform(
                    anchorLocal
                )
        ) {
            state.chargeVisual.
                ReleaseAll();

            return false;
        }


        state.parent =
            parent;

        state.chargeBegun =
            true;


        logger::debug(
            "[TOAM][S05C2] CHARGE_VISUAL_BEGIN");


        return true;
    }


    bool UpdateChargeVisual(
        RE::PlayerCharacter* player,
        const float effectiveCharge,
        const float deltaSeconds) noexcept
    {
        if (
            !player ||
            !std::isfinite(
                effectiveCharge
            ) ||
            effectiveCharge <
                0.0F ||
            !std::isfinite(
                deltaSeconds
            ) ||
            deltaSeconds <=
                0.0F
        ) {
            return false;
        }


        RE::NiNode* parent{};
        RE::NiTransform anchorLocal{};


        if (
            !ResolveChargeAnchor(
                player,
                parent,
                anchorLocal
            )
        ) {
            return false;
        }


        auto& state =
            GetVisualState();


        if (
            ApplyChargeVisualState(
                state,
                parent,
                anchorLocal,
                effectiveCharge,
                deltaSeconds
            )
        ) {
            return true;
        }


        logger::warn(
            "[TOAM][S05C2] CHARGE_VISUAL_REBUILD | Charge={:.3f}",
            effectiveCharge);


        state.chargeVisual.
            ReleaseAll();

        state.parent =
            nullptr;

        state.chargeBegun =
            false;


        return
            ApplyChargeVisualState(
                state,
                parent,
                anchorLocal,
                effectiveCharge,
                deltaSeconds
            );
    }


    void ReleaseChargeVisual() noexcept
    {
        auto& state =
            GetVisualState();


        const bool hadChargeVisual =
            state.chargeBegun;


        state.chargeVisual.
            ReleaseAll();

        state.parent =
            nullptr;

        state.chargeBegun =
            false;


        if (hadChargeVisual) {
            logger::debug(
                "[TOAM][S05C2] CHARGE_VISUAL_RELEASE");
        }
    }


    void ResetVisualState() noexcept
    {
        auto& state =
            GetVisualState();


        state.chargeVisual.
            ReleaseAll();

        state.beamRenderer.
            Release();

        state.parent =
            nullptr;

        state.chargeBegun =
            false;
    }


    FireResult ExecuteFinalRelease(
        RE::PlayerCharacter* player,
        const float effectiveCharge,
        const float costMultiplier) noexcept
    {
        FireResult result{};


        const float damageMultiplier =
            std::isfinite(costMultiplier) &&
            costMultiplier >= 0.5F &&
            costMultiplier <= 1.0F ?
                costMultiplier : 1.0F;

        result.effectiveCharge =
            effectiveCharge;


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
            ResolveTier(
                effectiveCharge
            );


        if (!profile) {

            logger::warn(
                "[TOAM][S05C1] FIRE_REJECTED | Reason=NO_TIER | Charge={:.3f}",
                effectiveCharge);

            return result;
        }


        result.tier =
            profile->tier;


        result.beamDamage =
            profile->beamDamage * damageMultiplier;


        RE::NiPoint3 origin{};
        RE::NiPoint3 direction{};


        if (
            !ResolveAim(
                player,
                origin,
                direction
            )
        ) {

            logger::error(
                "[TOAM][S05C1] FIRE_REJECTED | Reason=AIM_UNRESOLVED | Tier={} | Charge={:.3f}",
                result.tier,
                effectiveCharge);

            return result;
        }


        result.aimResolved =
            true;


        FirePlan firePlan{};


        if (
            !BuildFirePlan(
                effectiveCharge,
                origin,
                direction,
                firePlan
            ) ||
            !firePlan.valid ||
            !firePlan.profile
        ) {

            logger::error(
                "[TOAM][S05C1] FIRE_REJECTED | Reason=FIRE_PLAN | Tier={} | Charge={:.3f}",
                result.tier,
                effectiveCharge);

            return result;
        }


        result.firePlanResolved =
            true;


        Combat::BeamCollisionResult
            collision{};


        if (
            !Combat::ResolveBeamCollision(
                firePlan.beam,
                player,
                collision
            ) ||
            !collision.resolved
        ) {

            logger::error(
                "[TOAM][S05C1] FIRE_REJECTED | Reason=COLLISION_UNRESOLVED | Tier={} | Charge={:.3f}",
                result.tier,
                effectiveCharge);

            return result;
        }


        result.collisionResolved =
            true;


        result.worldBlocked =
            collision.worldStop.blocked;


        result.resolvedDistance =
            collision.worldStop.distance;


        result.actorIntersections =
            collision.actorHits.size();


        for (
            const auto& hit :
            collision.actorHits
        ) {

            auto target =
                hit.actor.get();


            if (!target) {
                continue;
            }


            if (
                Combat::ApplyFixedHealthDamage(
                    player,
                    target.get(),
                    result.beamDamage,
                    Combat::FirewallOrigin::
                        kMagicianBeam
                )
            ) {

                ++result.actorsDamaged;
            }
        }


        // Explosion gameplay only on an actual terrain/static world stop.
        // Actor intersections and maximum-range expiration do not trigger it.
        if (collision.worldStop.blocked) {
            // Independent presentation; Actor hits alone never create it.
            const bool impactVisual = ShowImpactExplosionVisual(
                collision.worldStop.point,
                firePlan.profile->explosionRadius);
            logger::info(
                "[TOAM][S05] IMPACT_VFX | Tier={} | Spawned={}",
                result.tier, impactVisual);

            std::size_t explosionQualified{};
            std::size_t explosionDamaged{};

            auto* lists = RE::ProcessLists::GetSingleton();

            if (!lists || !IsFinitePoint(collision.worldStop.point)) {
                logger::warn(
                    "[TOAM][S05C3] IMPACT_EXPLOSION_UNAVAILABLE | Tier={} | Lists={} | PointValid={}",
                    result.tier, lists != nullptr,
                    IsFinitePoint(collision.worldStop.point));
            } else {
                lists->ForAllActors([&](RE::Actor* actor) {
                    if (!IsHostileToSource(player, actor)) {
                        return RE::BSContainer::ForEachResult::kContinue;
                    }

                    Combat::ActorBounds bounds{};
                    if (!Combat::ResolveActorBounds(actor, bounds) ||
                        !IntersectsExplosionSphere(
                            collision.worldStop.point,
                            firePlan.profile->explosionRadius, bounds)) {
                        return RE::BSContainer::ForEachResult::kContinue;
                    }

                    ++explosionQualified;

                    if (Combat::ApplyFixedHealthDamage(
                        player, actor,
                        firePlan.profile->explosionDamage * damageMultiplier,
                        Combat::FirewallOrigin::kMagicianExplosion)) {
                        ++explosionDamaged;
                    }

                    return RE::BSContainer::ForEachResult::kContinue;
                });

                logger::info(
                    "[TOAM][S05C3] IMPACT_EXPLOSION | Tier={} | Radius={:.3f} | Damage={:.3f} | Hostiles={} | Applied={}",
                    result.tier,
                    firePlan.profile->explosionRadius,
                    firePlan.profile->explosionDamage * damageMultiplier,
                    explosionQualified,
                    explosionDamaged);
            }
        }

        result.executed =
            true;
        TOAM::Audio::MagicianFinal(result.tier,origin);


        if (
            !ShowFinalBeam(
                origin,
                direction,
                result.resolvedDistance,
                result.tier,
                effectiveCharge
            )
        ) {
            logger::warn(
                "[TOAM][S05C2] BEAM_VISUAL_FAILED | Tier={} | Distance={:.3f}",
                result.tier,
                result.resolvedDistance);
        }


        logger::info(
            "[TOAM][S05C1] FIRE | Tier={} | Charge={:.3f} | Damage={:.3f} | Range={:.3f} | Radius={:.3f} | WorldBlocked={} | StopDistance={:.3f} | Intersections={} | Damaged={}",
            result.tier,
            effectiveCharge,
            result.beamDamage,
            firePlan.profile->beamRange,
            firePlan.profile->beamRadius,
            result.worldBlocked,
            result.resolvedDistance,
            result.actorIntersections,
            result.actorsDamaged);


        logger::info(
            "[TOAM][QA] ARCANA_DAMAGE | Spell=Magician | Tier={} | Charge={:.3f} | Mult={:.3f} | BaseBeam={:.3f} | EffectiveBeam={:.3f} | BaseExplosion={:.3f} | EffectiveExplosion={:.3f} | Damaged={}",
            result.tier,
            effectiveCharge,
            damageMultiplier,
            firePlan.profile->beamDamage,
            result.beamDamage,
            firePlan.profile->explosionDamage,
            firePlan.profile->explosionDamage * damageMultiplier,
            result.actorsDamaged);

        return result;
    }
}