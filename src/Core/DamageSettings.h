#pragma once
namespace TOAM::Core::DamageSettings {
// One damage-only setting. Never a resource, tier, cadence, limiter or defense input.
[[nodiscard]] float GetMultiplier() noexcept;
[[nodiscard]] bool Reload() noexcept;
[[nodiscard]] bool Save(float value) noexcept;
void RegisterMenu() noexcept;
}
