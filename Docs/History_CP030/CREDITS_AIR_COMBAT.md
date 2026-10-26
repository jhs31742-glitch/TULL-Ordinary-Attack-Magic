# TULL Ordinary Attack Magic — Air combat and animation credits

## 개발 및 재사용 범위 / Development and reuse
사용자 TULL의 설계·아트디렉션 및 Codex(OpenAI)의 구현 지원으로 TOAM 통합, 비행 중 조준, 손별 주문·지팡이 자원 분리, 공중전투 연결과 상승 띠 연출을 개발했습니다. AI 지원 표시는 아래 원작 애니메이션의 저작자를 대체하지 않습니다.
The user TULL directed the design and art. Codex (OpenAI) assisted with Native implementation, integration and debugging. Original Arcana Staffs v0.7.4 FlightStateFix supplies the embedded flight controller and combat integration; its shared visual assets are v2.7.1. TOAM adapts those systems into one Native host.

## Imported motion assets
- 345 unmodified Flying Animations - Resource HKX files; original names, hashes and attribution are recorded in Provenance/animation_provenance070.json and CREDITS_FLIGHT.md.
- 452 derived firearm flight HKX files from the installed Call to Arms and a skrub's Skrub's Guns 3rd Person animations. Six lower-body rotation tracks were adapted from the credited flying resource. Original/derived hashes are in Provenance/gun_animation_provenance073.json.
- Original author credits: HydroBirous / Hydro Birous; Derwan; Redfall6; gp160; Khenta; RavenKZP; QTRModding; a skrub. The collected source notice does not provide a reliable per-file author split for the resource or a verified separate Call to Arms author name. Those unresolved attributions are not invented.
- Ershin: Open Animation Replacer, an external dependency. Its binary is not redistributed in this package.

## Technical references and tools
Flight Framework - SKSE by SkyrimThiago, RavenKZP / QTR-Modding was a technical reference for character-controller behavior. No Flight Framework DLL, ESP, code file, ring or hook implementation is bundled. The actual source commit and links are preserved below.
CommonLibSSE-NG / SKSE / Address Library supply Native interfaces; ret2end/hkxconv, figment/hkxcmd, PyNifly/nifly and Grimrukh's soulstruct-havok spline-layout description were used during authoring/analysis. These tools are not included as game runtime dependencies or claimed as TOAM-original work.
FlightHum_Loop.wav was independently synthesized in Arcana. The new ascent effect reuses the credited halo/outer inscription-band geometry and textures; it is not imported from Flight Framework.

## Rights and audit trail
Original notices and file-level provenance remain verbatim below and in Provenance/. PATH_MIGRATION.json maps the previous TOOM package names into TOAM; that namespace change does not confer asset ownership. For the two animation groups, 345 + 452 = 797 shipped HKX files retain the previous release's exact bytes.
원저작자 크레딧과 사용 조건은 유지합니다. 개인 사용용 파생본과 공개 재배포 허가는 별개이며, 이 문서는 미확인 공개 배포·수익화 권한을 부여하지 않습니다. 사용자가 공개 배포하기 전 미확인 원저작자 조건은 확인이 필요합니다.

## Preserved original flight notice
# Flight animation provenance and reuse
This local QA candidate bundles 345 UNMODIFIED HKX files (8,739,536 bytes) from the user-downloaded Flying Animations - Resource v1. File-level original source, destination, size and SHA256 are in animation_provenance070.json. Original config.json files/meta.ini are not bundled; Arcana's own OAR conditions select the files. No HKX retargeting or animation editing was performed.
Credits: HydroBirous / Hydro Birous, Derwan (Soarin' Over Skyrim), Redfall6 (LEVitation Animations), gp160 (Dragon Priest Animation for Playable Races), Khenta (Hovering Idle and Combat Animations), RavenKZP and QTRModding (shared resource assembly/integration); Ershin (Open Animation Replacer).
Primary sources reviewed 2026-10-24:
- https://www.nexusmods.com/skyrimspecialedition/mods/192243
- https://www.nexusmods.com/skyrimspecialedition/mods/49160
- https://www.nexusmods.com/skyrim/mods/88796
- https://www.nexusmods.com/skyrim/mods/107487
- https://www.nexusmods.com/skyrim/mods/71391
- https://www.nexusmods.com/skyrim/mods/54151
The shared-resource page permits asset use with credit, prohibits selling, requires permission for modifications and Donation Points, and also flags third-party author permissions. The file-specific per-author mapping is not supplied. This document records those conditions; it is not blanket clearance for third-party assets, monetization, or re-uploading the resource itself. Public redistribution/revenue conditions must be resolved by the publisher before publication. This session does not upload or monetize anything.
Flight Framework - SKSE (SkyrimThiago, RavenKZP / QTR-Modding) was a technical reference for the native character-controller approach:
https://www.nexusmods.com/skyrimspecialedition/mods/178943
https://github.com/QTR-Modding/Flight-Framework/tree/9af69514c89db81ec277e2e00be8dfd9d7455c07
No Flight Framework DLL, ESP, code file, config, ring, or hook implementation is bundled. Arcana independently owns velocity control, toggle/lifecycle and magic integration against the pinned CommonLibSSE-NG declarations. Existing Arcana/CommonLib/TOOM asset notices remain applicable. The flight animations do not inherit an Arcana source-code license. OAR is required but its DLL is not redistributed here.

0.7.2 adds six leg rotation curves derived from the bundled female floating idle to a native post-pose correction. Original 345 HKX files remain byte-identical. Original author permission/credit conditions above still apply. This is a local QA candidate, with no public upload or monetization. Tools used for offline analysis: ret2end/hkxconv v0.2.0-alpha (MIT), figment/hkxcmd 1.4, PyNifly/nifly; tool binaries are not bundled in the mod. Generated data provenance is source/flight/leg_pose.json.

## Preserved original firearm flight notice
# 총기 비행 호환 자산 출처
개인 로컬 검증용 파생 모션입니다. 원본 설치본 Call to Arms와 a skrub의 Skrub's Guns 3rd Person 모션을 사용했습니다. 각 파일 원본 경로/SHA256/파생 파일 해시는 gun_animation_provenance073.json에 있습니다. 원본 HKX/스크립트/ESP/설치 폴더는 수정하지 않습니다.
하체 자세의 출처는 기존 Flying Animations Resource입니다. CREDITS_FLIGHT.md의 원저자·사용 조건을 유지합니다. 이 파생본을 공개 업로드/재배포할 권한을 독자적으로 부여하지 않습니다. 공개 배포 전 원저작자 조건의 별도 확인이 필요합니다.
Spline 스트림 레이아웃은 https://github.com/Grimrukh/soulstruct-havok/blob/main/src/soulstruct/havok/spline_compression.py 의 공개 구조 설명을 대조했습니다. 외부 코드를 DLL에 링크하지 않으며 이 작성 도구는 기존 압축 스트림을 보존하고 다리6회전만 교체합니다. 제작 시 hkxcmd/hkxconv를 사용했고 해당 실행파일은 모드에 포함하지 않습니다.
FlightHum_Loop.wav는 Arcana에서 직접 합성한 신규 음원입니다.

## CP029 measured package clarification /2026-10-26
The final physical package has635 HKX files after byte/path deduplication.345 flying-resource plus452 derived gun-flight entries describe797 authored source instances, not797 distinct shipped files. This count does not establish individual author permissions.
Private source repository contains code/research, not these imported animation binaries. Imported flight animation and derived firearm animation require per-file provenance/permission review before Nexus redistribution. The user withdrew Staff SMP; no reference mod content or SMP runtime is included in the final release.
Primary permissions checked2026-10-26:https://www.nexusmods.com/skyrimspecialedition/mods/192243 . The page permits credited asset use, restricts modification/Donation Points and identifies other authors. Its recommended OAR overrideAnimationsFolder route can reuse an external shared resource without copying HKX. This is a possible later packaging solution, not an applied untested rewrite of current flight.
