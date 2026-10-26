#pragma once

namespace TOAM::Spells::Temperance::Runtime
{
    void Install() noexcept;
    void ResetAllState() noexcept;

    [[nodiscard]] bool IsInstalled() noexcept;
    [[nodiscard]] bool IsEnabled() noexcept;
    [[nodiscard]] bool HasPending() noexcept;

    void UpdatePending(float deltaSeconds) noexcept;
    void UpdateHalo() noexcept;
}