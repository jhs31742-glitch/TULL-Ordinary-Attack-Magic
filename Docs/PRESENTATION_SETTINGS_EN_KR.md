# TOAM 1.0.1 Settings / 설정 안내
INI:Data/SKSE/Plugins/TOAM.ini in MO2 virtual Data. New sections below coexist with Balance/Flight/Menu and arbitrary custom keys. Optional existing Framework3.x adds Audio/Camera Shake/Circle Colors under TOAM with the existing EN/KR language toggle. Existing Damage/Flight pages remain. No new context,renderer,hotkey,frame hook or Framework binary.
Audio/Master,AudioSpell/<spell>,AudioEvent/<event>:finite0..1,default1. Effective volume retains original gain;0=mute. AudioSelect/<event> uses stable3-digit IDs in AUDIO_CATALOG. Loops and one-shots cannot mix. Invalid IDs restore that event's default.
CameraShake/Master:finite0..3,default1;menu step.05;0 skips owned requests. Engine attenuation and final0..1 clamp remain;duration/timing unchanged. Existing owners are Sun release,Hermit burst,Tower escalation/impact/finale. Other spells have no newly invented shake.
CircleColor/<spell>:Default,Blue,Red,Gold,Purple(case-sensitive INI names).13 independent keys. Default is original1.0.0,not neutral grey. Existing circles retain snapshots. See mapping for long-lived Halo/Fool.
Apply and save writes only these settings,flushes and rereads,then atomically publishes all values. Shared INI mutex prevents competition with existing menu saves. Reset restores that page's draft;Apply saves. Reload reads the INI. Invalid volume/camera/color rejects the entire reload and keeps previous valid values. On initial bootstrap previous values are defaults. Invalid source ID alone falls back to its original. Failed save rolls back prior INI bytes and keeps active settings. Without Framework,edit INI then restart;with Framework use Reload INI. No playback-time INI/file scans.
MO2 follows the existing absolute GetFullPathName(Data/...) and Windows profile write policy. VFS redirection/Overwrite placement is controlled by MO2;actual new writes under VFS remain user runtime QA. The host test exercised actual Win32 save/reload and custom-key retention outside the game,not a claim about MO2 VFS.

한국어:볼륨은 전체×마법×실제 이벤트×원래 Gain입니다. 기본1.00,0은 음소거,최대1.00입니다. 기존 부유음 HumVolume도 적용합니다. 재생 특성과 백엔드가 맞는 기존 음원만 ID로 선택하며 잘못된 ID는 원래 음원으로 돌아갑니다.
화면 흔들림은0~3,메뉴간격.05이며0이면 요청을 생략합니다. 엔진 상한 때문에 고배율이 항상3배 체감이 되는 것은 아닙니다.
색상은 마법별 독립이며 새 시전·효과 소유자부터 적용합니다. Default는 원래 외형입니다. 절제 헤일로·광대 비행은 껐다 켤 때 새 값을 고정합니다. 저장 실패·잘못된 볼륨/색은 이전 유효값을 유지합니다. 기본값 복원 후 적용을 누르면 저장됩니다. Framework가 없으면 INI 편집 후 게임을 다시 시작합니다.

## Complete default sections / 전체 신규 기본값
```ini
[Audio]
Master=1.00

[AudioSpell]
Magician=1.00
Judgement=1.00
World=1.00
Temperance=1.00
Emperor=1.00
Hermit=1.00
Chariot=1.00
Sun=1.00
Strength=1.00
Justice=1.00
Star=1.00
Tower=1.00
Fool=1.00

[AudioEvent]
MagicianLaunch=1.00
MagicianHeavyLaunch=1.00
JudgementShot=1.00
WorldRelease=1.00
TemperanceProc=1.00
EmperorShot=1.00
ChariotShot=1.00
StrengthStart=1.00
StrengthLoop=1.00
SunRelease=1.00
SunCancel=1.00
StarShot=1.00
JusticeLoop=1.00
TowerShot=1.00
HermitExplosion=1.00
TowerExplosion=1.00
FoolHum=1.00

[AudioSelect]
MagicianLaunch=001
MagicianHeavyLaunch=002
JudgementShot=003
WorldRelease=002
TemperanceProc=004
EmperorShot=005
ChariotShot=005
StrengthStart=011
StrengthLoop=006
SunRelease=016
SunCancel=012
StarShot=005
JusticeLoop=006
TowerShot=013
HermitExplosion=017
TowerExplosion=017
FoolHum=018

[CameraShake]
Master=1.00

[CircleColor]
Magician=Default
Judgement=Default
World=Default
Temperance=Default
Emperor=Default
Hermit=Default
Chariot=Default
Sun=Default
Strength=Default
Justice=Default
Star=Default
Tower=Default
Fool=Default
```
