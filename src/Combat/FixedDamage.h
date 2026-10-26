#pragma once

#include "RE/Skyrim.h"
#include "Combat/TriggerFirewall.h"

namespace TOAM::Combat
{
    [[nodiscard]] bool ApplyFixedHealthDamage(
        RE::Actor* source,
        RE::Actor* target,
        float damage,
        FirewallOrigin origin) noexcept;
}