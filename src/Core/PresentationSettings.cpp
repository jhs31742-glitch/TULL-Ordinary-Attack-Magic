#include "Core/PresentationSettings.h"
#include "Core/DamageSettings.h"
#include "VFX/CircleTint.h"
#include <Windows.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
namespace TOAM::Presentation {
namespace {
std::shared_ptr<const Values> active=std::make_shared<const Values>();
std::array<RE::BGSSoundDescriptorForm*,sources.size()> descriptors{};
std::wstring Path(){wchar_t b[32768]{};const auto n=GetFullPathNameW(L"Data\\SKSE\\Plugins\\TOAM.ini",32768,b,nullptr);return n&&n<32768?b:L"";}
std::wstring Wide(const char* s){std::wstring w;while(*s)w.push_back(wchar_t(*s++));return w;}
struct IniRollback {
 const std::wstring& path;const std::string& bytes;bool existed;bool committed=false;
 ~IniRollback(){
  if(committed)return;static_cast<void>(WritePrivateProfileStringW(nullptr,nullptr,nullptr,path.c_str()));
  if(!existed){DeleteFileW(path.c_str());return;}
  const auto h=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(h!=INVALID_HANDLE_VALUE){DWORD written{};const bool ok=WriteFile(h,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size();if(ok)FlushFileBuffers(h);CloseHandle(h);}
  static_cast<void>(WritePrivateProfileStringW(nullptr,nullptr,nullptr,path.c_str()));
 }
};
bool Read(const std::wstring& p,Values& v){
 auto get=[&](const wchar_t* section,const char* key,const char* fallback)->std::optional<std::string>{wchar_t b[128]{};const auto k=Wide(key),d=Wide(fallback);auto n=GetPrivateProfileStringW(section,k.c_str(),d.c_str(),b,128,p.c_str());if(n>=127)return {};std::string s;for(DWORD i=0;i<n;++i){if(b[i]>127)return {};s+=char(b[i]);}return s;};
 auto num=[&](const wchar_t* section,const char* key,float& target,float hi){auto s=get(section,key,"1.0");if(!s)return false;auto x=Number(*s,0,hi);if(!x)return false;target=*x;return true;};
 if(!num(L"Audio", "Master",v.master,1)||!num(L"CameraShake","Master",v.shake,3))return false;
 for(std::size_t i=0;i<spellCount;++i){if(!num(L"AudioSpell",spellKeys[i],v.spellVolume[i],1))return false;auto s=get(L"CircleColor",spellKeys[i],"Default");if(!s)return false;bool found=false;for(std::size_t c=0;c<colorKeys.size();++c)if(*s==colorKeys[c]){v.colors[i]=Color(c);found=true;break;}if(!found)return false;}
 for(std::size_t i=0;i<eventCount;++i){if(!num(L"AudioEvent",events[i].key,v.eventVolume[i],1))return false;char def[16]{};std::snprintf(def,sizeof(def),"%03u",events[i].original);auto s=get(L"AudioSelect",events[i].key,def);v.selected[i]=s?Select(Event(i),*s):events[i].original;}
 return Valid(v);
}
}
Values Get() noexcept {return *std::atomic_load_explicit(&active,std::memory_order_acquire);}
Color Snapshot(Spell s) noexcept {return Get().colors[std::size_t(s)];}
AudioChoice Resolve(Event e,float original) noexcept {const auto v=Get();return {v.selected[std::size_t(e)],Volume(v,e,original)};}
float CameraStrength(float original) noexcept {return Shake(original,Get().shake);}
RE::BGSSoundDescriptorForm* Descriptor(unsigned id) noexcept {return id&&id<=descriptors.size()?descriptors[id-1]:nullptr;}
void DataLoaded() noexcept {
 if(auto* data=RE::TESDataHandler::GetSingleton())for(std::size_t i=0;i<sources.size();++i)if(sources[i].descriptor)descriptors[i]=data->LookupForm<RE::BGSSoundDescriptorForm>(sources[i].descriptor,"TULL Ordinary Attack Magic.esp");
 VFX::CircleTint::Prepare();
}
bool Reload() noexcept {try{std::scoped_lock lock(Core::DamageSettings::CommonIniMutex());const auto p=Path();Values v;if(p.empty()||!Read(p,v)){logger::warn("[TOAM][Presentation] Invalid INI; previous settings retained");return false;}std::atomic_store_explicit(&active,std::make_shared<const Values>(v),std::memory_order_release);return true;}catch(...){return false;}}
bool Save(const Values& v) noexcept {
 if(!Valid(v))return false;
 try{
  std::scoped_lock lock(Core::DamageSettings::CommonIniMutex());const auto p=Path();if(p.empty())return false;const std::filesystem::path file(p);
  const bool existed=GetFileAttributesW(p.c_str())!=INVALID_FILE_ATTRIBUTES;
  std::ifstream in(file,std::ios::binary);if(existed&&!in.good())return false;std::string old((std::istreambuf_iterator<char>(in)),{});in.close();if(old.size()>4*1024*1024)return false;
  IniRollback rollback{p,old,existed};bool ok=true;
  auto put=[&](const wchar_t* section,const char* key,const char* value){const auto k=Wide(key),s=Wide(value);if(ok)ok=WritePrivateProfileStringW(section,k.c_str(),s.c_str(),p.c_str())!=0;};
  auto number=[&](const wchar_t* section,const char* key,float f){char b[48]{};std::snprintf(b,sizeof(b),"%.6f",double(f));put(section,key,b);};
  number(L"Audio","Master",v.master);number(L"CameraShake","Master",v.shake);
  for(std::size_t i=0;i<spellCount;++i){number(L"AudioSpell",spellKeys[i],v.spellVolume[i]);put(L"CircleColor",spellKeys[i],colorKeys[std::size_t(v.colors[i])]);}
  for(std::size_t i=0;i<eventCount;++i){number(L"AudioEvent",events[i].key,v.eventVolume[i]);char b[16]{};std::snprintf(b,sizeof(b),"%03u",v.selected[i]);put(L"AudioSelect",events[i].key,b);}
  static_cast<void>(WritePrivateProfileStringW(nullptr,nullptr,nullptr,p.c_str()));Values check;
  ok=ok&&Read(p,check)&&std::abs(check.master-v.master)<.00001F&&std::abs(check.shake-v.shake)<.00001F;
  for(std::size_t i=0;i<spellCount;++i)ok=ok&&std::abs(check.spellVolume[i]-v.spellVolume[i])<.00001F&&check.colors[i]==v.colors[i];
  for(std::size_t i=0;i<eventCount;++i)ok=ok&&std::abs(check.eventVolume[i]-v.eventVolume[i])<.00001F&&check.selected[i]==v.selected[i];
  if(!ok){logger::warn("[TOAM][Presentation] Save/readback failed; restoring prior INI; runtime unchanged");return false;}
  std::atomic_store_explicit(&active,std::make_shared<const Values>(check),std::memory_order_release);rollback.committed=true;return true;
 }catch(...){return false;}
}
}
