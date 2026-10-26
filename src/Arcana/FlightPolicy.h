#pragma once
#include <algorithm>
#include <cmath>
namespace Arcana {
enum class FlightPhase { Off, Flying };
struct FlightSettings {float speed=300,boost=650,vertical=220,takeoffSpeed=160,takeoffSeconds=.55f,response=7;bool enabled=true;};
struct FlightVector {float x=0,y=0,z=0;};
inline FlightVector FlightDesired(float x,float y,float vertical,float yaw,bool boost,FlightPhase phase,const FlightSettings& s,float pitch=0){
 const float length=std::hypot(x,y);if(length>1){x/=length;y/=length;}
 const float speed=boost?s.boost:s.speed;
 const float forward=y*std::cos(pitch);
 return {(std::cos(yaw)*x+std::sin(yaw)*forward)*speed,(-std::sin(yaw)*x+std::cos(yaw)*forward)*speed,-std::sin(pitch)*y*speed+std::clamp(vertical,-1.f,1.f)*s.vertical};
}
inline float FlightHeightCorrection(float error){return std::clamp(error*4.f,-60.f,60.f);}
inline float FlightApproach(float current,float target,float dt,float response){return current+(target-current)*(1-std::exp(-response*std::clamp(dt,0.f,.05f)));}
inline float FlightTakeoffVertical(float requested,float age,bool down,const FlightSettings& s){
 if(down||age>=s.takeoffSeconds)return requested;
 float t=std::clamp((age/s.takeoffSeconds-.35f)/.65f,0.f,1.f);t=t*t*(3-2*t);return std::max(requested,s.takeoffSpeed*(1-t));
}
inline float FlightSigilAlpha(float age){float x=std::clamp((age-.34f)/.42f,0.f,1.f);return 1-x*x*(3-2*x);}
inline constexpr float flightSigilSeconds=.76f;
inline FlightVector AirWorldNormal(int index){
 // Twelve distributed directions; each third deployment adds both hemispheres.
 index=(index%2?6+index/2:index/2);constexpr float golden=2.39996322973f;const float z=1.f-2.f*(index+.5f)/12.f,r=std::sqrt(std::max(0.f,1-z*z)),a=index*golden;
 return {std::cos(a)*r,std::sin(a)*r,z};
}
inline FlightVector AirCircleOffset(float x,float y,float z){return {x,y,z-98.f};}
}
