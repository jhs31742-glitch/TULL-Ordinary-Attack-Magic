#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string_view>
namespace Arcana {
enum class FlightHandKind { Bare, Melee, Ranged, Staff, Spell, Other };
enum class FlightAttackSide { Left, Right, Dual, None };
struct FlightCombatSettings { bool rangedLegPose=true,blockMelee=true; };
struct FlightQuat { float w=1,x=0,y=0,z=0; };
struct FlightLegKey { float time; FlightQuat q; };
inline bool FlightMeleeKind(FlightHandKind k){return k==FlightHandKind::Bare||k==FlightHandKind::Melee;}
inline bool FlightMeleeButton(FlightHandKind left,FlightHandKind right,FlightAttackSide side){
 if(left==FlightHandKind::Ranged||right==FlightHandKind::Ranged)return false; // Keep ranged aim/zoom/release input.
 if(side==FlightAttackSide::Left)return FlightMeleeKind(left);
 if(side==FlightAttackSide::Right)return FlightMeleeKind(right);
 return side==FlightAttackSide::Dual&&(FlightMeleeKind(left)||FlightMeleeKind(right));
}
inline char FlightLower(char c){return c>='A'&&c<='Z'?char(c-'A'+'a'):c;}
inline bool FlightStarts(std::string_view s,std::string_view prefix){
 if(s.size()<prefix.size())return false;
 for(std::size_t i=0;i<prefix.size();++i)if(FlightLower(s[i])!=prefix[i])return false;return true;
}
inline bool FlightContains(std::string_view s,std::string_view part){
 for(std::size_t i=0;i+part.size()<=s.size();++i)if(FlightStarts(s.substr(i),part))return true;return false;
}
inline bool FlightMeleeGraphEvent(std::string_view tag,FlightHandKind left,FlightHandKind right){
 if(FlightStarts(tag,"bashstart"))return true;
 if(!FlightStarts(tag,"attackstart")&&!FlightStarts(tag,"attackpowerstart")&&!FlightStarts(tag,"attackrelease")&&
    !FlightStarts(tag,"mco_nextattack")&&!FlightStarts(tag,"mco_nextpowerattack"))return false;
 FlightAttackSide side=FlightAttackSide::Right;
 if(FlightContains(tag,"dual"))side=FlightAttackSide::Dual;
 else if(FlightContains(tag,"lefthand")||FlightContains(tag,"h2hleft"))side=FlightAttackSide::Left;
 return FlightMeleeButton(left,right,side);
}
inline FlightQuat FlightNormalize(FlightQuat q){
 float n=std::sqrt(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
 if(!std::isfinite(n)||n<.0001f)return {};return {q.w/n,q.x/n,q.y/n,q.z/n};
}
inline FlightQuat FlightNlerp(FlightQuat a,FlightQuat b,float t){
 a=FlightNormalize(a);b=FlightNormalize(b);t=std::clamp(t,0.f,1.f);
 if(a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z<0)b={-b.w,-b.x,-b.y,-b.z};
 return FlightNormalize({a.w+(b.w-a.w)*t,a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.z+(b.z-a.z)*t});
}
inline FlightQuat FlightSampleLeg(const FlightLegKey* keys,std::size_t count,float time){
 if(!count)return {};if(count==1||time<=keys[0].time)return keys[0].q;
 for(std::size_t i=1;i<count;++i)if(time<=keys[i].time)
  return FlightNlerp(keys[i-1].q,keys[i].q,(time-keys[i-1].time)/(keys[i].time-keys[i-1].time));
 return keys[count-1].q;
}
}
