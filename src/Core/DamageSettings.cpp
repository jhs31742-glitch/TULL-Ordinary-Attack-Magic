#include "Core/DamageSettings.h"
#include "Core/DamagePolicy.h"
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
TextFn text=nullptr;InputFn input=nullptr;SliderFn slider=nullptr;ButtonFn button=nullptr;SameLineFn sameLine=nullptr;
void __stdcall Render(){
    static float draft=GetMultiplier();static std::string status;
    text("One common multiplier for offensive spell and staff damage.",nullptr);
    text("Temperance, marking damage, costs, limiter and Hermit defense stay fixed.",nullptr);
    slider("Damage multiplier",&draft,0.1F,10.0F,"%.2fx",0);
    input("Exact value (0.01 - 100)",&draft,0.1F,1.0F,"%.3f",0);
    if(button("Apply and save INI"))status=Save(draft)?"Saved. New releases/casts use this value.":"Save failed or invalid value. Active setting retained.";
    sameLine(0,-1);
    if(button("Reset to 1x")){draft=1.0F;status=Save(draft)?"Default saved.":"Save failed. Active setting retained.";}
    sameLine(0,-1);
    if(button("Reload INI")){const bool ok=Reload();draft=GetMultiplier();status=ok?"INI reloaded.":"Invalid/unreadable INI. Active setting retained.";}
    char active[80]{};std::snprintf(active,sizeof(active),"Active damage multiplier: %.3fx",GetMultiplier());text(active,nullptr);
    if(!status.empty())text(status.c_str(),nullptr);
}
}
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
    auto version=reinterpret_cast<float(*)()>(GetProcAddress(module,"GetMenuFrameworkVersion"));
    if(!add||!text||!input||!slider||!button||!sameLine||!version){SKSE::log::warn("[TOAM][DamageMenu] Missing required exports; optional page disabled");return;}
    const float v=version();if(!std::isfinite(v)||v<3.0F||v>=4.0F){SKSE::log::warn("[TOAM][DamageMenu] Unverified framework ABI {}; INI remains available",v);return;}
    add("TULL Ordinary Attack Magic/Damage",Render);registered=true;
    SKSE::log::info("[TOAM][DamageMenu] Page registered; framework={}; runtime visual QA pending",v);
}
}
