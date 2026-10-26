#include "Core/PresentationSettings.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <mutex>
#include <iostream>
namespace TOAM::Core::DamageSettings {std::mutex& CommonIniMutex() noexcept {static std::mutex m;return m;}}
namespace TOAM::VFX::CircleTint {void Prepare() noexcept {}}
static unsigned checks=0;
void check(bool v){++checks;if(!v){std::cerr<<"FAILED "<<checks<<'\n';std::exit(1);}}
int main(){
 using namespace TOAM::Presentation;namespace fs=std::filesystem;
 const fs::path path="Data/SKSE/Plugins/TOAM.ini";fs::create_directories(path.parent_path());
 {std::ofstream out(path);out<<"[Balance]\nDamageMultiplier=7.25\n[Flight]\nHumVolume=0.42\nCustomFlight=keep\n[User]\nPrivateSetting=keep\n[Audio]\nFutureKey=keep\n";}
 auto read=[](const wchar_t* section,const wchar_t* key){wchar_t value[120]{};GetPrivateProfileStringW(section,key,L"MISSING",value,120,L".\\Data\\SKSE\\Plugins\\TOAM.ini");return std::wstring(value);};
 check(Reload());auto v=Get();check(v.master==1);v.master=.35F;v.shake=3;v.spellVolume[0]=.5F;v.eventVolume[0]=.25F;v.selected[0]=17;v.colors[0]=TOAM::Presentation::Color::Red;v.colors[1]=TOAM::Presentation::Color::Purple;
 check(Save(v));check(Reload());auto active=Get();check(active.master==.35F&&active.shake==3&&active.selected[0]==17&&active.colors[0]==TOAM::Presentation::Color::Red&&active.colors[1]==TOAM::Presentation::Color::Purple);
 check(read(L"Balance",L"DamageMultiplier")==L"7.25");check(read(L"Flight",L"HumVolume")==L"0.42");check(read(L"Flight",L"CustomFlight")==L"keep");check(read(L"User",L"PrivateSetting")==L"keep");check(read(L"Audio",L"FutureKey")==L"keep");
 auto bad=v;bad.master=2;check(!Save(bad));check(Get().master==.35F);
 WritePrivateProfileStringW(L"CircleColor",L"Magician",L"Unknown",L".\\Data\\SKSE\\Plugins\\TOAM.ini");check(!Reload());check(Get().colors[0]==TOAM::Presentation::Color::Red);WritePrivateProfileStringW(L"CircleColor",L"Magician",L"Red",L".\\Data\\SKSE\\Plugins\\TOAM.ini");
 WritePrivateProfileStringW(L"AudioSelect",L"MagicianLaunch",L"999",L".\\Data\\SKSE\\Plugins\\TOAM.ini");check(Reload());check(Get().selected[0]==1);
 WritePrivateProfileStringW(L"AudioSelect",L"StrengthLoop",L"005",L".\\Data\\SKSE\\Plugins\\TOAM.ini");check(Reload());check(Get().selected[8]==6);
 WritePrivateProfileStringW(L"Audio",L"Master",L"nan",L".\\Data\\SKSE\\Plugins\\TOAM.ini");check(!Reload());check(Get().master==.35F);WritePrivateProfileStringW(L"Audio",L"Master",L"0.35",L".\\Data\\SKSE\\Plugins\\TOAM.ini");
 const auto h=CreateFileW(L"Data\\SKSE\\Plugins\\TOAM.ini",GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);check(h!=INVALID_HANDLE_VALUE);v.master=.2F;check(!Save(v));check(Get().master==.35F);CloseHandle(h);
 v.master=0;check(Save(v));check(Reload());check(Get().master==0);check(Resolve(Event::MagicianLaunch,1).volume==0);
 std::wstring longID(200,L'9');WritePrivateProfileStringW(L"AudioSelect",L"MagicianLaunch",longID.c_str(),L".\\Data\\SKSE\\Plugins\\TOAM.ini");check(Reload());check(Get().selected[0]==1);
 WritePrivateProfileStringW(L"AudioSelect",L"MagicianLaunch",L"\uD55C\uAE00",L".\\Data\\SKSE\\Plugins\\TOAM.ini");check(Reload());check(Get().selected[0]==1);
 std::cout<<"ACTUAL_SETTINGS_IO_PASS assertions="<<checks<<" custom_balance_flight_user_keys_preserved=true invalid_reload_retains_active=true lock_failure_retains_active=true\n";
}