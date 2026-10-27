#include "Core/ExclusiveCast.h"
#include "VFX/CircleOwnershipPolicy.h"
#include "Core/PresentationSettings.h"
#include <algorithm>
#include <atomic>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
std::atomic<unsigned> checks{};
void check(bool value){++checks;if(!value){std::cerr<<"FAIL at check "<<checks.load()<<'\n';std::abort();}}
namespace TOAM::Presentation {
Values active{};bool saveOK=true,reloadOK=true;unsigned saves=0;
Values Get() noexcept{return active;}
bool Save(const Values& v) noexcept{++saves;if(!saveOK||!Valid(v))return false;active=v;return true;}
bool Reload() noexcept{return reloadOK;}
}
namespace UI {
bool korean=false;std::string click;int desiredSpell=-1,desiredColor=-1,desiredSound=-1;unsigned sounds=0,levels=0;int tab=0;
const char* L(const char* en,const char* kr){return korean?kr:en;}
void Language(){}
struct MenuVec2 {float x,y;};struct MenuVec4 {float x,y,z,w;};
unsigned swatches=0;int clickSwatch=-1;
bool ColorButton(const char* s,const MenuVec4 rgb,int flags,const MenuVec2 size){
 ++swatches;int c=s[std::string(s).size()-1]-'0';check(c>=1&&c<=4);const auto expected=TOAM::Presentation::colorRGB[c];
 check(rgb.x==expected.red&&rgb.y==expected.green&&rgb.z==expected.blue&&rgb.w==1&&flags==0&&size.x>0&&size.y>0);return c==clickSwatch;
}
auto colorButton=ColorButton;
void Text(const char*,const char*){}
void Same(float,float){}
void Separate(){}
bool Button(const char* s){if(!click.empty()&&std::string(s).find(click)!=std::string::npos){click.clear();return true;}return false;}
bool Slider(const char* s,float*,float,float,const char*,int){if(std::string(s).find("###event")!=std::string::npos)++levels;return false;}
bool Combo(const char* s,int* selected,const char* const*,int,int){std::string name(s);if(name.find("audioSpell")!=name.npos||name.find("colorSpell")!=name.npos){if(desiredSpell>=0){*selected=desiredSpell;return true;}}else if(name.find("circlePreset")!=name.npos){if(desiredColor>=0){*selected=desiredColor;return true;}}else if(name.find("###sound")!=name.npos){++sounds;if(desiredSound>=0){*selected=desiredSound;return true;}}return false;}
bool BeginBar(const char*,int){return true;}
bool BeginItem(const char* name,bool*,int){return tab==0?std::string(name)=="Event volume":std::string(name)=="Sound selection";}
void End(){}
auto text=Text;auto sameLine=Same;auto separator=Separate;auto button=Button;auto slider=Slider;auto combo=Combo;auto beginTabBar=BeginBar;auto endTabBar=End;auto beginTabItem=BeginItem;auto endTabItem=End;
#include "Core/PresentationMenu.inl"
}
int main(){
 using namespace TOAM::Core::ExclusiveCast;const std::array<Owner,3> owners{Owner::HermitLeft,Owner::HermitRight,Owner::World};
 check(!Acquire(Owner::None));
 for(auto first:owners){check(Acquire(first));check(Current()==first);check(Acquire(first));for(auto later:owners)if(later!=first){check(!Allows(later));check(!Acquire(later));Release(later);check(Current()==first);}Release(first);check(Current()==Owner::None);}
 std::atomic<unsigned> inCritical{};std::array<std::thread,3> workers;
 for(unsigned k=0;k<3;++k)workers[k]=std::thread([&,k]{for(unsigned n=0;n<200;++n){while(!Acquire(owners[k]))std::this_thread::yield();check(inCritical.fetch_add(1)==0);check(Current()==owners[k]);check(inCritical.fetch_sub(1)==1);Release(owners[k]);}});
 for(auto& worker:workers)worker.join();check(Current()==Owner::None);
 using namespace TOAM::VFX::CircleOwnership;
 for(unsigned parent=0;parent<2;++parent)for(unsigned attached=0;attached<2;++attached)for(unsigned current=0;current<2;++current){unsigned expected=Expected(parent,attached,current);check(IsPrivate(expected,parent,parent,attached,current));check(!IsPrivate(expected+1,parent,parent,attached,current));check(!IsPrivate(expected-1,parent,parent,attached,current));check(!IsPrivate(expected,2,parent,attached,current));}
 using namespace TOAM::Presentation;
 UI::PresentationDraft();UI::presentationDraft.master=.2F;UI::presentationDraft.shake=2.5F;UI::presentationDraft.colors[0]=Color::Red;active.master=.7F;active.shake=.9F;
 UI::click="###presSave";UI::PresentationActions(2);check(active.master==.7F&&active.shake==.9F&&active.colors[0]==Color::Red);check(UI::presentationDraft.master==.2F);
 UI::click="###presSave";UI::PresentationActions(0);check(active.master==.2F&&active.shake==.9F&&active.colors[0]==Color::Red);
 UI::click="###presReset";UI::PresentationActions(0);check(UI::presentationDraft.master==1&&UI::presentationDraft.shake==2.5F&&UI::presentationDraft.colors[0]==Color::Red);
 saveOK=false;UI::click="###presSave";UI::PresentationActions(0);check(active.master==.2F);saveOK=true;
 reloadOK=false;UI::click="###presReload";UI::PresentationActions(0);check(UI::presentationDraft.master==1);reloadOK=true;UI::click="###presReload";UI::PresentationActions(0);check(UI::presentationDraft.master==active.master);
 for(unsigned spell=0;spell<spellCount;++spell){UI::desiredSpell=int(spell);unsigned expected=0;for(auto& event:events)if(event.spell==Spell(spell))++expected;
  UI::tab=0;UI::levels=UI::sounds=0;UI::RenderAudio();check(UI::levels==expected&&UI::sounds==0);
  UI::tab=1;UI::levels=UI::sounds=0;UI::RenderAudio();check(UI::levels==0&&UI::sounds==expected);
 }
 auto saved=UI::presentationDraft.selected;for(unsigned event=0;event<eventCount;++event){UI::desiredSound=999;UI::PresentationSound(event);check(UI::presentationDraft.selected==saved);}UI::desiredSound=-1;
 UI::presentationDraft=Values{};UI::clickSwatch=3;UI::swatches=0;auto before=active;
 UI::RenderAllCircleColors();check(UI::swatches==4);for(auto c:UI::presentationDraft.colors)check(c==Color::Gold);check(active.colors==before.colors);
 UI::clickSwatch=-1;UI::click="###colorPage0";UI::swatches=0;UI::RenderIndividualCircleColors();check(UI::swatches==20);
 UI::click="###circleColorSpell2_2";UI::RenderIndividualCircleColors();check(UI::presentationDraft.colors[2]==Color::Red&&UI::presentationDraft.colors[1]==Color::Gold);
 UI::click="###colorPage1";UI::swatches=0;UI::RenderIndividualCircleColors();check(UI::swatches==20);
 UI::click="###circleColorSpell9_4";UI::RenderIndividualCircleColors();check(UI::presentationDraft.colors[9]==Color::Purple&&UI::presentationDraft.colors[2]==Color::Red);
 UI::click="###colorPage2";UI::swatches=0;UI::RenderIndividualCircleColors();check(UI::swatches==12);
 UI::click="###circleColorSpell12_0";UI::RenderIndividualCircleColors();check(UI::presentationDraft.colors[12]==Color::Default);
 UI::click="###circleColorAll0";UI::RenderAllCircleColors();for(auto c:UI::presentationDraft.colors)check(c==Color::Default);
 UI::korean=true;UI::RenderIndividualCircleColors();check(Valid(UI::presentationDraft));
 check(std::string(spellDisplayEN[0])=="I Magician"&&std::string(spellDisplayEN[8])=="VIII Strength"&&std::string(spellDisplayEN[9])=="XI Justice");
 check(std::string(spellKeys[0])=="Magician"&&std::string(spellKeys[8])=="Strength");
 std::cout<<"CP034_D3_HOST_POLICY_MENU_PASS assertions="<<checks.load()<<" exclusive_competitors=3 ownership_alias_combinations=8 spells=13 events=17 save_page_merge=true invalid_selection_rejected=true\n";
}