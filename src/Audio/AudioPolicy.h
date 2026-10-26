#pragma once
#include <array>
#include <cstddef>
namespace TOAM::Audio::Policy
{
    inline constexpr std::array<float,8> kMagicianGain{.85F,.90F,.95F,1.0F,.95F,1.0F,1.0F,1.0F};
    inline constexpr std::array<float,3> kWorldGain{.85F,.95F,1.0F};
    constexpr bool JudgementAudibleShot(std::size_t zeroBasedShot) noexcept
    {return zeroBasedShot%3==0;}
    inline constexpr std::size_t kJudgementVoices=3;
    inline constexpr std::size_t kTemperanceVoices=2;
}
