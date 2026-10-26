#pragma once

#include "RE/Skyrim.h"

#include <cstddef>
#include <cstdint>

namespace TOAM::Spells::World::Runtime
{
    enum class InputDisposition :
        std::uint8_t
    {
        kForwardVanilla = 0,
        kConsume
    };

    struct ReleaseState final
    {
        bool wasCharging{};
        bool shouldFire{};

        std::size_t stage{};
        std::size_t circleCount{};

        float accumulatedCharge{};
    };

    [[nodiscard]]
    bool IsWorldSelected(
        RE::PlayerCharacter* player) noexcept;

    [[nodiscard]]
    bool IsCharging() noexcept;

    [[nodiscard]]
    bool BeginCharge(
        RE::PlayerCharacter* player) noexcept;

    void UpdateCharge(
        RE::PlayerCharacter* player,
        float deltaSeconds) noexcept;

    [[nodiscard]]
    ReleaseState ProcessPendingRelease(
        RE::PlayerCharacter* player) noexcept;

    [[nodiscard]]
    ReleaseState ReleaseCharge(
        RE::PlayerCharacter* player) noexcept;

    void CancelCharge() noexcept;

    [[nodiscard]]
    InputDisposition HandleShoutButton(
        RE::PlayerCharacter* player,
        RE::ButtonEvent* event) noexcept;

    [[nodiscard]]
    bool InstallInputHook() noexcept;

    [[nodiscard]]
    bool IsInputHookInstalled() noexcept;

    void ResetTransientState() noexcept;
}