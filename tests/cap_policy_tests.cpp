#include "Design.h"
#include <cassert>
#include <limits>
int main(){using namespace Arcana;
for(float f:{0.f,.05f,.2f,.3f,.4f,1.f,1.3f}){float got=CapArcanaCostFactor(f);assert(got==(f<.2f?.2f:f));}
assert(CapArcanaCostFactor(-.01f)<0);assert(CapArcanaCostFactor(std::numeric_limits<float>::infinity())<0);assert(CapArcanaCostFactor(std::numeric_limits<float>::quiet_NaN())<0);
assert(modes[Rapid].cost*CapArcanaCostFactor(0.f)==21.f);
assert(modes[SolarCrown].cost*CapArcanaCostFactor(0.f)==930.f);
assert(modes[Rain].cost*CapArcanaCostFactor(0.f)==960.f);
assert(modes[SolarCrown].damage==3040.f&&modes[Rain].damage==150.f);
}
