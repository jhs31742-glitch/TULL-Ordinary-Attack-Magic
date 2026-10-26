#pragma once

namespace TOAM::VFX::Runtime
{
    void Install() noexcept;

    void ResetTracking() noexcept;

    [[nodiscard]]
    bool EnsureCoreAssetsReady() noexcept;

    [[nodiscard]]
    bool AreCoreAssetsReady() noexcept;

    [[nodiscard]]
    bool IsInstalled() noexcept;
}