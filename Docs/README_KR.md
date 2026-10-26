# TULL Ordinary Attack Magic v0.8.11

본체 ZIP을 MO2의 TULL Ordinary Attack Magic 모드로 설치하고 동명의 본체 ESP를 활성화합니다. Skyrim SE1.5.97, 해당 SKSE와 Address Library, Open Animation Replacer가 필요합니다. AE와 VR은 지원을 활성화하지 않았습니다.
단일 Native 호스트는 SKSE/Plugins/TOAM.dll, 설정은 TOAM.ini, 로그는 OrdinaryAttackMagic.log입니다. 구 TOOM 및 독립 Arcana DLL을 중복 활성화하지 마십시오.

스탠스 애드온은 별도 선택 ZIP입니다. 본체와 실제 Stances - Dynamic Weapon Movesets SE.esp를 먼저 설치하십시오. 애드온의 필수 마스터는 Skyrim.esm, TULL Ordinary Attack Magic.esp, Stances - Dynamic Weapon Movesets SE.esp입니다. 현재 확인한 Stances 자체는 Update.esm, Dawnguard.esm, Dragonborn.esm, Keytrace.esp도 요구합니다. 해당 모드의 선행 조건을 따르십시오.
여사제·달 주문과 최대40000 충전량의 해방 지팡이를 제공합니다. 시전 중 스탠스를 변경해도 현재 시전의 비용·모드는 고정되며 다음 시전에 새 스탠스가 적용됩니다.

루트 기본 ESP는 영어입니다. Language/en에 영어 원본, Language/kr에 한국어 원본을 보관했습니다. 원하는 언어의 본체·애드온 ESP를 각각 루트로 복사하십시오. 언어 폴더를 보존하므로 되돌릴 수 있습니다. 언어판은 표시문만 다릅니다.
공격 마법책:일반공격마법: 아르카나 - 카드명 / Ordinary Attack Magic: Arcana - Card.
공중부양 마법책:일반교양마법: 아르카나 - 광대 (공중부양) / Ordinary General Magic: Arcana - Fool (Levitation).
반지:술식 출력 제한 반지 / Arcana Limiter.

일반 지팡이 충전량20000, 해방 지팡이40000이며 소울젬 재충전이 가능합니다. 플레이어 매지카는 소모하지 않습니다. 고유 인챈트 보호 설정은 유지합니다.
0.8.11에서 Arcana8종 기본 비용을 모두3배로 높였으며 바닐라 파괴마법 비용 감소를 적용합니다. 정의는3초간0.2초마다110 피해, 태양 최대3040, 탑 빔당150·폭발900·대상별상한27900입니다. 상세는 BALANCE.md를 참조하십시오. TOAM 네 주문의 피해·비용은 변경하지 않았습니다.

SMP 옵션은 철회하여 최종 배포에서 제외했습니다. 과거 Staff SMP 모드와 관련 ESP/패치를 비활성화하십시오. 지팡이 천은 고정 모델입니다.
CP028 핵심 전투·리미터·동시 사용은 사용자 실측 정상입니다. 이번 CP029 비용·피해 및 애드온 마스터 변경은 게임 미검증입니다. 기존158문항은 그대로 보존하며 변경 부분만 다음 수동 QA에서 확인합니다.
GitHub는 비공개로 먼저 보관하고 넥서스 배포 시 함께 공개합니다. 외부 자산의 배포 조건을 확인하기 전 바이너리를 공개 업로드하지 않습니다. 출처와 권리 상태는 크레딧 문서에 명시했습니다.
