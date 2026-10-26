#pragma once
#include <array>
#include <cmath>
namespace TOAM::Core::FlightMenuPolicy {
inline constexpr std::array<const wchar_t*,12> keys{L"Enabled",L"Speed",L"BoostSpeed",L"VerticalSpeed",L"TakeoffSpeed",L"TakeoffSeconds",L"Response",L"RangedLegPose",L"BlockMeleeWhileFlying",L"GunAnimations",L"HumEnabled",L"HumVolume"};
inline constexpr std::array<float,12> defaults{1,300,650,220,160,.55F,7,1,1,1,1,.35F};
inline constexpr std::array<float,12> minimum{0,60,60,40,60,.2F,2,0,0,0,0,0};
inline constexpr std::array<float,12> maximum{1,900,1200,600,320,1.2F,16,1,1,1,1,1};
inline bool Boolean(std::size_t i) noexcept {return i==0||i==7||i==8||i==9||i==10;}
inline bool Valid(const std::array<float,12>& v) noexcept {
 for(std::size_t i=0;i<v.size();++i)if(!std::isfinite(v[i])||v[i]<minimum[i]||v[i]>maximum[i]||(Boolean(i)&&v[i]!=0&&v[i]!=1))return false;
 return true;
}
}
