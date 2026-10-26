#pragma once
#include <array>
#include <cmath>
#include <cstddef>
namespace TOAM::VFX::CircleMotion {
// Presentation only. Local+Y is the normal; spin leaves muzzle positions/normals intact.
inline constexpr std::array<float,8> layerRates{.78F,-.57F,-1.03F,.66F,-.88F,1.17F,.94F,-.71F};
inline float LayerSpeed(std::size_t i,float tier) noexcept {
    if(i>=layerRates.size()||!std::isfinite(tier))return 0;
    const float t=tier<1?1:(tier>8?8:tier);
    return layerRates[i]*(1.F+.20F*(t-1.F));
}
inline float RevealSpeed(float age,float steady) noexcept {
    return steady+(steady<0?-1.F:1.F)*2.8F*std::exp(-(age>0?age:0)/.18F);
}
inline float Advance(float phase,float rate,float dt) noexcept {
    if(!std::isfinite(phase)||!std::isfinite(rate)||!std::isfinite(dt)||dt<=0)return phase;
    return std::remainder(phase+rate*(dt>.1F?.1F:dt),6.28318530718F);
}
}
