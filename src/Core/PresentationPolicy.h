#pragma once
#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>
namespace TOAM::Presentation {
enum class Spell : std::uint8_t { Magician,Judgement,World,Temperance,Emperor,Hermit,Chariot,Sun,Strength,Justice,Star,Tower,Fool,Count };
enum class Color : std::uint8_t { Default,Blue,Red,Gold,Purple,Count };
enum class Event : std::uint8_t { MagicianLaunch,MagicianHeavyLaunch,JudgementShot,WorldRelease,TemperanceProc,EmperorShot,ChariotShot,StrengthStart,StrengthLoop,SunRelease,SunCancel,StarShot,JusticeLoop,TowerShot,HermitExplosion,TowerExplosion,FoolHum,Count };
inline constexpr std::size_t spellCount=std::size_t(Spell::Count),eventCount=std::size_t(Event::Count);
inline constexpr std::array<const char*,spellCount> spellKeys{"Magician","Judgement","World","Temperance","Emperor","Hermit","Chariot","Sun","Strength","Justice","Star","Tower","Fool"};
inline constexpr std::array<const char*,spellCount> spellKR{"마법사","심판","세계","절제","황제","은둔자","전차","태양","힘","정의","별","탑","광대"};
inline constexpr std::array<const char*,spellCount> spellDisplayEN{"I Magician","XX Judgement","XXI World","XIV Temperance","IV Emperor","IX Hermit","VII Chariot","XIX Sun","VIII Strength","XI Justice","XVII Star","XVI Tower","0 Fool"};
inline constexpr std::array<const char*,spellCount> spellDisplayKR{"I 마법사","XX 심판","XXI 세계","XIV 절제","IV 황제","IX 은둔자","VII 전차","XIX 태양","VIII 힘","XI 정의","XVII 별","XVI 탑","0 광대"};
struct PresetRGB {float red,green,blue;};
inline constexpr std::array<PresetRGB,5> colorRGB{{{1,1,1},{.30F,.60F,1.F},{1.F,.16F,.12F},{1.F,.67F,.18F},{.68F,.25F,1.F}}};
inline constexpr std::array<const char*,5> colorKeys{"Default","Blue","Red","Gold","Purple"};
struct SoundSource { const char* name;const char* korean;const char* path;std::uint32_t descriptor;bool loop; };
// Stable IDs are the one-based positions. Append only; never renumber.
inline constexpr std::array<SoundSource,18> sources{{
 {"I Magician launch","I 마법사 발사","sound\\fx\\TOAM\\Spells\\Arcana_Magician_Launch.xwm",0,false},
 {"I Magician heavy launch","I 마법사 강한 발사","sound\\fx\\TOAM\\Spells\\Arcana_Magician_HeavyLaunch.xwm",0,false},
 {"XX Judgement launch","XX 심판 발사","sound\\fx\\TOAM\\Spells\\Arcana_Judgement_Launch.xwm",0,false},
 {"XIV Temperance proc","XIV 절제 발동","sound\\fx\\TOAM\\Spells\\Arcana_Temperance_Proc.xwm",0,false},
 {"IV Emperor shot","IV 황제 발사","sound\\fx\\TOAM\\Arcana\\Arcana_Emperor_Shot.wav",0xA51,false},
 {"VIII Strength loop","VIII 힘 지속음","sound\\fx\\TOAM\\Arcana\\Arcana_Strength_Loop.wav",0xA52,true},
 {"Formation (silent)","전개음 (무음 원본)","sound\\fx\\TOAM\\Arcana\\Arcana_Formation.wav",0xA53,false},
 {"Charge loop (silent)","충전 지속음 (무음 원본)","sound\\fx\\TOAM\\Arcana\\Arcana_Charge_Loop.wav",0xA54,true},
 {"XIX Sun release","XIX 태양 발사 원본","sound\\fx\\TOAM\\Arcana\\Arcana_Sun_Release.wav",0xA55,false},
 {"XIX Sun charge","XIX 태양 충전음","sound\\fx\\TOAM\\Arcana\\Arcana_Sun_Charge.wav",0xA56,true},
 {"VIII Strength start","VIII 힘 시작음","sound\\fx\\TOAM\\Arcana\\Arcana_Strength_Start.wav",0xA57,false},
 {"VIII Strength end","VIII 힘 종료음","sound\\fx\\TOAM\\Arcana\\Arcana_Strength_End.wav",0xA58,false},
 {"XVI Tower shot","XVI 탑 발사","sound\\fx\\TOAM\\Arcana\\Arcana_Tower_Shot.wav",0xA59,false},
 {"IX Hermit detonation","IX 은둔자 폭발 원본","sound\\fx\\TOAM\\Arcana\\Arcana_Hermit_Detonation.wav",0xC77,false},
 {"IX Hermit aftershock","IX 은둔자 여진 원본","sound\\fx\\TOAM\\Arcana\\Arcana_Hermit_Aftershock.wav",0xC78,false},
 {"XIX Sun impact","XIX 태양 충격음","sound\\fx\\TOAM\\Arcana\\Arcana_Sun_Impact.wav",0xC85,false},
 {"Burst explosion","공통 폭발","sound\\fx\\TOAM\\Arcana\\Arcana_Burst_Explosion.wav",0xC88,false},
 {"0 Fool hover","0 광대 부유음","sound\\fx\\TOAM\\Arcana\\Arcana_Fool_Hover.wav",0xCA9,true}
}};
struct EventDefinition {const char* key;const char* korean;Spell spell;unsigned original;};
inline constexpr std::array<EventDefinition,eventCount> events{{
 {"MagicianLaunch","마법사 발사",Spell::Magician,1},{"MagicianHeavyLaunch","마법사 강한 발사",Spell::Magician,2},
 {"JudgementShot","심판 포격",Spell::Judgement,3},{"WorldRelease","세계 해제",Spell::World,2},{"TemperanceProc","절제 발동",Spell::Temperance,4},
 {"EmperorShot","황제 발사",Spell::Emperor,5},{"ChariotShot","전차 발사",Spell::Chariot,5},
 {"StrengthStart","힘 시작",Spell::Strength,11},{"StrengthLoop","힘 지속",Spell::Strength,6},
 {"SunRelease","태양 발사",Spell::Sun,16},{"SunCancel","태양 취소",Spell::Sun,12},
 {"StarShot","별 발사",Spell::Star,5},{"JusticeLoop","정의 지속",Spell::Justice,6},{"TowerShot","탑 발사",Spell::Tower,13},
 {"HermitExplosion","은둔자 폭발",Spell::Hermit,17},{"TowerExplosion","탑 폭발",Spell::Tower,17},{"FoolHum","광대 부유",Spell::Fool,18}
}};
inline constexpr std::array<const char*,eventCount> eventDisplayEN{"I Magician Launch", "I Magician Heavy Launch", "XX Judgement Shot", "XXI World Release", "XIV Temperance Proc", "IV Emperor Shot", "VII Chariot Shot", "VIII Strength Start", "VIII Strength Loop", "XIX Sun Release", "XIX Sun Cancel", "XVII Star Shot", "XI Justice Loop", "XVI Tower Shot", "IX Hermit Explosion", "XVI Tower Explosion", "0 Fool Hum"};
inline constexpr std::array<Spell,8> modeSpell{Spell::Emperor,Spell::Hermit,Spell::Chariot,Spell::Sun,Spell::Strength,Spell::Justice,Spell::Star,Spell::Tower};
constexpr bool Compatible(Event e,unsigned id){return std::size_t(e)<eventCount&&id>0&&id<=sources.size()&&(std::size_t(e)<5||sources[id-1].descriptor!=0)&&sources[id-1].loop==sources[events[std::size_t(e)].original-1].loop;}
struct Values {
 float master=1,shake=1;std::array<float,spellCount> spellVolume{};std::array<float,eventCount> eventVolume{};std::array<unsigned,eventCount> selected{};std::array<Color,spellCount> colors{};
 constexpr Values(){spellVolume.fill(1);eventVolume.fill(1);for(std::size_t i=0;i<eventCount;++i)selected[i]=events[i].original;}
};
inline bool Valid(const Values& v){
 if(!std::isfinite(v.master)||v.master<0||v.master>1||!std::isfinite(v.shake)||v.shake<0||v.shake>3)return false;
 for(float f:v.spellVolume)if(!std::isfinite(f)||f<0||f>1)return false;
 for(std::size_t i=0;i<eventCount;++i)if(!std::isfinite(v.eventVolume[i])||v.eventVolume[i]<0||v.eventVolume[i]>1||!Compatible(Event(i),v.selected[i]))return false;
 for(auto c:v.colors)if(c>=Color::Count)return false;return true;
}
inline std::optional<float> Number(std::string_view s,float lo,float hi){
 auto first=s.find_first_not_of(" \t"),last=s.find_last_not_of(" \t");if(first==s.npos)return {};s=s.substr(first,last-first+1);float f{};
 auto r=std::from_chars(s.data(),s.data()+s.size(),f);if(r.ec!=std::errc{}||r.ptr!=s.data()+s.size()||!std::isfinite(f)||f<lo||f>hi)return {};return f;
}
inline unsigned Select(Event e,std::string_view s){unsigned id{};auto r=std::from_chars(s.data(),s.data()+s.size(),id);return r.ec==std::errc{}&&r.ptr==s.data()+s.size()&&Compatible(e,id)?id:events[std::size_t(e)].original;}
inline float Volume(const Values& v,Event e,float original){const auto i=std::size_t(e);return original*v.master*v.spellVolume[std::size_t(events[i].spell)]*v.eventVolume[i];}
// SE ID32275 attenuates then clamps strength to0..1. Bound requests without changing duration.
inline float Shake(float original,float master){return std::isfinite(original)&&original>0&&std::isfinite(master)&&master>0?(original*master>3?3:original*master):0;}
}
