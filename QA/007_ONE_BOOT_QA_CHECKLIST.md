<!-- TOOM_QA1_CP011_POSTQA_RECOVERY_20261024 -->
# CP011 — 통합 QA 중간 관찰, 기존 158문항 유지

현재 Production CP009 P0-B DLL:
61A9862F148FF165AD31355DD86CAA7DD9ACB9E2948AD67E27202514BA5E2DA5

2026-10-24 사용자 관찰: 빔·사운드·착탄과 절제 헤일로는 이전보다 개선.
열린 결함: Magician 마법진 손 추종 의심, World 마법진 위치 불안정/미표시, Judgement 마법진 배치 실패.
Temperance 헤일로는 참고 이미지보다 큼. Temperance/Judgement는 시체에도 반응.
Judgement 충전 얇은 조준선은 신규 요구이며 아직 미구현.
로그의 CIRCLE_POSE READY/WAIT와 HALO 3D_READY는 실제 화면 성공 판정이 아니다.
아래 기존 A~J 158개 문항과 원래 체크 상태는 변경하지 않는다.
본 CP011은 문서 전용. RUNTIME=PARTIAL_PASS_WITH_OPEN_DEFECTS. LOCK=NO.

---
<!-- TOOM_QA1_CP010_PREQA_RECOVERY_20261024 -->
# 현재 One-Boot QA 기준 — QA1-CP010

- 구현 권위: QA1-CP009 P0-B (SOURCE/BUILD/DEPLOY/DOCSYNC PASS).
- 배포 DLL SHA256: 61A9862F148FF165AD31355DD86CAA7DD9ACB9E2948AD67E27202514BA5E2DA5.
- ESP SHA256: 934D78504EBF75DD3397C3FE1D4296C0A9609F81DC65C7BEB58C81C19E65709A.
- P0-A 기본 ARTO 빔 폭·표시 소유권 및 P0-B 마법진 행렬·절제 헤일로 보정이 포함됨.
- 이전 게임 영상 결과: VFX 부분 실패. CP009 P0-A/B 적용 후 실제 시각 출력은 아직 미검증.
- 아래에 보존된 옛 '현재 빌드' 항목은 역사적 기록이며 최신 배포본이 아니다.
- A~J 원래 158개 문항·순서·O/X/- 수동 판정 및 P 비채점 명령을 변경하지 않는다.
- 우선 관찰: 마법사 8겹 배치·빔, 심판 마법진/포격/사운드, 세계 4/8/12 반구·빔·구름, 절제 C2 헤일로 ON/OFF와 잔류.
- 실제 화면에서 보이지 않는다면 READY/SPAWN 로그가 있어도 VFX는 X로 기록한다.
- 영상의 해당 시각과 SKSE 로그 시각을 구분해 기록한다. 일대일 대응이 확인되지 않으면 추정하지 않는다.
- 이번 CP010은 문서 갱신일 뿐 게임 테스트 PASS나 Stage LOCK을 뜻하지 않는다.


---
# TOOM — Skyrim SE 1.5.97 One-Boot QA

> 활성 문서: `007_ONE_BOOT_QA_CHECKLIST.md`
> 작성: 2026-10-19 / O=정상, X=실패, -=미시험
> BUILD PASS / MO2 미배포 / Runtime QA 미실시

## 현재 빌드

- DLL: `OrdinaryOffensiveMagic.dll`
- 크기: 761,856 bytes
- SHA256: `85C8E597284A186CE24A2FECB653F7043B3805D7706AFA9459EE9244ECFB2443`
- 배포 후 빌드 DLL과 MO2 DLL의 해시 일치 확인 필수

## 문서 권위와 판정

- 게임플레이 수치는 `004_SPELL_GAMEPLAY_AUTHORITY.md`가 권위다.
- 마법진 배치 및 좌표는 `005_FORMATION_COORDINATE_AUTHORITY.md`가 권위다.
- 이 문서는 기능 시험 절차이며 설계 정본을 변경하지 않는다.
- 사용자 수동 O/X/- 및 기존 SKSE `OrdinaryOffensiveMagic.log`를 대조한다.
- 별도 QA 수집기, qa.enable, 자동 O/X 판정 및 증거 ZIP 도구를 만들지 않는다.
- 계획 피해량, 피해 적용 건수, 실제 대상 HP 감소량을 구분한다.
- 청각 및 VFX의 방향·크기·잔류는 사용자가 직접 확인한다.
- `S08TRACE` 512건 제한 때문에 절제 쌍수·다중 접촉부터 우선 시험한다.
- 실제 게임 시험 전까지 전체 항목을 미시험으로 유지한다.

## 시험 정보

- 날짜 / QA 전용 세이브:
- Skyrim / SKSE 버전:
- 개발 DLL SHA256 / 배포 DLL SHA256:
- ESP SHA256 / 활성화 여부:
- 플레이어 Health / Magicka / Destruction / MagickaRate:
- Arcana Limiter 착용 여부 / Temperance ON/OFF:
- 대상 Base FormID / Reference ID / HP 전후:

## [P] 콘솔 명령 — 시험 준비 참고용, O/X 채점 제외

| 목적 | 콘솔 명령 |
| --- | --- |
| 기본 능력치 | `player.getav health`, `player.getav magicka`, `player.getav destruction`, `player.getav magickarate` |
| 고매지카 | `player.setav magicka 12000` 후 `player.restoreav magicka 99999` |
| 현재 MP 0 | `player.damageav magicka 99999` |
| 현재 MP 100 회복 | `player.restoreav magicka 100` |
| MP 전체 회복 | `player.restoreav magicka 99999` |
| MP 자연회복 제한 | 기존 수치 기록 후 `player.setav magickarate 0` |
| 플레이어 고체력 | `player.setav health 3000` 후 `player.restoreav health 99999` |
| 전투 AI 토글 | `tcai` |
| 선택적 불사 토글 | `tim` |
| 드로거 1마리 | `player.placeatme 000387C0 1` |
| 드로거 3마리 | `player.placeatme 000387C0 3` |
| 화염 아트로나크 | `player.placeatme 00023AA6 1` |
| 대상 HP 조회 | 콘솔에서 적 Reference 선택 후 `getav health`, `getavinfo health` |
| 대상 고체력 | 적 Reference 선택 후 `setav health 10000`, `restoreav health 99999` |
| 파괴마법 20/40/60 | `player.setav destruction 20`, `40`, `60`을 각각 시험 |

**주의:** `tgm`은 MP 소비 검증에 사용하지 않는다. `tcai`와 `tim`은 토글이므로 실제 상태를 확인한다. `MagickaRate=0`에서는 자연회복 시험 전에 원래 값을 복원한다. 콘솔 변경은 QA 전용 세이브에서 진행하고 일반 플레이는 기존 세이브로 복귀한다.

## QA 판정 기준

- **O:** 실제 동작과 기대 결과가 일치하고 필요한 내부 로그가 확보됨.
- **X:** 기대 동작 실패, 잘못된 수치, CTD, 자원 이중 결제 등.
- **-:** 미시험 또는 근거 부족.
- 발생하지 않은 오류·거부 로그가 없다는 이유만으로 X를 부여하지 않는다.

## [A] 배포·부팅·기본 등록 (10개)

- **A01** [ ] 빌드 DLL과 MO2 배포 DLL SHA256 일치
- **A02** [ ] ESP와 DLL이 모두 MO2에서 활성화됨
- **A03** [ ] Skyrim SE 1.5.97 정상 부팅
- **A04** [ ] 게임 부팅 중 CTD 없음
- **A05** [ ] 기존 저장 데이터 정상 불러오기
- **A06** [ ] SKSE 플러그인 로드 정상
- **A07** [ ] DataLoaded 및 PostLoadGame 처리 정상
- **A08** [ ] 필수 주문 Form 4/4 정상 확인
- **A09** [ ] Magician / Judgement / The World / Temperance 등록 확인
- **A10** [ ] 주문 선택·해제 및 교체 정상

## [B] 공통 충전·Magicka 관리 (16개)

- **B01** [ ] 주문 충전 시작 정상
- **B02** [ ] 충전 중 Magicka 정상 감소
- **B03** [ ] 충전 종료 시 누적 충전량 정상
- **B04** [ ] 충전 취소 시 예정 공격 미발동
- **B05** [ ] Magicka 0 도달 시 충전 상태 유지
- **B06** [ ] Magicka 회복 후 충전 재개
- **B07** [ ] 한손 충전 500 MP/s 요청 확인
- **B08** [ ] 같은 주문 양손 충전 1000 MP/s 요청 확인
- **B09** [ ] 동일 주문 양손 충전 세션 통합 정상
- **B10** [ ] Magician + Judgement 서로 독립적인 양손 세션 정상
- **B11** [ ] 교차 충전 총 요청 1000 MP/s 및 실제 결제 정상
- **B12** [ ] MP 부족 시 양 세션 비례 배분 정상
- **B13** [ ] 한쪽 주문 해제 시 다른 세션 유지
- **B14** [ ] Magicka 이중 결제 없음
- **B15** [ ] 충전 시작 시 CostMult 스냅샷 정상
- **B16** [ ] 충전 해제 시 누적 요청·배분·결제 로그 확인

## [C] The Magician / 마법사 (22개)

- **C01** [ ] T1 100 MP·광선 100 고정피해
- **C02** [ ] T2 300 MP·광선 300 고정피해
- **C03** [ ] T3 600 MP·광선 600 고정피해
- **C04** [ ] T4 1200 MP·광선 1200 고정피해
- **C05** [ ] T5 2000 MP·광선 2000 고정피해
- **C06** [ ] T6 3000 MP·광선 3000 고정피해
- **C07** [ ] T7 4000 MP·광선 4000 고정피해
- **C08** [ ] T8 5000 MP·광선 5000 고정피해
- **C09** [ ] 티어 사이 피해 보간 없음
- **C10** [ ] T8 고정 충전량 5000 유지·MP 소비 지속
- **C11** [ ] 광선 복수 적 관통 피해
- **C12** [ ] 적 관통 시 피해 감쇠 없음
- **C13** [ ] 지형/벽 충돌 시 광선 정지
- **C14** [ ] 지형 충돌 지점에만 일반 폭발
- **C15** [ ] Actor 접촉만으로 일반 폭발 없음
- **C16** [ ] 최대 사거리 종료 시 일반 폭발 없음
- **C17** [ ] 폭발 피해에 거리 감쇠 없음
- **C18** [ ] 폭발 피해 아군/중립 제외
- **C19** [ ] 광선·충돌 폭발 대상이 겹치면 각각 독립 처리
- **C20** [ ] 다른 손 무기·방패·같은 주문·Judgement 조합 허용
- **C21** [ ] 다른 손 일반 주문·Ward·Staff·Scroll 제한
- **C22** [ ] 충전량·티어·예정 피해·피해 적용 로그 확인

## [D] Judgement / 심판 (19개)

- **D01** [ ] 충전 시작·해제 정상
- **D02** [ ] 표식 광선이 처음 교차한 Actor 획득
- **D03** [ ] 표식 피해 1 고정
- **D04** [ ] 300 MP 시 3발
- **D05** [ ] 900 MP 시 6발
- **D06** [ ] 1800 MP 시 9발
- **D07** [ ] 1800 MP 상한 후 추가 MP 소비 중단
- **D08** [ ] 첫 포격 약 0.45초 지연
- **D09** [ ] 이후 약 0.12초 간격
- **D10** [ ] 광선별 150 고정피해
- **D11** [ ] 폭발별 50 고정피해
- **D12** [ ] 광선/폭발이 독립적으로 피해 적용
- **D13** [ ] 포격별 발사 직전 타깃 스냅샷
- **D14** [ ] 대상 이동·회피 시 궤적 고정 정책
- **D15** [ ] 대상 사망 후 이미 생성된 포격 계속 실행
- **D16** [ ] Schedule/SHOT 구분 로그 확인
- **D17** [ ] 발사별 Mult·PlannedBeamPerHit·PlannedExplosionPerHit 기록
- **D18** [ ] 명중/피해 적용 건수 로그 확인
- **D19** [ ] 포격 후 원형/광선 VFX 정리

## [E] The World / 세계 (14개)

- **E01** [ ] 선택한 Shout/Power 입력 가로채기 정상
- **E02** [ ] The World 선택 시 전용 충전 시작
- **E03** [ ] 다른 외침/Power 사용 시 바닐라 전달
- **E04** [ ] 0.7초 / 350 MP / 4개 마법진
- **E05** [ ] 1.4초 / 700 MP / 8개 마법진
- **E06** [ ] 2.1초 / 1050 MP / 12개 마법진
- **E07** [ ] 최대 2.1초 상한 및 단계별 MP 소비
- **E08** [ ] 승인된 슬롯: I=01/06/08/11, II=01/02/03/04/05/06/08/11, III=전체
- **E09** [ ] 각 마법진 외향 광선과 끝점 폭발
- **E10** [ ] 각 폭발 100 고정피해 독립 중첩
- **E11** [ ] 벽 차폐/LOS 요구 없음·적대 대상만 판정
- **E12** [ ] 입력 해제·취소 시 처리 정상
- **E13** [ ] 단계/배율/발사/피해 건수 로그 확인
- **E14** [ ] 종료·불러오기 후 VFX 잔류 없음

## [F] Temperance / 절제 (28개)

**S08TRACE 제한 전에 쌍수·다중 접촉 우선 시험.**

- **F01** [ ] 토글 ON 정상
- **F02** [ ] 토글 OFF 정상
- **F03** [ ] ON 상태의 적중한 일반 근접 공격 발동
- **F04** [ ] 일반 공격 50 MP 원자적 지불
- **F05** [ ] 일반 공격 마법진/광선 1개
- **F06** [ ] 해당 빔 실제 교차 대상당 고정피해 50
- **F07** [ ] 강타 적중 시 별도 발동
- **F08** [ ] 강타 150 MP 원자적 지불
- **F09** [ ] 강타 마법진/광선 3개
- **F10** [ ] 강타 각 빔 실제 교차 대상당 고정피해 80
- **F11** [ ] 일반 pending 풀 3건 정책
- **F12** [ ] 강타 pending 풀 2건 이상 독립 처리
- **F13** [ ] 일반·강타 그룹 간 pending/시각 슬롯 간섭 없음
- **F14** [ ] 일반 pending 포화 시 추가 발동 거부
- **F15** [ ] 강타 pending 포화 시 추가 발동 거부(실제 지원 상한 기준)
- **F16** [ ] MP 부족 시 0결제·0발동
- **F17** [ ] MP 부족 후에도 ON 토글 유지
- **F18** [ ] 방패치기 제외
- **F19** [ ] Timed Bash 제외
- **F20** [ ] 활/석궁 등 비근접 공격 제외
- **F21** [ ] 쌍수 좌/우 각각 실제 적중한 독립 스트라이크 처리
- **F22** [ ] 동일 스트라이크 다중 HitData로 중복 Proc 없음
- **F23** [ ] Trigger Firewall로 2차 재귀 없음
- **F24** [ ] 토글 OFF 후 이미 지불한 pending Proc 완료
- **F25** [ ] POOL_FULL 거부 사유 로그 확인
- **F26** [ ] BASH_OR_TIMED_BASH / NON_MELEE / FIREWALL 사유(해당 시험 중) 로그 확인
- **F27** [ ] Proc committed/resolved·비용·실제 교차 대상 기록
- **F28** [ ] 1/3 마법진 위치·안쪽 빔 및 종료 정리

## [G] Arcana Limiter / 비용 할인 (10개)

- **G01** [ ] Limiter 미착용 배율 정상
- **G02** [ ] Limiter 착용 판정 정상
- **G03** [ ] Destruction 20 착용 할인 10%
- **G04** [ ] Destruction 40 착용 할인 20%
- **G05** [ ] Destruction 60 착용 할인 30%
- **G06** [ ] Temperance ON 추가 할인 20%
- **G07** [ ] Limiter+Temperance 할인 덧셈 적용
- **G08** [ ] 충전 시작 후 장비 변경해도 해당 세션 스냅샷 유지
- **G09** [ ] 새 충전에서는 변경된 착용/토글 상태 적용
- **G10** [ ] COST_SNAPSHOT와 BEGIN/RELEASE 배율 로그 일치

## [H] 사운드·VFX·렌더링 (14개)

- **H01** [ ] 주문별 충전 사운드(해당 매핑 존재 시)
- **H02** [ ] Magician 티어별 발사 사운드 정상
- **H03** [ ] Judgement 발사 1/4/7 음향 정상
- **H04** [ ] World 최종 해제 사운드 정상
- **H05** [ ] Temperance 결제 Proc 음향 정상
- **H06** [ ] 각 사운드 중복·과도한 재생·누락 없음
- **H07** [ ] 마법진 생성 위치 정상
- **H08** [ ] 마법진 방향·크기 정상
- **H09** [ ] 광선 시작점·방향 정상
- **H10** [ ] 광선 끝점·길이 정상
- **H11** [ ] Magician/Judgement/World/Temperance 주요 VFX 정상
- **H12** [ ] 동시 VFX 중첩 시 화면 표시 정상
- **H13** [ ] 종료/취소 후 임시 VFX 정리
- **H14** [ ] 반복 실행 후 VFX 누적·잔류 없음

## [I] 저장·로드·스트레스 (12개)

- **I01** [ ] 충전 도중 취소 후 정상 재사용
- **I02** [ ] 장비 변경 후 정상 재사용
- **I03** [ ] 주문 변경 후 정상 재사용
- **I04** [ ] 저장 후 불러오기 정상
- **I05** [ ] 불러오기 후 과거 충전 세션 오염 없음
- **I06** [ ] 불러오기 후 잔류 광선/마법진 없음
- **I07** [ ] 새 게임 시작 후 상태 초기화
- **I08** [ ] 여러 주문 연속 실행
- **I09** [ ] 여러 주문 동시에 사용
- **I10** [ ] 반복 사용 중 CTD 없음
- **I11** [ ] 반복 사용 중 프리징 없음
- **I12** [ ] 반복 사용 후 피해·MP 계산 정상

## [J] SKSE 기존 로그 검증 (13개)

- **J01** [ ] OrdinaryOffensiveMagic.log 생성·쓰기 정상
- **J02** [ ] DLL 로드·DataCache/Forms 기록 확인
- **J03** [ ] Magician 충전·티어·피해 로그 확인
- **J04** [ ] Judgement FINAL_RELEASE·Schedule·SHOT 로그 확인
- **J05** [ ] World 충전 단계·발동·피해 로그 확인
- **J06** [ ] Temperance Proc·거부 사유 로그 확인
- **J07** [ ] Arcana Limiter 착용·숙련도·할인 배율 확인
- **J08** [ ] MP 요청·비례 배분·실제 결제 기록 확인
- **J09** [ ] 사운드 재생 요청 및 발생한 실패 사유 확인
- **J10** [ ] VFX 생성·정리·발생한 실패 기록 확인
- **J11** [ ] S08TRACE 512건 한도 관리(경고는 도달했을 때만)
- **J12** [ ] 실패 항목별 관련 이벤트·거부·결과 로그 확보
- **J13** [ ] 로그만으로 확인 불가능한 시각/청각 항목은 사용자 수동 판정

## 실패·재현 기록

- QA 번호 / 판정:
- 사용한 콘솔 명령 및 당시 능력치:
- 대상 Base FormID / Reference ID:
- 대상 HP 공격 전 / 후:
- 재현 순서:
- 기대 결과 / 실제 결과:
- SKSE 로그 시각 및 관련 키워드:
- VFX / 사운드 관찰 결과:

## 로그 분석 기준

1. 개발 DLL과 MO2 배포 DLL 해시를 먼저 대조한다.
2. SKSE 로드·DataCache 및 각 주문의 이벤트 로그를 확인한다.
3. BEGIN/RELEASE, FINAL_RELEASE/SHOT, WORLD_CHARGE_RELEASE, Proc committed/resolved, COST_SNAPSHOT 등 해당 이벤트를 대조한다.
4. Planned 피해는 예정값, Damaged는 적용 건수이며 실제 HP 감소량은 별도 실측이다.
5. VFX와 사운드의 실제 출력은 사용자 관찰 기록과 함께 판정한다.
6. 누락된 로그가 확인될 때만 기존 Native 로그를 최소한으로 보강한다.

## QA 집계

| 구역 | 전체 | O | X | - |
| --- | ---: | ---: | ---: | ---: |
| A | 10 |  |  |  |
| B | 16 |  |  |  |
| C | 22 |  |  |  |
| D | 19 |  |  |  |
| E | 14 |  |  |  |
| F | 28 |  |  |  |
| G | 10 |  |  |  |
| H | 14 |  |  |  |
| I | 12 |  |  |  |
| J | 13 |  |  |  |
| **합계** | **158** |  |  |  |

**최종 결과:** PASS / PARTIAL / FAIL / 미실시

**RUNTIME PASS 및 Stage LOCK:** 실측 증거 확인 전에는 부여하지 않음.


<!-- TOOM_FIRST_RUNTIME_BLOCKER_CP_20261019 -->
## 2026-10-19 — FIRST RUNTIME QA BLOCKER

Overall status: BLOCKED / first smoke FAIL. Full QA suspended.

User-observed:
- Four spells learned, only The World appears in the magic list.
- The Magician, Judgement and Temperance do not appear.
- The World equips but does not activate on Z.
- Arcana Limiter ring equips; discount untested.

Log-observed:
- DLL bootstrap, DataLoaded, Forms=4/4 and VFX assets Ready Count=17.
- Charge, Temperance and World input hooks were reported installed.
- PostLoadGame GameReady=true.
- DOWN/UP Hand=R was logged.
- No confirmed World activation or damage event.

Related tests:
- A09/A10: basic spell listing/equip behavior failed.
- E01-E03: World activation blocked; internal cause unresolved.
- J01/J02: plugin and forms were found, not gameplay PASS.
- G01-G10: discount tests not completed.
- Other unperformed cases remain untested.

Restart gates:
1. Three normal spells appear in the magic menu after learning.
2. Normal spell equip and baseline activation work.
3. The World selection and Z activation work.
4. Relevant SKSE events agree with user-observed actions.
5. Only then resume the original 158-case QA.

This appended incident record does not alter existing QA IDs,
order, labels, or checkbox states.


<!-- TOOM_20261020_PREQA_CHECKPOINT -->
## Current pre-QA deployment checkpoint - 2026-10-20 00:39:00 +09:00

This appendix supersedes old build/deploy headings as CURRENT status;
historical failed smoke tests and existing 158 test IDs remain intact.

Build/Deploy:
- DLL length: 764416 bytes
- SHA256:
  9F6DA706CE3923FB8530C09A51D057231CD100A30C1FD5489DC2D8DECA189D85
- ESP SHA256:
  4255C58A299F23DF263EAAD6199042F322B0E9F57D8BDE95AD5258BE5221634A
- E2-A R3 ownership diagnostic compiled and deployed.
- Pre-QA source/build/ESP/R11-art integrity: 14/14 PASS.
- Production files were not altered by that integrity check.
- E1 width envelope is implemented but not visually verified.
- E2-B alpha fade remains explicitly unimplemented.

At the integrated One-Boot QA, correlate player-visible presentation
with SCENE_READY, BEAM_ENVELOPE_START and E2_SHADER_OWNER where
those log lines actually occur. Their absence is evidence to inspect,
not an automatic FAIL without a corresponding expected event.

Check four-spell listing/equipment and World input first, then
Temperance melee procs, VFX, sound, early collision, limiter and
remaining original 158 tests. Preserve O/X/- manual scoring.
Do not create a collector or automatic pass/fail mechanism.

Active artifact path/hash register: DEV_ASSETS/008_SOURCE_ASSET_INVENTORY.md.
RUNTIME PASS / STAGE LOCK: NOT GRANTED.


<!-- TOOM_E2B_QA_TARGET_20261020 -->
## E2-B 추가 런타임 관찰 지점 — 2026-10-20 01:54:14 +09:00

- 테스트 DLL: 768000 bytes; SHA256 D88AE3B9357663BA594C47742F15467239BCCF19392597509C5D312FD246BDA8.
- E2-B 빔: 시각적으로 0.12s까지 유지 후 0.48s까지 자연스럽게 사라지는지 관찰. E1 폭 성장/감소 및 전체 Y 길이를 함께 확인한다.
- E2-B 마법진: Retire 발생 후 0.60s 잔광이 실제로 점진적으로 사라지는지 관찰한다. 유지 후 순간 삭제인지 구분한다.
- 다른 동시 이펙트의 Alpha가 영향을 받는지, 이전 시전 잔여 VFX/CTD가 없는지도 함께 확인한다.
- 기존 A-J 158개 QA ID 및 O/X/- 표기는 변경하지 않는다. 해당 항목의 관찰 메모에만 기록한다.
- Fade STATIC/BUILD/DEPLOY와 실제 화면에서의 RUNTIME PASS는 별도로 판정한다.


<!-- TOOM_QA_LIMITER_CONSOLE_R1_20261020 -->
## [P-G] Arcana Limiter 콘솔 준비 및 판정 보강 — 2026-10-20 02:05:21 +09:00

현재 테스트 배포본:
- DLL: 768000 bytes
- Build/MO2 SHA256: D88AE3B9357663BA594C47742F15467239BCCF19392597509C5D312FD246BDA8
- 런타임 할인 및 실제 마법진/빔 페이드: 아직 미검증

이 섹션은 기존 [P] 콘솔 참고표와 G01~G10에 대한 보충 절차다.
A~J 158개 시험 번호, O/X/- 판정 및 기존 이력은 변경하지 않는다.

### 파괴마법 능력치 조회

스카이림 콘솔에서 각 명령을 한 줄씩 실행한다.

- 현재값: `player.getav destruction`
- 기본값: `player.getbaseav destruction`
- 상세 정보: `player.getavinfo destruction`

Arcana Limiter의 숙련도 기반 할인 검증에는 기본값 확인이 중요하다.
임시 버프 등으로 현재값과 기본값이 달라지면 그 차이를 기록한다.

### 레벨 설정 명령 — 한 줄씩 실행

- 기본 비교: `player.setav destruction 15`
- 첫 경계 직전: `player.setav destruction 19`
- 첫 경계: `player.setav destruction 20`
- 두 번째 경계 직전: `player.setav destruction 39`
- 두 번째 경계: `player.setav destruction 40`
- 세 번째 경계 직전: `player.setav destruction 59`
- 세 번째 경계: `player.setav destruction 60`
- 최대 구간 비교: `player.setav destruction 100`

각 설정 직후 `player.getbaseav destruction`으로 기본값을 확인한다.
그다음 새로운 충전 세션을 시작하여 COST_SNAPSHOT을 기록한다.
충전이 이미 진행 중인 세션은 시작 시점 스냅샷 규칙을 우선한다.

### 기대 배율 — 기존 G01~G10 기준

| Limiter | Temperance | 파괴마법 | 기대 할인 | 기대 CostMult |
| --- | --- | --- | --- | --- |
| OFF | OFF | 임의 | 0% | 1.00 |
| ON | OFF | 15, 19 | 0% | 1.00 |
| ON | OFF | 20, 39 | 10% | 0.90 |
| ON | OFF | 40, 59 | 20% | 0.80 |
| ON | OFF | 60, 100 | 30% | 0.70 |
| OFF | ON | 임의 | 20% | 0.80 |
| ON | ON | 20 | 30% | 0.70 |
| ON | ON | 40 | 40% | 0.60 |
| ON | ON | 60 | 50% | 0.50 |

수치는 QA 기대값이다. 실제 판정은 Native 실행 로그 및
실제 Magicka 결제량으로 검증한다.

### G01~G10 실행 순서

1. Limiter 미착용, Temperance OFF에서 기본 배율 확인(G01).
2. Limiter를 착용하고 실제 착용 판정 확인(G02).
3. 파괴마법을 19→20, 39→40, 59→60 순서로 변경하여
   경계 진입과 할인 10/20/30% 확인(G03~G05).
4. 15와 100에서 하한/상한 구간을 추가 확인한다.
5. Limiter 미착용 상태로 Temperance ON 추가 할인 확인(G06).
6. Limiter 착용 + Temperance ON에서 덧셈 할인 확인(G07).
7. 충전 도중 Limiter 착용 변경 또는 Temperance 토글 변경 후
   기존 CostMult 유지 확인(G08).
8. 충전 종료 후 새 세션에서 갱신된 배율 적용 확인(G09).
9. 실제 생성된 COST_SNAPSHOT, BEGIN, RELEASE 로그를 대조하고
   Magicka 소비 및 할인 배율을 확인(G10).

주의:
- console 설정 명령을 실행한 사실 자체로 G01~G10을 PASS 처리하지 않는다.
- 숙련도 변경과 장비/토글 변경 후 새 세션에서 각각 확인한다.
- tgm은 Magicka 비용 검증에 사용하지 않는다.
- 전용 QA 세이브에서만 setav를 사용한다.
- 일반 플레이로 돌아갈 때는 테스트 전 원본 세이브를 불러온다.
- 기존 마법사/심판/세계/절제와 VFX/오디오/충돌 QA도 함께 수행한다.
- E1 빔 폭 및 E2-B 점진적 페이드는 사용자 시각 관찰로 별도 판정한다.
- Skyrim 런타임 PASS 및 Stage LOCK은 실제 시험 전까지 미부여한다.

<!-- TOOM_E3_D1_ONE_BOOT_ADDENDUM_20261020 -->
## E3 integrated One-Boot QA supplemental checkpoint — 2026-10-20

The historical DLL/build notes earlier in this document describe
earlier checkpoints. The new isolated integrated candidate is:

- DLL size: 772096 bytes.
- Isolated build SHA256:
  C377791110C96CBFBCF84539C217810A4EF2EAA6FEB6370AE6C707DBC2582C42
- At E3-D0, production build/MO2 still held the historical DLL:
  D88AE3B9357663BA594C47742F15467239BCCF19392597509C5D312FD246BDA8
- Integrated deployment: NOT EXECUTED at this document checkpoint.
- Human One-Boot QA: NOT RUN.

Retain all original A-J 158 case IDs, order and O/X/- states.
Do not automatically classify the current build as Runtime PASS.

### Focus areas within the existing cases

1. Visible VFX: verify magician charge circles, beams, launch,
   World VFX, Temperance-related circles and fade persistence.
   An attached or nonculled NIF is not proof of visible pixels.
2. E3 same-path reference: first eligible CircleStatic may spawn
   additional TOOM and optional TULL reference NIFs offset to
   opposite sides. This is a same native direct-attach comparison,
   not the TULL engine ACTI rendering path. Do not treat this
   diagnostic as ordinary gameplay art.
3. Bounds: compare beam geometry visibility and movement with
   bounded E3_RENDER_TREE and E3_BOUND_CONTRACT log evidence.
   Conservative bound repair is not a verified GPU-render fix.
4. World: verify equipped Power selection, relevant Z inputs,
   native activation and actual effect separately. The new
   selectedPower fallback is a candidate, not runtime proof.
5. Audio: listen to all relevant sounds. Correlate PLAY_REQUEST,
   Source, PlayerDistanceProxy, SincePreviousMs, CapReplaced,
   FadeRequested and ImmediateStops. Distance proxy is not
   the actual audio-listener metric.
6. Judgement: distinguish a valid beam intersection and target
   acquisition from a legitimately empty target-selection case.
7. Also retain Temperance melee, collision and Arcana Limiter
   cost-snapshot coverage.

Use only the existing OrdinaryOffensiveMagic.log and normal
user-observed gameplay results. No extra automated QA collector,
QA ZIP generator, separate game boot or auto-scored checklist.


<!-- TOOM_R1_C2C_D1_20261020 -->
## R1-C2B/C2C integration supplement - 2026-10-20

This is a focus note for EXISTING A-J test cases, not an expansion
of the 158 counted cases. Preserve all original case IDs and states.

Use the isolated R1-C2C C1 staging DLL as the build reference
when checking its byte identity against the deployed MO2 DLL:
774144 bytes, SHA256
66C44352C845F82858409824BA625BBDDC8EE65D6920D02F2F6096172C34C365.
The earlier build/msvc DLL is intentionally preserved as historical
output, not the build authority for this acceptance.

Within existing spell/charge cases, manually check:
- Magician requires at least 100 times snapshotted multiplier MP.
- Judgement requires at least 300 times multiplier MP.
- World requires at least 350 times multiplier MP.
- Test immediately below and at the new-charge threshold.
- Verify Limiter and Temperance additive discounts 1.00 to 0.50.
- Existing charges remain active at zero MP and can resume.
- A dual hand joining an active same-spell session is not denied.
- Admission does not reserve or deduct MP prematurely.
- Judgement 3/6/9 positions vary while preserving target ownership.
- Temperance Normal avoids its last two positions.
- Temperance Power avoids the previous three-slot triple.
- Already-paid pending procs keep their own coordinate snapshots.

Also retain all original VFX, audio, World activation, collision,
proc payment, limiter and game-loading cases. Visual/audio
acceptance requires actual human observation. BUILD/DEPLOY are not
RUNTIME PASS. No new QA collector or standalone extra game boot.


<!-- TOOM_RUNTIME_CTD_E1_20261020 -->
## 2026-10-20 - One-Boot blocker: MagicMenu CTD

The first integrated QA boot reached a loaded game, after
which spell books were obtained and learned.
Opening the MagicMenu immediately caused a CTD.
Record as an actual functional/runtime failure, not a
successful full One-Boot QA. Do not pre-mark untested cases O.

Crash signature:
OrdinaryOffensiveMagic.dll ArcanaLimiterRuntime.cpp:115
SnapshotCostMultiplier / GetActiveEffectList virtual dispatch;
ChargeRuntime.cpp:336 CheckCast admission call path.
Check the 007 original A-J cases without adding to 158 count.
Resume after verified repair deployment, preserving remaining
untested items and the original crash evidence.


<!-- TOOM_RUNTIME_CTD_E1_B2_20261020 -->
## E1-B2 repair deployment / QA continuation note

First QA interruption:
Opening the learned-spell MagicMenu caused a Native CTD.
Preserve that result as historical failure evidence.

E1-A repaired the suspected active-effect interface
dispatch and removed admission checks from menu CheckCast.
E1-B1 isolated MSVC x64 build passed.

Use the deployed E1-B2 DLL identity for the next run:
774144 bytes.
SHA256 E7C3FD2C3384539A85899EDA31EDEA867FFAB8DB76F359B8AED79054A193C519.

Within the EXISTING 158 A-J cases, test first:
- MagicMenu opens after learning all four TOOM spells.
- Menu selection and switching are stable.
- Magician/Judgement minimum MP checks still occur on
  actual new casting requests, not menu enumeration.
- World minimum MP admission remains functional.
- Repeat save/load, VFX, collision and other original cases.

This note does not increase the 158-case count.
Do not mark unobserved cases PASS.
Repair deployment alone does not resolve the runtime verdict.


<!-- TOOM_F1E_F2_CHECKPOINT_20261021 -->
## F1-E observed runtime QA - supplementary only

Existing A01-J13 158 cases and original scoring text are
preserved. This is NOT a blanket PASS of the 158 cases.

F1-01 - First-trigger recording preparation: not scored.
F1-02 O: F1_DIRECT_QUAD confirmed in SKSE log.
F1-03 X: A original TOOM CircleStatic invisible.
F1-04 O: B POT SiegeRune direct visibly rendered;
          user screenshot and explicit confirmation.
F1-05 X: C TOOM OuterBand invisible.
F1-06 X: D OwnEmit F1 Demand failed (not a shader test).
F1-07 X: E TOOM ACTI created but not visibly rendered.
F1-08 O: F1_ACTI_CREATED and F1_ACTI_RETIRED logged.
F1-09 X: requested 12s timeout was observed at 20.491s.
F1-10 -: repeated-trigger boundedness not fully verified.
F1-11 -: cleanup after save/load/cancel not fully verified.
F1-12 -: full CTD/MP/damage regression suite not verified.

Observed same-boot World charge:
Stage=3, Charge=1050, 12 circles scheduled;
actual TOOM VFX not visible.
Judgement and Magician gameplay logs exist;
not equivalent to full numeric QA acceptance.
Temperance toggle and CostMult=0.800 logged.

Audio: PLAY_REQUEST logs exist, user heard no TOOM
effect. Audio human FAIL, cause not yet isolated.

Previous E1 MagicMenu CTD regression was not reported
during this observation, but full A-J acceptance remains
PENDING. No final Runtime PASS or Stage LOCK.

Reference: user-uploaded OrdinaryOffensiveMagic log
covering 2026-10-21 01:53 to later runtime, plus the
2026-10-21 conversation screenshot of visible POT rune.
Screenshot is external chat evidence, not falsely claimed
as a physical local G: or H: project file.

<!-- TOOM_PREQA_CP001_20261022 -->
## PRE-QA CP001 - 기존 158항목 보충 판정 지침
- A~J 158개 기존 체크 항목과 번호는 변경하지 않는다.
- 새 자동 수집기, 테스트 ZIP, 게임 재부팅 단계는 추가하지 않는다.
- 게임 실행은 주요 변경을 모두 통합한 후 단 한 번의 수동 QA에서 진행한다.
- 마법사: T1/T8 빔 방향·길이·두께, 지형 충돌점 폭발과 단계별 크기.
- 마법사: Actor만 관통하고 지형에 닿지 않으면 폭발이 없어야 한다.
- 심판: 좌표 지정 최초 빔의 소리와 마법진에서 발사되는 각 빔의 소리를 구분 확인.
- 심판: 여러 마법진·빔의 실제 출력·방향·적중·종료 확인.
- 세계: 다중 빔·각 종착점 폭발의 실제 출력과 잔류 여부 확인.
- 절제: 다중 빔의 실제 출력·방향·수명과 반복 시전 검증.
- 공통: 마법진/빔/폭발 연속 시전, 화면 잔상·중복·수명 종료 확인.
- IMPACT_VFX, P04C_FLASH_TIME, E3_RENDER_TREE 로그는 보조 증거이다.
- Spawned=true 또는 CPU 장면 트리 정상만으로 화면 출력 O 판정 금지.
- 실제 화면에 보이지 않으면 해당 VFX 항목 X로 기록한다.
- 모든 항목 O 또는 사용자 승인된 예외 없이는 LOCK 승인 불가.

<!-- TOOM_QA1_CP002_20261022 -->
## QA1 CP002 - Partial runtime findings; original 158 cases retained
- This section supplements the existing A-J checklist; it does not replace or renumber any original item.
- QA1 is a PARTIAL result, not 158/158 PASS.
- Observed O/PARTIAL: some in-game magic-circle rendering is now visible.
- Observed X: intended beams and explosions remain visually absent.
- Observed X: World/Temperance formation distance and spread too short/narrow.
- Observed X: Magician position, frontal alignment, orientation and circle scale unstable.
- Observed X: Judgement intended visuals absent.
- Observed X: Magician apparent firing range inconsistent; precise cause pending.
- Log-only: World Circles/Beams/Impacts, Judgement BeamVisual/ImpactVisual and Magician Spawned=true cannot override visual X.
- Log-only: P04C_FLASH_TIME records a controller-update event, not proof of explosion pixels.
- Pending checks: first-person/third-person, standing/turning/moving, charge/release aim timing, indoor/outdoor obstacles.
- Pending checks: consistent circle size/orientation, formation radius, beam length and endpoints, hit-point explosion.
- Pending checks: repeated spawning, resource consumption, stale visual cleanup, Judgement sound layering.
- Keep O/X manual marking; do not auto-promote old unchecked cases.

