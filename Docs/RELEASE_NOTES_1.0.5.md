# TOAM 1.0.5 — Experimental AE 1.6.1170 support

## English
One TOAM.dll targets Skyrim SE 1.5.97.0 and AE 1.6.1170.0. AE support is experimental: static checks and the Windows x64 compile/link passed, but in-game compatibility, Experience XP awards, VFX and stability have not been verified. Other AE runtimes, Skyrim 1.7.x and VR remain blocked.

Includes the 1.0.4 attacker/kill-credit improvement via Actor::DoDamage and the approved UltraRare dungeon drop patch. Main books: 0.1%; main staffs: 0.03%; Stances books: 0.001%; Stances staffs: 0.0002%. These are patch target probabilities, not measured rates for every container. Merchant distribution and CID INIs are unchanged.

Install Main only, plus the optional Stances Addon if desired. Use the SKSE64 and Address Library packages matching your exact game runtime. Keep existing TOAM.ini settings when upgrading. English ESPs are the defaults; Korean ESP alternatives are under Language/kr. Do not load duplicate legacy host DLLs.

## 한국어
SE 1.5.97 및 AE 1.6.1170을 대상으로 하는 시험 버전입니다. 정적 검사와 x64 빌드는 통과했으나, AE 실게임 호환성·Experience 경험치 지급·VFX·안정성은 미검증입니다. 그 외 AE 런타임, Skyrim 1.7.x 및 VR은 차단됩니다.

1.0.4 처치 귀속 개선과 기존 던전 저확률 패치를 통합했습니다. 일반 책 0.1%·지팡이 0.03%, Stances 책 0.001%·지팡이 0.0002%이며, 상인 유통과 CID INI는 변경하지 않았습니다. 각 확률은 기존 패치의 목표값으로 컨테이너별 실측 확률은 아닙니다.

메인 파일을 설치하고 Stances 애드온은 필요할 때 별도 설치하십시오. 런타임에 맞는 SKSE64와 Address Library를 사용하고, 기존 TOAM.ini 설정을 보존하십시오. 한국어 ESP는 Language/kr에 있습니다.

## Verification and source scope
- Native source: 113 C++/header/include files byte-identical to the verified local 1.0.5 candidate; pinned dependencies unchanged.
- DLL SHA256: `A8B407613E08223EBE995A08EFF17C2C591E37F50FDE3AF0C8E65FC32AB0E058`
- Build: MSVC Hostx64/x64, existing Ninja build; actual compile/link passed.
- Runtime: NOT_TESTED. Final LOCK: NO. Publication does not certify stability.
- The repository's DataSources/PluginRecords and RUNTIME_ASSETS_MANIFEST remain the historical 1.0.3 authoring snapshot. The approved low-drop ESPs are included in the attached 1.0.5 install archives. Their original 1.0.3/SE-only header descriptions are preserved; the current native gate and this notice define the experimental runtime scope.
- Source code is available at this release tag and GitHub's automatic source archives. Submodule contents are not included in source archives; use the pinned revisions in DEPENDENCIES.lock.json.
- No game executable, Address Library database, SKSE distribution, private native audit, personal log, or internal handoff is published.

## Install archive SHA256
```text
8CD679B7D47ECE679F298D74C2054B011A9718F59F0F3D777549760BAF3E3AE2  TOAM_1.0.5_SE_AE_Main.zip
1AA107D582D0AF70621B51FA6FBEECA9CD8172B34339D375C03CD935B1B69649  TOAM_1.0.5_SE_AE_Stances_Addon.zip
```

Third-party material retains its original terms. Existing credits, license scope and notices remain applicable. This prerelease does not change the immutable CP032 anchor, installed mods, MO2 profiles, activation or saves.
