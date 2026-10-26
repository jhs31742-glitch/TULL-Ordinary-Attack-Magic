#pragma once
#include <cmath>
#include <cstddef>
namespace TOAM::VFX::Presentation
{
    inline float GrowthFactor(float elapsed,float duration) noexcept
    {
        if(duration<=0.0F) {return 1.0F;}
        const float u=elapsed<=0.0F ? 0.0F : (elapsed<duration ? elapsed/duration : 1.0F);
        return 0.02F+0.98F*u*u*u*(u*(u*6.0F-15.0F)+10.0F);
    }
    inline std::size_t InstanceCount(float length,bool wholeLens) noexcept
    {
        if(!std::isfinite(length) || length<=0.0F || length>16384.0F) {return 0;}
        return wholeLens ? 1 : static_cast<std::size_t>(std::ceil(length/512.0F));
    }
}
