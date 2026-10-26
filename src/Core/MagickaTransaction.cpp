#include "Core/MagickaTransaction.h"

#include <cmath>

namespace TOAM::Core
{
    MagickaSpendResult TrySpendMagicka(
        RE::Actor* actor,
        float amount) noexcept
    {
        MagickaSpendResult result{};
        result.requested = amount;

        if (!actor ||
            !std::isfinite(amount) ||
            amount < 0.0F) {
            return result;
        }

        auto* av = actor->AsActorValueOwner();
        if (!av) {
            return result;
        }

        result.before =
            av->GetActorValue(RE::ActorValue::kMagicka);

        if (amount == 0.0F) {
            result.after = result.before;
            result.paid = true;
            return result;
        }

        if (result.before < amount) {
            result.after = result.before;
            return result;
        }

        av->DamageActorValue(
            RE::ActorValue::kMagicka,
            amount);

        result.after =
            av->GetActorValue(RE::ActorValue::kMagicka);

        result.paid = true;
        return result;
    }
}