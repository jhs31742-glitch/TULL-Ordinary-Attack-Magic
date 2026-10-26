#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "WorldFormationData.h"
#include "LayoutData.h"
#include "RainTimeline.h"
namespace Arcana {
enum ModeId:int {Rapid=0,Hermit=1,Fivefold=2,SolarCrown=3,Sustain=4,Sentry=5,Parry=Sentry,Tracking=6,Rain=7};
struct Mode {float interval,damage,cost;int maxCircles;};
// CP021: cost is the undiscounted billing unit, not universally MP/second.
// CP029 user-approved: every Arcana base resource cost x3; TOAM owns separate costs.
// Emperor/shot, Hermit/second, Chariot/five shots (300 each), Sun/full eight seconds,
// Strength/second, Justice/placement, Star/activation (+150/deployed lane/second), Tower/activation.
inline constexpr std::array<Mode,8> modes{{{.20f,55,105,1},{3.f,120,132,12},{.40f,64,1500,5},{.10f,3040,4650,8},{.10f,19.5f,750,1},{.20f,110,1500,3},{.10f,1,480,5},{1.f/15.f,150,4800,2}}};
inline constexpr float trackingUpkeepPerLane=150.f,rainTargetBaseCap=27900.f;
inline float Smooth(float x){x=std::clamp(x,0.f,1.f);return x*x*(3-2*x);}
inline constexpr float ordinaryWidthMultiplier=1.62f,ordinaryReviewMultiplier=2.1f;
inline constexpr float stageSeconds=.80f,growSeconds=.24f,ultimateChargeSeconds=8,ultimateFireSeconds=0;
inline constexpr float circleTailHold=.16f,circleTailFade=.26f;
struct Offset {float x,y,z,scale;};
inline constexpr std::array<Offset,5> formation{{{0.0f,57.0f,100.0f,0.23f},{69.5652174f,-17.0f,125.0f,0.23f},{-69.5652174f,-17.0f,125.0f,0.23f},{64.3478261f,-32.0f,68.0f,0.23f},{-64.3478261f,-32.0f,68.0f,0.23f}}};
inline float Spread(float t){return 1+.15f*std::clamp(t/3.2f,0.f,1.f);}
inline int StanceMode(bool low,bool mid,bool high){if(int(low)+int(mid)+int(high)>1)return 0;return high?3:(mid?2:(low?1:0));}
inline int SelectMode(int family,int stance){return family*4+std::clamp(stance,0,3);}
inline float CircleGrowth(int i,float t){return Smooth((t-i*.8f)/.24f);}
inline int CircleCount(int mode,float t){return mode==SolarCrown?std::clamp(1+int(t/.8f),1,8):(mode==Fivefold?std::clamp(1+int(t/.8f),1,5):1);}
inline constexpr std::array<float,8> ultimateRadii{{1.f,1.35f,.70f,1.55f,1.05f,.65f,1.40f,.90f}};
inline constexpr std::array<bool,8> ultimateFull{{true,false,true,false,true,false,false,true}};
inline constexpr float ultimateSizeMultiplier=1.30f,circleRadiusBound=365,circleGroundRadius=244,feetClearance=8,ultimateBeamRadius=56.7f;
inline float UltimateProgress(float t){return std::clamp(t/8.f,0.f,1.f);}
inline float UltimateGap(float t){return 22-15*Smooth(UltimateProgress(t));}
inline float UltimateGrowth(int i,float t){return Smooth((UltimateProgress(t)-i*.085f)/.35f);}
inline constexpr float ultimateStableFit=100.f/(ultimateSizeMultiplier*(*std::max_element(ultimateRadii.begin(),ultimateRadii.end()))*circleGroundRadius);
inline float UltimateScale(int i,float){return ultimateSizeMultiplier*ultimateRadii[i]*ultimateStableFit;}
inline float GroundFitFactor(float s,float h){if(!std::isfinite(s)||!std::isfinite(h)||s<=0)return 0;return std::clamp((h-feetClearance)/(s*circleGroundRadius),0.f,1.f);}
inline bool SunReleaseDue(float t,bool released,bool ready){return t>=ultimateChargeSeconds&&!released&&ready;}
inline bool UltimateFiring(float t){return t>=8;}
inline bool UltimateDone(float t){return t>=10;}
inline constexpr float ultimateKickStrength=.85f,ultimateKickSeconds=.5f,ultimateRumbleStrength=.38f,ultimateRumbleSeconds=.24f,ultimateRumbleInterval=.2f;
inline constexpr std::uint16_t sustainFadeMS=650;
inline constexpr int beamsPerHand=48; inline constexpr float beamPulseSeconds=.48f,beamAttackSeconds=.10f,beamHoldSeconds=.14f,beamFadeSeconds=.24f,beamReleaseHold=.12f,beamReleaseFade=.30f;
// Presentation and damage transport are independent: Fivefold is an anchored beam pulse.
enum class BeamStyle {Traveling,ConnectedPulse,Held};
inline BeamStyle BeamStyleFor(int mode){return (mode==Sustain||mode==Sentry)?BeamStyle::Held:((mode==Fivefold||mode==Rain)?BeamStyle::ConnectedPulse:BeamStyle::Traveling);}
inline float BoltSpeed(int mode){return mode==SolarCrown?1600.f:((mode==Sustain||mode==Fivefold||mode==Rain||mode==Sentry)?40000.f:6000.f);}
inline constexpr float solarSweepRadius=88.f,solarMaxRange=8192.f,solarTrailLength=1600.f;
inline bool SolarSweepContact(float along,float lateralSquared,float previous,float head,float tail){
 if(!std::isfinite(along)||!std::isfinite(lateralSquared)||lateralSquared<0||head<0)return false;
 float separation=along-std::clamp(along,std::min(previous,tail),head);
 return lateralSquared+separation*separation<=solarSweepRadius*solarSweepRadius;
}
inline bool RainFinaleReady(unsigned fired,bool actualLastContact,bool detonated){return !detonated&&fired>=180&&actualLastContact;}
inline float BoltTrail(int mode,float distance){return std::min(mode==SolarCrown?solarTrailLength:260.f,distance*.55f);}
inline float BoltHold(int mode){return mode==SolarCrown?.12f:.025f;}
inline float BoltFade(int mode){return mode==SolarCrown?.26f:.10f;}
inline float BoltLifetime(int mode,float distance){return distance/BoltSpeed(mode)+BoltHold(mode)+BoltFade(mode);}
struct TravelingPulse {float head=0,tail=0,width=0,alpha=0;};
inline TravelingPulse BoltState(int mode,float distance,float age){
 if(!std::isfinite(age)||age<0||distance<=0||age>=BoltLifetime(mode,distance))return {};
 const float hit=distance/BoltSpeed(mode),head=std::min(distance,age*BoltSpeed(mode));
 const float alpha=Smooth(age/.008f)*(1-Smooth((age-hit-BoltHold(mode))/BoltFade(mode)));
 return {head,std::max(0.f,head-BoltTrail(mode,distance)),.15f+.85f*Smooth(age/.008f),alpha};
}
inline float PulseOpacity(float t){if(t<0||t>=beamPulseSeconds)return 0;return t<beamAttackSeconds?Smooth(t/.025f):(t<=beamAttackSeconds+beamHoldSeconds?1:1-Smooth((t-beamAttackSeconds-beamHoldSeconds)/beamFadeSeconds));}
inline float PulseWidth(float t){if(t<0||t>=beamPulseSeconds)return 0;return t<.025f?.1f+.9f*Smooth(t/.025f):1-.15f*Smooth((t-beamAttackSeconds-beamHoldSeconds)/beamFadeSeconds);}
inline float BeamVisualLifetime(int mode,float distance){return BeamStyleFor(mode)==BeamStyle::Held?.30f:(BeamStyleFor(mode)==BeamStyle::ConnectedPulse?beamPulseSeconds:BoltLifetime(mode,distance));}
inline TravelingPulse BeamPulseState(int mode,float distance,float age){
 if(BeamStyleFor(mode)!=BeamStyle::ConnectedPulse)return BoltState(mode,distance,age);
 if(!std::isfinite(age)||age<0||distance<=0||age>=beamPulseSeconds)return {};
 return {distance,0,PulseWidth(age),PulseOpacity(age)};
}
inline float ReleaseOpacity(float t){return t<beamReleaseHold?1:1-Smooth((t-beamReleaseHold)/beamReleaseFade);}
inline float CircleTailAlpha(float age){return age<circleTailHold?1:1-Smooth((age-circleTailHold)/circleTailFade);}
inline constexpr std::array<float,5> fivefoldIntervals{{.36f,.39f,.42f,.45f,.40f}};
inline constexpr float FivefoldDPS(){float sum=0;for(float p:fivefoldIntervals)sum+=10/p;return sum;}
struct FivefoldCadence {
 std::array<float,5> next{};std::array<bool,5> armed{};
 unsigned Tick(float elapsed,int count){
  if(!std::isfinite(elapsed)||elapsed<0)return 0;
  unsigned mask=0;count=std::clamp(count,0,5);
  for(int i=0;i<count;i++){float ready=i*.8f+.24f;if(elapsed+.00001f<ready)continue;if(!armed[i]){armed[i]=true;next[i]=ready;}if(elapsed+.00001f<next[i])continue;mask|=1u<<i;next[i]+=fivefoldIntervals[i];if(next[i]<=elapsed+.00001f)next[i]=elapsed+fivefoldIntervals[i];}
  return mask;
 }
};
struct Cadence {float elapsed=0,next=0;bool Tick(float dt,float interval){if(!std::isfinite(dt)||dt<=0)return false;dt=std::clamp(dt,0.f,.1f);elapsed+=dt;next-=dt;if(next>.00001f)return false;next+=interval;if(next<=0)next=interval;return true;}};
inline constexpr float worldChargeSeconds=3.f,worldShellRadius=113.85f,worldDomeScale=.81f,worldBurstOffset=36,worldBurstSpread=165,worldDamageRadius=300,worldBurstSize=192,worldBurstSeconds=2.20f,worldReleaseSeconds=1.31f;
enum class BurstKind {World,Rain,SolarImpact};
inline constexpr int baseBurstSlots=24,solarImpactSlots=4;
inline constexpr float vanillaExplosionSeconds=5.352f,burstVoiceSeconds=5.502f,solarImpactSize=240.f;
inline bool BurstSoundDue(BurstKind kind,int local){return kind!=BurstKind::SolarImpact&&local==0;}
inline bool SolarTerrainImpactDue(bool geometry,float age,float distance,bool played){return geometry&&!played&&std::isfinite(age)&&std::isfinite(distance)&&distance>=0&&age>=distance/BoltSpeed(SolarCrown);}
inline Offset WorldBurstDisplacement(float x,float y,float z){return {x*worldBurstSpread,y*worldBurstSpread,z*worldBurstOffset,worldBurstSize};}
inline constexpr std::array<int,12> worldOrder{{0,5,7,10,1,2,3,4,6,8,9,11}};
inline constexpr std::array<float,12> worldDeployTimes{{0.f,.14f,.31f,.49f,.70f,.90f,1.12f,1.34f,1.58f,1.80f,2.02f,2.23f}};
inline constexpr std::array<float,12> worldGrowthPowers{{1.15f,.92f,1.32f,1.02f,1.25f,.88f,1.18f,1.38f,.98f,1.28f,1.08f,.94f}};
inline int WorldCount(float t){int count=0;for(float at:worldDeployTimes)if(t>=at)++count;return count;}
inline float WorldDeployTime(int i){return worldDeployTimes[std::clamp(i,0,11)];}
inline float WorldCircleGrowth(int i,float t){float at=WorldDeployTime(i),u=std::clamp((t-at)/(worldChargeSeconds-at),0.f,1.f);return std::pow(Smooth(u),worldGrowthPowers[std::clamp(i,0,11)]);}

inline constexpr float parryWindowSeconds=1.20f,parryCooldownSeconds=6.f,parryDotMinimum=.5735764f;
inline bool InParryCone(float x,float y){float len=std::hypot(x,y);return std::isfinite(len)&&len>.001f&&y/len>=parryDotMinimum;}
inline constexpr float trackingChargeSeconds=.35f,trackingRadius=1200,trackingCircleScale=.14f,trackingBuild=.14f,trackingHold=.16f,trackingFade=.26f;
inline constexpr std::array<float,5> trackingPeriods{{.72f,.79f,.75f,.86f,.82f}};
inline int TrackingCount(float t){return t<trackingChargeSeconds?0:std::clamp(1+int((t-trackingChargeSeconds)/.8f),1,5);}
inline constexpr float rainChargeSeconds=8,rainRadius=850,rainForward=850,rainSkyHeight=520,rainBeamDrop=24,rainCooldownSeconds=24;
inline constexpr int rainShots=180;inline constexpr float rainFirstShotSeconds=1.2f,rainPulseSeconds=beamPulseSeconds,worldCircleScale=.3564f;
inline constexpr float rainRadiusStart=100,rainRadiusSpeed=(rainRadius-rainRadiusStart)/rainChargeSeconds,rainDissolveSeconds=1.2f;
inline float RainFootprint(float t){return rainRadiusStart+rainRadiusSpeed*std::max(0.f,t);}
inline float RainBombardmentRadius(float t){return std::min(rainRadius,RainFootprint(t));}
inline constexpr float rainSpinRamp=9.6f;
inline float RainAngularSpeed(float t){float u=std::clamp(t/rainSpinRamp,0.f,1.f);return .08f+2.6f*u*u*u;}
inline float RainRotation(float t){float x=std::clamp(t,0.f,rainSpinRamp),u=x/rainSpinRamp;return .08f*x+.65f*x*u*u*u+RainAngularSpeed(rainSpinRamp)*std::max(0.f,t-rainSpinRamp);}
inline float RainGrowth(float t){return std::max(0.f,t)/rainChargeSeconds;}
inline float RainShotTime(int index){return index>=0&&index<rainShots?rainShotTimes[index]:100000.f;}
inline int RainShotsDue(float t){int count=0;for(int i=0;i<rainShots;i++)if(t+.00001f>=RainShotTime(i))count++;return count;}
inline float RainPower(int fired){return std::clamp(float(fired)/float(rainShots-1),0.f,1.f);}
inline float RainWidth(int fired){return 10.8f*(1+2.f*RainPower(fired)*RainPower(fired));}
inline float RainShake(int fired){return .20f+.48f*RainPower(fired);}
inline constexpr int rainMuzzlesPerHand=96,burstsPerHand=12;
inline constexpr int rainFinalePlumes=9;
inline Offset RainFinaleOffset(int i){if(i==0)return {0,0,48,700};float angle=(i-1)*6.2831853f/8.f+.3926991f;return {std::cos(angle)*rainRadius*.62f,std::sin(angle)*rainRadius*.62f,48,480};}
inline constexpr float rainMuzzlePrepare=.10f,rainMuzzleHold=.12f,rainMuzzleScale=.18f,rainImpactStart=6;
inline float DomeDamageFactor(float coverage){return 1-.8f*std::clamp(coverage,0.f,1.f);}
inline float BurstScale(float t){return t<.08f?.18f+.82f*Smooth(t/.08f):1+.35f*Smooth((t-.08f)/.87f);}
inline float BurstOpacity(float t){return t<0||t>=worldBurstSeconds?0:(t<.045f?Smooth(t/.045f):1-Smooth((t-.12f)/.83f));}
inline bool GroundImpactEligible(float phase,bool actorHit,bool geometry,float normalZ,float error){return phase>=6&&std::isfinite(phase)&&!actorHit&&geometry&&std::isfinite(normalZ)&&normalZ>=.45f&&std::isfinite(error)&&std::abs(error)<=8;}
inline float GroundShakePower(float energy){return std::min(.95f,std::sqrt(std::max(0.f,energy)));}
inline bool IsUltimate(int mode){return mode==SolarCrown||mode==Rain;}
inline bool CanBeginMode(int desired,int other,bool otherActive){return !IsUltimate(desired)||!otherActive||desired!=other;}
inline float BeamRadius(int mode){return mode==SolarCrown?ultimateBeamRadius:(mode==Rain?10.8f:(mode==Sustain?5.f:3.8f)*ordinaryWidthMultiplier*((mode==Rapid||mode==Fivefold||mode==Sustain||mode==Sentry)?ordinaryReviewMultiplier:1.f));}
}
