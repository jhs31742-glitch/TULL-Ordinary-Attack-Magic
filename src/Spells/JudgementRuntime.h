#pragma once

#include "Combat/ActorBounds.h"
#include "Combat/BeamCollision.h"
#include "Combat/TargetFormation.h"
#include "Spells/JudgementProfile.h"

#include "RE/Skyrim.h"

#include <array>
#include <cstddef>

namespace TOAM::Spells::Judgement::Runtime
{
    // Persistent damage-free guide; bounded 10 Hz enemy selection. Release revalidates once.
    void BeginChargeGuide() noexcept;
    void UpdateChargeGuide(RE::PlayerCharacter* player) noexcept;
    void SuspendChargeGuide() noexcept;
    void ReleaseChargeGuide() noexcept;
    // Starting gameplay-tuning value.
    //
    // This is intentionally centralized because the final Judgement
    // formation radius must be adjusted through runtime visual testing.
    //
    // It is NOT a Stage LOCK value.
    inline constexpr float
        kFormationRadiusScale{
            1.50F
        };


    struct MarkTargetSnapshot final
    {
        RE::ActorHandle target{};

        RE::NiPoint3 initialHitPoint{
            0.0F,
            0.0F,
            0.0F
        };

        Combat::ActorBounds
            initialBounds{};

        bool valid{};
    };


    struct ReleasePlan final
    {
        const StageProfile* profile{};

        MarkTargetSnapshot mark{};

        bool valid{};
    };


    // This structure deliberately contains every spatial value required
    // after the mark has been accepted.
    //
    // Later artillery execution must not need to re-resolve the Actor
    // simply to preserve its original firing geometry.
    struct WorldFixedArtilleryPlan final
    {
        const StageProfile* profile{};

        // Preserved only for each shot's live aim snapshot.
        //
        // Formation positions themselves remain world-fixed.
        RE::ActorHandle target{};

        Combat::TargetFormation
            formation{};

        RE::NiPoint3 fixedAimPoint{
            0.0F,
            0.0F,
            0.0F
        };

        float formationRadius{};

        bool valid{};
    };


    // Scheduler timing authority.
    //
    // These remain centralized so runtime balance adjustment does not
    // require changing scheduler logic.
    inline constexpr float
        kFirstShotDelaySeconds{
            0.45F
        };

    inline constexpr float
        kSubsequentShotIntervalSeconds{
            0.12F
        };


    struct ArtillerySchedule final
    {
        // The scheduler owns a copy of all required spatial data.
        //
        // No live Actor lookup is required after BeginArtillerySchedule().
        WorldFixedArtilleryPlan plan{};

        // Initial value = original target bounds center.
        //
        // While target is alive, each due shot refreshes this from
        // current ActorBounds.
        //
        // If the target dies or its handle becomes invalid, remaining
        // already-created artillery continues toward this remembered
        // snapshot instead of being cancelled.
        RE::NiPoint3 lastAimPoint{
            0.0F,
            0.0F,
            0.0F
        };

        float elapsedSeconds{};

        float nextShotTime{};

        std::size_t nextShotIndex{};

        // Frozen at charge start and copied into each delayed schedule.
        float damageMultiplier{ 1.0F };
        float configuredDamageMultiplier{ 1.0F };

        bool active{};

        bool complete{};
    };


    struct ArtilleryScheduleAdvance final
    {
        std::array<
            std::size_t,
            Combat::TargetFormation::kCapacity
        > dueShotIndices{};

        std::size_t dueCount{};

        bool completedThisAdvance{};
    };


    [[nodiscard]]
    bool BeginArtillerySchedule(
        const WorldFixedArtilleryPlan& plan,
        ArtillerySchedule& result) noexcept;


    [[nodiscard]]
    bool AdvanceArtillerySchedule(
        float deltaSeconds,
        ArtillerySchedule& schedule,
        ArtilleryScheduleAdvance& result) noexcept;


    [[nodiscard]]
    bool BuildReleasePlan(
        float effectiveCharge,
        const Combat::BeamCollisionResult& markingCollision,
        ReleasePlan& result) noexcept;


    [[nodiscard]]
    bool BuildWorldFixedArtilleryPlan(
        const ReleasePlan& releasePlan,
        const RE::NiPoint3& forwardAxis,
        float radiusScale,
        WorldFixedArtilleryPlan& result) noexcept;


    [[nodiscard]]
    bool BuildDefaultWorldFixedArtilleryPlan(
        const ReleasePlan& releasePlan,
        const RE::NiPoint3& forwardAxis,
        WorldFixedArtilleryPlan& result) noexcept;


    struct ScheduledShotCollision final
    {
        Combat::BeamSegmentQuery
            query{};

        Combat::BeamCollisionResult
            collision{};

        std::size_t shotIndex{};

        bool queryBuilt{};

        bool collisionResolved{};
    };


    [[nodiscard]]
    bool BuildFixedShotQuery(
        const RE::NiPoint3& circleOrigin,
        const RE::NiPoint3& fixedAimPoint,
        float shotRadius,
        Combat::BeamSegmentQuery& result) noexcept;


    [[nodiscard]]
    bool ResolveScheduledShotCollision(
        RE::Actor* collisionIgnoreActor,
        const WorldFixedArtilleryPlan& plan,
        std::size_t shotIndex,
        const RE::NiPoint3& shotAimPoint,
        float shotRadius,
        ScheduledShotCollision& result) noexcept;


    struct ScheduledShotDamageResult final
    {
        ScheduledShotCollision shot{};

        RE::NiPoint3 explosionCenter{
            0.0F,
            0.0F,
            0.0F
        };

        std::size_t beamIntersections{};
        std::size_t beamHostileIntersections{};
        std::size_t beamActorsDamaged{};

        std::size_t explosionActorsInRadius{};
        std::size_t explosionActorsDamaged{};

        bool explosionResolved{};
        bool executed{};
    };


    [[nodiscard]]
    bool ExecuteScheduledShotDamage(
        RE::Actor* sourceActor,
        const WorldFixedArtilleryPlan& plan,
        std::size_t shotIndex,
        const RE::NiPoint3& shotAimPoint,
        float shotRadius,
        float damageMultiplier,
        ScheduledShotDamageResult& result,
        float configuredDamageMultiplier = 1.0F) noexcept;


    [[nodiscard]]
    bool SnapshotScheduledShotAim(
        ArtillerySchedule& schedule,
        RE::NiPoint3& result) noexcept;


    struct FinalReleaseResult final
    {
        float effectiveCharge{};

        std::size_t
            markingActorIntersections{};

        std::uint8_t stage{};

        bool attempted{};
        bool aimResolved{};
        bool markingCollisionResolved{};
        bool targetAcquired{};
        bool markingDamageApplied{};
        bool formationResolved{};
        bool scheduleStarted{};
        bool executed{};
    };


    [[nodiscard]]
    FinalReleaseResult ExecuteFinalRelease(
        RE::PlayerCharacter* player,
        float effectiveCharge,
        float costMultiplier) noexcept;


    void UpdateArtillery(
        RE::PlayerCharacter* player,
        float deltaSeconds) noexcept;


    void ResetTransientState() noexcept;


    [[nodiscard]]
    bool HasActiveArtillery() noexcept;
}