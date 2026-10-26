#include "Arcana/Integration.h"
#include "Audio/SpellAudio.h"

#include "Charge/ChargeRuntime.h"

#include "Charge/ChargeProfile.h"
#include "Charge/ChargeSession.h"
#include "Charge/MagickaChargeArbiter.h"
#include "Core/DataCache.h"
#include "Core/Runtime.h"
#include "Spells/ArcanaLimiterRuntime.h"
#include "Spells/JudgementRuntime.h"
#include "Spells/MagicianRuntime.h"
#include "Spells/TemperanceRuntime.h"
#include "Spells/WorldRuntime.h"

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include <cstdint>

#include <array>
#include <cmath>
#include <cstddef>
#include <span>

namespace TOAM::Charge::Runtime
{
    namespace
    {
        enum class Owner : std::uint8_t
        {
            kNone = 0,
            kMagician,
            kJudgement
        };

        struct Metrics
        {
            double requested{};
            double allocated{};
            double handSeconds{};
        };

        struct State
        {
            ChargeSession magician{ MagicianProfile() };
            ChargeSession judgement{ JudgementProfile() };

            std::array<Owner, 2> handOwner{
                Owner::kNone,
                Owner::kNone
            };

            Metrics magicianMetrics{};
            Metrics judgementMetrics{};

            bool installed{};
            bool magickaEmpty{};
            bool constrained{};
        };

        [[nodiscard]]
        State& GetState() noexcept
        {
            static State state{};
            return state;
        }

        [[nodiscard]]
        constexpr std::size_t HandIndex(
            const Hand hand) noexcept
        {
            return hand == Hand::kLeft ?
                0U :
                1U;
        }

        [[nodiscard]]
        constexpr const char* HandName(
            const Hand hand) noexcept
        {
            return hand == Hand::kLeft ?
                "LEFT" :
                "RIGHT";
        }

        [[nodiscard]]
        constexpr const char* OwnerName(
            const Owner owner) noexcept
        {
            switch (owner) {
            case Owner::kMagician:
                return "Magician";

            case Owner::kJudgement:
                return "Judgement";

            default:
                return "None";
            }
        }

        [[nodiscard]]
        ChargeSession& Session(
            const Owner owner) noexcept
        {
            auto& state = GetState();

            return owner == Owner::kJudgement ?
                state.judgement :
                state.magician;
        }

        [[nodiscard]]
        Metrics& SessionMetrics(
            const Owner owner) noexcept
        {
            auto& state = GetState();

            return owner == Owner::kJudgement ?
                state.judgementMetrics :
                state.magicianMetrics;
        }

        // R1-C2C: an admission check only; no Magicka reservation or deduction.
        [[nodiscard]]
        bool HasFirstTierMagicka(
            RE::PlayerCharacter* player,
            const Owner owner,
            const float multiplier) noexcept
        {
            if (!player || owner == Owner::kNone ||
                !std::isfinite(multiplier) || multiplier < 0.5F ||
                multiplier > 1.0F) {
                return false;
            }
            const auto& profile = Session(owner).Profile();
            if (profile.thresholdCount == 0 ||
                !(profile.thresholds[0] > 0.0F)) {
                return false;
            }
            auto* av = player->AsActorValueOwner();
            if (!av) {
                return false;
            }
            const float magicka = av->GetActorValue(
                RE::ActorValue::kMagicka);
            const float required = profile.thresholds[0] * multiplier;
            return std::isfinite(magicka) && magicka >= required;
        }

        [[nodiscard]]
        Owner EquippedOwner(
            RE::PlayerCharacter* player,
            const Hand hand) noexcept
        {
            if (
                !player ||
                !TOAM::Core::DataCache::IsReady()
            ) {
                return Owner::kNone;
            }

            auto* equipped =
                player->GetEquippedObject(
                    hand == Hand::kLeft
                );

            if (
                equipped ==
                TOAM::Core::DataCache::GetMagician()
            ) {
                return Owner::kMagician;
            }

            if (
                equipped ==
                TOAM::Core::DataCache::GetJudgement()
            ) {
                return Owner::kJudgement;
            }

            return Owner::kNone;
        }

        [[nodiscard]]
        bool AnyActive() noexcept
        {
            const auto& state = GetState();

            return
                state.magician.IsActive() ||
                state.judgement.IsActive();
        }

        // S05-C4: preserve independent Magician/Judgement charge sessions.
        using CheckCastFn = bool (*)(
            RE::ActorMagicCaster*, RE::MagicItem*, bool, float*,
            RE::MagicSystem::CannotCastReason*, bool);

        using RequestCastFn = void (*)(RE::ActorMagicCaster*);

        std::uintptr_t g_originalCheckCast{};
        std::uintptr_t g_originalRequestCast{};
        bool g_checkCastHookInstalled{};
        bool g_requestCastHookInstalled{};

        [[nodiscard]]
        bool DenyOtherHandMagic(
            RE::ActorMagicCaster* caster,
            RE::MagicItem* attempted) noexcept
        {
            if (!caster ||
                !TOAM::Core::Runtime::IsGameReady() ||
                !TOAM::Core::DataCache::IsReady()) {
                return false;
            }

            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player || caster->GetCasterAsActor() != player) {
                return false;
            }

            auto& state = GetState();
            if (!state.magician.IsActive()) {
                return false;
            }

            const auto source = caster->GetCastingSource();
            std::size_t opposite{};

            if (source == RE::MagicSystem::CastingSource::kLeftHand) {
                opposite = 1U;
            } else if (source == RE::MagicSystem::CastingSource::kRightHand) {
                opposite = 0U;
            } else {
                return false;
            }

            if (state.handOwner[opposite] != Owner::kMagician) {
                return false;
            }

            auto* magician = TOAM::Core::DataCache::GetMagician();
            auto* judgement = TOAM::Core::DataCache::GetJudgement();

            if (attempted == magician || attempted == judgement) {
                return false;
            }

            // User-authorized CP019 exception: Arcana owns its caster costs/hand cadence.
            // Only resolved Arcana carrier/enchantment pointers bypass the other-hand gate.
            if (Arcana::OwnsHandMagic(attempted)) {
                return false;
            }

            // A non-null non-TOAM, non-Arcana MagicItem is forbidden on the
            // hand opposite an actively charging Magician.
            if (attempted) {
                return true;
            }

            // RequestCastImpl has no spell parameter. Inspect the
            // verified equipped form as a conservative fallback.
            auto* equipped = player->GetEquippedObject(
                source == RE::MagicSystem::CastingSource::kLeftHand);

            if (!equipped || equipped == magician ||
                equipped == judgement) {
                return false;
            }

            if (equipped->As<RE::SpellItem>()) {
                return true;
            }

            if (auto* weapon = equipped->As<RE::TESObjectWEAP>()) {
                return weapon->IsStaff() && !Arcana::OwnsHandMagic(weapon->formEnchanting);
            }

            return equipped->GetFormType() == RE::FormType::Scroll;
        }

        // CheckCast/RequestCast only gate a NEW TOAM charge session.
        [[nodiscard]]
        bool AllowsNewOOMCast(
            RE::ActorMagicCaster* caster,
            RE::MagicItem* attempted) noexcept
        {
            if (!caster || !TOAM::Core::Runtime::IsGameReady() ||
                !TOAM::Core::DataCache::IsReady()) {
                return true;
            }
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player || caster->GetCasterAsActor() != player) {
                return true;
            }
            const auto source = caster->GetCastingSource();
            bool isLeft{};
            if (source == RE::MagicSystem::CastingSource::kLeftHand) {
                isLeft = true;
            } else if (source != RE::MagicSystem::CastingSource::kRightHand) {
                return true;
            }
            auto* magician = TOAM::Core::DataCache::GetMagician();
            auto* judgement = TOAM::Core::DataCache::GetJudgement();
            if (!attempted) {
                auto* equipped = player->GetEquippedObject(isLeft);
                attempted = equipped ? equipped->As<RE::SpellItem>() : nullptr;
            }
            Owner owner = Owner::kNone;
            if (attempted && attempted == magician) {
                owner = Owner::kMagician;
            } else if (attempted && attempted == judgement) {
                owner = Owner::kJudgement;
            }
            if (owner == Owner::kNone || Session(owner).IsActive()) {
                return true;
            }
            const float multiplier =
                TOAM::Spells::ArcanaLimiter::Runtime::
                    SnapshotCostMultiplier(player);
            return HasFirstTierMagicka(player, owner, multiplier);
        }

        bool HookedCheckCast(
            RE::ActorMagicCaster* caster,
            RE::MagicItem* spell,
            bool dual,
            float* strength,
            RE::MagicSystem::CannotCastReason* reason,
            bool baseCost)
        {
            if (DenyOtherHandMagic(caster, spell)) {
                if (reason) {
                    *reason =
                        RE::MagicSystem::CannotCastReason::kCustomReasonNoStart;
                }
                logger::info("[TOAM][S05C4] Other-hand CheckCast blocked");
                return false;
            }

            // CheckCast runs during MagicMenu spell enumeration.
            // The admission gate belongs to RequestCast and Native Begin.
            const auto original =
                reinterpret_cast<CheckCastFn>(g_originalCheckCast);

            return original ?
                Arcana::CheckFromHost(original, caster, spell, dual, strength, reason, baseCost) :
                false;
        }

        void HookedRequestCast(RE::ActorMagicCaster* caster)
        {
            if (DenyOtherHandMagic(
                caster, caster ? caster->currentSpell : nullptr)) {
                logger::info("[TOAM][S05C4] Other-hand RequestCast blocked");
                return;
            }

            if (!AllowsNewOOMCast(
                    caster, caster ? caster->currentSpell : nullptr)) {
                logger::info("[TOAM][R1C2C] RequestCast blocked: below T1 MP");
                return;
            }

            const auto original =
                reinterpret_cast<RequestCastFn>(g_originalRequestCast);

            if (original) {
                Arcana::RequestFromHost(caster);
            }
        }

        bool InstallMagicCasterHooks() noexcept
        {
            if (g_checkCastHookInstalled &&
                g_requestCastHookInstalled) {
                return true;
            }

            try {
                REL::Relocation<std::uintptr_t> vtable{
                    RE::ActorMagicCaster::VTABLE[0]
                };

                if (!g_requestCastHookInstalled) {
                    const auto original =
                        vtable.write_vfunc(0x03, HookedRequestCast);
                    if (!original) {
                        return false;
                    }
                    g_originalRequestCast = original;
                    Arcana::BindHostRequest(reinterpret_cast<RequestCastFn>(original));
                    g_requestCastHookInstalled = true;
                }

                if (!g_checkCastHookInstalled) {
                    const auto original =
                        vtable.write_vfunc(0x0A, HookedCheckCast);
                    if (!original) {
                        return false;
                    }
                    g_originalCheckCast = original;
                    g_checkCastHookInstalled = true;
                }
            } catch (...) {
                logger::error("[TOAM][S05C4] Cast hook installation threw");
                return false;
            }

            logger::info(
                "[TOAM][S05C4] Cast hooks installed | Request={} Check={}",
                g_requestCastHookInstalled, g_checkCastHookInstalled);

            return g_requestCastHookInstalled &&
                g_checkCastHookInstalled;
        }

        void Begin(
            RE::PlayerCharacter* player,
            const Hand hand) noexcept
        {
            auto& state = GetState();

            const auto index =
                HandIndex(hand);

            if (
                state.handOwner[index] !=
                Owner::kNone
            ) {
                return;
            }

            const auto owner =
                EquippedOwner(
                    player,
                    hand
                );

            if (owner == Owner::kNone) {
                return;
            }

            auto& session =
                Session(owner);

            const bool wasActive =
                session.IsActive();

            if (!wasActive) {
                SessionMetrics(owner) = {};
            }

            // Preserve the first hand's snapshot for a shared two-hand session.
            const float costMultiplier = wasActive ?
                session.CostMultiplier() :
                TOAM::Spells::ArcanaLimiter::Runtime::
                    SnapshotCostMultiplier(player);

            if (!wasActive &&
                !HasFirstTierMagicka(player, owner, costMultiplier)) {
                logger::info(
                    "[TOAM][R1C2C] Native Begin blocked | Profile={} | T1={:.1f} | Mult={:.3f}",
                    OwnerName(owner),
                    session.Profile().thresholds[0],
                    costMultiplier);
                return;
            }

            if (!session.BeginHand(hand, costMultiplier)) {
                return;
            }

            state.handOwner[index] =
                owner;

            if (owner == Owner::kJudgement && !wasActive) {
                TOAM::Spells::Judgement::Runtime::BeginChargeGuide();
            }

            if (
                owner ==
                    Owner::kMagician &&
                !wasActive &&
                !TOAM::Spells::Magician::Runtime::
                    BeginChargeVisual(
                        player
                    )
            ) {
                logger::warn(
                    "[TOAM][S05C2] CHARGE_VISUAL_BEGIN_FAILED");
            }

            logger::info(
                "[TOAM][S02D] BEGIN | Profile={} | Hand={} | Hands={} | Charge={:.3f} | Tier={} | CostMult={:.3f}",
                OwnerName(owner),
                HandName(hand),
                session.HeldHandCount(),
                session.AccumulatedCharge(),
                session.Tier(),
                session.CostMultiplier());
        }

        void FinalReleaseLog(
            RE::PlayerCharacter* player,
            const Owner owner,
            const ReleaseResult& release) noexcept
        {
            float magicka{};

            if (player) {
                if (
                    auto* av =
                        player->AsActorValueOwner()
                ) {
                    magicka =
                        av->GetActorValue(
                            RE::ActorValue::kMagicka
                        );
                }
            }

            const auto& metrics =
                SessionMetrics(owner);

            logger::info(
                "[TOAM][S02D] RELEASE | Profile={} | Charge={:.3f} | Tier={} | ShouldFire={} | RequestedTotal={:.3f} | AllocatedTotal={:.3f} | HandSeconds={:.4f} | Magicka={:.3f} | CostMult={:.3f}",
                OwnerName(owner),
                release.accumulatedCharge,
                release.tier,
                release.shouldFire,
                metrics.requested,
                metrics.allocated,
                metrics.handSeconds,
                magicka,
                release.costMultiplier);
        }

        void Release(
            RE::PlayerCharacter* player,
            const Hand hand,
            const bool cancelled) noexcept
        {
            auto& state = GetState();

            const auto index =
                HandIndex(hand);

            const auto owner =
                state.handOwner[index];

            if (owner == Owner::kNone) {
                return;
            }

            auto& session =
                Session(owner);

            const auto result =
                session.ReleaseHand(hand);

            state.handOwner[index] =
                Owner::kNone;

            if (owner == Owner::kJudgement &&
                (result.finalRelease || !result.wasHeld)) {
                // Remove the no-damage guide before firing the 1-damage mark.
                TOAM::Spells::Judgement::Runtime::ReleaseChargeGuide();
            }

            if (!result.wasHeld) {

                logger::warn(
                    "[TOAM][S02D] RELEASE_STATE_MISMATCH | Profile={} | Hand={}",
                    OwnerName(owner),
                    HandName(hand));

                return;
            }

            if (cancelled) {

                logger::info(
                    "[TOAM][S02D] CANCEL_EQUIP_CHANGE | Profile={} | Hand={} | Final={} | Charge={:.3f} | Tier={}",
                    OwnerName(owner),
                    HandName(hand),
                    result.finalRelease,
                    result.accumulatedCharge,
                    result.tier);

                if (result.finalRelease) {

                    if (
                        owner ==
                            Owner::kMagician
                    ) {
                        TOAM::Spells::Magician::Runtime::
                            ReleaseChargeVisual();
                    }

                    session.Reset();
                    SessionMetrics(owner) = {};
                }

                return;
            }

            if (!result.finalRelease) {

                logger::info(
                    "[TOAM][S02D] RELEASE_PARTIAL | Profile={} | Hand={} | RemainingHands={} | Charge={:.3f} | Tier={}",
                    OwnerName(owner),
                    HandName(hand),
                    session.HeldHandCount(),
                    result.accumulatedCharge,
                    result.tier);

                return;
            }

            FinalReleaseLog(
                player,
                owner,
                result
            );

            if (
                owner ==
                    Owner::kMagician &&
                result.shouldFire
            ) {

                const auto fireResult =
                    TOAM::Spells::Magician::Runtime::
                        ExecuteFinalRelease(
                            player,
                            result.accumulatedCharge,
                            result.costMultiplier
                        );

                logger::info(
                    "[TOAM][S05C1] FINAL_RELEASE | Executed={} | Aim={} | FirePlan={} | Collision={} | Tier={} | Charge={:.3f} | Intersections={} | Damaged={}",
                    fireResult.executed,
                    fireResult.aimResolved,
                    fireResult.firePlanResolved,
                    fireResult.collisionResolved,
                    fireResult.tier,
                    fireResult.effectiveCharge,
                    fireResult.actorIntersections,
                    fireResult.actorsDamaged);
            }

            if (
                owner ==
                    Owner::kJudgement &&
                result.shouldFire
            ) {

                const auto fireResult =
                    TOAM::Spells::Judgement::Runtime::
                        ExecuteFinalRelease(
                            player,
                            result.accumulatedCharge,
                            result.costMultiplier
                        );

                logger::info(
                    "[TOAM][S06] CHARGE_FINAL_RELEASE | Executed={} | Attempted={} | Aim={} | MarkCollision={} | Target={} | MarkDamage={} | Formation={} | Schedule={} | Stage={} | Charge={:.3f} | MarkHits={}",
                    fireResult.executed,
                    fireResult.attempted,
                    fireResult.aimResolved,
                    fireResult.markingCollisionResolved,
                    fireResult.targetAcquired,
                    fireResult.markingDamageApplied,
                    fireResult.formationResolved,
                    fireResult.scheduleStarted,
                    fireResult.stage,
                    fireResult.effectiveCharge,
                    fireResult.markingActorIntersections);
            }

            if (
                owner ==
                    Owner::kMagician
            ) {
                TOAM::Spells::Magician::Runtime::
                    ReleaseChargeVisual();
            }

            SessionMetrics(owner) = {};
        }

        void ReconcileEquipment(
            RE::PlayerCharacter* player) noexcept
        {
            auto& state = GetState();

            constexpr std::array<Hand, 2>
                hands{
                    Hand::kLeft,
                    Hand::kRight
                };

            for (const auto hand : hands) {

                const auto owner =
                    state.handOwner[
                        HandIndex(hand)
                    ];

                if (owner == Owner::kNone) {
                    continue;
                }

                if (
                    EquippedOwner(
                        player,
                        hand
                    ) == owner
                ) {
                    continue;
                }

                Release(
                    player,
                    hand,
                    true
                );
            }
        }

        [[nodiscard]]
        float AllocatedTo(
            const CommitResult& result,
            const ChargeSession* session) noexcept
        {
            for (
                std::size_t i = 0;
                i < result.plan.count;
                ++i
            ) {
                if (
                    result.plan.entries[i].session ==
                    session
                ) {
                    return
                        result.plan.entries[i].allocated;
                }
            }

            return 0.0F;
        }

        void Update(
            RE::PlayerCharacter* player) noexcept
        {
            if (
                !player ||
                !AnyActive()
            ) {
                return;
            }

            const float dt =
                RE::Main::QFrameAnimTime();

            if (
                !std::isfinite(dt) ||
                dt <= 0.0F
            ) {
                return;
            }

            auto& state =
                GetState();

            std::array<DrainRequest, 2>
                requests{};

            std::size_t count{};

            const auto collect =
                [
                    &requests,
                    &count,
                    dt
                ](
                    ChargeSession& session,
                    Metrics& metrics
                )
            {
                if (!session.IsActive()) {
                    return;
                }

                metrics.handSeconds +=
                    static_cast<double>(
                        session.HeldHandCount()
                    ) *
                    static_cast<double>(dt);

                const float requested =
                    session.RequestedDrain(dt);

                if (requested <= 0.0F) {
                    return;
                }

                metrics.requested +=
                    static_cast<double>(
                        requested
                    );

                requests[count++] =
                    DrainRequest{
                        &session,
                        requested
                    };
            };

            collect(
                state.magician,
                state.magicianMetrics
            );

            collect(
                state.judgement,
                state.judgementMetrics
            );

            if (count == 0) {

                if (state.magician.IsActive()) {
                    static_cast<void>(
                        TOAM::Spells::Magician::Runtime::
                            UpdateChargeVisual(
                                player,
                                state.magician.
                                    AccumulatedCharge(),
                                dt
                            )
                    );
                }

                return;
            }

            const auto magicianTierBefore =
                state.magician.Tier();

            const auto judgementTierBefore =
                state.judgement.Tier();

            const auto result =
                CommitChargeDrain(
                    player,
                    std::span<
                        const DrainRequest>{
                        requests.data(),
                        count
                    }
                );

            const float magicianAllocated =
                AllocatedTo(
                    result,
                    &state.magician
                );

            const float judgementAllocated =
                AllocatedTo(
                    result,
                    &state.judgement
                );

            if (result.committed) {

                state.magicianMetrics.allocated +=
                    static_cast<double>(
                        magicianAllocated
                    );

                state.judgementMetrics.allocated +=
                    static_cast<double>(
                        judgementAllocated
                    );
            }

            const bool emptyNow =
                result.plan.totalRequested > 0.0F &&
                result.plan.totalAllocated <= 0.0F &&
                result.magickaBefore <= 0.0F;

            if (
                emptyNow &&
                !state.magickaEmpty
            ) {
                logger::info(
                    "[TOAM][S02D] MP_EMPTY_HOLD | Requested={:.3f}",
                    result.plan.totalRequested);
            }
            else if (
                !emptyNow &&
                state.magickaEmpty &&
                result.plan.totalAllocated > 0.0F
            ) {
                logger::info(
                    "[TOAM][S02D] MP_RESUME | Requested={:.3f} | Allocated={:.3f} | MagickaBefore={:.3f}",
                    result.plan.totalRequested,
                    result.plan.totalAllocated,
                    result.magickaBefore);
            }

            state.magickaEmpty =
                emptyNow;

            if (
                result.plan.constrained &&
                !state.constrained
            ) {
                logger::info(
                    "[TOAM][S02D] ARBITER_CONSTRAINED | Available={:.3f} | Requested={:.3f} | Allocated={:.3f} | Magician={:.3f} | Judgement={:.3f}",
                    result.plan.available,
                    result.plan.totalRequested,
                    result.plan.totalAllocated,
                    magicianAllocated,
                    judgementAllocated);
            }

            state.constrained =
                result.plan.constrained;

            const auto magicianTierAfter =
                state.magician.Tier();

            const auto judgementTierAfter =
                state.judgement.Tier();

            if (
                magicianTierAfter !=
                magicianTierBefore
            ) {
                logger::info(
                    "[TOAM][S02D] TIER | Profile=Magician | {}->{} | Charge={:.3f}",
                    magicianTierBefore,
                    magicianTierAfter,
                    state.magician.AccumulatedCharge());
            }

            if (
                judgementTierAfter !=
                judgementTierBefore
            ) {
                logger::info(
                    "[TOAM][S02D] TIER | Profile=Judgement | {}->{} | Charge={:.3f}",
                    judgementTierBefore,
                    judgementTierAfter,
                    state.judgement.AccumulatedCharge());
            }


            if (state.magician.IsActive()) {
                static_cast<void>(
                    TOAM::Spells::Magician::Runtime::
                        UpdateChargeVisual(
                            player,
                            state.magician.
                                AccumulatedCharge(),
                            dt
                        )
                );
            }
        }

        class InputSink final :
            public RE::BSTEventSink<
                RE::InputEvent*>
        {
        public:
            RE::BSEventNotifyControl
            ProcessEvent(
                RE::InputEvent* const* events,
                RE::BSTEventSource<
                    RE::InputEvent*>*) override
            {
                if (
                    !TOAM::Core::Runtime::IsGameReady()
                ) {
                    return
                        RE::BSEventNotifyControl::kContinue;
                }

                auto* player =
                    RE::PlayerCharacter::GetSingleton();

                if (
                    !player ||
                    !TOAM::Core::DataCache::IsReady()
                ) {
                    return
                        RE::BSEventNotifyControl::kContinue;
                }


                auto* controls =
                    RE::ControlMap::GetSingleton();

                if (
                    !controls ||
                    !controls->
                        IsFightingControlsEnabled()
                ) {
                    if (
                        AnyActive() ||
                        TOAM::Spells::Judgement::Runtime::
                            HasActiveArtillery() ||
                        TOAM::Spells::World::Runtime::
                            IsCharging()
                    ) {
                        ResetTransientState();
                    }

                    return
                        RE::BSEventNotifyControl::kContinue;
                }

                // BSInputDeviceManager::PollInputDevices() is called by
                // Main::Update() in the pinned CommonLib authority.
                //
                // Keep Judgement artillery progression independent from
                // active charge-session state so firing continues after
                // the final hand has already been released.
                if (
                    TOAM::Spells::Judgement::Runtime::
                        HasActiveArtillery()
                ) {
                    const float artilleryDelta =
                        RE::Main::
                            QFrameAnimTime();


                    if (
                        std::isfinite(
                            artilleryDelta
                        ) &&
                        artilleryDelta >
                            0.0F
                    ) {
                        TOAM::Spells::Judgement::Runtime::
                            UpdateArtillery(
                                player,
                                artilleryDelta
                            );
                    }
                }


                if (
                    TOAM::Spells::World::Runtime::
                        IsCharging()
                ) {
                    const float worldDelta =
                        RE::Main::
                            QFrameAnimTime();

                    if (
                        std::isfinite(
                            worldDelta
                        ) &&
                        worldDelta >
                            0.0F
                    ) {
                        TOAM::Spells::World::Runtime::
                            UpdateCharge(
                                player,
                                worldDelta
                            );
                    }
                }

                static_cast<void>(
                    TOAM::Spells::World::Runtime::
                        ProcessPendingRelease(
                            player
                        )
                );

                ReconcileEquipment(player);

                const auto* userEvents =
                    RE::UserEvents::GetSingleton();

                // Independent input-side witness for the World Shout/Power key.
                // Diagnostic only; never consumes or re-routes the event.
                if (events && *events && userEvents) {
                    static std::uint32_t shoutTraceCount{};
                    for (auto* trace = *events; trace; trace = trace->next) {
                        auto* button = trace->AsButtonEvent();
                        if (!button || button->GetUserEvent() != userEvents->shout ||
                            (!button->IsDown() && !button->IsUp())) {
                            continue;
                        }
                        if (shoutTraceCount < 32) {
                            ++shoutTraceCount;
                            logger::info(
                                "[TOAM][S07TRACE] INPUT_SINK | Down={} | Up={} | Selected={}",
                                button->IsDown(),
                                button->IsUp(),
                                TOAM::Spells::World::Runtime::IsWorldSelected(player));
                        }
                    }
                }

                #pragma region TOAM_S02D_BEGIN_PHASE

                if (
                    events &&
                    *events &&
                    userEvents
                ) {
                    for (
                        auto* event = *events;
                        event;
                        event = event->next
                    ) {
                        auto* button =
                            event->AsButtonEvent();

                        if (
                            !button ||
                            !button->IsDown()
                        ) {
                            continue;
                        }

                        const auto& name =
                            button->GetUserEvent();

                        if (
                            name ==
                            userEvents->leftAttack
                        ) {
                            Begin(
                                player,
                                Hand::kLeft
                            );
                        }
                        else if (
                            name ==
                            userEvents->rightAttack
                        ) {
                            Begin(
                                player,
                                Hand::kRight
                            );
                        }
                    }
                }

                #pragma endregion

                // Exactly one arbitration update per input dispatch.
                // DOWN is processed before this update.
                // UP is processed after this update.
                Update(player);

                #pragma region TOAM_S02D_RELEASE_PHASE

                if (
                    events &&
                    *events &&
                    userEvents
                ) {
                    for (
                        auto* event = *events;
                        event;
                        event = event->next
                    ) {
                        auto* button =
                            event->AsButtonEvent();

                        if (
                            !button ||
                            button->IsPressed()
                        ) {
                            continue;
                        }

                        const auto& name =
                            button->GetUserEvent();

                        if (
                            name ==
                            userEvents->leftAttack
                        ) {
                            Release(
                                player,
                                Hand::kLeft,
                                false
                            );
                        }
                        else if (
                            name ==
                            userEvents->rightAttack
                        ) {
                            Release(
                                player,
                                Hand::kRight,
                                false
                            );
                        }
                    }
                }

                #pragma endregion

                return
                    RE::BSEventNotifyControl::kContinue;
            }
        };

        [[nodiscard]]
        InputSink& GetSink() noexcept
        {
            static InputSink sink{};
            return sink;
        }
    }

    void Install() noexcept
    {
        auto& state =
            GetState();

        if (state.installed) {
            return;
        }

        auto* input =
            RE::BSInputDeviceManager::GetSingleton();

        if (!input) {
            logger::error(
                "[TOAM][S02D] BSInputDeviceManager unavailable");
            return;
        }

        if (!InstallMagicCasterHooks()) {
            logger::error(
                "[TOAM][S05C4] Cast hook installation incomplete");
        }

        input->AddEventSink(
            &GetSink()
        );

        state.installed = true;

        logger::info(
            "[TOAM][S02D] Input sink installed");
    }

    void ResetTransientState() noexcept
    {
        TOAM::Audio::Reset();
        auto& state =
            GetState();

        const bool hadActive =
            state.magician.IsActive() ||
            state.judgement.IsActive() ||
            TOAM::Spells::Judgement::Runtime::
                HasActiveArtillery() ||
            TOAM::Spells::World::Runtime::
                IsCharging();

        state.magician.Reset();
        state.judgement.Reset();

        state.handOwner = {
            Owner::kNone,
            Owner::kNone
        };

        state.magicianMetrics = {};
        state.judgementMetrics = {};

        state.magickaEmpty = false;
        state.constrained = false;

        TOAM::Spells::Magician::Runtime::
            ResetVisualState();

        TOAM::Spells::Judgement::Runtime::
            ResetTransientState();

        TOAM::Spells::World::Runtime::
            ResetTransientState();

        if (hadActive) {
            logger::info(
                "[TOAM][S02D] Transient charge state reset");
        }
    }

    bool IsJudgementChargeActive() noexcept
    {
        return GetState().judgement.IsActive();
    }

    bool IsJudgementHandHeld(bool leftHand) noexcept
    {
        return GetState().judgement.IsHeld(leftHand ? Hand::kLeft : Hand::kRight);
    }

    bool IsInstalled() noexcept
    {
        return GetState().installed;
    }
}