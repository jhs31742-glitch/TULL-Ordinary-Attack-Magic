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

        if (!source) {
            return false;
        }

        // Preserve the resolved amount while letting the engine record
        // the attacker. The final flag skips difficulty adjustment.
        const bool engineResult = target->DoDamage(damage, source, true);

        if (auto* values = target->AsActorValueOwner();
            values && values->GetActorValue(RE::ActorValue::kHealth) <= 0.0F) {
            auto* killer = target->GetKiller();
            logger::info(
                "[TOAM-DAMAGE-CREDIT] Source={:08X} Target={:08X} Requested={:.3f} EngineResult={} Killer={:08X}",
                source->GetFormID(), target->GetFormID(),
                damage, engineResult, killer ? killer->GetFormID() : 0);
        }

        // Preserve the existing caller contract: valid damage was submitted.
        return true;
    }
}