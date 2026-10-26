#include "Combat/FixedDamage.h"

#include <cmath>

namespace TOAM::Combat
{
    bool ApplyFixedHealthDamage(
        RE::Actor* source,
        RE::Actor* target,
        float damage,
        FirewallOrigin origin) noexcept
    {
        if (!target ||
            !std::isfinite(damage) ||
            damage <= 0.0F ||
            IsTriggerSuppressed()) {
            return false;
        }

        ScopedTriggerFirewall firewall{ origin };

        (void)source;

        auto* av = target->AsActorValueOwner();
        if (!av) {
            return false;
        }

        av->ModActorValue(
            RE::ACTOR_VALUE_MODIFIERS::kDamage,
            RE::ActorValue::kHealth,
            -damage);

        return true;
    }
}