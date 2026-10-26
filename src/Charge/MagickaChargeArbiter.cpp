#include "Charge/MagickaChargeArbiter.h"

#include "Core/MagickaTransaction.h"
#include "RE/Skyrim.h"

#include <cmath>

namespace TOAM::Charge
{
    CommitResult CommitChargeDrain(
        RE::Actor* actor,
        const std::span<const DrainRequest> requests) noexcept
    {
        CommitResult result{};

        if (!actor) {
            return result;
        }

        auto* actorValueOwner =
            actor->AsActorValueOwner();

        if (!actorValueOwner) {
            return result;
        }

        const float rawMagicka =
            actorValueOwner->GetActorValue(
                RE::ActorValue::kMagicka
            );

        float available = 0.0F;

        if (
            std::isfinite(rawMagicka) &&
            rawMagicka > 0.0F
        ) {
            available = rawMagicka;
        }

        result.magickaBefore =
            available;

        result.magickaAfter =
            available;

        result.plan =
            ResolveProportional(
                requests,
                available
            );

        if (result.plan.truncated) {
            return result;
        }

        if (result.plan.totalAllocated <= 0.0F) {
            return result;
        }

        const auto spend =
            TOAM::Core::TrySpendMagicka(
                actor,
                result.plan.totalAllocated
            );

        result.magickaBefore =
            spend.before;

        result.magickaAfter =
            spend.after;

        if (!spend.paid) {
            return result;
        }

        result.committed = true;

        double totalApplied = 0.0;

        for (
            std::size_t i = 0;
            i < result.plan.count;
            ++i
        ) {
            auto& entry =
                result.plan.entries[i];

            if (!entry.session) {
                continue;
            }

            if (entry.allocated <= 0.0F) {
                continue;
            }

            entry.session->
                ApplyConsumedMagicka(
                    entry.allocated
                );

            totalApplied +=
                static_cast<double>(
                    entry.allocated
                );
        }

        result.totalAppliedToSessions =
            static_cast<float>(
                totalApplied
            );

        return result;
    }
}