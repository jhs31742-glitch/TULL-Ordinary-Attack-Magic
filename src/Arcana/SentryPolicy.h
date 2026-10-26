#pragma once
#include <array>
#include <cstdint>
#include <cmath>
#include <algorithm>
namespace Arcana {
inline constexpr int sentryLimit=3;
inline constexpr float sentrySeconds=3.f,sentryRadius=1200.f,sentryInterval=.20f,sentryScanSeconds=.10f;
inline constexpr float sentryTurnRadians=3.14159265359f,sentryAimResponse=12.f,sentryDamageAlignment=.995f;
struct SentryLife {bool active=false;std::uint64_t serial=0;float age=0;};
inline int SentryReplacement(const std::array<SentryLife,3>& life){for(int i=0;i<3;i++)if(!life[i].active)return i;int oldest=0;for(int i=1;i<3;i++)if(life[i].serial<life[oldest].serial)oldest=i;return oldest;}
inline bool SentryExpired(float age){return age>=sentrySeconds;}
inline float SentryTurnAngle(float from,float to,float dt){
 if(!std::isfinite(from)||!std::isfinite(to)||!std::isfinite(dt)||dt<=0)return from;
 dt=std::min(dt,.1f);float delta=std::remainder(to-from,6.28318530718f);
 float step=delta*(1-std::exp(-sentryAimResponse*dt));step=std::clamp(step,-sentryTurnRadians*dt,sentryTurnRadians*dt);
 return std::remainder(from+step,6.28318530718f);
}
inline bool SentryDamageReady(float alignment,bool clear,bool modelReady){return std::isfinite(alignment)&&alignment>=sentryDamageAlignment&&clear&&modelReady;}
}
