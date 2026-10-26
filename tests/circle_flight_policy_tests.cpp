#include "VFX/CircleMotion.h"
#include "Core/FlightMenuPolicy.h"
#include "Core/DamagePolicy.h"
#include <cassert>
#include <limits>
int main(){namespace M=TOAM::VFX::CircleMotion;namespace F=TOAM::Core::FlightMenuPolicy;
assert(F::Valid(F::defaults));auto v=F::defaults;
for(unsigned i=0;i<v.size();++i){v=F::defaults;v[i]=F::minimum[i];assert(F::Valid(v));v[i]=F::maximum[i];assert(F::Valid(v));v[i]=F::maximum[i]+1;assert(!F::Valid(v));v[i]=std::numeric_limits<float>::quiet_NaN();assert(!F::Valid(v));}
v=F::defaults;v[0]=.5f;assert(!F::Valid(v));
for(unsigned i=0;i<8;i++){assert(std::abs(M::LayerSpeed(i,8))>std::abs(M::LayerSpeed(i,1)));for(unsigned j=i+1;j<8;j++)assert(M::LayerSpeed(i,1)!=M::LayerSpeed(j,1));}
assert(M::LayerSpeed(0,1)>0&&M::LayerSpeed(1,1)<0);assert(M::RevealSpeed(0,1)>M::RevealSpeed(1,1));assert(M::RevealSpeed(0,-1)<M::RevealSpeed(1,-1));
float phase=0;for(int i=0;i<100000;i++){phase=M::Advance(phase,M::LayerSpeed(6,8),.016f);assert(std::isfinite(phase)&&std::abs(phase)<3.142f);}
assert(TOAM::Core::DamagePolicy::Parse("2.0").value()==2.f);return 0;}
