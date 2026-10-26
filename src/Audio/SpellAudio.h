#pragma once
#include "RE/Skyrim.h"
#include <cstddef>

namespace TOAM::Audio
{
    // Event-only audio ownership. No frame sink or gameplay decisions.
    void MagicianFinal(std::size_t tier, const RE::NiPoint3& origin) noexcept;
    void WorldFinal(std::size_t stage, const RE::NiPoint3& origin) noexcept;
    void JudgementShot(std::size_t shotIndex, const RE::NiPoint3& aim) noexcept;
    void TemperanceProc(const RE::NiPoint3& committedCenter) noexcept;
    void Reset() noexcept;
}
