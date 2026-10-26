#pragma once

namespace TOAM::Charge::Runtime
{
    [[nodiscard]] bool IsJudgementChargeActive() noexcept;
    [[nodiscard]] bool IsJudgementHandHeld(bool leftHand) noexcept;
    void Install() noexcept;
    void ResetTransientState() noexcept;

    [[nodiscard]] bool IsInstalled() noexcept;
}