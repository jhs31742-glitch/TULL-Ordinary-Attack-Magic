#include "Core/DamageSettings.h"
#include "Core/PresentationSettings.h"
#include "Core/DamagePolicy.h"
#include "Core/FlightMenuPolicy.h"
#include "Arcana/Integration.h"
#include <array>
#include <charconv>
#include <vector>
#include <atomic>
#include <cstdio>
#include <mutex>
#include <string>
#include <Windows.h>
#include <SKSE/SKSE.h>
namespace TOAM::Core::DamageSettings {
namespace {
std::atomic<float> multiplier{DamagePolicy::kDefault};
std::mutex iniMutex;
std::wstring IniPath(){
    wchar_t path[32768]{};
    const auto size=GetFullPathNameW(L"Data\\SKSE\\Plugins\\TOAM.ini",32768,path,nullptr);
    return size&&size<32768?std::wstring(path):std::wstring{};
}
std::optional<float> Read(const std::wstring& path){
    wchar_t text[128]{};const auto n=GetPrivateProfileStringW(L"Balance",L"DamageMultiplier",L"1.0",text,128,path.c_str());
    if(n>=127)return std::nullopt;
    std::string ascii;ascii.reserve(n);
    for(DWORD i=0;i<n;++i){if(text[i]>127)return std::nullopt;ascii.push_back(static_cast<char>(text[i]));}
    return DamagePolicy::Parse(ascii);
}
// Minimal dynamically resolved ABI; declarations verified against the official
// SKSE-Menu-Framework-3-API and installed DLL exports. No linked framework/ImGui.
// https://github.com/QTR-Modding/SKSE-Menu-Framework-3-API
using RenderFn=void(__stdcall*)();
using AddPageFn=void(*)(const char*,RenderFn);
using TextFn=void(*)(const char*,const char*);
using InputFn=bool(*)(const char*,float*,float,float,const char*,int);
using SliderFn=bool(*)(const char*,float*,float,float,const char*,int);
using ButtonFn=bool(*)(const char*);
using SameLineFn=void(*)(float,float);
using CheckboxFn=bool(*)(const char*,bool*);
CheckboxFn checkbox=nullptr;
TextFn text=nullptr;InputFn input=nullptr;SliderFn slider=nullptr;ButtonFn button=nullptr;SameLineFn sameLine=nullptr;
bool korean=true,languageLoaded=false;
const char* L(const char* en,const char* kr){return korean?kr:en;}
void Language(){
 if(!languageLoaded){const auto p=IniPath();korean=GetPrivateProfileIntW(L"Menu",L"Korean",1,p.c_str())!=0;languageLoaded=true;}
 if(checkbox("Korean / 한국어",&korean)){
  std::scoped_lock lock(iniMutex);const auto p=IniPath();
  if(!WritePrivateProfileStringW(L"Menu",L"Korean",korean?L"1":L"0",p.c_str()))korean=!korean;
 }
}
using FlightValues=std::array<float,12>;
bool ReadFlight(const std::wstring& path,FlightValues& values){
 values=FlightMenuPolicy::defaults;
 for(std::size_t i=0;i<values.size();++i){
  wchar_t wide[96]{};auto n=GetPrivateProfileStringW(L"Flight",FlightMenuPolicy::keys[i],L"",wide,96,path.c_str());
  if(!n)continue;if(n>=95)return false;std::string bytes;
  for(DWORD j=0;j<n;++j){if(wide[j]>127)return false;bytes.push_back(static_cast<char>(wide[j]));}
  const auto first=bytes.find_first_not_of(" \t"),last=bytes.find_last_not_of(" \t");if(first==std::string::npos)return false;
  float value{};const char* begin=bytes.data()+first;const char* end=bytes.data()+last+1;
  const auto r=std::from_chars(begin,end,value);if(r.ec!=std::errc{}||r.ptr!=end)return false;values[i]=value;
 }
 return FlightMenuPolicy::Valid(values);
}
bool ReadFlightDraft(FlightValues& values){std::scoped_lock lock(iniMutex);const auto p=IniPath();return !p.empty()&&ReadFlight(p,values);}
bool SaveFlight(const FlightValues& values){
 if(!FlightMenuPolicy::Valid(values))return false;
 std::scoped_lock lock(iniMutex);const auto path=IniPath();if(path.empty())return false;
 std::array<wchar_t,8192> previous{};
 const auto n=GetPrivateProfileSectionW(L"Flight",previous.data(),static_cast<DWORD>(previous.size()),path.c_str());
 if(n>=previous.size()-2)return false;
 std::vector<wchar_t> section;
 for(std::size_t i=0;i<values.size();++i){
  wchar_t number[64]{};swprintf_s(number,L"%.6f",static_cast<double>(values[i]));
  const std::wstring entry=std::wstring(FlightMenuPolicy::keys[i])+L"="+number;
  section.insert(section.end(),entry.begin(),entry.end());section.push_back(0);
 }section.push_back(0);
 const bool wrote=WritePrivateProfileSectionW(L"Flight",section.data(),path.c_str())!=0;
 static_cast<void>(WritePrivateProfileStringW(nullptr,nullptr,nullptr,path.c_str()));
 FlightValues check{};bool ok=wrote&&ReadFlight(path,check);
 if(ok)for(std::size_t i=0;i<values.size();++i)ok=ok&&std::abs(values[i]-check[i])<=.00001F;
 if(!ok){
  const bool restored=WritePrivateProfileSectionW(L"Flight",previous.data(),path.c_str())!=0;
  SKSE::log::warn("[TOAM][FlightMenu] Save failed; prior section restore={}; runtime unchanged",restored);return false;
 }
 Arcana::QueueFlightSettingsReload();return true;
}
void __stdcall RenderFlight(){
 Language();static FlightValues values=FlightMenuPolicy::defaults;static bool loaded=false;static std::string status;
 if(!loaded){static_cast<void>(ReadFlightDraft(values));loaded=true;}
 text(L("Flight settings. Saved values apply on the next game frame.","비행 설정: 저장 후 다음 게임 프레임에 적용됩니다."),nullptr);
 const char* en[]{"Enable flight###f0","Move speed###f1","Boost speed###f2","Vertical speed###f3","Takeoff speed###f4","Takeoff duration###f5","Response###f6","Ranged leg pose###f7","Block melee while flying###f8","Firearm animations###f9","Hover hum###f10","Hum volume###f11"};
 const char* kr[]{"비행 사용###f0","이동 속도###f1","가속 속도###f2","상하 이동 속도###f3","이륙 속도###f4","이륙 시간###f5","반응 속도###f6","원거리 전투 다리 자세###f7","비행 중 근접 공격 차단###f8","총기 호환 모션###f9","부유 소리###f10","부유 소리 음량###f11"};
 for(std::size_t i=0;i<values.size();++i){
  if(FlightMenuPolicy::Boolean(i)){bool value=values[i]>.5F;if(checkbox(L(en[i],kr[i]),&value))values[i]=value?1.F:0.F;}
  else slider(L(en[i],kr[i]),&values[i],FlightMenuPolicy::minimum[i],FlightMenuPolicy::maximum[i],i==5||i==11?"%.2f":"%.0f",0);
 }
 if(button(L("Apply and save###flightSave","적용 및 저장###flightSave")))status=SaveFlight(values)?L("Saved; next game frame applies the settings.","저장했습니다. 다음 게임 프레임에 적용됩니다."):L("Save failed or invalid settings; active flight settings retained.","저장 실패 또는 잘못된 값입니다. 현재 비행 설정을 유지합니다.");
 sameLine(0,-1);
 if(button(L("Reset defaults###flightDefault","기본값 복원###flightDefault")))values=FlightMenuPolicy::defaults;
 sameLine(0,-1);
 if(button(L("Reload INI###flightReload","INI 다시 읽기###flightReload"))){FlightValues next{};if(ReadFlightDraft(next)){values=next;Arcana::QueueFlightSettingsReload();status=L("INI reloaded.","INI를 다시 읽었습니다.");}else status=L("Invalid INI; active settings retained.","INI 값이 잘못되어 현재 설정을 유지합니다.");}
 text(L("Speeds use game units per second. These controls do not change spell costs or damage.","속도 단위는 게임 유닛/초입니다. 주문 비용과 피해량은 변경하지 않습니다."),nullptr);
 if(!status.empty())text(status.c_str(),nullptr);
}
#include "PresentationMenu.inl"
void __stdcall Render(){
 Language();static float draft=GetMultiplier();static std::string status;
 text(L("One common multiplier for offensive spell and staff damage.","공격 주문과 지팡이 피해에 적용하는 공통 배율입니다."),nullptr);
 text(L("Temperance, marking damage, costs, limiter and Hermit defense stay fixed.","절제·표식 피해·자원 소비·리미터·은둔자 방어는 유지됩니다."),nullptr);
 slider(L("Damage multiplier###damageSlider","피해 배율###damageSlider"),&draft,0.1F,10.0F,"%.2fx",0);
 input(L("Exact value (0.01 - 100)###damageExact","직접 입력 (0.01 - 100)###damageExact"),&draft,0.1F,1.0F,"%.3f",0);
 if(button(L("Apply and save INI###damageSave","적용 및 INI 저장###damageSave")))status=Save(draft)?L("Saved. New releases/casts use this value.","저장했습니다. 새로 발사하거나 시전하는 주문에 적용됩니다."):L("Save failed or invalid value. Active setting retained.","저장 실패 또는 잘못된 값입니다. 현재 배율을 유지합니다.");
 sameLine(0,-1);
 if(button(L("Reset to 1x###damageReset","1배로 복원###damageReset"))){draft=1.0F;status=Save(draft)?L("Default saved.","기본 배율을 저장했습니다."):L("Save failed. Active setting retained.","저장에 실패하여 현재 배율을 유지합니다.");}
 sameLine(0,-1);
 if(button(L("Reload INI###damageReload","INI 다시 읽기###damageReload"))){const bool ok=Reload();draft=GetMultiplier();status=ok?L("INI reloaded.","INI를 다시 읽었습니다."):L("Invalid/unreadable INI. Active setting retained.","INI를 읽지 못해 현재 배율을 유지합니다.");}
 char active[120]{};std::snprintf(active,sizeof(active),L("Active damage multiplier: %.3fx","현재 피해 배율: %.3fx"),GetMultiplier());text(active,nullptr);
 if(!status.empty())text(status.c_str(),nullptr);
}
}
std::mutex& CommonIniMutex() noexcept {return iniMutex;}
float GetMultiplier() noexcept {return multiplier.load(std::memory_order_relaxed);}
bool Reload() noexcept {
    try {
        std::scoped_lock lock(iniMutex);const auto path=IniPath();if(path.empty())return false;
        const auto value=Read(path);if(!value){SKSE::log::warn("[TOAM][Damage] Invalid INI multiplier; active value retained={}",GetMultiplier());return false;}
        multiplier.store(*value,std::memory_order_relaxed);SKSE::log::info("[TOAM][Damage] Loaded common damage multiplier={}",*value);return true;
    }catch(...){return false;}
}
bool Save(float value) noexcept {
    if(!DamagePolicy::Valid(value))return false;
    try {
        std::scoped_lock lock(iniMutex);const auto path=IniPath();if(path.empty())return false;
        wchar_t number[48]{};swprintf_s(number,L"%.6f",static_cast<double>(value));
        if(!WritePrivateProfileStringW(L"Balance",L"DamageMultiplier",number,path.c_str()))return false;
        // The documented cache-flush form returns0 here even on success.
        static_cast<void>(WritePrivateProfileStringW(nullptr,nullptr,nullptr,path.c_str()));
        const auto readback=Read(path);if(!readback||std::abs(*readback-value)>0.00001F)return false;
        multiplier.store(*readback,std::memory_order_relaxed);SKSE::log::info("[TOAM][Damage] Saved common damage multiplier={}",*readback);return true;
    }catch(...){return false;}
}
void RegisterMenu() noexcept {
    static bool registered=false;if(registered)return;
    auto module=GetModuleHandleW(L"SKSEMenuFramework.dll");
    if(!module){SKSE::log::info("[TOAM][DamageMenu] Optional framework absent; INI remains available");return;}
    auto add=reinterpret_cast<AddPageFn>(GetProcAddress(module,"AddSectionItem"));
    text=reinterpret_cast<TextFn>(GetProcAddress(module,"igTextUnformatted"));
    input=reinterpret_cast<InputFn>(GetProcAddress(module,"igInputFloat"));
    slider=reinterpret_cast<SliderFn>(GetProcAddress(module,"igSliderFloat"));
    button=reinterpret_cast<ButtonFn>(GetProcAddress(module,"igSmallButton"));
    sameLine=reinterpret_cast<SameLineFn>(GetProcAddress(module,"igSameLine"));
    checkbox=reinterpret_cast<CheckboxFn>(GetProcAddress(module,"igCheckbox"));
    auto version=reinterpret_cast<float(*)()>(GetProcAddress(module,"GetMenuFrameworkVersion"));
    if(!add||!text||!input||!slider||!button||!sameLine||!checkbox||!version){SKSE::log::warn("[TOAM][DamageMenu] Missing required exports; optional page disabled");return;}
    const float v=version();if(!std::isfinite(v)||v<3.0F||v>=4.0F){SKSE::log::warn("[TOAM][DamageMenu] Unverified framework ABI {}; INI remains available",v);return;}
    add("TULL Ordinary Attack Magic/Damage - 피해 배율",Render);
    add("TULL Ordinary Attack Magic/Flight - 비행 설정",RenderFlight);
    add("TULL Ordinary Attack Magic/Audio - 효과음",RenderAudio);
    add("TULL Ordinary Attack Magic/Camera Shake - 화면 흔들림",RenderCamera);
    add("TULL Ordinary Attack Magic/Circle Colors - 마법진 색상",RenderColors);registered=true;
    SKSE::log::info("[TOAM][DamageMenu] Page registered; framework={}; runtime visual QA pending",v);
}
}
