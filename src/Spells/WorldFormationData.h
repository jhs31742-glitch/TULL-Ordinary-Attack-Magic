#pragma once
#include <array>
#include <cstddef>
namespace TOAM::Spells::World::FormationData
{
    struct Direction {float right;float forward;float up;};
    // R11 authorized D bank. Gameplay radius/center/length are deliberately separate.
    inline constexpr std::array<Direction,12> kDirections{{
        {0.000000000F, 0.000000000F, 1.000000000F},
        {0.000000000F, 0.573576436F, 0.819152044F},
        {0.573576436F, 0.000000000F, 0.819152044F},
        {0.000000000F, -0.573576436F, 0.819152044F},
        {-0.573576436F, -0.000000000F, 0.819152044F},
        {0.000000000F, 0.939692621F, 0.342020143F},
        {0.734681275F, 0.585888766F, 0.342020143F},
        {0.916132565F, -0.209101280F, 0.342020143F},
        {0.407717348F, -0.846633797F, 0.342020143F},
        {-0.407717348F, -0.846633797F, 0.342020143F},
        {-0.916132565F, -0.209101280F, 0.342020143F},
        {-0.734681275F, 0.585888766F, 0.342020143F}
    }};
    inline constexpr std::array<std::size_t,4> kStage1{0,5,7,10};
    inline constexpr std::array<std::size_t,8> kStage2{0,1,2,3,4,5,7,10};
    constexpr std::size_t ActiveSlot(std::size_t stage,std::size_t index) noexcept
    {return stage==1 ? kStage1[index] : (stage==2 ? kStage2[index] : index);}
}
