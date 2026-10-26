#include "Core/PresentationPolicy.h"
#include <iostream>
#include <limits>
#include <string>
using namespace TOAM::Presentation;
int main(){
 unsigned checks=0;auto check=[&](bool ok){++checks;if(!ok){std::cerr<<"FAIL assertion "<<checks<<'\n';std::exit(1);}};
 Values v;check(Valid(v));
 constexpr unsigned original[]{1,2,3,2,4,5,5,11,6,16,12,5,6,13,17,17,18};
 for(std::size_t i=0;i<eventCount;++i){const auto e=Event(i);check(v.selected[i]==original[i]);check(Compatible(e,original[i]));check(Select(e,"000")==original[i]);check(Select(e,"999")==original[i]);check(Select(e,"bad")==original[i]);check(Select(e,"001junk")==original[i]);
  for(unsigned id=1;id<=18;++id){const bool expected=sources[id-1].loop==sources[original[i]-1].loop&&(i<5||sources[id-1].descriptor!=0);check(Compatible(e,id)==expected);check(Select(e,std::to_string(id))==(expected?id:original[i]));}
  for(float gain:{.34F,.47F,.5F,.54F,.65F,.68F,.78F,.85F,.9F,.94F,.95F,1.F})check(Volume(v,e,gain)==gain);
  auto mute=v;mute.master=0;check(Volume(mute,e,.95F)==0);mute=v;mute.eventVolume[i]=0;check(Volume(mute,e,.95F)==0);mute=v;mute.spellVolume[std::size_t(events[i].spell)]=0;check(Volume(mute,e,.95F)==0);
 }
 for(float bad:{-1.F,1.01F,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){auto x=v;x.master=bad;check(!Valid(x));x=v;x.spellVolume[0]=bad;check(!Valid(x));x=v;x.eventVolume[0]=bad;check(!Valid(x));}
 for(auto text:{"nan","inf","-0.1","1.1","1x","","2e100"})check(!Number(text,0,1));check(Number(" 0.25 ",0,1).value()==.25F);
 for(float strength:{.01F,.2F,.28F,.68F,.95F,1.F}){check(Shake(strength,0)==0);check(Shake(strength,1)==strength);check(std::abs(Shake(strength,3)-strength*3)<.000001F);}
 check(Shake(0,3)==0);check(Shake(std::numeric_limits<float>::quiet_NaN(),3)==0);auto invalid=v;invalid.shake=3.01F;check(!Valid(invalid));invalid.shake=3;check(Valid(invalid));
 for(std::size_t s=0;s<spellCount;++s)for(unsigned c=0;c<5;++c){auto next=v;const auto existing=next.colors[s];next.colors[s]=Color(c);check(existing==Color::Default);check(Valid(next));for(std::size_t other=0;other<spellCount;++other)if(other!=s)check(next.colors[other]==Color::Default);}
 std::cout<<"POLICY_PASS assertions="<<checks<<" sources="<<sources.size()<<" events="<<eventCount<<" spells="<<spellCount<<'\n';return 0;
}