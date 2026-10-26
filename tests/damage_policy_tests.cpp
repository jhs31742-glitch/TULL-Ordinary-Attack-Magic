#include "Core/DamagePolicy.h"
#include <cassert>
#include <limits>
int main(){using namespace TOAM::Core::DamagePolicy;
for(float x:{0.01F,0.1F,0.5F,1.F,1.5F,2.F,3.F,100.F})assert(Valid(x));
for(float x:{-1.F,0.F,0.001F,101.F,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()})assert(!Valid(x));
for(auto s:{"", " ","NaN","inf","-1","0","100.1","1.5x","1,5"})assert(!Parse(s));
assert(Parse(" 1.5 ").value()==1.5F);assert(Parse("0.01").value()==0.01F);assert(Parse("100").value()==100.F);
assert(150.F*Parse("2").value()==300.F);assert(900.F*Parse("2").value()==1800.F);
assert((180.F*150.F+900.F)*Parse("2").value()==55800.F);return 0;}
