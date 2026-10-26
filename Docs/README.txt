TULL Ordinary Attack Magic 1.0.1

ENGLISH
Requires Skyrim SE1.5.97,SKSE64 2.0.20,SE Address Library,BEES,Open Animation Replacer and the x64 Visual C++runtime. AE/VR are not supported by this Native build.
For merchant/dungeon supplies install Container Item Distributor2.1.3+ and powerofthree Tweaks. Tomes are cheap and randomly stocked by many vendors; dungeon chests,urns and other eligible loot containers have lower rates. Staffs are soul-gem rechargeable;normal charge20000,Unbound40000;staff playerMagicka cost0.
Optional SKSE Menu Framework3.x adds Damage/Flight/Audio/Camera Shake/Circle Colors pages with English/Korean toggle. Without it edit SKSE/Plugins/TOAM.ini. DamageMultiplier defaults1.0. Temperance,Limiter,Hermit defense,resource costs and shot counts remain fixed. Flight: Jump ascends,Sneak descends,Sprint boosts.
Root ESPs are English. Language/en and Language/kr keep matching variants;copy the chosen ESP to the mod root. Optional Stances Addon requires the main mod and Stances - Dynamic Weapon Movesets SE.esp with its upstream dependencies. No standalone Arcana DLL/ESP or SMP option is used.
Source:https://github.com/jhs31742-glitch/TULL-Ordinary-Attack-Magic

한국어
Skyrim SE1.5.97,SKSE64 2.0.20,SE Address Library,BEES,OAR,VC++x64가 필요합니다. 이 Native 빌드는 AE/VR을 지원하지 않습니다.
상점·던전 자동 배포에는 CID2.1.3 이상과 powerofthree Tweaks가 필요합니다. 마법책·지팡이는 확률로 등장하며 던전 출현율은 낮습니다. 지팡이는 소울젬으로 충전하고 플레이어MP를 쓰지 않습니다.
선택 사항인 SKSE Menu Framework3.x에서 피해·비행·효과음·화면 흔들림·마법진 색상 설정 및 한국어 전환을 제공합니다. 메뉴가 없으면 TOAM.ini를 편집하세요. 비행 중 점프=상승,은신=하강,질주=가속입니다.
Language/kr 또는 en의 ESP를 루트에 복사해 언어를 선택합니다. 스탠스 애드온은 본체와 실제 Stances 선행 모드가 필요합니다. 구 SMP 옵션은 사용하지 않습니다.

1.0.1 SETTINGS / 신규 설정
Audio:Master x spell x event x original gain,0.00..1.00(default1). AudioSelect uses stable shipped sound IDs; incompatible IDs restore original. Flight/HumVolume remains an additional factor. No preview or external audio registration.
CameraShake/Master:0.00..3.00(default1,menu step.05).0 skips owned requests;duration unchanged. Skyrim clamps the final strength,so already strong events can saturate.
CircleColor:Default/Blue/Red/Gold/Purple,independent per spell. Default retains the original1.0.0 material;selection is fixed at cast/effect creation. Unsafe private-material checks retain the original appearance. Existing NIF/DDS/ESP bytes are preserved.
Menu:Apply and save;reset draft then apply;reload INI. Invalid values retain the previous valid settings. Detailed EN/KR settings/catalog and one-boot QA accompany the delivery/source. No Framework is needed for INI configuration.
효과음:전체×마법×이벤트×원래Gain,0.00~1.00(기본1). 고정 ID로 기존 포함 음원만 선택하며 미리듣기·외부 음원 등록은 없습니다. 기존 HumVolume도 함께 적용합니다.
흔들림:0.00~3.00(기본1,메뉴간격.05).0은 요청 생략입니다. 엔진 상한 때문에 강한 이벤트는 고배율에서 포화될 수 있습니다.
마법진:마법별 독립 Default/Blue/Red/Gold/Purple. Default는1.0.0원본이며 새 시전·효과 생성 때 선택을 고정합니다. 개별 재질 안전 검사 실패 시 원본을 유지합니다.
1.0.0/CP032 remains the immutable anchor.1.0.1 user runtime QA and final LOCK are pending;SE1.5.97 only.
