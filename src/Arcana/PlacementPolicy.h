#pragma once
namespace Arcana {
// These coordinates are the user's imported actor-local emitter positions.
inline bool SecondaryStack(int side,bool matching){return matching&&side==1;}
inline Offset At(const LayoutData::XYZ& p,float scale){return {p.x,p.y,p.z,scale};}
inline Offset FrontOffset(int mode,int side,bool matching){
 if(mode==Sustain)return At(LayoutData::sustain[SecondaryStack(side,matching)?1:0],.25f);
 if(mode==Sentry)return At(LayoutData::rapid[0],.25f);
 if(mode==SolarCrown)return At(LayoutData::ultimate[0],1);
 return At(SecondaryStack(side,matching)?LayoutData::rapidSecondary[0]:LayoutData::rapid[0],.25f*(SecondaryStack(side,matching)?.72f:1.f));
}
inline Offset FivefoldOffset(int i,float t,bool secondary){
 const auto& point=secondary?LayoutData::fivefoldSecondary[i]:LayoutData::fivefold[i];auto p=At(point,.23f*(secondary?.72f:1.f));
 if(i)p.x*=Spread(t)/1.15f;return p;
}
inline Offset UltimateLayerOffset(int i,float t){auto p=At(LayoutData::ultimate[i],1);p.y+=i*(UltimateGap(t)-7.f);return p;}
// A single aim-space axis owns all layer centers, including pitch and yaw.
inline Offset UltimateAxisOffset(int i,float t){auto base=At(LayoutData::ultimate[0],1);auto layer=UltimateLayerOffset(i,t);return {0,layer.y-base.y,0,1};}
inline float UltimateBeamDepth(float t){float depth=0;for(int i=0;i<8;i++)depth=std::max(depth,UltimateLayerOffset(i,t).y-LayoutData::ultimate[0].y);return depth+18.f;}
inline constexpr float trackingDomeMin=185,trackingDomeMax=280,trackingRepeatDistance=100;
inline constexpr float trackingLaneClearance=85;
}
