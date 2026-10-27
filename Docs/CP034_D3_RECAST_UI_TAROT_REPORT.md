# TOAM v1.0.3 / CP034-D3 구현·검증 보고서

## 과업과 관찰
- v1.0.2에서 세계·은둔자 중복 시전 차단: 사용자 PASS. 해당 코드 및 비용·피해·방어·타이밍은 이번 수정에서 유지한다.
- 색상: 최초 적용 후 원본으로 되돌아온다는 사용자 FAIL. 로그에서 루트 검사346회 통과, 적용5회, 재질·형상 통합 거부341회. 통합 로그만으로 정확한 내부 실패 조건은 확인 불가.
- 13-37-36 영상은 원본색 마법진을 포함한다. 최초 변경색 장면은 사용자 미촬영이며 적용 로그를 실제 발색 증거로 승격하지 않는다.

## 색상 수정
확인된 전용 효과 루트는 교체하지 않는다. 해당 루트가 단독 소유한 최상위 표시 가지를 한 번 복제해 새 노드·형상과 원본 사이의 포인터 비공유, 참조 수, 변환·경계·정점 기술자 및 셰이더 타입을 검사한다. 모든 셰이더·재질·컨트롤러를 전용으로 검증한 뒤 가지 연결만 교체한다. 공유된 원본의 RGB, 알파, 텍스처, 정점 데이터는 수정하지 않는다. 각 최상위 원본 가지도 참조1개와 정확한 부모 관계를 요구하므로 Native SetAt가 공유 노드의 parent를 변경하지 않는다.
성공한 소유자는 복원 lease를 유지한다. 마지막 소유자가 종료되거나 부분 실패하면 아직 자신의 교체 가지인 연결만 원본으로 복구한다. 양손/지연 퇴장 복사·이동은 같은 lease를 유지하고 마지막 소유자에서 정리한다. 다음 세대는 새 상태다. Default는 원본 경로를 즉시 사용한다. 복제·RGB 적용은 최초 준비 시점 한 번이며 기존 프레임 처리나 시전 타이밍을 바꾸지 않는다.
이 수정은 기존 거부 원인의 유일한 확정 진단이 아니라, 공유 형상에 직접 쓰지 않는 검증 가능한 대체 경로다. 실제 엔진에서 새로운 안전 검사가 거부되면 원본을 유지하고 세부 이유를 기록한다. 보장하지 못한 엔진 공유를 강제 허용하지 않는다.

## 색상 화면
- 전체 색상 변경 / 마법별 개별 변경 두 탭.
- 전체: 한 색 선택으로 13종 편집값을 일괄 변경하며 아래 적용 및 저장으로 확정.
- 개별: 한 화면에 최대5개, 5·5·3개 묶음 페이지. 각 행은 로마숫자+마법 이름, 선택색 및 원본/파랑/빨강/금색/보라 선택.
- 실제 Framework igColorButton 색상 견본을 사용한다. 견본과 셰이더는 같은 RGB 표를 사용하며 실제 게임 발광·조명 차이를 명시한다. Default는 회색 견본으로 가장하지 않는다.
- 기존 적용/저장/초기화/INI 재읽기와 피해·비행 페이지는 유지한다. 재생 미리듣기는 추가하지 않는다.

## 이름 변경
사용자 확정: 모든 타로 이름에 번호, 기존 분류명 유지. 6개 ESP의233 FULL 이름 및6개 TES4 SNAM 설명 문자열을 변경했다. 주문·지팡이·주문서·관련 효과 표시명 및 메뉴를 포함한다. 기존 Internal FULL에도 타로 이름 토큰이 있으면 번호를 넣되 EDID/내부 조회키는 그대로다. 원래 Arcana/아르카나, Ordinary Attack Magic/일반공격마법, Ordinary General Magic/일반교양마법 분류를 보존한다. 광대는0, 힘VIII, 정의XI를 사용한다.
번호 대응: 0광대,I마법사,II여사제,IV황제,VII전차,VIII힘,IX은둔자,XI정의,XIV절제,XVI탑,XVII별,XVIII달,XIX태양,XX심판,XXI세계.
기준 원전: https://original.sacred-texts.com/tarot/pkt/pktar08.htm 및 https://original.sacred-texts.com/tarot/pkt/pktar11.htm (A. E. Waite).
전체 전후 표시명은014_TAROT_NAME_MAPPING.json. 승인된 TES4 SNAM 설명을 제외한 비FULL 데이터와 기존 FormID/MASTER는 동일하다. EN/KR는 기존 번역 표시 필드만 다르다.

## 검증 수준
|구분|범위|
|---|---|
|STATIC|실제16종 활성 NIF 구조·자산 불변,6ESP 게임플레이 데이터 불변·언어 구조,원본 aimed-carrier 조건,설정 경로|
|HOST|실제 색상 소스215검사/100재시전/2동시/500프레임 재복제 없음/부분 실패 복원; 실제 정책·메뉴2199검사; 기존 실제Win32 INI31검사|
|BUILD|기존 build/msvc의 MSVCx64/Ninja. 실제 결과는021_RESULT.json과025_MSVC_NINJA_BUILD.log에 기록|
|DEPLOY|새 패키지 준비만 수행. 활성 설치와 MO2 프로필은 변경하지 않음|
|USER_RUNTIME|신규 색상·UI·이름은 미검증. 중복 시전 차단은 기존 사용자 PASS만 유지|
|LOCK|NO|

Host double은 Native 그래픽·실제 Skyrim 메모리 소유권을 실행하지 않는다. 기존 3개 재질/컨트롤러 안전 검사와 루트 소유권 검사는 유지한다. 새 lease는 색상 소유 상태에 shared_ptr 하나를 추가하며 실제 게임 CPU/메모리/FPS는 미계측이다. 객체 수와 수명의 호스트 검사를 게임 힙 계측으로 오해하지 않는다. NIF/DDS 자산 추가·삭제·변경0개. 음원·카메라·독점 시전의 동작 소스는 변경하지 않는다.

## 사용자 최소 통합 QA
1. 전체 탭에서 Gold 적용 → 마법사·세계·은둔자 각3회 반복. 최초뿐 아니라 다음 시전도 유지되는지 확인.
2. 개별 탭에서 서로 다른 두 마법을 Red/Blue로 지정 → 동시 시전. 설정을 바꿔도 기존 마법진은 유지하고 다음 시전에 새 색 적용.
3. Default 전체 복원 및 마법별 복원. 원래 색·알파·발광·회전·성장·퇴장 확인.
4. 5·5·3 페이지, EN/KR, 전체/개별 탭, 적용/기본값/INI 재읽기, 재실행 후 저장 유지 확인.
5. 마법사8겹/세계12개/심판9개와 양손 Arcana, 빠른 취소·재시전·늦은 퇴장 후 잔류·프리징·CTD 확인. 독점 차단 유지.
6. 주문·주문서·지팡이의 번호 및 기존 분류명 확인. 저장 게임은 자동 변경하지 않는다.

## 디렉터 관점 핵심 검토 3문항
1. 최초 이후 거부의 정확한 원인은 확정됐는가? 아니오. 이전 로그가 하위 조건을 합쳐 기록했다. 새 구현은 세부 거부 사유를 남기며 전용 표시 가지에서만 변경한다.
2. 모든 색을 미리 보고 여러 마법을 함께 편집할 수 있는가? 새 메뉴는 두 탭,5개씩 표시하는 행과 실제 색상 견본을 제공한다. 최종 인게임 배치는 사용자 확인 전이다.
3. 이름 변경으로 게임플레이나 세이브 참조가 바뀌는가? 233 FULL 이름·6 TES4 SNAM 설명 및 길이 필드 외에는 바꾸지 않았으며 FormID/MASTER/EDID/비용/피해 등은 전후 동일 검증한다. 실제 표시명은 사용자 확인 전이다.


### CP034-D3 packaging R1 failure and coherent recovery
First actual MSVCx64/Ninja compile/link passed in29.078s, DLLF7CDFD444717414CC0DC3126EDAD0BB74578B595ECC9A77D041CC9975B8CD371. Packaging then failed because the assistant inserted README.txt into the5-file addon whose actual existing name is README_Stances.txt, producing6files. This repeats guessed filename/validator lessons; it is not a C++ or game failure. All changed source/docs/assets/index and675mutable Ninja files were restored and independently hash-verified in022_ROLLBACK_VERIFICATION.json. Current installed1.0.2 and CP032 anchors never changed. Preserve021 failure evidence. R2 enumerates the exact archive paths before any source write, reuses verified675file rollback backups, corrects the existing README entry and prevalidates source privacy plus six exact ordered-record reconstructions. Coherent rollback invalidated the temporary Ninja build, so R2 must build again in the same existing cache; never copy the old DLL alone.


### CP034-D3 final description/privacy refinement
Latest user screenshot requests MO2 plugin tooltip descriptions. All six TES4 SNAM values now state1.0.3, Main vs optional Stances roles, supportedSE1.5.97 and actual dependencies. Author CNAM remainsTULL. This explicit later request adds six SNAM display-description changes to the233 FULL name changes; all other bytes except serialized length fields stay exact. Main and addon install ZIP filenames follow TULL Ordinary Attack Magic - v1.0.3.zip / TULL Ordinary Attack Magic - Stances Addon - v1.0.3.zip. Existing active Origin folder1.0.1/1.0.2 is not renamed by editing its profile.
R2 preflight correctly rejected a private absolute origin in src/Arcana/SOURCE_PROVENANCE.md before source/build mutation. This document is private provenance, not C++ code. The new Source ZIP carries an explicitly redacted current edition with its original and output SHA recorded; all cpp/h/inl bytes remain exact. No gameplay file is altered to satisfy privacy. Source privacy, archive path sets and all six ordered-record reconstructions are validated before the next commit. R2 is FAIL_PRE/NO_WRITES/NO_ROLLBACK_REQUIRED, not a failed C++ build. R1 rollback proof and675-file baseline remain valid.


## 최종 납품 확인
STATIC·BUILD·DOCSYNC·PACKAGE 검증 완료. 최종 빌드 로그는025_MSVC_NINJA_BUILD.log, 독립 검증은026_FINAL_VERIFICATION.json. 첫 패키징 실패 및 원복 기록은 역사로 보존했다. 최종 해시는 외부027_FINAL_RESULT.json과 납품 SHA256SUMS.txt를 따른다. 실제 신규 게임 발색/UI/표시는 미검증이며 LOCKNO. 설치본과 프로필은 그대로다.
