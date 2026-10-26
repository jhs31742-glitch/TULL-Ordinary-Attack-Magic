> 최신 CP029 R2: Arcana 일반·스탠스 주문과 지팡이 충전 비용 감소는 최대80%, 최소20% 지불이다. 과거100%무료 허용은 이 범위에서 대체되었다. TOAM 비용/리미터는 그대로다. GitHub는 Nexus 동시 공개 전까지 비공개다.

# TULL Ordinary Attack Magic — 통합 기술·아트 연구록

현재 편집판:CP029 /0.8.11 /2026-10-26. 계보:TOOM → TOAM. 본문과 역사 부록을 합친 하나의 연구록이다. 소스·기능·아트 제작·시행착오·실수·주의사항을 함께 보존한다. 이 문서가 코드의 현재 바이트를 대체하지 않으며 부록의 과거 Current/PASS는 당시 판정이다.

## 읽기 순서와 유지 방법
먼저 현재 상태와 기능 표, 관련 기능/아트 절을 읽고 해당 소스를 확인한다. 변경 전에는 누적 부록에서 동일 실패를 검색한다. 신규 작업은 날짜,부모,증상,사실/가설,실패 시도,수정 파일,전후 해시,검증 단계,한계,재사용 주의사항을 이 문서에 누적한다. 과거 실패를 삭제하거나 성공으로 바꾸지 않는다. SOURCE_MANIFEST와 Research/RESEARCH_SOURCE_INDEX.csv는 위치·바이트의 추적표이며 지식 본문을 대신하지 않는다.

목차:1현재와 증거 /2기능과 소유권 /3자원·밸런스 /4Native·ABI /5엔진VFX /6아트 제작 흐름 /7마법진 /8지팡이 /9책 /10비행·SMP /11레코드·패키징 /12운영 실수 /13남은 검증 /14역사 원문 부록.

## 1. 현재와 증거
VERIFIED_SOURCE는 실제 코드/바이트,STATIC은 구조 검사,BUILD는 실제 컴파일·링크,DEPLOY는 설치본 해시,USER_RUNTIME은 사용자 게임 관찰이다. READY/FIRST_POSE는 픽셀 성공이 아니다. 프리뷰 렌더와 스키닝 수학은 엔진 물리 성공이 아니다.
CP028 사용자 실측:CTD 없음,리미터 구간 수치 정상,충전 시작 시 반지·절제 스냅샷 정상,Arcana 비용이 TOAM 충전 단계에 유입되지 않음,TOAM+Arcana 허용/다른 마법 차단 정상. 이 관측 범위만 PASS이며 기존158문항 전부 완료라는 뜻은 아니다. SMP는 미작동,철회되었다.
CP029는 비용·피해·정의·애드온 MASTER의 신규 변경이다. 이전 런타임 성공을 여기에 자동 적용하지 않는다. SE1.5.97만 활성화하며 AE는 ABI 검증 미완료로 진입 차단한다.
GitHub는 지금 PRIVATE. Nexus 출시 때 함께 공개한다. 권리 검토,최종 라이선스 선택,해당 빌드의 수동 QA가 공개 전 남은 일이다. 내부 로그,개인 경로,전체 SNAPSHOTS,외부 바이너리는 공개 소스에 복사하지 않는다.

## 2. 구현 기능과 상태 소유권
| 기능 | 소유자/소스 | 반드시 지킬 경계 |
|---|---|---|
| TOAM 충전 |ChargeSession,MagickaChargeArbiter,MagickaTransaction|자기 결제 할당만 티어에 더한다. 전체MP변화로 계산하지 않는다|
| 마법사 |MagicianRuntime/MagicianChargeVisual|8단계·기존 피해비용 유지,Sun크기 전방축,첫 앵커 안정,보조진 성장/압축|
| 심판 |JudgementRuntime|무피해 지속 지시빔,해제 시 살아있는 적 확정,3/6/9 고정 포격진,발사직전 목표스냅샷|
| 세계 |WorldRuntime/WorldFormation|4/8/12,.7/1.4/2.1초,Arcana Hermit 배치 참고,TOAM 폭발·피해 유지,방어효과 없음|
| 절제 |TemperanceRuntime|.15초 진 요청/.27초 빔피해,헤일로.095와.12rad/s 회전,시각 지연이 피해를 무한 지연하지 않음|
| Arcana8/해방 |src/Arcana/Balance,Casting,HandUpdate|손·주문·스탠스 스냅샷 별도,TOAM리미터와 결제 혼용 금지|
| 지팡이 |StaffInstance/ExtraCharge|실제 착용 인스턴스·손 일치,MP0,원장·HUD동기,일반20000/해방40000|
| 비행 |Flight/FlightCombat/GunFlight|전처리→원래프레임→후처리,중력·골격 복구 및 입력 권위|
| VFX |EngineCircle/CircleRenderer/Arcana presentation|요청폼·세대·effect 정확 대응,준비 후 최초포즈,유한검색·격리·정리|

Native 설치 권위는 TOAM 호스트 하나다. PlayerCharacter0xAD/ActorMagicCaster0x03 등을 설치 순서의 우연한 체인에 맡기지 않는다. Arcana 전후처리와 실제 원본 호출 순서를 보존한다. 이름이 같은 주문도 플러그인+로컬FormID로 식별한다.
마법사와 심판 지시진은 엔진 aim heading/pitch를 따른다. 세계 돔,생성된 심판 포격진,절제Proc는 각각 기존 공간 정책을 따른다. 심판 최신 지시빔 원점은 오른쪽 어깨 앞(34,22,126),12u 작은 진이다. 과거 손 노드 요구는 최신 사용자 지시로 대체되었다. 마법사 지상 Sun앵커(0,74,108),비행 FlightCastOffset을 사용한다.

## 3. 비용·피해와 바닐라 시스템
CP029에서 사용자 지시로8종 할인 전 비용을 모두3배로 높였다. 은둔자 유지비44→132도 명시적 최신 변경이며 방어식·120해제폭발은 그대로다. 임의 숨은 증폭계수는 없다. 단일 Design cost 값이 실제 지불·입장·완충검사를 연결한다. Star유지비는 별도 진당150이다.
ResourceFactor=carrier.CalculateMagickaCost(actor)/carrier.costOverride. SPEL 수동기본비용도3배로 바꾸므로 할인율과 실제 기본 금액이 일치한다. 일반 Hermit는 엔진 결제,다른 Arcana는 Native 결제이고 currentSpellCost0으로 엔진 이중결제를 막는다. 지팡이는 동일 할인율로 ExtraCharge만 차감하며 ENCH엔진비용0 유지. 손별 원장,주문일치,장착인스턴스 조건을 우회하지 않는다.
두 손이 같은 실제MP를 경쟁하지만 어느 손의 감소도 다른 TOAM 티어에 합산하지 않는다. 스탠스는 시전 시작 시 고정한다. 현재시전 중 변경된 스탠스를 다시 읽어 무료효과·중복결제를 만들지 않는다. 100%비용감소는0을 허용하지만 이미0충전인 지팡이는 재충전 전 사용 불가라는 기존 계약이다.
정의는 설치1회1500,기존3초/.2초/회110을 유지하고5회cap만 제거했다. 빔준비,조준정렬,차폐,생존적을 확인할 때만 피해를 요청한다. 이론550/초는 실제 총피해 보장이 아니다. 무제한프레임피해나 새투사체를 추가하지 않았다.
태양은8초 총4650점진결제,3040최대피해. 완충선결제·추가준비대기 중 재결제 금지. 탑은180분산빔×150+900finale,대상/손/세대별27900상한. 실제 EFIT와native override/impact nominal/cap을 함께 바꿔야 하며 Design표만 고치면 실제피해가 남는다. 저항·난이도·기존충돌은 그대로다.

## 4. Native ABI 실패에서 얻은 것
ApplyArtObject SE ID22289는BOOL이다. 성공값1을ModelReferenceEffect*로 받아IncRefCount하면RCX1에서CTD가 난다. 요청은정확한BOOL ABI로 수행하고 ProcessLists에 등록된 실제 이펙트만 찾아참조한다. 포인터값검사로틀린반환형을숨기지 않는다.
MagicTarget 다중상속:player->GetActiveEffectList 직접경로의SE 잘못된인터페이스조정으로MagicMenu에서CTD가났다. AsMagicTarget을사용하고 메뉴열거CheckCast에서새세션결제를하지 않는다.
StaffRefresh SDK의SE/AE ID쌍을이름만믿어호출한것이CTD원인이었다. SE38752/RVA679000은다른컨테이너함수,검증된SE37803/RVA6313D0은충전동기와UpdateWeaponAbility를함께한다. ABI를고쳐도casterUpdate내능력재설치는재진입위험이다. CP026은ExtraCharge와rawCharge AV만맞추며결제중장착능력재설치를제거했다. 당시특정freeze의정확한스택은미확인이라는한계를남긴다.
CP027새aimed concentration carrier는기존selfScript MGEF를복사하며projectile을비워두었다. SE입력33657→GetAVEffect가이효과를배제한후FBA60에서null+C8을읽어즉시CTD. Update내타이머억제보다앞선엔진메타데이터계약이다. CP028은기존빈모델PROJ A6D를DCE.DATA+72에연결했다. SPEL/EFIT무피해·Native소유결제는보존하고 실제잘못된CP027을negative control로거부한다.
오디오SetPosition(NiPoint3)는컴파일돼도SE의scalarXYZ ABI와다르다. 실제ID66370/RVABED920와caller레지스터를확인하여지역어댑터bool(handle*,float,float,float)를사용했다. AE는추측하지않는다. 함수이름·헤더·어셈블리·게임QA는서로다른증거다.

## 5. VFX 생성·표시·수명
이전CC_Root ACTI배치,큰bounds,.6초grace 반복은안정표시를해결하지못했다. CP019에서Arcana0.7.4의실제ApplyArtObject/ModelReferenceEffect/AS_CirclePose기법을이식했다. 요청,정확한effect획득,3D준비,필수노드,최초포즈,성장/회전,퇴장,격리재사용을하나의객체소유권으로연결한다.
Actor에부착된효과의parent월드변환을고려하여desiredWorld를parentLocal로역변환한다. world좌표를local행렬에그대로넣지않는다. 최초3D전성장/퇴장시간은시각시계와분리하며게임플레이시간은독립이다. 준비완료로그는씬이있다는증거뿐이다.
요청검색은1.2초/120회,요청수명2초와pending격리2.2초등유한조건을지킨다. 취소된슬롯이늦게돌아와재등장하지않게세대/폼을검사한다. 매프레임전체Actor나무제한ProcessLists를검색하지않는다. 기존프레임훅을재사용한다.
WholeLens원본반경49.92와ARTO1.92의26:1은단면어댑터경계에서한번만환산한다. 입력폭검증과변환후허용폭을분리한다. 게임플레이폭·길이·충돌은렌더폭보정으로바꾸지않는다. ACTI/ARTO동일슬롯에표시권위둘을주지않는다.

## 6. 첨부 이미지에서 게임 아트까지의 공통 제작 과정
사용자 이미지는형태·색·배치의참고권위였다. 확인된구현은마법진의SVG/래스터분해와절차적NIF기하,지팡이의Blender절차적모델링이다. 단일이미지의자동3D추론/포토그래메트리/특정AI메시서비스를사용했다고주장하지않는다. 사용자기억과현재스크립트의증거범위를구분한다.
이미지→실루엣/파트/단면/재질구분→축/기준크기→UV/알파→기하생성→NIF구조조립→DDS/밉맵→독립재읽기→실제출력NIF렌더→SkyrimQA의순서다. 원본이미지의픽셀을무단텍스처로복사한것과이미지형태를참고하여새표면을만든것은다르다.
모든최적화를decimation이라고부르지않는다. 실제확인된것은파트별기하예산,UV/법선경계정점분리,단순충돌재사용,공용메시/STAT,바이트중복제거,재질별텍스처해상도/압축/밉맵,불필요노드/참조제거,조건부전용파생자산이다. 측정하지않은FPS/VRAM개선수치를만들지않는다.

## 7. 마법진 제작·교정 연구
ChargeCannon/r3의make_textures.py는참고문양을OuterRing,Face,HebrewBand,Glow,BeamStreak,Impact로분해하여SVG원을래스터합성한다. Face2048,Outer1024,Hebrew띠4096×256,Glow256등역할별해상도와알파를쓴다. 히브리어102문자를원주/띠UV비율에맞춰가로압축한다. 문장의언어정확성은원어민검수와별도다.
gen_nifs.py의quad_xz/annulus/cylinder/half_sphere가정점·삼각형·UV·법선을만든다. 마법진평면XZ,법선/빔장축+Y를명시한다. annulus는seam정점을포함한seg+1구조,빔단위길이512와주기UV를쓴다. Static은자동회전컨트롤러를제거하고Native노드회전과충돌하지않게한다. NeutralTintable은알파·기하를유지하고RGB만중성화하지만공유material의런타임변경격리는별도검증대상이다.
S03파생OuterBand/HebrewBand/OuterRing은원본에서필요한CC_*부분만선택한다. Full/OuterBand를섞은8중구성은티어별모놀리식NIF8개를만드는대신공통파트를인스턴스화한다. 과거S03전용작업의전체불변성gate는외부Production변경으로STOP이었으며자체NIF정적PASS와구분한다. 승인없이그때후보를현재Production에승격하지않는다.
CP021두께는기존전면/UV/재질을보존한sidewall파생이었다. CP026에서탑FIELD scale8.5에depth3이곱해져25.5u벽이된문제가확인됐다. FIELD depth.06으로.51u,SKY별도모델depth.1032로t8.4386u가되게국소보정했다. 단일공유진을모두얇게줄여다른주문을훼손하지않는다.
CP027세계는바깥띠197.8에비해내부Face105가작아Face/NodeGlow만1.65배173.25로맞췄다. 위치·방향·반경·폭발중심은그대로다. 풀에섞인서로다른아트모델은동일Hermit모델로정리했다. 헤일로는띠전용3-shape와독립ARTO DC5로유지한다.
중요실패:CreateShapeFromData가전달한property ID를실제복제하지않는래퍼동작,누락getShaderFlags1 export,Vector2.u/v와x/y혼동. 원본shader/alpha블록을보존하는CloneShape경로로바꾸고출력NIF재읽기및전면불변성을검사했다. Nifly가텍스처경로를메모리에서정규화하므로Load성공만으로직렬화된rawpath를검증하지않는다.

## 8. 지팡이 이미지 기반 제작과 조정
확인가능한FernStaff초기ASSET_REPORT와build_model.py는첨부이미지의굽은목재·비대칭상단·보라감개/리본·은장식·붉은펜던트를새절차기하로만들었다고기록한다. PROFILE표의z별중심/반경을loft하고표면frame을계산하며링·감개·장식·리본·보석을별파트로만든다. bmesh로면방향을재계산하고UVseam/법선경계에서출력정점을분리했다. 이미지픽셀은표면에복사하지않았다.
초기Fern출력은9 BSTriShape/24146정점/44064삼각형으로기록돼있다. 각파트15000미만은작업예산이며엔진일반상한이라는뜻이아니다. 이수치는초기Fern버전이고현재Arcana Silver/Gold에그대로인용할수없다. v0.2는납작한상단/가는몸체/반복재질을추가참고이미지에맞춰수정한사례다.
바닐라staff01/1stpersonstaff01에서장착/INV/PrnWeaponStaff/BSX/간단한캡슐충돌을보존하고기하만교체했다. BlenderZ장축→NIFY장축변환(x,y,z)→(x,z,-y)를명시했다. 초기바닐라길이134.60506u를기준으로잡아손잡이원점과게임크기를독립감사했다. 드롭물리와옷감충돌은다르다.
초기표면은목재/천2048,은1024,보석512의전용diffuse와normal+spec alpha,DXT압축과완전밉체인. 실제후속Arcana입력은금속환경맵/마스크와별도재질을쓴다. staff_assemble.cpp는FrierenStaff.spec의MESH파트별정점/UV/법선/삼각형을읽고shaderflags,금속·보석cubemap/mask,tangents,bounds를작성한뒤다시읽는다. 금색모델제작의모든초기프롬프트/원이미지/모델링전단계는현재자료만으로완전복원되지않았다. 보존된spec와조립코드의범위만확인사실이다.
현재Silver는FirstClassStaff,Gold는MillennialStaff계보이며이름교체가새모델8개를뜻하지않는다. 8일반+2해방WEAP가2모델/2공용1인칭STAT를사용한다. 카드별STAT/메시복제시안은불필요하여철회했다. MODL과WNAM을모두검사해야1인칭과3인칭경로가동시에맞는다.
SMP는첨부Riilungolz참고의skinnedweapon+helperarmor/XML+장착이벤트구조를분석했다. 참고자산은복사하지않았다. 자체Silver/Gold리그,체인9뼈/kinematicanchor,dynamic8,slot61/60보조체,2모델동일양손fallback,피부weight/partition/inversebind검사를작성했다. genericweaponroot대신privatebone00을skinroot로쓰는후보도실패했다. 정적스키닝재구성과XML검증은실제solver활성을증명하지못했다. 사용자최종관측은천미작동이며CP029배포에서제외한다. 원인미확인을물리성공으로수정하지않는다.

## 9. 책표지·메시·텍스처 개선
사용자는화려한ARCANA장식을거부하고기본전공필수교재,히브리어제목,기존마법진띠기능4문장,하단저자Frieren을지시했다. 실제image_gen요청/최종PNG/후면생성메타데이터가BooksAuthoring에있다. 단순한전공교재같은slateblue/charcoalcloth,오래된무광표면,절제된테두리/문자만최종채택했다.
제목קסם התקפי רגיל,기능הקסם נאגר בטבעות / הטבעות מתיישרות לקנה אחד / הקרן נורית ועוברת בכל אויב / בפגיעה באדמה האור מתפוצץ,저자מחברת: פרירן. 극중저자표시는실제자산저작자크레딧과다르다.
바닐라BSA의세계저폴리책과별도고폴리inventory책,텍스처/UV/충돌/컨트롤러를검증추출하여전용경로로연결했다. BOOK.MODL만바꾸면INAM의vanillaSTAT가재고화면에서남는다.15권공용inventorySTAT하나DC0를만들어15개복제를피했다. Bethesda원본추출물은GitHub소스에넣지않는다.
초기frontatlas를양면이공유하여뒷표지에도제목이반복됐다. 해결은텍스트없는back생성과2×2 atlas2048,앞면BC3block을mip0..8에서보존하는조립,back/innerUVtile분리다. 기존integer/negativeUV를유효하게처리하며전체UV를.5배하고후면tile을옮긴다. 최종1×1등작은mip는블록공유때문에앞면만독립보존된다고주장하지않는다.
책UV는NiSkinPartition에도들어가므로영향받는정점의4UVbytes를허용하되weight,partitionindex,컨트롤러,충돌,삼각형은독립비교한다. nifly기본optimize/sortBlocks는불필요한전블록재배열을유발하므로국소수정시둘다false로하고실제rawblock을비교했다.
DDS는각레벨을별도압축한뒤toplinearsize,mipmapcount,caps를실제payload로고쳐완전밉체인을조립했다. BC3/DXT5알파,바닐라plainback normal,blackemissionmask를썼다. 생성PNG가좋아도게임purpletexture/잘못된path라면완료가아니다. 책과지팡이purple회귀는실제NIFrawslash검증으로교정했다.

## 10. 비행·모션·외부 참고
비행컨트롤은Arcana0.7.4FlightStateFix를TOAM단일호스트에연결했다. 활/석궁다리6뼈복구와후처리,총기별OAR마커17개,원본무기28개프로필을분리했다. 무기피해/탄약은원모드소유다. 입력/중력/애니메이션/발밑효과의정리순서를보존한다.
상승효과는Jumpheld일때띠를생성하고키해제는새생성만끊는다. 이미표시된띠는.68초잔여페이드를완료한다. 메뉴/로드/사망/셀·비행종료는강제정리경로다. 갑작스런전체삭제와pending취소후지연ghost를동시에막아야한다.
635개물리HKX는345비행+452파생총기모션의797제작인스턴스와다른집계다. 크레딧은HydroBirous,Derwan,Redfall6,gp160,Khenta,RavenKZP,QTRModding,a skrub및OAR Ershin등을구분한다. 파일별원저자불명이면불명으로남긴다. FlightFramework는기술참고이며DLL/코드를번들하지않는다.
2026-10-26FlyingAnimationsResource원페이지는크레딧조건자산사용과별도수정/DonationPoints제약,다른저작자자산을함께표시한다. 출처가있다는이유만으로공개배포허가를얻은것이아니다. OAR overrideAnimationsFolder로외부리소스를요구하는방식은차후검토안이며현재검증된OAR구조를무단변경하지않는다.

## 11. ESP·경로·배포
한DLL/한MO2폴더통합은진짜ESP병합이아니다. CP021은657Arcana레코드를typedmap으로이관했고349충돌localID를별도재배치했다. 기존TOAM레코드localID는보존한다. 일반32bit치환은숫자/float/문자열을훼손하므로금지한다. xEdit정의에서검증한FormID필드offset만바꾸고역매핑전체바이트비교로증명한다.
CP029는애드온의실제StancesMASTER를다시추가한다. 이는사용자명시적패키지의존성요청이다. 과거MASTER가freeze원인이었다고단정하지않는다. 자체레코드/참조index2→3,mainindex1은보존한다. TES4MAST/DATA와다중EFID/EFIT는순서있는목록으로다루며dict로반복키를떨어뜨리지않는다.
OOM_부분치환이이미TOOM_인문자열을TTOOM_/TTTOOM_으로망가뜨렸다. 정확토큰과최종source literal/file closure로검증한다. NIFrawtexture는역슬래시를지키고라이브러리정규화이전바이트를검사한다. 저수준NIF로더가충돌메시를지원하지않아반환한오류는정상지팡이손상증거가아니다.
배포는Main과선택StancesZIP분리,루트영어+Language/en/kr보관,단일TOAM.dll,자료/소스/GitHub/인계폴더분리다. 불필요미사용자산은reference closure와전체바이트중복으로판정하고파일명이낯설다는이유로삭제하지않는다. SMP관련최종패키지는0개다.

## 12. 실행·검증 도구의 실수와 예방
PowerShellHOME/PID등예약변수,중첩배열자동평탄화,독립else/finally,인용중복,UTF16로그를UTF8로가정,62자리해시,예상후보해시추측,008자기행요구가실패를반복했다. 완결된하나의scriptblock,이름있는레코드,실제바이트/타임스탬프와현재상태PRE/POST/UNKNOWN분류를사용한다.
헤더전체문자열일치/추측함수명/토큰동일함수공존을API존재검증으로쓰지않는다. symbol→실제선언→native계약→호출자전수순서다. min/max/GetObject매크로와PCH경계를검사한다. CommonLib테이블이맞아도ABI나호출시점안전이틀릴수있다.
기존build/msvc MSVCx64/Ninja증분빌드만쓴다. source/config/docs/assets/deploy/008과mutableNinja상태를모두백업하여실패시일관복구한다. 옛DLL만새객체위에덮어쓰지않는다. 완료한단계는입력이안바뀌면반복하지않는다. 공중모션/픽셀/소리/스태프0MP/추출불가는각각수동판정이며정적설정성공과구분한다.

## 13. 후속 검증과 재사용 조건
CP029:할인0/50/100%,모든8종비용3배,태양4650완충후추가지불없음,정의5회이후계속피해·3초종료·차폐/시체차단,탑150/900/27900·다중대상,지팡이MP0/충전·20000/40000·소울젬/인챈트보호,스탠스전환소유권,언어교체·모델가시성을같은수동세션에서확인한다. 원158질문은고치지않는다.
본체핵심이전에사용자PASS지만신규밸런스는NOT_RUN이다. AE/VR,철회한SMP,외부권리미해결을PASS로표시하지않는다. 공개소스는라이선스미결정이므로열람을재허가와혼동하지않는다. 다음AI는실제현재파일과manifest를먼저대조하고이문서의과거실패를예방검사로연결해야한다.

## 14. 누적 원문 부록 — 역사 자료
아래는이전준칙·교훈·연구·보고서의복원본이다. 개인경로를비식별화하고원본SHA를출처표에남겼다. 과거수치,설계,Current,미완료/FAIL은의도적으로보존한다. 본문CP029가현재권위이며부록은자동실행지시가아니다.


### 원문 부록: 전체 누적 개발 교훈 S00–CP028
원본 SHA256: 1366137719F673C7C8C104D340234186834CEB8BF03FE511A5D912CCEC9D04B5

# Basic Attack Magic — DEV LESSONS

이 문서는 Basic Attack Magic 개발 중 발생한 시행착오, 오진, 실패, 원인, 해결책 및 재발방지 준칙의 누적 정본이다.

향후 새 시행착오가 발생하면 별도 DEV LESSONS 문서를 만들지 않고 이 파일에 계속 추가한다.

---

## S00-001 — PowerShell host bitness 과잉 추적

### 상황/증상
초기 개발환경 확인에서 PowerShell64=False가 표시되었으나,
MSVC Host/Target은 x64였고 cl.exe 역시 x64 compiler로 정상 동작했다.

### 잘못된 가정/실수
PowerShell 프로세스 자체가 64-bit가 아니면 x64 DLL 빌드가 불가능한 것처럼 취급했다.
그 결과 Sysnative를 통한 64-bit PowerShell 재실행을 불필요하게 시도했다.

### 실제 증거
- HostArch=x64
- TargetArch=x64
- MSVC 19.51 x64
- CMake 4.3.1
- Ninja 1.13.2

### 원인
PowerShell host bitness와 MSVC target architecture를 혼동했다.

### 해결
DLL 빌드 권위는 Host/Target architecture 및 실제 cl.exe target으로 판정한다.
PowerShell64=False 자체는 blocker로 취급하지 않는다.

### 이후 준칙
- PowerShell bitness만으로 빌드 가능 여부를 판정하지 않는다.
- cl.exe가 x64 target인지 먼저 확인한다.
- 환경과 프로젝트 오류를 분리한다.

---

## S00-002 — CMake configure 반복 초기화

### 상황/증상
cmake configure가 매번 `Found Threads: TRUE` 부근에서 멈춘 것처럼 보여
configure 실패 또는 hang으로 오인했다.

### 잘못된 가정/실수
재시도마다 다음 명령을 사용했다.

    cmake --fresh --preset release-windows

이 때문에 CMake cache가 매번 삭제되고 CommonLibSSE-NG configure가 처음부터 반복되었다.

### 실제 증거
vcpkg install, MSVC 탐지, CXX ABI 탐지, Threads 탐지는 모두 정상 통과했다.
실제 오류 메시지는 존재하지 않았다.

### 원인
`--fresh`가 기존 configure 진행 상태와 cache를 제거한다는 점을 고려하지 않고 반복 실행했다.

### 해결
다음 명령으로 cache를 유지한 채 configure를 계속 수행했다.

    cmake --preset release-windows

그 결과:

    -- Configuring done
    -- Generating done
    -- Build files have been written to: [PROJECT_ROOT]/build/msvc

까지 정상 완료했다.

### 이후 준칙
- `--fresh`는 초기 생성이나 cache 폐기가 실제로 필요한 경우에만 사용한다.
- 재시도 목적으로 습관적으로 `--fresh`를 사용하지 않는다.
- `Found Threads: TRUE`는 정상 탐지 결과이며 오류가 아니다.
- 오류 메시지가 없으면 임의로 FAIL 판정하지 않는다.

---

## S00-003 — Bootstrap build 성공

### 상황
configure 완료 후 Windows/MSVC preset으로 최초 DLL build 수행.

### 결과

    [490/490] Linking CXX shared library BasicAttackMagic.dll

### 산출물
- DLL: [PROJECT_ROOT]\build\msvc\BasicAttackMagic.dll
- Length: 545792
- SHA256: 12B0827090FAA190C9D1DCF8ABE1B7406A63D48B58F5C81BD35ADC87D49E6E66

### 판정
- Configure: PASS
- Generate: PASS
- Build: PASS
- Runtime Load: NOT TESTED
- S00 Final: PRE-LOCK

### 이후 준칙
Build PASS와 Runtime Load PASS를 구분한다.
DLL이 링크되었다는 이유만으로 Stage를 LOCK하지 않는다.

---

## 누적 운영 규칙

1. 시행착오/오진/실패/복구는 이 문서에 계속 누적한다.
2. 기존 항목은 특별한 이유 없이 삭제하지 않는다.
3. 후속 사실이 기존 판정을 뒤집으면 기존 기록을 지우지 않고 정정 이력을 추가한다.
4. 기록 순서는 Stage 및 발생 순서를 따른다.
5. 각 항목은 `상황/증상 → 실수 → 증거 → 원인 → 해결 → 이후 준칙` 구조를 우선 사용한다.
6. 중요한 실패는 성공 결과와 함께 보존한다.
7. 콘솔 출력만으로 충분한 사소한 사항은 기록을 남발하지 않는다.
8. 같은 유형의 실수가 재발하면 최초 항목을 참조하고 재발 항목을 별도로 추가한다.

---

## S01-L01 — Native stderr / PowerShell wrapper false-stop

상황/증상:
vcpkg가 정상 완료됐지만 Windows PowerShell이 NativeCommandError를 표시하고 실제 compiler 단계 전에 wrapper가 중단됨.

잘못된 가정/실수:
$ErrorActionPreference=Stop 상태에서 native command의 stderr를 2>&1 + Tee-Object로 PowerShell error stream과 결합해 build verdict로 사용함.

실제 증거:
재실행 결과 EXIT=0, FixedDamage/MagickaTransaction/WeaponHitSource 컴파일 및 BasicAttackMagic.dll 링크 성공.

원인:
실제 CMake/compiler 실패가 아니라 Windows PowerShell 5.1 native stderr wrapper false-stop.

해결:
native build는 exit code + compiler/link result + artifact를 authority로 판정하고 로그는 필요시 cmd /c redirection을 사용.

이후 준칙:
stderr 출력 자체를 FAIL로 판정하지 않는다.
PowerShell NativeCommandError만으로 compiler failure를 선언하지 않는다.
---

## S01-L02 — 채팅 실행 블록 분절

상황/증상:
사용자 실행용 PowerShell 블록 내부의 문서 here-string에 Markdown triple-backtick을 다시 삽입하여 채팅 렌더러에서 하나의 실행 블록이 여러 조각으로 분리됨.

잘못된 가정/실수:
PowerShell here-string 내부 문자열과 ChatGPT Markdown code fence가 서로 독립적으로 처리될 것이라고 가정함.

원인:
채팅 Markdown renderer가 내부 triple-backtick을 외부 코드블록 종료로 해석함.

해결:
사용자 실행용 코드블록 내부에는 Markdown triple-backtick을 넣지 않는다.

이후 준칙:
긴 PowerShell은 기능별 짧은 블록으로 나눈다.
문서 문자열 안의 예시는 plain text로 기록한다.
---

## S01-L03 — sandbox 문서와 로컬 문서 혼동

상황/증상:
ChatGPT sandbox에서 생성한 002/003/004 문서를 사용자 G: 로컬 evidence에 이미 존재하는 것처럼 후속 스크립트가 가정함.

잘못된 가정/실수:
sandbox artifact 생성과 사용자 로컬 파일 저장을 동일한 상태로 취급함.

실제 증거:
[PROJECT_ROOT]\evidence 실제 목록에는 002/003/004가 존재하지 않았음.

해결:
sandbox artifact는 사용자가 실제 로컬 경로에 저장했다고 확인되기 전까지 로컬 정본으로 간주하지 않는다.

이후 준칙:
후속 스크립트가 로컬 파일 존재를 요구할 경우 먼저 실제 경로를 확인한다.
sandbox:/ 링크와 [LOCAL_PATH_REDACTED]
---

## S01-L04 — 부분 코드 수정을 사용자에게 위임

### 상황/증상
Runtime.cpp의 C4834 경고를 수정하면서 ChatGPT가 두 코드 구간을 설명만 하고 사용자에게 직접 부분 편집하도록 요청했다.

### 잘못된 가정/실수
수정 범위가 작다는 이유로 소스 편집 자체를 사용자에게 넘겼다.

### 실제 증거
사용자가 부분변경을 직접 맡기지 말고 한 번에 실행 가능한 블록으로 제공하도록 교정을 요청했다.

### 원인
개발 지원 역할과 설명 역할을 혼동하여, 자동화 가능한 반복 편집을 사용자 수작업으로 전가했다.

### 해결
현재 로컬 파일을 먼저 검증하고, 예상되는 정확한 수정 지점 수를 확인한 뒤 원자적 PowerShell 패치로 수정한다. 이어서 build, warning 확인, deploy, hash 검증까지 같은 작업 단위에서 수행한다.

### 이후 준칙
- 자동화 가능한 소스 부분 변경을 사용자 수동 편집에 맡기지 않는다.
- 패치는 현재 파일의 정확한 anchor와 예상 match count를 먼저 검증한다.
- 예상 match count가 다르면 write 전에 중단한다.
- 소스 수정 후 build와 관련 warning/error 검증까지 함께 제공한다.
- 가능한 경우 build/deploy/hash 검증까지 하나의 완결된 작업 단위로 제공한다.
---

## S01-L05 — xEdit Pascal encoding 및 repair false-stop

### 상황/증상
Ordinary Offensive Magic S01 generator 실행 시 SSEEdit 4.1.5f가 UserScript line 584에서 `Expression expected` parser error를 냈다. 이어진 복구 스크립트는 project-local/install SHA256 불일치를 발견하자 실제 복구 전에 다시 중단됐다.

### 잘못된 가정/실수
1. 일반 source의 UTF-8 no-BOM 규칙을 xEdit Pascal에도 그대로 적용했다.
2. encoding repair가 필요한 상황에서 deploy copy와 source copy의 사전 byte-identical 상태를 필수 조건으로 만들었다.
3. 복구 대상이 이미 불일치할 수 있는데도 검증 guard를 repair보다 앞에 두었다.

### 실제 증거
기존 PASS 이력이 있는 TULL Stage12A v5와 Stage15 MageFamily xEdit Pascal 파일은 실제 raw bytes가 EF BB BF로 시작하는 UTF-8 BOM 파일이다.
BAM generator는 최초 작성 시 UTF-8 no-BOM이었다.
S01-014 첫 복구 시도는 `Local/install generator mismatch before encoding fix`로 write 전에 false-stop했다.

### 원인
검증된 project-local xEdit script의 실제 encoding을 확인하기 전에 일반 파일 encoding 규칙을 적용했고, 복구용 validator가 복구 자체보다 강한 선행조건이 되었다.

### 해결
project-local generator를 source authority로 사용한다.
네 Korean FULL 문자열은 EDID/NewSpellShell 구조 anchor로 복구한다.
xEdit `.pas`는 UTF-8 BOM으로 다시 기록한다.
그 후 project-local source를 Edit Scripts 설치본으로 배포하고 마지막에 SHA256 byte identity를 검증한다.

### 이후 준칙
- xEdit `.pas` encoding은 검증된 xEdit Pascal precedent를 우선한다.
- 비ASCII 문자열이 있는 현재 xEdit `.pas`는 UTF-8 BOM을 사용한다.
- source/deploy mismatch는 repair 작업의 자동 실패 조건으로 삼지 않는다.
- source authority를 먼저 복구하고 이후 deploy/hash identity를 검증한다.
- validator가 repair보다 앞서 새로운 false-stop 계층이 되지 않게 한다.
- xEdit 문제 발생 시 같은 계열 DEV_LESSONS와 실제 PASS script를 먼저 검사한다.
---

## S02-R01 — CMake anchor validator false-stop recurrence

### 상황/증상
S02 ChargeProfile/ChargeSession 최초 production integration 시도에서
CMakeLists.txt에 실제로 `src/Core/MagickaTransaction.cpp`가 존재했지만
사전 validator가 `CMake MagickaTransaction anchor count=0, expected=1`로 중단했다.

### 실수
이미 직전 read-only 감사에서 실제 CMake 등록 행이 확인되어 있었음에도,
`^공백*src/Core/MagickaTransaction.cpp공백*$` 형태의 whole-line 정규식을
추가 권위처럼 사용했다.

### 실제 증거
직전 감사 출력:
`83: src/Core/MagickaTransaction.cpp`

첫 S02 시도:
`CMake MagickaTransaction anchor count=0, expected=1`

### 원인
실제 파일 내용보다 새로 만든 validator의 형식 가정이 더 엄격했다.

### 해결
whole-line 정규식 판정을 폐기하고 실제 행에
`src/Core/MagickaTransaction.cpp`가 포함되는지만 사용한다.

### 이후 준칙
이 항목은 새로운 일반 원칙이 아니라 기존
"validator 자체가 failure layer가 되지 말 것" 준칙의 재위반 기록이다.
이미 확인된 local source authority를 불필요한 형식 validator로 다시 부정하지 않는다.

---

## S02-L01 — standalone compile과 production PCH의 min/max macro 차이

### 상황/증상
ChargeProfile / ChargeSession standalone semantic test는 PASS했으나
production CommonLib/PCH build에서 C2589가 발생했다.

### 실제 증거
Production build:
- `ChargeProfile.cpp(76): error C2589`
- `ChargeProfile.cpp(82): error C2589`
- `ChargeProfile.cpp(95): error C2589`
- `ChargeSession.cpp(137): error C2589`
- `ChargeSession.cpp(145): error C2589`

오류 위치는 새 코드의 `std::min(...)` / `std::max(...)` 호출이었다.

### 실수
standalone compile 환경과 production PCH/Windows preprocessing 환경을
동일한 것으로 취급했다.

### 원인
production PCH에서 Windows `min` / `max` macro가 노출되어
`std::min(...)` / `std::max(...)` 토큰과 충돌했다.

### 해결
이번 Charge core에서는 전역 build policy를 변경하지 않고
`std::min/std::max`를 명시적 비교식으로 교체한다.

### 이후 준칙
- standalone semantic PASS와 production target BUILD PASS를 구분한다.
- CommonLib/Windows PCH가 적용되는 코드의 최종 compile authority는 production target이다.
- Windows macro 충돌 가능 표준 함수는 실제 프로젝트 PCH 정책을 확인한다.

---

## S02-L02 — PowerShell Test-Path boolean expression ParserError

### 상황/증상
S02 R3 실행 직후 PowerShell parser가 다음 오류로 중단했다.

`'if' 문의 식 뒤에 닫는 ')'가 없습니다.`
`식 또는 문에서 예기치 않은 ')' 토큰입니다.`

### 실수
다음과 같이 PowerShell command invocation을 직접 `-or`로 연결했다.

`if (Test-Path $PH -or Test-Path $PCPP ...)`

### 실제 영향
ParserError는 스크립트 실행 전 발생했다.
따라서 해당 R3 시도로 인한 source/CMake/DLL/deploy 변경은 0이다.

### 원인
PowerShell에서는 각 command invocation을 boolean expression으로 쓸 때
각 호출 결과를 괄호로 분리해야 한다.

### 해결
복합 `Test-Path` 표현을 제거하고,
향후에는 경로 배열을 `foreach`로 순회해 existing-file 목록을 만드는 방식으로 단순화한다.

### 이후 준칙
- 긴 실행 블록에서 복잡한 command-expression boolean chaining을 피한다.
- parser-level 오류가 난 시도는 실행/변경 여부를 별도로 판정한다.
- 사용자에게 부분 수정을 맡기지 않고 수정된 전체 실행 블록을 다시 제공한다.

---

## S02-L03 — Magician T8 stat cap과 drain cap 혼동

### 상황/증상
S02 R4의 ChargeProfile/ChargeSession standalone semantic test와 production build는 PASS했지만,
후속 S02-D source authority 감사에서 Magician의 effective charge가 5000을 넘어
5250 이상으로 증가할 수 있는 구현이 확인됐다.

### 잘못된 구현
`ChargeProfile::ClampAccumulatedCharge()`가
`stopDrainAtStatCap == false`일 때 stat cap 자체도 적용하지 않았다.

R4 semantic test 역시 5000 이후 250을 추가 소비한 결과를
`5250`으로 기대하여 동일한 잘못된 가정을 검증했다.

### 실제 설계 권위
Magician T8 = 5000은 stat/power cap이다.
다만 consumption cap이 아니므로 T8 도달 뒤에도 held 상태에서는
500 MP/s per hand 소비를 계속한다.

즉 다음 두 규칙은 독립이다.

- effective/stat charge cap
- drain stop/continue policy

### 원인
`stopDrainAtStatCap` 하나를
"자원 소비를 멈출지"와 "누적 charge를 cap할지" 두 의미에 동시에 사용했다.

### 해결
`ClampAccumulatedCharge()`는 profile 종류와 관계없이 항상
`chargeStatCap`을 적용한다.

`stopDrainAtStatCap`은 `RequestedDrain()`에서만 사용한다.

따라서:

- Magician: Charge=5000 유지 + drain 계속
- Judgement: Charge=1800 유지 + drain 중단

### 이후 준칙
자원 소비 상한과 효과/stat 상한을 하나의 boolean 의미로 결합하지 않는다.
테스트에서는 반드시
"cap에 도달한 뒤 실제 소비가 계속되는가"와
"effective value가 cap을 넘지 않는가"를 별개 assertion으로 검증한다.

이 교정은 historical R4 evidence를 삭제하거나 수정하지 않고
별도 correction evidence로 보존한다.

---

## S03-L01 — PowerShell `$ARCH` / `$Arch` 환경 변수명 충돌

### 상황/증상
S03-A Direct NIF Foundation 실행본이 source write 전에 중단됐다.

오류:

`값은 Arch 변수에 대해 올바른 값이 아니므로 변수의 유효성을 검사할 수 없습니다.`

`FullyQualifiedErrorId : ValidateSetFailure`

### 잘못된 부분
스크립트에서 architecture evidence 경로 변수명을 `$ARCH`로 사용했다.

PowerShell 변수명은 대소문자를 구분하지 않으므로
Visual Studio Developer PowerShell 환경에 이미 존재하던 `$Arch`
변수와 충돌했다.

기존 `$Arch`에는 허용값 검증이 걸려 있어서 파일 경로 문자열 대입이
거부됐다.

### 영향
변수 선언 단계에서 중단되었으므로 production 변경은 없었다.

- source write 0
- CMake write 0
- build 0
- deploy 0
- asset promotion 0
- ESP mutation 0

### 해결
경로 변수명을 `$ArchitectureEvidencePath`처럼 충돌 가능성이 낮은
구체적 이름으로 교체한다.

### 이후 준칙
VS Developer PowerShell에서 실행할 대형 자동화 블록은
`Arch`, `Host`, `Home`, `Profile`, `Error`, `Input`, `Args`처럼
환경/자동 변수로 쓰일 가능성이 높은 짧은 이름을 피한다.

실행 전에 모든 top-level PowerShell 변수명을 환경 충돌 관점에서도
감사한다.

### S03-R02 — asset filename validator authority recurrence

S03-A R2 stopped before production write because the script expected
`Beam_Body_Static.nif`, while the immutable r3 ZIP actually contains
`Beam_Body_Unit_Static.nif`.

The r3 ZIP itself passed its locked SHA256 authority.

This is treated as a recurrence of the existing authority/validator rule:
do not manufacture a failure layer from reconstructed expectations when
the actual authoritative artifact can be enumerated directly.

For archive-backed assets, derive exact filenames from the locked archive
before creating required-file validators.

Production impact of this occurrence was zero.

### S03-R03 — production PCH max macro 재위반

분류:
새 교훈이 아니라 기존 `S02-L01 — standalone compile과 production PCH의 min/max macro 차이`
준칙의 재위반이다.

상황/증상:
S03-A R3 production build에서 `TransientVFXManager.h`의
`std::numeric_limits<std::uint16_t>::max()` 두 곳이 Windows `max`
macro와 충돌하여 C4003/C2589가 발생했다. 이후 다수의 class/parser
오류는 최초 전처리 오류의 연쇄 오류였다.

잘못된 부분:
이전 S02에서 `std::min(...)` / `std::max(...)`가 production PCH에서
실패한 사실을 이미 기록했지만, 준칙을 표준 algorithm 함수에만
좁게 적용했다. Windows의 function-like `max` macro는
`numeric_limits<T>::max()`의 `max` 토큰에도 동일하게 개입한다.

실제 영향:
production build는 FAIL했고 build acceptance 전에 rollback되었다.
source/CMake는 원복되었고 DLL deploy, ESP, Codex 산출물은 변경되지 않았다.

해결:
uint16 sentinel에는 `std::numeric_limits<T>::max()`를 사용하지 않고
명시 상수 `0xFFFFu`를 사용한다.

이후 준칙:
CommonLib/Windows PCH 코드에서 기존 min/max macro 교훈은
`std::min/std::max` 함수에 한정하지 않는다.
function-like `min`/`max` 토큰이 호출 형태로 나타나는 모든 표현을
production build 전에 확인한다.
같은 문서화된 실패 유형을 반복하면 새 lesson 번호를 만들지 않고
재위반 기록으로 누적한다.

<!-- OOM_SESSION_CUTOFF_20261017_045219 -->

## 2026-10-17 — Session cutoff consolidation

This is a consolidation/index entry. Existing detailed lesson IDs remain
authoritative and are not renumbered or duplicated here.

Session ended after S05-C1 BUILD/DEPLOY PASS and before execution of S05-C2.

Revalidated cross-stage lessons:

- Rules-First reading is mandatory before production code.
- Search existing DEV_LESSONS for the same failure family before inventing a fix.
- current-local source/artifact + SHA256 outranks chat/sandbox assumptions.
- guessed API/type/path/file names are prohibited.
- enumerate actual asset contents before writing validators.
- use exact-one patch anchors; zero or multiple matches abort.
- preserve encoding/newline conventions.
- xEdit Pascal BOM requirements are distinct from normal no-BOM source rules.
- native stderr alone is not proof of build failure.
- build failure prohibits deployment.
- build/deploy equality requires byte/hash verification.
- partial release must never be promoted to Magician final-fire authority.
- transient VFX remains under managed cleanup authority.
- rolled-back/failed attempts remain development evidence.
- STATIC / BUILD / DEPLOY / RUNTIME / LOCK states must never be conflated.
- SSEEdit remains deferred to the planned integrated S03-S08 pass.

Detailed cutoff state is recorded in:

STAGE_EVIDENCE\S05_THE_MAGICIAN\S05-013_SESSION_CUTOFF_HANDOFF_20261017_045219.md

<!-- OOM_S06_R01_GUESSED_TRIGGERFIREWALL_CPP_RECURRENCE_20261017 -->

## S06-R01 — guessed TriggerFirewall.cpp collector false-stop recurrence

### 상황/증상
S06 current-local audit package collector가
`src\Combat\TriggerFirewall.cpp`를 필수 입력으로 요구했고,
실제 current-local source에는 해당 파일이 없어 package 생성 전에 중단됐다.

### 실수
문서의 `TriggerFirewall.*` 모듈 표기를 근거로
실제 파일을 열거하지 않은 채 `.h + .cpp` 쌍이 존재한다고 추정했다.

### 실제 증거
후속 current-local source 전체 열거 결과:
- `src\Combat\TriggerFirewall.h` 존재
- `src\Combat\TriggerFirewall.cpp` 없음
- Trigger Firewall 구현은 현재 header-only authority

실패 시점은 source/build/deploy/ESP mutation 전이었다.

### 원인
기존 S02-R01 / S03-R02와 같은 failure family:
실제 local authority보다 reconstructed filename expectation을
더 강한 validator로 사용했다.

### 해결
S06 audit package는 `src` 전체를 실제 열거해 수집했다.
현재 source tree의 실제 파일 목록과 SHA256을 authority로 사용한다.

### 이후 준칙
- 문서의 `Foo.*` 표기를 `.h + .cpp` 존재 증거로 해석하지 않는다.
- 기존 파일/path/API는 current-local tree에서 실제 확인한 뒤에만 validator에 사용한다.
- collector는 실제 authority를 수집해야 하며 guessed inventory를 강제하지 않는다.
- 같은 failure family의 validator false-stop을 반복하지 않는다.

---

<!-- OOM_S06_FAILURE_RECURRENCES_20261018 -->

## 2026-10-18 — S06 validator / PowerShell failure recurrence consolidation

This entry preserves the S06 false-stop chain without rewriting or deleting
the failed evidence.

### S06-R01 — unrelated ESP strict-gate false-stop

S06-B1E0 R1 reported an ESP authority mismatch even though the rendered
Expected and Actual SHA256 values were identical.

Actual impact:
- read-only audit stopped
- source mutation: none
- build/deploy mutation: none
- ESP mutation: none

Cause:
an unrelated ESP boundary had been promoted into a strict implementation
precondition, making the validator stronger than the work being audited.

Correction:
the unrelated ESP blocker was removed.
Only directly relevant current-local implementation authority remained strict.

Reusable rule:
do not manufacture failure layers from unrelated historical hash gates.


### S06-R02 — cross-file definition/use validator false-stop

S06-B1G-A reported:

First-shot delay authority exists = FAIL

The implementation itself was correct.

Actual architecture:
- JudgementRuntime.h defines kFirstShotDelaySeconds = 0.45F
- JudgementRuntime.cpp consumes kFirstShotDelaySeconds
- JudgementRuntime.h defines kSubsequentShotIntervalSeconds = 0.12F
- JudgementRuntime.cpp consumes kSubsequentShotIntervalSeconds

Cause:
the validator incorrectly required both the symbol and numeric literal to exist
in the consumer cpp file.

Correction:
definition ownership and runtime consumption are audited separately.

Reusable rule:
a centralized constant must be validated at its definition owner and consumer;
absence of the numeric literal in the consumer is not evidence of missing
authority.


### S06-R02B — document-scope contamination

The same B1G-A audit checked generic beam/explosion vocabulary across all of
004_SPELL_GAMEPLAY_AUTHORITY.md.

Magician wording therefore contaminated the Judgement freshness test.

Correction:
document semantic checks must first isolate the target spell/stage section.

Reusable rule:
a multi-system authority document must never be freshness-validated using
unscoped generic vocabulary.


### S06-R03 — PowerShell statement-as-expression ParserError recurrence

S06-B1G-A R2 failed during PowerShell parsing.

Problem form:
an if/else statement was placed inside a parenthesized string-concatenation
expression.

This is a recurrence of:
S02-L02 — PowerShell Test-Path boolean expression ParserError

Actual impact:
the parser failed before execution.
No project mutation occurred.

Correction:
avoid inline statement-style conditionals inside expression composition.
Compute state in a separate variable or use simple control flow.

Reusable rule:
Windows PowerShell 5.1 audit/patch blocks should prefer simple statements over
compact expression tricks.


### S06-R04 — brittle post-write natural-language validator

The first S06-B1G-B documentation reconciliation wrote candidate documents,
then rejected them because an exact natural-language token named
"004 target death continuation" was not found.

The catch path restored the backed-up documents.

Actual lasting impact:
none.

Cause:
post-write validation tested one reconstructed wording rather than the actual
semantic/canonical document result.

Correction:
do not use arbitrary prose fragments as hard blockers after a canonical section
replacement.

Reusable rule:
post-write validators must validate the artifact actually produced, not a
second independently reconstructed prose expectation.


### S06-R05 — PowerShell return-newline section reader bug

S06-B1G-B R2 attempted to inspect the current-local 004 Judgement section with
a helper containing a standalone:

return

followed on the next statement by:

$Text.Substring(...)

PowerShell returned from the function before evaluating the substring.
The caller therefore received an empty section and falsely concluded that the
current-local 004 was not in the expected state.

Actual impact:
the failure occurred before document write.
No project mutation occurred.

Correction:
return values remain on the same statement, for example:

return ($Text.Substring(...))

The current-local document was then re-audited directly before another repair
attempt.

Reusable rule:
never infer current-local document state through an unverified parsing helper.
When a helper result contradicts known file/hash evidence, inspect the actual
section directly before creating another validator.


### S06-R06 — canonical section replacement resolution

The successful S06-B1G-B R3 abandoned brittle semantic preconditions.

Successful method:

1. verify the actual current-local full-file SHA256
2. isolate the real current section boundaries
3. generate the complete canonical replacement section in memory
4. replace exactly one section
5. write with original encoding/newline convention
6. re-read the saved file
7. compare generated expected-section SHA256 against actual saved-section SHA256
8. verify non-target production boundaries remained unchanged

Result:
- 002 canonical S06 section: PASS
- 004 canonical Judgement section: PASS
- source: unchanged
- CMake: unchanged
- Build DLL: unchanged
- Deploy DLL: unchanged
- ESP: unchanged

Reusable rule:
for full authority-section reconciliation, canonical-section identity is
stronger and safer than a pile of guessed natural-language validators.

---

<!-- OOM_S07_C2_VALIDATOR_FALSESTOP_20261018 -->

## 2026-10-18 — S07-C2 exact-signature validator false-stop

S07-C2 R1 stopped before any source write with:

`Pinned REL::Relocation::write_vfunc API missing.`

Actual production impact:

- source mutation: none
- CMake mutation: none
- build: not invoked
- deploy mutation: none
- ESP mutation: none

The failure was caused by the audit validator, not by the pinned CommonLib API.

### Failure mechanism

The validator searched the pinned CommonLib headers for one guessed literal
signature fragment:

`write_vfunc(std::size_t`

That check incorrectly assumed:

- the declaration must be on one line
- the parameter spelling must exactly match the guessed form
- no `const` may appear before `std::size_t`
- template formatting cannot split the declaration

The actual pinned CommonLib declaration in `REL\Relocation.h` is template-based
and formatted across multiple lines.

The pinned audit subsequently found three `write_vfunc` symbol matches in one
file and confirmed the relevant overload:

`std::uintptr_t write_vfunc(const std::size_t a_idx, const F a_newFunc)`

The same pinned audit also confirmed:

- `RE::ShoutHandler::VTABLE = VTABLE_ShoutHandler`
- `ShoutHandler::ProcessButton(ButtonEvent*, PlayerControlsData*)`
- `ProcessButton` virtual slot = `04`

### Correction

S07-C2A stopped searching for a guessed complete signature.

It instead:

1. searched the actual symbol name `write_vfunc`
2. collected the surrounding pinned-local declaration
3. inspected the declaration itself
4. inspected the actual ShoutHandler header
5. used those exact pinned declarations to rebuild C2

S07-C2 R2 then completed:

- ShoutHandler slot-04 input ownership hook
- The World selected -> consume vanilla processing
- other Shout/Power -> forward original processing
- DOWN -> begin World charge
- UP -> pending release
- final recurring charge tick -> release
- build PASS
- deploy PASS
- build/deploy byte identity PASS

### Reusable rule

Do not prove an API absent by searching for one guessed full-signature string.

For C++ APIs, templates, overloads, qualifiers, formatting, and line breaks can
make a valid declaration fail an exact textual-signature search.

Required order:

1. search the symbol name
2. locate its actual owning file
3. read the declaration with surrounding context
4. derive the production call from that pinned declaration

A validator must not be stricter than the language syntax or formatting of the
source it is validating.

A zero match for one guessed signature is not evidence that the API itself is
missing.
---

<!-- OOM_S08_R01_WEAPON_HIT_VALIDATOR_FALSESTOP_20261018 -->

## S08-R01 — Weapon HitData classification validator false-stop recurrence

### 상황/증상

S08-B1 최초 production integration block이 production write 전에 다음 오류로
중단됐다.

`Could not identify one current HitData classification function. CandidateCount=0`

### 실수

validator가 실제 current-local WeaponHitSource public interface를 먼저 소비하지
않고 하나의 reconstructed function shape를 권위처럼 요구했다.

후보 판정은 다음 가정을 동시에 강제했다.

- 특정 `Observe` 계열 full parameter shape
- 동일 함수 탐색 범위 안에 `kPowerAttack` 존재
- 동일 함수 탐색 범위 안에 `kBash` 존재

### 실제 증거

current-local `WeaponHitSource.cpp`에는:

- `void Observe(...)` 존재
- `kPowerAttack` classification 존재
- `kBash` literal은 존재하지 않음
- callback dispatch 존재

current-local `WeaponHitSource.h`에는 이미:

- `using Callback`
- `SetCallback(...)`
- `IsInstalled()`
- `Install()`

public consumer surface가 존재했다.

따라서 CandidateCount=0은 HitData authority 부재가 아니라 validator의 ownership /
token-colocation 가정 때문에 발생한 false-stop이었다.

실패 시점은 production write 전이었다.

Actual impact:

- production source mutation: none
- CMake mutation: none
- build: not invoked
- deploy mutation: none
- ESP mutation: none

### 원인

기존 S02-R01 / S06-R01~R06 / S07-C2와 같은 failure family다.

실제 current-local architecture보다 guessed full-signature / token-colocation
validator가 더 엄격했다.

또 producer가 반드시 bash exclusion까지 소유해야 한다고 가정했지만 실제
architecture에서는 WeaponHitSource가 raw HitData + power classification을
공개 callback으로 전달하고 Temperance consumer가 bash/ranged eligibility를
판정할 수 있다.

### 해결

S08-B1 retry는:

1. guessed full-signature search를 폐기했다.
2. current-local `SetCallback()` public surface를 직접 소비했다.
3. `WeaponHitSource.cpp`를 수정하지 않았다.
4. `kPowerAttack`은 existing producer classification을 사용했다.
5. `kBash` / ranged exclusion은 Temperance consumer에서 판정했다.
6. pinned CommonLib API는 symbol search -> actual declaration -> production call
   순서로 확인했다.
7. Build/Deploy 후 WeaponHitSource.h/.cpp byte identity를 확인했다.

Retry result:

- static wiring PASS
- configure PASS
- build PASS
- deploy PASS
- build/deploy byte identity PASS
- ESP unchanged
- Runtime PASS: NO
- S08 LOCK: NO

### 이후 준칙

- 한 producer 함수 안에 여러 policy token이 같이 존재해야 한다고 추정하지 않는다.
- classification ownership과 eligibility filtering ownership을 분리해서 감사한다.
- existing public consumer surface가 있으면 내부 producer patch보다 우선한다.
- C++ API 존재 여부를 guessed complete signature로 판정하지 않는다.
- zero match는 먼저 validator 가정을 의심하고 actual source context를 다시 읽는다.
- 같은 failure family 재발은 기존 lesson을 참조하면서 별도 recurrence evidence로 보존한다.
---

<!-- OOM_S08_R02_TIMED_BASH_FLAG_FAMILY_20261018 -->

## S08-R02 — Eligibility flag-family completeness / Timed Bash omission

### 상황

S08 Temperance B1/B2 static implementation은 `bash/shield bash`를 gameplay
비대상으로 정의했지만 consumer filter는 처음에 `RE::HitData::Flag::kBash`
하나만 검사했다.

### 실제 current-local API 증거

Pinned CommonLibSSE-NG:

`lib\commonlibsse-ng\include\RE\H\HitData.h`

SHA256:

`7959E83086F206AF83B5654B99191084D9B98759955C3CEBE64928BFB8D6D806`

실제 enum declaration:

- `kBash = 1 << 14`
- `kTimedBash = 1 << 15`
- `kPowerAttack = 1 << 16`

따라서 `kTimedBash`는 별도 bit이며 `kBash`와 항상 동반된다고 추정할
근거가 없다.

### 원인

Gameplay authority가 하나의 의미 범주인 `bash/shield bash`를 제외한다고
적혀 있었지만, 실제 API에서는 그 의미 범주가 여러 독립 flag로 표현될 수
있다는 점을 B1 static review에서 끝까지 열거하지 못했다.

이는 guessed API를 사용한 오류와 반대 방향의 문제다.

- 존재하지 않는 flag를 추측해서도 안 됨
- 실제 pinned API에 존재하는 독립 flag를 의미상 같은 범주라는 이유로
  암묵적으로 포함됐다고 추정해서도 안 됨

### 해결

Runtime QA 전에 current-local `HitData.h`를 직접 감사하고 Temperance
eligibility filter를 다음 두 flag 모두 제외하도록 교정했다.

- `kBash`
- `kTimedBash`

교정 후:

- configure PASS
- build PASS
- deploy PASS
- build/deploy byte identity PASS
- ESP unchanged
- gameplay fixed-damage call count = 1
- Temperance Trigger Firewall origin preserved
- Runtime PASS = NO
- S08 LOCK = NO

Current Temperance source:

`601728BAAEFAEAAAC5BB48E92CD65484CEB9E53FDDD40071BC300606745B6219`

Current Build/Deploy DLL:

- Length `722944`
- SHA256 `9065CB248EAF8BCF5D728D5131FA2A81885CEEDA60601844B6556A72DA320485`

### 재발 방지 준칙

- gameplay authority의 한 의미 범주가 실제 API에서 여러 독립 flags로
  표현되는지 pinned declaration을 확인한다.
- flag 간 implication은 실제 API/engine evidence 없이는 가정하지 않는다.
- eligibility exclusion은 consumer가 소유해도 되지만, 그 consumer는
  authority가 요구하는 실제 flag family를 완전하게 열거해야 한다.
- Runtime QA 직전에는 positive eligibility뿐 아니라 excluded-path flag family도
  current-local API와 다시 대조한다.

---

<!-- OOM_S08_DOCSYNC_FALSESTOPS_20261018 -->

## S08-DOC-R1/R2 — authority-document synchronizer false-stop recurrence

Situation:
- R1 passed all current-local document SHA256 guards, then stopped on
  `Unexpected encoding/newline` while processing 002, before any write.
- R2 passed all document SHA256 guards, then stopped while constructing
  004 in memory: `Missing anchor: 004 / Current DEV_LESSONS SHA256`.
- The user cancelled R2. Neither attempt reached document commit.

Evidence and cause:
- The uploaded 002 reference matching the validated hash is UTF-8 without
  BOM and uses LF. The exact PowerShell condition causing R1 is not isolated;
  it must not be blamed on a proven-corrupt document.
- R2's expected SHA256 fragment was shorter than the actual 004 text.
  A reconstructed exact-token check became an unnecessary failure layer.
- R2 was also presented despite containing unfinished executable content.

Correction and recurrence prevention:
- Read 000 and full cumulative 001 before generating production blocks.
- Compare exact current-local hashes and original section boundaries.
- Validate ALL candidate replacements before writing any authority document.
- Do not add speculative BOM/newline rejection beyond an exact file hash.
- Treat a zero-match as a possible validator fault; inspect real source.
- Preserve original byte encoding and unrelated sections where possible.
- Commit with original-byte backups, post-write hash checks and rollback.
- Never provide an incomplete executable block as an action-ready script.
- Keep failed historical evidence; only evidence-backed commit merits PASS.

---

## S03-R11-AUDIO-01 — pinned declaration is not sufficient native ABI proof

2026-10-19, caught during pre-deployment R11/audio integration verification.

Situation: the pinned BSSoundHandle header declares SetPosition(NiPoint3).
Its cpp wrapper targets SE ID66370. Static compilation alone accepts that call.

Actual local SE1.5.97 evidence: ID66370 maps to RVA0xBED920. The function reads
XMM1/XMM2/XMM3 as three scalar float arguments, queues those three coordinates,
and returns a success byte. Vanilla caller RVA0x5E915D..0x5E916F loads all three
scalars then calls that function. A 12-byte NiPoint3 parameter has a different
MSVC x64 ABI. The raw pinned overload therefore must not be used here.

The earlier audit draft also mislabeled RVA0xBED740 as SetPosition; ID66363
and the BGSSoundOutput secondary base at +0x20 identify it as SetOutputModel.
The corrected native proof is retained with the superseded draft in S03-013.

Correction: project-local SE1.5.97-only adapter to the same named SE ID66370,
typed bool(BSSoundHandle*,float,float,float). No CommonLib files are changed.
The engine's initialized DialogueOutputModel3D is resolved by the pinned
DefaultObject enum and attached through SetOutputModel before positioning.
Missing backend/model/position skips only audio. No FormID or flags are guessed.

Additional ownership proof: native FadeOutAndRelease (SE ID66385) invalidates
the passed handle's ID. The owner retains an original-ID copy while passing a
separate request copy, allowing explicit Stop of a fading tail on reset or cap
replacement. Audio events prune bounded ownership; no audio frame sink is added.

Evidence: S03-013_R11_AUDIO_PRODUCTION_20261019/NATIVE_AUDIO_UNPACKED_AUDIT.json,
NATIVE_FILE_CONTRACT_PROOF.json and NATIVE_AUDIO_CONTRACT.md.
The game executable was only analyzed via a temporary copy, never launched or
replaced. These intermediate DLLs were not deployed before correction.

Rule: separate declaration/signature proof, native ABI proof, build proof and
runtime playback proof. An available local header and BUILD PASS do not prove
the wrapper's argument ABI is correct. Actual audibility remains UNVERIFIED.

Retry note: the first ABI-correct production compile hit Windows GDI GetObject ->
GetObjectA macro expansion at the pinned default-object API call. Build failed
before deployment and all 25 source/document targets plus the prior build DLL
were restored. The retry uses push_macro/undef/pop_macro only in SpellAudio.cpp.
This extends the existing Windows/PCH token-collision lesson beyond min/max;
no global macro/build policy or CommonLib source is changed.


<!-- TOOM_INLINE_PS_WORKFLOW_LOCK_20261019 -->

---

## 2026-10-19 — C2-B2 R2 delivery regression and inline PowerShell workflow lock

### Situation / symptoms
- C2-B2 compiled ArcanaLimiterRuntime.cpp but failed linking with LNK2001 for InventoryChanges::IItemChangeVisitor destructor.
- The C2-B2 evidence later confirmed build-output rollback as OUTPUT_HASH_VERIFIED. MO2 deployment was unchanged.
- During preparation of R2, ChatGPT changed the established delivery method and provided a downloadable .ps1 file instead of a self-contained inline PowerShell transaction.
- The user was instructed to execute a file at [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\S08_TEMPERANCE\C2_B2_R2.ps1 without first establishing that it had been downloaded and saved there.
- The command failed with SCRIPT NOT FOUND. R2 patch was not executed.

### Previously successful workflow
- ChatGPT gave one atomic PowerShell block directly in chat.
- PowerShell read and verified actual current-local files and hashes, created backups, generated source or SSEEdit scripts in the appropriate local paths, executed the permitted step and validated the result.
- Detailed or lengthy audit/build output was saved automatically under stage evidence/_audit_logs and zipped when appropriate.
- The user transferred evidence ZIPs, not terminal dumps or manually staged executable scripts.

### Incorrect deviation
- ChatGPT switched delivery to an external downloadable .ps1 and expected the user to find, save and relocate it before executing.
- The supplied local execution path was treated as real before a local existence check or user confirmation.
- This repeated S01-L03 (sandbox artifact versus local-file confusion) and contradicted the established user workflow.
- The ensuing failure was a delivery and execution-gate fault, not a new C++ compiler or gameplay fault.
- Previous assistant advice to discontinue executable files entirely was also incorrect: the prohibition concerns requiring user-managed script downloads, not creating files automatically through PowerShell.

### Related failure family
- C1 earlier used sequential top-level PowerShell commands. Preflight throw statements did not prevent subsequently pasted commands from continuing. C1 ended in rollback with seven files verified against transaction baselines.
- C2-B2 used Start-Process -Wait, which remained pending after CMake/Ninja had terminated, plausibly due to mspdbsrv descendant processes. A similar stall occurred in Path of TULLBUNG.
- After the blocking descendant processes were resolved, C2-B2 reported FAIL and OUTPUT_HASH_VERIFIED.
- R2 must fix the pinned API/link defect separately from process-wait and script-delivery defects.
- Never claim build PASS after an unresolved linker error; never claim deployment without matching deployed-artifact evidence.

### Corrective action
- Keep the prior proven workflow: one complete inline Windows PowerShell transaction, not a downloaded executable file.
- Perform an explicit 64-bit PowerShell, VS Developer host/target and toolchain check at execution time; distinguish read-only audits from builds.
- If actual source files or required API declarations are missing from the available evidence, ask the user for the specific original files as attachments before generating production code.
- Re-read current 000 and full cumulative 001; identify the matching previous failure before writing any correction.
- Verify actual file hashes and candidate contents before mutation, back up original bytes and verify SHA256 after commit or rollback.
- Avoid Start-Process -Wait for MSVC builds. Capture the direct process exit and compiler/link diagnostics and ensure output artifacts match the intended result.
- Save long results and failures to the project's existing stage evidence/_audit_logs; emit a small console summary and provide a single ZIP path for user attachment.
- When an SSEEdit script is needed, generate it with PowerShell at the verified actual SSEEdit Scripts directory prior to launching SSEEdit.
- Never require the user to locate, download, reposition or manually edit script/source files.

### Acceptance / checkpoint
- C2-B1 source registration: PASS according to its evidence.
- C2-B2: C++ compilation completed, DLL link FAIL (LNK2001), output rollback SHA-verified.
- C2-B2 R2: NOT EXECUTED; SCRIPT NOT FOUND was a delivery failure.
- This documentation entry does not repair the C2 C++ linker error.
- Actual Windows document commit requires a separately verified local atomic transaction.


<!-- TOOM_SSEEDIT_C2_CHRONOLOGY_20261019_R1 -->

---

## 2026-10-19 — C1 through C2-B2 chronological development and recurrence record

### C1 — existing candidate and unsafe interactive execution

Situation:
- ChargeSession.h / ChargeSession.cpp were already at the C1 candidate hashes when the first C1 patch began.
- Actual header: 0923F5A84C958E8741226B1462F1B0AD532A1A1B653B5A0B0BC9E598B5509053
- Actual cpp: 7E12724510451645FFB0F72A492B49721CF9336967E3B6BDF53EDFAC678B231B
- Build DLL and deployed DLL did not match one another before that attempt.

Failure:
- A previous-source-only SHA gate rejected the already-updated files.
- The long block contained multiple independent top-level PowerShell statements.
- A thrown preflight error did not prevent subsequent pasted statements from continuing.
- Reapplying string replacements to existing C1 code caused repeated anchor failures.

Evidence / recovery:
- Transaction AL_C1_20261019_024152 retained original_0 through original_6.
- All seven current files matched their corresponding transaction backups after rollback.
- This proved restoration to the transaction baseline, not restoration to historical R11.
- No new C1 build/deployment was performed by the failed patch.

Correction:
- Determine whether the requested patch is already applied before generating mutation.
- Use a single closed PowerShell transaction for all commands.
- Verify all candidates in memory before writing.
- Verify every touched file after rollback.
- Never use a historical SHA gate when actual current-local authority disproves it.

### C1 BUILD-ONLY — no-work result

- Exact C1 candidate source hashes were verified.
- Windows x64 CMake/Ninja command exited 0.
- Ninja reported: no work to do.
- Build DLL SHA256 remained F102881EA545DB6A5E04E04C0A2582BFD107C64B4A5E6890DDC83B3C2135FEC3.
- MO2 deploy DLL SHA256 remained 1DD28035BDD54A198D0FDC1EA17BE422DACE993F8FE92CCA1DDC2B56D7466E17.
- This was a build-system/passive-current-state check, not evidence of a new C1 compile.
- In-game runtime acceptance was deliberately deferred.

### C2-A1 through C2-A4 — CommonLib API evidence

A1:
- Confirmed InventoryEntryData::GetEnchantment and IsWorn.
- Confirmed ActorValueOwner::GetBaseActorValue.
- Confirmed TESForm::LookupByEditorID.
- Confirmed ChargeRuntime BeginHand call and separate WorldRuntime charge path.

A2:
- Confirmed TESObjectREFR::GetInventory and inventory changes access.
- Confirmed EnchantmentItem::data.baseEnchantment.
- Confirmed Effect::baseEffect.
- Confirmed individual equipment extra data fields.

A3:
- Header report and its manifest hash were verified.
- MagicItem::effects is an array of Effect pointers.
- InventoryChanges wearable enumeration and ExtraDataList APIs were identified.
- One guessed header name ActorValue.h was missing; actual declaration was in ActorValues.h.
- Missing guessed filenames must not be treated as missing functionality.

A4:
- Confirmed item-instance ExtraEnchantment, extra-data worn flags, effect list and charge start contexts.
- Decided to avoid accepting an enchantment on an unequipped copy of the same base item.
- Enchantment magnitude and multiple equipped copies must not stack the fixed discount.
- These were STATIC/API audit checkpoints; no runtime QA was performed.

### C2-B1 — source registration

- Created ArcanaLimiterRuntime.h/.cpp and registered the cpp in CMake.
- Connected optional marker resolution in Core/Runtime.cpp.
- Existing ChargeSession, ChargeRuntime, WorldRuntime, ESP and deployed DLL were not changed by B1.
- B1 source patch passed according to its manifest evidence.
- The feature was not yet wired to charge initiation or spell damage.

### C2-B2 — compilation and unresolved linker symbol

- The new ArcanaLimiterRuntime.cpp reached MSVC compilation.
- DLL linking failed with LNK2001 on RE::InventoryChanges::IItemChangeVisitor destructor.
- The resulting failure was LNK1120 (one unresolved external).
- Do not classify this as BUILD PASS merely because compilation completed.
- The chosen visitor API requires pinned link compatibility checking.
- Proposed R2 correction: remove this inheritance dependency and inspect equipped entry instances via verified APIs, preserving anti-false-positive behavior.

### C2-B2 — build-process wait and recovery

- Start-Process -Wait remained blocked after cmake/ninja/cl/link had stopped.
- mspdbsrv processes remained active; another project showed a similar wait symptom.
- The surviving descendant process explanation is strongly supported but must not be generalized as the proven cause of every future stall.
- Waiting was resolved and the TOOM script resumed.
- C2-B2 reported FAIL and ROLLBACK: OUTPUT_HASH_VERIFIED.
- Build artifacts DLL/PDB/LIB/EXP were restored to their pre-build hashes.
- The existing MO2 deployment DLL remained untouched.
- Detailed failure evidence was saved in the S08_TEMPERANCE audit ZIP.
- Future build wrappers should wait for the specific main CMake process rather than the entire process tree; verify exit code, link result and artifact hashes.

### C2-B2 R2 — script delivery regression

- ChatGPT unexpectedly supplied a sandbox .ps1 for download and manual placement.
- The user had not saved that file at the assumed G: local path.
- Execution stopped with SCRIPT NOT FOUND before any R2 mutation.
- This repeated S01-L03 sandbox-versus-local-path confusion.
- The user had previously succeeded with inline atomic PowerShell blocks, not user-managed downloaded executable scripts.
- Correction: do not change the established execution delivery mechanism without request. Produce a complete, ready-to-paste inline transaction.

### SSEEdit authoring correction

- The first 2026-10-19 workflow rule incorrectly made PowerShell-generated SSEEdit script files mandatory.
- This was stricter than the user's actual workflow and needlessly excluded SSEEdit's native New Script operation.
- If SSEEdit is already open, ChatGPT may supply the complete Pascal source plus exact filename for New Script creation.
- If SSEEdit is not yet open, inline PowerShell may write the script to the verified installed Scripts directory.
- This corrects the earlier mandatory-PowerShell statement without deleting its historical lesson.
- Neither method authorizes user-managed downloads of external .ps1 executable scripts.

### Current checkpoint after this documentation correction

- C1 candidate present; passive build command PASS; fresh C1 compile not independently confirmed.
- C2-A1 through A4 static API review PASS.
- C2-B1 module source registration PASS according to manifest.
- C2-B2 link FAIL; old build outputs rollback SHA-verified.
- C2-B2 R2 patch NOT EXECUTED.
- C3 charge snapshot/damage integration pending.
- C4 integrated build/deployment pending.
- Final one-boot Skyrim SE 1.5.97 runtime QA deferred.
- No failure or recovery entry may be silently deleted when a later attempt succeeds.


<!-- TOOM_C3_SESSION_CUTOFF_AND_DOCSYNC_20261019 -->

## 2026-10-19 — C3-C R2 through C3-D1 session cutoff / documentation recovery

### Confirmed inputs and limits
- Current uploaded `src` archive, development DLL, CMakeLists, CMakePresets, ESP, and 000–006 DEV_ASSETS were inspected as **uploaded snapshots**, not as direct access to the user's [LOCAL_PATH_REDACTED]
- `src/Plugin.cpp` = SHA256 `8A6B3087034C53BCEF87936D1D25713515CE2BEF3F24A764FCFD32644ABAA8D0`; `src/Core/Runtime.cpp` = `C10E90A215EA7D6DF3CD9BD646889EBB117B677CBF6E10242A52A5FD4CBDCCBE`.
- CMakeLists SHA256 = `D3673A1D90AA187AABCB07B1DAB47FD530105FB8F29A4774D52F51746D292C67`; uploaded ESP SHA256 = `04A2C530400FDE7193BE09D17EE2D684747CEE4B7060229F4B384EC61572DA1E`.
- Uploaded development DLL: 756736 bytes; SHA256 `0CF57FA89585D72066A5A8E0647728119A15537CF2D583BC753F321C7E30711E`. Matches the prior C3-C R2 BUILD PASS report. The user explicitly confirms this DLL has NOT been moved to the actual MO2 mod folder.
- Historical MO2 deployed DLL was reported as `1DD28035BDD54A198D0FDC1EA17BE422DACE993F8FE92CCA1DDC2B56D7466E17`; its current local bytes were not included in the upload and thus were NOT freshly verified.
- Earlier C3-A / C3-B1 / C3-B2 / C3-C R2 BUILD PASS states are previous execution reports corroborated by current source/build snapshot where possible; no new Windows build or deployment was performed for this documentation update.

### Cutoff and recovery
- The last prior response planned C3-D1 opt-in `QADiagnostics.h` plus `Plugin.cpp` diagnostics: RunID/Epoch/CastID, event/shot/result tokens, SKSE lifecycle, `OrdinaryOffensiveMagic.qa.enable` gate, event-only logging, 8192-event budget, and no free-form player text.
- Its atomic PowerShell block was cut off mid-statement by the chat length limit. A partial block is **INVALID / NOT EXECUTED**. No C3-D1 BUILD/DEPLOY/RUNTIME PASS follows from prior in-memory candidate validation.
- The uploaded source has no `src/Core/QADiagnostics.h`; Plugin.cpp matches the C3-D1 PRE hash. Current restored checkpoint is **C3-D1 / Phase A complete for uploaded snapshots; Windows current-local recheck required before Phase B patch**.
- Existing Magician/World `[OOM][QA] ARCANA_DAMAGE` log calls are separate from proposed QADiagnostics opt-in; do not claim the opt-in controls existing log calls without explicit integration.
- Current CMakePresets `deploy` build preset points to `release-linux`; Windows workflow must select an explicitly audited x64/MSVC build/deploy route, not assume that preset deploys the development Windows DLL.

### Documentation sync and recurrence prevention
- The previously attempted documentation synchronizers suffered speculative BOM/newline or reconstructed prose-anchor false-stops. Verify exact current-local file hashes, produce canonical additions from actual sections, validate complete candidate bytes before any write, preserve encoding and other sections, then commit atomically with SHA-verified backup/rollback.
- Distinguish **prepared archive** from **applied local update**; only actual Windows post-write proof grants DOCUMENT SYNC PASS.
- A prior cleanup created redundant evidence directories alongside ZIP copies. On the successful 2026-10-19 cleanup, a SOURCE archive was verified file-for-file (40/40 matching relative path, length, SHA256) before duplicate folder removal. Windows ZIP `\` separators must be normalized and safe parent-path checks must precede deletion; include intermediate SOURCE subfolders in relative path comparison.
- Preserve this cutoff record and older C1/C2 failure history rather than replacing those prior judgments. Future implementation requires renewed Rules-First reading and current-local PRE hashes, not a blind replay of the cut-off D1 script.

### Current disposition
- C3-C R2: BUILD PASS from previous evidence; development DLL snapshot hash confirmed.
- C3-D1: STATIC CANDIDATE ONLY / PRODUCTION UNAPPLIED / BUILD NOT RUN.
- Current development DLL: MO2 NOT DEPLOYED (user confirmed).
- Consolidated One-Boot QA: NOT RUN; RUNTIME PASS and Stage LOCK not granted.
- DOCSYNC: prepared updated documents; Windows authoritative commit remains PENDING until post-write verification.


<!-- TOOM_DOCSYNC_R1_GUARD_ARRAY_FLATTEN_FALSESTOP_20261019 -->

## 2026-10-19 — DOCSYNC-R1 PowerShell guard array flatten false-stop

### Symptom and impact
- Original C3 checkpoint documentation sync stopped with `MISSING: G` at `Assert-TDocHash` before any backup, source write, document write, build or deploy.
- The `TDocGuard` nested `@(...)` representation was flattened by PowerShell enumeration, so `foreach` received a file-path string rather than a two-field record. Indexing `$TDocItem[0]` then selected the first character `G` instead of the entire path.
- Impact: PREFLIGHT FAILURE / DOCUMENT WRITE=0 / NO ROLLBACK REQUIRED. No DOCUMENT SYNC PASS.

### Prevention
- Never represent path/hash strict-guard pairs as untyped nested PowerShell arrays if automatic enumeration can change shape.
- Use `[pscustomobject]@{ Path = ...; Expected = ... }` or a keyed hashtable with explicit property access.
- Assert guard collection item counts/types and nonempty full paths before any mutation.
- Validate the actual complete PowerShell execution shape, not just generated candidate document SHA256. A successful Python simulation does not prove Windows PowerShell behavior.
- Corrected retry must retain all original local PRE hashes and avoid any C++/DLL/ESP/MO2 mutation.

<!-- TOOM_QA007_REGISTRY_20261019 -->
## 2026-10-19 — C3-D1 QA scope and patch lessons

- User requires a manual O/X/- QA checklist and measured event-driven logs in the existing OrdinaryOffensiveMagic.log. No QADiagnostics.h, qa.enable, collector, Start/Finish tools, evidence ZIP automation, or automatic QA judgement.
- Initial C3-D1 patch false-stopped before writing: PATCH ANCHOR COUNT NOT ONE. Cause: old/new identical no-op replacement matched three occurrences. Similar no-op debug edits were invalid. Corrected exact-one real edits and prewrite candidate verification.
- C3-D1 five-file log patch SOURCE PASS and MSVC BUILD PASS: 760320 bytes.
- Judgement three log-only edits SOURCE PASS. Integrated development DLL MSVC BUILD PASS: 761856 bytes, SHA256 85C8E597284A186CE24A2FECB653F7043B3805D7706AFA9459EE9244ECFB2443.
- MO2 deployment NOT RUN; Skyrim runtime QA NOT RUN; Stage LOCK NOT GRANTED.
- Maintain 007_ONE_BOOT_QA_CHECKLIST.md in DEV_ASSETS with 158 A-J cases. Console commands are unscored preparation references, not additional QA test cases.
- No separate P-I restoration checklist. Use a dedicated QA save and restore original play from the original save.
- The 004 gameplay authority controls numeric expectations; do not confuse planned damage, applied-hit counts, and observed HP delta.

<!-- TOOM_FIRST_RUNTIME_BLOCKER_CP_20261019 -->
## C3-RUNTIME-001 — First One-Boot runtime blocker (2026-10-19)

Symptoms:
- The user learned four spells; only The World appeared in the magic list.
- The Magician, Judgement and Temperance were absent.
- The World could be equipped but pressing Z did not activate it.
- The Arcana Limiter ring could be equipped; discount behavior was not tested.
- Full 158-case QA was stopped because basic spell use was blocked.

Existing evidence:
- Build/deployed DLL SHA256: 85C8E597284A186CE24A2FECB653F7043B3805D7706AFA9459EE9244ECFB2443.
- ESP SHA256: 04A2C530400FDE7193BE09D17EE2D684747CEE4B7060229F4B384EC61572DA1E.
- SSEEdit Check for Errors: 0, but no gameplay implication.
- OrdinaryOffensiveMagic.log: DLL bootstrap, DataLoaded, Forms=4/4,
  VFX Ready Count=17, hooks installed, PostLoadGame GameReady=true.
- Input DOWN Hand=R and UP Hand=R were logged.
- No successful spell release, World charge start or damage was established.

Root cause:
- NOT CONFIRMED.
- Shared NativeCastCarrier MGEF, magic-school/display settings,
  SPEL equipment configuration and World Z-input selection are candidates.
- Sharing one MGEF is not independently proven to be the defect.
- Do not approve a four-way MGEF split without current ESP/Native evidence.

Resolution:
- NOT FIXED.
- Inspect spell visibility and World input as separate failure paths.
- Apply only evidence-backed minimum changes.
- Repeat basic four-spell smoke QA before returning to 158 full cases.

Prevention:
- Forms=4/4, hook installation and xEdit Errors=0 do not establish
  in-game spell visibility or activation.
- Keep BUILD/DEPLOY PASS separate from Runtime FAIL and Stage LOCK.

## C3-DOC-RUNTIME-001 — Failed checkpoint document commit

Symptoms:
- Previous documentation preflight passed all hashes and QA 158/158.
- The first commit failed in IO.File.Replace(source,destination,null)
  with a malformed-path exception on the user's Windows environment.
- The wrapper reported ORIGINAL DOCUMENTS RESTORED; independent
  current-local SHA verification is still required before retry.

Error ownership:
- Assistant-generated transaction defect, not a gameplay or user error.
- The exact internal origin of the malformed path was not isolated.

Correction:
- Reuse the already successful byte-write plus verified original backup flow.
- Do not call the failing File.Replace overload in the retry.
- Mark a target for rollback BEFORE attempting a write.
- Read current-local complete document bytes and validate known hashes.
- Verify each final file against its in-memory expected SHA256.
- Do not claim a commit succeeded until the actual Windows result proves it.

Status:
- Previous commit = FAIL.
- This entry is evidence of the failed attempt, not an automatic PASS.


<!-- TOOM_PREQA_RECOVERY_20261019_R1 -->

## 2026-10-19 — Second runtime smoke and integrated recovery checkpoint

### Second runtime smoke

- ESP-R1 corrected shared Carrier MGEF school to Destruction and normal-spell ETYP to EitherHand.
- ESP-R1 SHA256: 771F6C995FC3FF3555DCDBA9EA11BD4B7793738A13A24173DDA1D5981FA48338.
- SSEEdit Check for Errors: zero errors.
- All four learned spells appeared. Magician/Judgement under Destruction; World under Powers.
- Magician/Judgement Native charge and some actual damage events were logged.
- User observed VFX not visible, sound too quiet, and World not activating.
- Temperance TESSpellCastEvent toggle ON/OFF was logged, but qualifying melee proc was not established.
- Arcana Limiter equipment state was recognized; measured Destruction=0, so tiered discount remained unverified.
- Full 158-case QA was not completed. Runtime PASS/Stage LOCK were not granted.

### VFX-V1

- VFXInstance.cpp dynamically attached NIF clones without explicit post-attach world/bound update.
- Added scene update after Spawn attachment and after local-transform mutation.
- Added bounded SCENE_READY diagnostics.
- Source SHA256: BA68F2B7D750FF526F7D7AED2C3F338F309C1B83F53623B405B2C573D556CB34.
- BUILD/DEPLOY PASS. Actual visible output remains unverified.
- Further static audit found no confirmed immediate self-release via BeamRenderer::Detach.
- Nonorthogonal beam transform, asset transverse dimensions, VFX lifetime scheduling and true renderer visibility remain conditional investigation targets.
- Do not mistake NIF load, spawned=true, or SCENE_READY for an on-screen visibility PASS.

### World-W1

- Added bounded input evidence to shared InputSink and World ShoutHandler hook.
- World source SHA256: E4BCAC49F13702C4CCCCD2000E9ADA18E043AA7C2B5078FC3ABE74F9523BB089.
- Charge source SHA256: 99B50779442041D90A9EE8BDD444098B1806D5662DEFDFCAB2D2C81BD822C06D.
- BUILD/DEPLOY PASS. World gameplay logic was not corrected yet.
- Distinguish INPUT_SINK, HANDLER, Selected, and Consumed in the consolidated QA log.

### Temperance-P1

- Changed only Temperance SPEL Type to LesserPower and ETYP to Voice.
- Current ESP SHA256: 4255C58A299F23DF263EAAD6199042F322B0E9F57D8BDE95AD5258BE5221634A.
- Original FormIDs and native toggle code remained unchanged.
- SpellCastEvent emission after this type change must be verified; melee proc remains unverified.

### Audio-A1 failure and R2 recovery

- Initial Audio-A1 PRE-FLIGHT failed with ANCHOR NOT UNIQUE: k.
- Cause: nested PowerShell replacement arrays flattened so a single character was treated as the anchor.
- This repeats DOCSYNC-R1 array-flattening lessons.
- Initial A1 source mutation/build/deploy: NOT EXECUTED.
- R2 removed nested positional tuples and performed four explicit verified replacements.
- AudioPolicy.h SHA256: 2C71942B814A5AB610E8ACCCF646922AFFF990D55A9D5B484D2184D9A322A330.
- SpellAudio.cpp SHA256: E0A76DFFAD0F30D43A0AA0B586BF07D27BBE0C8FA086FEA23566FBB4EAFA8A98.
- R2 BUILD/DEPLOY PASS. Actual in-game audibility remains unverified.
- Prevention: typed named records or explicit replacement calls; verify execution shape, not only candidate hashes.

### Collision/Limiter-D1 false-stop and confirmed local state

- D1 PRE-FLIGHT stopped because BeamCollision.cpp already had the intended POST SHA256.
- The script accepted only PRE and incorrectly treated an existing POST state as failure.
- BeamCollision.cpp current SHA256: C281B8DECB1B84A122130C0CEE33606C0E8BCAADADF4B65318FF9DD96F1BC68B.
- ArcanaLimiterRuntime.cpp current SHA256: EBB9771E6CEB66FBCCC06334867FAB735342FF663015152C4384BD1AC1F87E3F.
- Current build and MO2 deploy DLL are byte-identical by SHA256:
  472E6E3BFCB60093EB741A634DE3928F978A4D1CCDB5E53CB7C375F9104E9751.
- The exact operation that first committed both D1 source changes is not established.
- D1 diagnostics are source-present; their in-game execution remains unverified.
- Prevention: distinguish PRE / POST / UNKNOWN, preserve already-applied changes, and never force a historic PRE state onto current-local files.

### Consolidated QA gate

- Do not ask for separate game boots after individual changes.
- Perform read-only integrity checks and authority sync before one integrated Skyrim SE 1.5.97 boot.
- At that boot, prioritize visible VFX, World power selection/activation, Temperance toggle/melee proc, audio, early collision, limiter discount, and all applicable QA007 cases.
- Logs do not prove visible or audible presentation.
- BUILD/DEPLOY, ESP PATCH, runtime observation, and Stage LOCK remain separate verdicts.


<!-- TOOM_E2_PREQA_INVENTORY008_LESSONS_20261020 -->
## 2026-10-20 00:39:00 +09:00 - E2 recovery, pre-QA verification and 008 governance

Observed sequence:
- E1 beam-width animation: BUILD/DEPLOY PASS.
- E2-A R1: real MSVC BUILD FAIL. An invented RE::netimmerse_cast
  usage caused the initial parser errors. Source/DLL rollback reported.
- E2-A R2: PREFLIGHT FAIL, no new source mutation. An invented no-op
  anchor containing a nonexistent source fragment was required.
- These repeat the recorded guessed API, reconstructed anchor,
  no-op validator and incomplete-block failure families.
- E2-A R3: actual current-local source and pinned CommonLib reviewed.
  SOURCE/BUILD/DEPLOY PASS.
  VFXInstance.cpp SHA256:
  332A7862F505D4DBAC9F496E779856CB07DBE1CD7373BBF588E62DCBB406500C
  DLL length: 764416.
  BUILD and MO2 DEPLOY SHA256:
  9F6DA706CE3923FB8530C09A51D057231CD100A30C1FD5489DC2D8DECA189D85
- E2-A records readonly shader/material pointer ownership evidence.
  It does not change alpha or prove independence of two live clones.
- E2-B gradual alpha fade remains blocked pending ownership proof.
- Read-only PREQA verified 14/14 expected source/build/ESP/NIF/DDS
  hashes with zero mismatch and zero production mutation.
- Integrated 158-case One-Boot QA is NOT RUN after these changes.
  Runtime PASS and Stage LOCK are NOT GRANTED.

New recurrence prevention:
- Code generation is blocked until actual full current 000/001
  review and relevant lessons are applied, not merely mentioned.
- Compiling or hashing an invented candidate does not prove API validity.
- Do not add guessed or no-op patch anchors.
- Work mode must not be invoked without the user's direct request.
- 008 must be a first-class transaction participant for every tracked
  source, document, build or asset mutation.
- Maintain individual physical file paths, hashes, lengths, last-write
  times and inventory-check times.
- Update affected files and 008 together with candidate-first validation,
  SHA-verified backup, commit and rollback.
- Preserve unknown external authoring origins as UNKNOWN, not guessed.

Historical entries are retained without retroactively changing their
original PASS/FAIL verdicts.


<!-- TOOM_E2B_STAGING_R2_DEPLOY_20261020 -->
## E2-B 빔·마법진 페이드 — 2026-10-20 01:54:14 +09:00

- 첫 staging 시도는 헤더 후보의 잘못된 하드코딩 SHA256으로 PRE-FLIGHT에서 중단되었다. 헤더 실제 후보는 966631A17A5519449352C80BCD60F282D04DE511233936D3CC9A83D3ADEEED3B이며 소스 변경은 없었다.
- E2-B staging R2: 두 후보 소스 SHA256 검증 및 MSVC x64 링크 PASS. Staging DLL 768000 bytes / SHA256 D88AE3B9357663BA594C47742F15467239BCCF19392597509C5D312FD246BDA8.
- 이번 트랜잭션은 staging 산출물을 production 소스·빌드 DLL·MO2 DLL로 검증 배포하고 008 및 문서 정본을 동일 작업 안에서 갱신한다.
- fadeAmount는 각 복제 NiAVObject/geometry 인스턴스에만 기록하며 공유 shader/material alpha는 변경하지 않는다. 빔은 0.12~0.48초 감소, Retire된 마법진은 남은 holdSeconds 동안 감소한다.
- 중간 Skyrim 테스트 없음. 실제 시각적 Alpha 적용·메시 개체 효과·게임플레이는 Runtime NOT TESTED로 보류한다.
- 재발 방지: 후보 SHA는 실제 생성 바이트에서 산출해 검증하고 추측한 하드코딩 값으로 정상 후보를 거부하지 않는다. 008은 모든 변경 대상과 같은 트랜잭션에서 갱신·롤백한다.

<!-- TOOM_PS_DOCSYNC_RECOVERY_20261020_R1 -->
## 2026-10-20 - PowerShell audit delivery and false document-commit recurrence

### Situation

A read-only TOOM SDK/TULL audit PowerShell block was supplied.
During execution, PowerShell reported a standalone `else`
CommandNotFoundException at line 1, character 1.

The user then separately executed `READ_ONLY_AUDIT_COMPLETE`.

Subsequent responses presented a sandbox document ZIP as an update,
although no atomic Windows local document transaction had run.
The user identified that the development documents had not changed.

### Mistakes and prior recurrence

- Compound PowerShell input was not reliably delivered as one unit.
- A standalone completion marker could be displayed despite audit failure.
- S01-L02 execution-block fragmentation lessons were not applied.
- S01-L03 sandbox/local authority separation was repeated.
- C3-DOC-RUNTIME-001 commit/rollback evidence requirements were bypassed.
- Full original Rules-First review was overstated without sufficient proof.
- Existing explicit no-Work-mode instructions were not consistently respected.
- The earlier offline ZIP was confused with a completed local update.

### Evidence and limits

User-observed PowerShell error:
`else : CommandNotFoundException`, line 1, character 1.

This proves `else` was interpreted as a standalone command.
The exact input segmentation mechanism is not independently established.

`READ_ONLY_AUDIT_COMPLETE` does not certify the preceding audit.

No evidence establishes a production source or DLL mutation from
this read-only audit error.

The offline prepared ZIP did not update G: local development authority.
The prior document-sync attempt was NOT EXECUTED locally.

### Correction

- Use one complete inline PowerShell `& { ... }` transaction.
- Precompute every document and 008 candidate before production writes.
- Verify backups by SHA256.
- Mark attempted writes before each write operation.
- Commit 000/001/006 and 008 together.
- Verify all post-write hashes and timestamps.
- Roll back every attempted target and verify restored hashes on failure.
- Report DOCSYNC PASS only after local commit and manifest verification.
- Do not require an externally downloaded script or patch archive.

### Prevention

- Do not classify a detached completion marker as PASS.
- Never treat an uploaded or generated document as locally committed.
- Preserve prior failure records; append new corrections only.
- Do not invent or hardcode expected candidate SHA256 values.
- Use typed path records rather than nested positional arrays.
- Treat changing local hashes as current-state evidence to inspect;
  historical mismatches alone are not automatic corruption.
- Reject concurrent changes detected between preflight and commit.
- Do not invoke or suggest Work-mode handoff without explicit permission.
- Keep Codex parallel investigation separate from production changes.
- Do not request intermediate Skyrim boots for documentation corrections.

### Verdict

Previous read-only audit: INCOMPLETE / NOT PASS.
Previous sandbox-only documentation update: PREPARED / NOT COMMITTED.
Production source/build/deploy: unchanged by this correction.
Runtime VFX and audio failures: unresolved.
Stage LOCK: not granted.

This entry becomes locally committed only after the current
atomic documentation transaction verifies POST hashes.

<!-- TOOM_E3_D1_20261020 -->
## E3-B1 to E3-D0 integrated recovery and lessons — 2026-10-20

Historical records remain authoritative for their own dates.
This checkpoint does not retroactively convert previous FAIL into PASS.

### Current build and evidence

- E3-B1: combat-end cleanup retains bounded timed VFX; source/build PASS.
- E3-B2: bounded clone/render-tree diagnostics; source/build PASS.
- E3-B3: a TULL rune NIF and TOOM circle NIF are compared through
  the same native direct-attachment path. This is NOT an engine ACTI
  versus direct-attachment runtime comparison. Runtime untested.
- E3-B4B R1: false-stop before mutation because the script required
  literal GetModelData() in BSGeometry.h. The pinned SDK actually
  declares GetModelData through RUNTIME_DATA_ACCESSOR_EX.
- E3-B4B R2 and B4C: cloned-beam bounds correction compiled and
  linked successfully. It is not proof of on-screen visibility.
- E3-B5A R1: false-stop before mutation caused by incorrectly
  hardcoded candidate SHA256 and candidate length (10668).
  Actual generated source candidate: 10653 bytes and SHA256
  AE4C754AE5D013F621E79F3B2453E1910E560242E801B5988C2810B453CC49CC.
- E3-B5A R2 and B5B: audio request diagnostics compiled and linked.
  Volume, attenuation policy, concurrency cap, and SE 1.5.97
  SetPosition ABI were not changed. Audibility remains unverified.
- E3-C1/C2: World selectedPower comparison with existing
  IsCurrentShout fallback compiled and linked. Actual selection
  and activation remain unverified.
- E3-D0: READ-ONLY PASS, 0 reported integrity issues.
  Mandatory NIF count 17; optional TULL reference present.
  QA007 A-J 158 items preserved.

Isolated MSVC x64 DLL:
- Size: 772096 bytes.
- SHA256:
  C377791110C96CBFBCF84539C217810A4EF2EAA6FEB6370AE6C707DBC2582C42

Production build DLL and MO2 DLL at this checkpoint:
- Historical deployed SHA256:
  D88AE3B9357663BA594C47742F15467239BCCF19392597509C5D312FD246BDA8
- E3 integrated candidate NOT YET DEPLOYED.
- Skyrim One-Boot QA NOT RUN.
- Runtime PASS and Stage LOCK NOT GRANTED.

### Recurrence prevention

- Never gate valid pinned macro API declarations on invented literal
  expansion syntax. Compare the actual header contract.
- Never hardcode a guessed candidate hash or length as an independent
  authority over correctly generated bytes.
- Validate candidate semantics, uniqueness, inventory records and
  backups before writing; compute actual candidate hashes in memory.
- Keep SOURCE, BUILD, DEPLOY, and RUNTIME verdicts separate.
- Preserve R1 false-stop evidence and R2 correction history.
- Do not declare VFX visible or XWM audible from scene-ready,
  request logs, or a successful DLL link alone.
- Keep only one final integrated human One-Boot QA, with no extra
  collector, automated QA package or intermediate Skyrim boot.


<!-- TOOM_R1_C2B_R2_CRC_RECOVERY_20261020 -->
## 2026-10-20 — R1-C2B GZip CRC preflight failure

R1 failed while decoding an in-chat GZip/Base64 payload.
ReadToEnd raised a decompressed-content CRC mismatch before BACKUP
or COMMIT. No production source, document, DLL or ESP was changed.
Exact corruption location was not established.
Prevention: reject the damaged payload; use visible source edits,
unique current-source anchors, candidate-first verification and
one atomic source/document/008 transaction. Preserve this failure
without retroactively marking the original attempt PASS.
R2-B1 covers shared selection plus Judgement only.
Temperance integration remains a subsequent separate transaction.


<!-- TOOM_R1_C2B_R2_B2_20261020 -->
## R1-C2B R2-B2 Temperance integration

R2-B1 shared selector and Judgement commit were previously reported PASS.
R2-B2 connects the Temperance normal and power branches to that
shared nine-slot selection mechanism.

Normal history excludes the last two successful normal selections.
Power history excludes the preceding successful power triple.
History is recorded after successful visual geometry construction.
Each existing paid PendingProc retains its own circleWorld snapshot.
ResetAllState clears selection histories.

No gameplay damage, collision, Magicka charge or VFX timing values
were intentionally changed. Windows BUILD/DEPLOY and Runtime QA
remain separate, unverified stages until their own evidence exists.


<!-- TOOM_R1_C2C_B1_R2_LF_20261020 -->
## R1-C2C B1 R2 - Native candidate LF recovery

B1 R1 stopped in PRE with candidate SHA256
BDFB09C66F73BB4B7624F3F339D775AC03A10C36BC6B13DF978821D2E4A5EA16.
It performed no production mutation, build or deployment.

Comparison against the independently constructed 39869-byte candidate
identified one missing LF before the existing session.BeginHand call.
Restoring that LF reproduces SHA256
7B95E2324E4324969196F02F403F65A9FC21C4C8024C946E8067B6A8408F3B75.

Recurrence prevention: normalize inserted source fragments and
require exact candidate byte length and SHA256 before all writes.
Never bypass a failed hash guard by replacing expected values.
This source commit does not grant BUILD or RUNTIME PASS.


<!-- TOOM_R1_C2C_B2_20261020 -->
## R1-C2C B2 - World minimum charge admission

Previous B1 minimum admission source/document/008 commit passed.
World B2 is a separate source/document/008 transaction.
The original World BeginCharge had no minimum first-tier MP
admission. It called SnapshotCostMultiplier during BeginHand.

B2 calculates that multiplier once, verifies current Magicka
against the discounted World stage-one threshold and uses
the same snapshot for BeginHand.
The check precedes World visual cleanup and charge-state changes.
It performs no Magicka deduction or reservation.

Prior successful staging builds do not validate this new source.
A new MSVC compile/link and integrated runtime acceptance are required.
No intermediate Skyrim boot is requested.


<!-- TOOM_R1_C2C_D1_20261020 -->
## R1-C2B/C2C integrated build and deployment checkpoint

R1-C2B R2-B1/R2-B2 dynamic formation source commits passed.
R2-C1 isolated MSVC x64 build passed: 772608 bytes.
R1-C2C B1 minimum MP source commit passed after a missing-LF
candidate hash false-stop was corrected.
R1-C2C B2 World minimum MP source commit passed.

R1-C2C C1 isolated integrated MSVC x64 compile/link passed:
774144 bytes, SHA256
66C44352C845F82858409824BA625BBDDC8EE65D6920D02F2F6096172C34C365.

R1-C2C D1 uses this isolated DLL as build authority and promotes
it to the existing MO2 development DLL with matching SHA256.
The old build/msvc DLL remains unchanged because its linked
debug artifacts belong to the previous build.

D1 must update MO2 and documents/008 together and verify rollback.
No source, ESP, NIF, DDS or XWM modification is authorized here.
Deployment does not establish runtime behavior.
Only the existing human 158-case One-Boot QA can verify gameplay.


<!-- TOOM_RUNTIME_CTD_E1_20261020 -->
## 2026-10-20 - RUNTIME-CTD E1 MagicMenu crash

The first integrated One-Boot QA reached the MagicMenu after
obtaining and learning the TOOM spell books. Entering the menu
crashed Skyrim SE 1.5.97 before spell casting could be tested.

CrashLogger reported EXCEPTION_ACCESS_VIOLATION at
ArcanaLimiterRuntime.cpp:115 in SnapshotCostMultiplier.
The caller was the new minimum MP CheckCast path at
ChargeRuntime.cpp:336 during MagicMenuAddSpellVisitor.
RAX was zero and the faulting instruction was call [rax+0x38].
PlayerCharacter was non-null; the receiver register used
player+0x80 for a MagicTarget virtual slot 07 call.

Source review: direct player->GetActiveEffectList() uses a
potentially incorrect multi-inheritance adjustment in SE.
CommonLib-NG Actor::AsMagicTarget provides the runtime-correct
interface offset (SE +0x98, later AE +0xA0).
Fix candidate uses AsMagicTarget before GetActiveEffectList.
CheckCast no longer performs new-session MP admission during
menu enumeration. RequestCast and Native Begin retain admission.

This is a source fix candidate, not proof of CTD resolution.
Static, Build, Deploy and Runtime verdicts must remain separate.
The existing 158-case QA was interrupted; no replacement
automatic QA collection or additional intermediate boot.


<!-- TOOM_RUNTIME_CTD_E1_B2_20261020 -->
## 2026-10-20 - RUNTIME CTD E1-B1/E1-B2 checkpoint

E1-A source recovery and its four authority documents plus
008 were committed with SHA256 verification.

The first integrated QA previously failed at MagicMenu
while learned spells were enumerated.
CrashLogger showed an access violation in
ArcanaLimiter::Runtime::SnapshotCostMultiplier.

E1-A updated the active-effect query to use AsMagicTarget
and removed the new-session MP gate from CheckCast.
RequestCast and Native Begin still enforce admission.
No gameplay runtime success is inferred from compilation.

E1-B1 isolated Windows MSVC x64 configure, compilation,
link and PE AMD64 validation passed.

E1-B1 staging DLL:
Size 774144 bytes.
SHA256 E7C3FD2C3384539A85899EDA31EDEA867FFAB8DB76F359B8AED79054A193C519.

E1-B2 promotes only the verified staging DLL to the
existing MO2 development plugin path, with documents and
008 in the same rollback-protected transaction.
Historical build/msvc DLL and its debug artifacts remain
unchanged.

This correction is BUILD verified; MagicMenu CTD repair is
RUNTIME NOT TESTED until the next integrated game session.
Preserve the original CTD evidence and all untested QA cases.


<!-- TOOM_F1_B1_20261020 -->
## F1-B1 - Three-way direct-attachment VFX comparison

Based on 2026-10-20 runtime evidence and Codex READ-ONLY
NIF/DDS inspection.

Original TOOM CircleStatic is not visible in game.
Original POT SiegeRune is visible through POT ACTI.
POT SiegeRune visibility through TOOM direct attachment
has NOT been established.

F1-B1 changes only the existing E3 paired direct comparison:
TOOM CircleStatic, POT SiegeRune and TOOM OuterBand.
All three are one-shot, use the same VFX manager, have bounded
12-second lifetime, and are separated along local X.

The OuterBand has no reported NodeGlow out-of-sphere defect.
The triple comparison does NOT prove ACTI behavior, Own Emit
shader behavior, or identical projected orientation.

Do not attribute visual success to SCENE_READY or source/build
status. No runtime success is granted in this checkpoint.
POT originals, ESP, NIF, DDS, audio and current DLL stay
unchanged until later integration and deployment.


<!-- TOOM_F1_B2_20261020 -->
## F1-B2 - Own Emit single-variable diagnostic

F1-B1 direct trio source is committed but not yet built.

F1-B2 creates a new optional diagnostic NIF copied from
the original CircleStatic without modifying that original.

Five BSEffectShaderProperty flags1 fields are changed:
0x80000000 -> 0x80400000 (Own Emit bit only).
Original bytes: 12667.
Candidate SHA256:
70AC9A59416E37AB0FAD332637BAC227CE8008549C99B5B8291953D1EA27D21E.

No vertex data, DDS, root, bounds, shader flags2 or
gameplay logic has been changed.

The Native diagnostic set now consists of TOOM original,
POT SiegeRune, TOOM OuterBand and OwnEmit candidate.

POT files are read-only. ACTI comparison remains pending.
SOURCE is committed; BUILD, DEPLOY and RUNTIME untested.


<!-- TOOM_F1_B3_20261020 -->
## F1-B3 - ACTI versus direct VFX comparison

F1-B1 and F1-B2 committed the four direct-attachment
diagnostic controls without building or deploying a new DLL.

F1-B3 adds one engine-managed ACTI comparison using the
existing OOM_VFX_CircleStatic_ACTI record. No ESP change.
Placement uses PlayerCharacter PlaceObjectAtMe,
SetTemporary, SetPosition and Update3DPosition(true).

The ACTI comparison is one-shot, placed near the player
at world X+160 and Z+128, with 12-second bounded lifetime.
Removal uses SetCollision(false), Disable and SetDelete(true).
The existing manager Update handles timeout; ReleaseAll
handles PreLoadGame, PostLoadGame and NewGame cleanup.
No new polling loop or frame hook is introduced.

The ACTI is a different render/placement route and its
world-axis offset is not camera-relative. Actual visibility
must be evaluated in-game in an open third-person scene.

POT originals, original TOOM NIF and existing ESP unchanged.
SOURCE PASS does not imply BUILD, DEPLOY or RUNTIME PASS.


<!-- TOOM_F1_C2_NIPOINTER_20261021 -->
## F1-C2 - ACTI reference compile repair

F1-C configure passed, but the first isolated MSVC build
failed in TransientVFXManager.cpp.

C3535/C2440: PlaceObjectAtMe returns
RE::NiPointer<RE::TESObjectREFR>, not a raw pointer.

C7595: fmt compile-time format validation also failed
while the ACTI placement expression had an invalid type.

Fix:
- Store the diagnostic ACTI reference as RE::NiPointer.
- Receive PlaceObjectAtMe result with auto.
- Retain a local NiPointer during terminal cleanup.
- Reset the stored NiPointer before Disable/SetDelete.

No gameplay changes. No ESP or NIF changes.
A successful source transaction alone does not establish
build, deploy, runtime or visibility PASS.


<!-- TOOM_F1_D_20261021 -->
## F1-D - Five-way VFX diagnostic deployment

F1-B1/B2/B3 and F1-C2 isolated MSVC x64 build
completed. F1-C initial compilation errors were
repaired using the NiPointer return/ownership contract.

The diagnostic set includes:
- Original TOOM CircleStatic direct
- Original POT SiegeRune NIF through TOOM direct
- TOOM OuterBand direct
- Own Emit-only CircleStatic direct
- Existing OOM_VFX_CircleStatic_ACTI engine reference

Built DLL:
[PROJECT_ROOT]\build\_f1c_20261021_004136\OrdinaryOffensiveMagic.dll
SHA256:
EEA8D0F7E82C091A8BD43C6C41D51FFEF9C259E6EF36DDBFE0BE33BF89F0083A

Previous deployed E1 DLL SHA256:
E7C3FD2C3384539A85899EDA31EDEA867FFAB8DB76F359B8AED79054A193C519

This is a DIAGNOSTIC deployment, not a permanent
VFX fix. Original NIF, POT project, ESP, gameplay
rules and charge/damage calculations are unchanged.

BUILD PASS is separate from DEPLOY PASS.
Actual visibility, stability and cleanup require
the next integrated One-Boot QA.
Stage LOCK is not granted.


<!-- TOOM_F1E_F2_CHECKPOINT_20261021 -->
## F1-E runtime and cumulative recovery lessons

Confirmed: POT SiegeRune visibly rendered through TOOM
native direct attachment (user screenshot + statement).
POT Magicka Barrier and other POT effects also work.
TOOM original CircleStatic, OuterBand and ACTI
CircleStatic remained invisible.

F1 OwnEmitF1 returned BSModelDB Demand Error=1.
Do not mark Own Emit ineffective: its trial never reached
the rendering stage. First verify path/model resolution.

F1_ACTI_CREATED Ref=FF002FD8 at 02:09:03.074.
F1_ACTI_RETIRED Timeout at 02:09:23.565.
The 12-second requested deadline was exceeded by 8.491s.
A deferred event-driven update is not an exact timer.

Audio PLAY_REQUEST is not actual audible playback.
SCENE_READY, Attached, Culled=false and renderer-tree
diagnostics do not certify visible pixels.

Implementation failures and recurrence history:
- Never pin an unverified candidate SHA256; calculate it
  after constructing bytes and validate independent facts.
- PowerShell UInt32 high-bit hex literals must avoid
  signed-int reinterpretation (e.g. 0x80000000).
- PlaceObjectAtMe returns NiPointer<TESObjectREFR> in the
  actual CommonLib used by TOOM: no guessed auto*.
- File.Replace three-argument call failed on F1-D;
  inspect real path/volume issues, do not blame the null
  backup argument without evidence.
- F1-D-R2 verified the first rollback, then committed
  byte writes with full-transaction backup/rollback.
  This is recoverable transactional deployment, NOT
  an OS-level atomic multi-file replacement.
- Do not assume POT has a 008 inventory; it does not.
- Diagnostics must distinguish VFX NIF load, scene attach,
  ACTI reference creation, actual human visibility,
  retirement request and observed disappearance.
- New documentation checkpoints must update all changed
  DOC rows in 008 within the same transaction.
- Do not create a second identical audit ZIP/folder pair.

Next F2: separate NIF/Demand repair from audio diagnosis;
no repeated intermediary game boots.


<!-- TOOM_999_RECOVERY_STANDARD_R2_20261021 -->
## 999 session recovery - correct delivery and portable lessons

The earlier external-download TOOM_999_Install_Refresh.ps1
proposal was rejected: it violates the established inline
PowerShell delivery rule. This repeats C2-B2 R2 / S01-L03
sandbox-path and user-managed-script delivery mistakes.

Accepted recovery workflow:
- One complete inline & { ... } PowerShell transaction.
- No downloaded executable .ps1, manual relocation or
  secondary installer.
- Create the SESSION_RECOVERY_999 folder locally.
- Produce stable 999 entry document, paste-prompt TXT,
  portable development lessons and verified CORE_DOCS.
- Keep DEV_ASSETS as source authority.
- Build snapshot copies only from verified current-local
  canonical documents.
- Synchronize tracked recovery files with 008 in the
  SAME commit as 001/002/006 changes.
- Distinguish source documents from derived inventory
  mirror, manifest and transfer ZIP; document the
  self-reference exclusion explicitly.
- Verify zip entries against the exact candidate bytes.
- Back up and restore every touched file on failure.

The 999 trigger alone does NOT allow access to [LOCAL_PATH_REDACTED]
A new chat must receive actual recovery attachments.
The model must read the real 000/001 content and not
claim current-local verification from a chat summary.

At future accepted checkpoints:
1. Complete current canonical document/008 transaction.
2. Rebuild 999 transfer snapshots from that checkpoint
   through a NEW inline verified transaction.
3. Replace only the single CURRENT transfer ZIP.
4. Verify snapshot/manifest consistency and preserve
   one useful update audit; no redundant evidence ZIPs.
5. Never treat a stale recovery copy as current authority.
6. No background watcher or automatic QA collector.

Portable engineering lessons must be reusable only
after adapting project-specific roots, governance,
toolchain, authority and evidence contracts.


<!-- TOOM_999_SIMPLIFIED_R3_20261021 -->
## 999 복구 체계 R3 — 이중 문서 관리 구조 정정

R2에서는 CORE_DOCS 복사본, 전송 ZIP,
매니페스트와 별도 갱신 절차까지 생성했다.

원래 요구는 개발 정본을 복제하는 시스템이 아니라
999 한 문서로 기존 DEV_ASSETS를 인지하고
새 개발 세션의 워크플로를 재구축하는 것이었다.

R2는 실행 검증 PASS인 역사적 체크포인트이지만
운영 설계는 R3의 2파일 구조로 대체한다.

R3 유지 대상:
999_TOOM_SESSION_BOOTSTRAP.md
999_NEW_SESSION_PROMPT.txt

개발 정본은 DEV_ASSETS 한 곳에서 관리한다.
999는 문서 경로, 권위, 읽기 순서, 체크포인트
선택 방법 및 세션 복구 절차만 제공한다.

사용자 PC에 접근할 수 없는 새 ChatGPT 채팅은
999의 절대 경로만으로 로컬 정본을 열 수 없다.
필요한 원본을 요청하고 확인한 뒤 진행한다.

미래의 일반 체크포인트마다 CORE_DOCS 복사본,
전송 ZIP, 매니페스트를 갱신하는 정책은 폐기한다.
999의 탐색·부트 절차 자체가 바뀔 때만 갱신한다.

기존 R2 성공 증거는 삭제하지 않는다.
R3 파일 정리는 원본 백업, SHA256, 롤백,
008 동일 트랜잭션 동기화로 검증한다.

타 프로젝트에 재사용할 노하우는 별도 복제
문서가 아닌 해당 프로젝트의 DEV_LESSONS와
세션 워크플로 원칙에서 선별한다.

<!-- TOOM_F2_H2_B2_R3_VFX_P0_20261021 -->
## F2-H2-B2-R3 - ACTI candidate LF recovery and isolated build
- R1 pre-build SHA_DRIFT: Windows Git changed three candidate files LF to CRLF; no production writes.
- R2 restored pinned LF bytes for 3/3 files; candidate hash verification PASS.
- MSVC x64 isolated BUILD PASS; DLL 788992 bytes; SHA256 40B22DEA5E4376660D9DFAF8C8219EECDECB355546FEB3026FE5E449490152AB.
- R3 uses compiled candidate bytes directly instead of reapplying Git patch.
- CircleStatic ACTI only. Other circles and beams remain pending.
- Isolated BUILD does not establish production BUILD, DEPLOY, RUNTIME or LOCK.

<!-- TOOM_F2_H2_B2_R5_DEPLOY_20261021 -->
## F2-H2-B2-R5 - VFX ACTI deployment
- R3 SOURCE/DOCSYNC and R4 production-source MSVC x64 BUILD PASS.
- R4 DLL 787456 bytes, SHA256 91F12BFDCA070C33F98818A916FEABDA943B26A96EAF2C56F4B005E2B4144F44.
- R2 isolated DLL size/hash differed from R4; no byte identity is claimed.
- R5 deploys only the verified R4 DLL to the TOOM MO2 development mod.
- Previous MO2 DLL is preserved in transaction backup.
- CircleStatic ACTI integration only; OuterBand and beam VFX remain pending.
- Production deployment is not runtime visibility proof. LOCK remains NO.

<!-- TOOM_F2_H2_P02A_R2_ESP_RECONCILE_20261022 -->
## F2-H2-P0-2A-R2 - OuterBand ACTI ESP reconciliation
- SSEEdit created OOM_VFX_CircleOuterBandStatic_ACTI, FormID 0100081A.
- Check for Errors: 22 processed records, 0 errors.
- Saved ESP: 5785 bytes; SHA256 C4987973304FAF039FBA30260D6F4C5932C292E111DAFF5B19201A335C8DCE3B.
- Binary comparison: one ACTI added; TES4 record count and next FormID updated; other 20 existing records byte-identical.
- SSEEdit saved ESP separately before this transaction. This checkpoint reconciles documents and 008 afterward; it does not claim atomic ESP creation.
- Native OuterBand mapping and actual runtime visibility remain unverified.

<!-- TOOM_F2_H2_P02B_R1_OUTERBAND_ACTI_20261022 -->
## F2-H2-P0-2B-R1 - OuterBand ACTI Native mapping
- Current P0-2A-R2 ESP and 008 verification PASS.
- Native ACTI routing extended to CircleOuterBandStatic.
- ACTI EditorID selected by asset, not hardcoded numeric FormID.
- Existing handle, transform, growth, fade and retirement paths retained.
- Prior CircleStatic-only header comment updated.
- Candidate source bytes and 008 updated in one rollback-protected transaction.
- Beam and explosion rendering remain separate unresolved P0 tasks.
- Production build, deploy and runtime are not certified by this step.

<!-- TOOM_F2_H2_P02B_R2_BUILD_DEPLOY_20261022 -->
## F2-H2-P0-2B-R2 - OuterBand ACTI production build/deployment
- R1 Native mapping SOURCE/DOCSYNC PASS.
- R2 builds the current production source with MSVC x64.
- Verified built DLL: 787456 bytes; SHA256 B972B6250328EE64E44B7A10EAC88CF9B3119F423FEE9D7F22FD82A377882C47.
- DLL and 001/002/006/008 are committed and rollback-verified together.
- Previous DLL retained in transaction backup.
- OuterBand visual animation, beam range and explosion VFX remain runtime unverified.
- No intermediate Skyrim boot; final One-Boot QA remains pending.

<!-- TOOM_F2_H2_P03B_B1_ROOT_BOUND_20261022 -->
## F2-H2-P0-3B-B1 - Whole-lens root bounds
- A-R1 READ AUDIT false-stop: function wrote to null script:report; no production mutation.
- A-R2 READ AUDIT PASS: source, SDK, NIF and DDS verified.
- Previous E3 mesh modelBound inflation and NiUpdateData Update were not proof of GPU output.
- B1 adds bounded whole-lens NiNode worldBound center/radius correction after Update.
- Root bound uses 512u authored +Y length and a conservative 128u transverse guard.
- Both Spawn and SetLocalTransform use the same root-bound correction.
- Beam geometry, collision, gameplay range, NIF, DDS and audio are unchanged.
- Production BUILD, DEPLOY and actual on-screen visibility remain pending.

<!-- TOOM_F2_H2_P03B_B2_BUILD_DEPLOY_20261022 -->
## F2-H2-P0-3B-B2 - Beam root world-bound build and deploy
- B1 source and inventory synchronization PASS.
- B2 compiled current production sources under MSVC x64.
- Verified DLL: 791552 bytes; SHA256 02ABC7D038EF386231CAEDC26BB6B28BD5E9AF7B871BBBF3F9B20D7C88B01DAF.
- New DLL, 001/002/006 and 008 committed in one rollback-protected transaction.
- Original deployed DLL preserved in verified transaction backup.
- Beam root world-bound correction is a rendering candidate, not runtime proof.
- Beam geometry, NIF/DDS, collision logic and gameplay range unchanged.
- Beam visibility and impact/explosion VFX remain runtime unverified.
- No intermediate game boot; integrated One-Boot QA pending.

<!-- TOOM_F2_H2_P04B_B1_MAGICIAN_IMPACT_20261022 -->
## F2-H2-P0-4B-B1 - Magician impact VFX connection
- P0-4A read-only source, NIF, ESP and DOC authority audit PASS.
- Magician had world-stop explosion gameplay but no impact VFX Spawn call.
- Judgement and World already call FlashCloud; visibility remains unverified.
- B1 connects one FlashCloud per true Magician world/static stop.
- Visual scale tracks existing gameplay explosion radius from tier profile.
- Existing gameplay damage, collision, range and beam remain unchanged.
- FlashCloud uses the current transient clone renderer and bounded lifetime.
- ACTI conversion is not assumed successful: previous CircleStatic ACTI was invisible.
- SOURCE/DOCSYNC alone is not BUILD, DEPLOY, RUNTIME or LOCK.

<!-- TOOM_P04B_B2_R4_VERIFIED_DEPLOY_20261022 -->
## P0-4B-B2-R4 - Interrupted build and deployment recovery
- Initial full Ninja build was interrupted at 490/520 by Ctrl+C.
- Production SHA256 remained unchanged after interruption.
- Incremental resume compiled 30 remaining tasks and linked the DLL.
- Ninja dry run reported no work to do.
- Recovered DLL: 793088 bytes, SHA256 F559AB2C2DF27EE8CF8B4CDF07668F07B2DE8633E3C3DC0EF5E57BDC02D5C660.
- R2 deployment false-stop: UTF-16 LE BUILD_RESUME.log was incorrectly decoded as strict UTF-8.
- R2 ended with PRODUCTION_WRITES=0; no rollback was necessary.
- The proposed R3 was withheld for further review, with no R3 success evidence.
- R4 removes build-log parsing from deployment entirely.
- Reuse verified build bytes; do not rebuild merely to recover deployment.
- DLL, documents and 008 are backed up and committed together with verified rollback.
- No source, ESP, NIF, DDS, gameplay range or collision changes.
- Runtime VFX visibility remains unverified; Stage LOCK NO.

<!-- TOOM_P04C_B1_FLASH_CONTROLLER_TIME_20261022 -->
## P0-4C-B1 - FlashCloud animation time recovery
- P0-4C-A source, document, NIF and DDS audit PASS.
- FlashCloud NIF has 15 geometry meshes and 27 shader float controllers.
- All 15 shader alpha animation first keys have time 0 and alpha 0.
- This does not independently prove controller playback failure.
- Direct NiNode clone attachment previously used only initial graph Update.
- B1 explicitly advances FlashCloud controller time within the existing transient manager update.
- This uses existing active-slot processing; it adds no new polling loop.
- Growth animations and whole-lens beam envelopes remain untouched.
- Neither shared shader/material ownership nor NIF binaries are modified.
- Native build, deploy and GPU visibility remain unverified after this source commit.
- Do not claim visual restoration until integrated manual One-Boot QA.

<!-- TOOM_P04C_B2_R2_BUILD_DEPLOY_20261022 -->
## P0-4C-B2-R2 - Incremental build and deployment recovery
- P0-4C-B1 FlashCloud controller-time SOURCE/DOCSYNC PASS.
- First P0-4C-B2 attempt stopped before build or production mutation.
- Cause: PowerShell automatic variable HOME is read-only and case-insensitive.
- The previous script assigned a Regex Match to $home.
- R2 uses configuredSourceMatch and configuredSource instead.
- Reserved automatic variable names must be excluded in future PowerShell scripts.
- Previous R2 deployment false-stop: UTF-16 build log decoded as UTF-8.
- Native build logs are not parsed as deployment authority.
- Existing Ninja build tree reused without a clean rebuild.
- New DLL SHA256=CAF3A0CA1E5307136E6E628E80278F5CFE61A85EF7C9C210F4B4FFFE34B2D1B9; Bytes=793600.
- DLL, 001, 002, 006 and 008 are backup/rollback protected.
- No gameplay range, collision, damage, ESP, NIF or DDS changes.
- FlashCloud visual output remains unverified.

<!-- TOOM_PREQA_CP001_20261022 -->
## PRE-QA CP001 - F2-H2 누적 시행착오 및 재발 방지
- P0-4B-B2 초기 격리 빌드가 490/520에서 Ctrl+C로 중단됐다.
- 원인은 사용자 중단이며 컴파일 오류로 판정하지 않는다.
- Ninja 기존 객체를 재사용해 나머지 30/30과 링크를 성공시켰다.
- 교훈: 컴파일 중단 시 프로세스·로그·Production 해시부터 확인한다.
- P0-4B-B2-R2는 UTF-16 LE BUILD_RESUME.log를 strict UTF-8로 읽어 실패했다.
- 해당 실패는 PRODUCTION_WRITES=0이었고 DLL 손상은 없었다.
- 교훈: 빌드 로그의 인코딩을 가정하지 않으며 배포 권위는 DLL 실제 바이트와 SHA256이다.
- P0-4B-B2-R4는 불필요한 로그 파싱을 제거해 배포 PASS했다.
- P0-4C-B2 최초 시도는 $home과 읽기 전용 $HOME의 충돌로 빌드 전에 실패했다.
- PowerShell 변수명은 대소문자를 구별하지 않는다.
- 교훈: HOME, PID 등 자동 변수 충돌을 실행블록 작성 전 점검한다.
- P0-4C-B2-R2는 변수명을 수정하고 증분 빌드·배포·DOCSYNC PASS했다.
- 반복 false-stop은 검증 강화가 아니라 불필요한 고정 전제에서 비롯됐다.
- 검증 대상은 물리 파일·현재 008·최종 후보 바이트·백업·사후 해시여야 한다.
- FlashCloud NIF의 15개 셰이더 알파 곡선은 시작값 0이다.
- 알파 시간 미진행은 정적 분석에서 도출한 원인 후보이지 런타임 확정 원인이 아니다.
- 공통 매니저의 FlashCloud 시간 갱신 패치를 컴파일하고 배포했으나 화면 가시성은 미검증이다.
- 로그 Spawn 성공, 장면 부착 성공, 게임 화면 표시 성공을 서로 혼동하지 않는다.
- 기존 마법진 ACTI가 보이지 않았던 사례를 무시하고 ACTI 전환만으로 해결됐다고 추정하지 않는다.
- 다음 세션은 최신 000/001 전체 및 해당 시점 008을 실제 재독해야 한다.

<!-- TOOM_QA1_CP002_20261022 -->
## QA1 CP002 - First visible magic circles and remaining defects
- Previous PRE-QA CP001 passed documentation and 999 synchronization.
- The first runtime QA after FlashCloud controller-time deployment revealed actual visible magic circles.
- This is a partial rendering breakthrough, not VFX restoration PASS.
- User observed no visible intended beam effects or explosion effects.
- World formation appeared clustered too close to the character and too narrow.
- Temperance formation likewise appeared too close and narrow.
- Magician circles appeared away from the character front, sometimes behind or outside the expected screen region.
- Magician circle scale was excessive and generation position/orientation inconsistent between casts.
- Judgement intended magic-circle and beam presentation was not visible.
- Magician firing direction appeared inconsistent; user suspects this contributed to apparent range inconsistency.
- The direction-to-range causal relationship is unconfirmed and must be investigated.
- Logs report WORLD_CHARGE_RELEASE, SHOT, IMPACT_VFX and Temperance visual successes despite absent intended pixels.
- P04C_FLASH_TIME confirms animation update execution but not explosion visibility.
- F2_ACTI_EULER_NONUNIQUE occurred eight times; examine rotation conversion before changing constants.
- WORLD_STOP_NEAR included 173.089 and 142.107 world-unit stops referencing FE344804.
- Do not assume FE344804 is a false obstacle until the reference and geometry are checked.
- Separate muzzle/source transform, camera forward, actor forward, formation transform, beam render transform and collision ray.
- Compare charge-time pose with release-time pose and ensure a single explicitly chosen aim policy.
- Avoid blindly enlarging gameplay range to compensate for inconsistent aim or premature collision.
- Retain FlashCloud alpha-controller, root-bound, shader and lifetime hypotheses as unresolved if pixels remain absent.
- Resolve actual VFX visibility, placement, direction and lifetime before final LOCK.
- QA result is PARTIAL; all previously untested checklist items remain unresolved.

### QA1 submitted evidence
- Runtime log: ordinaryoffensivemagic(20261021-190355).log
- Log SHA256: 22C8034752A5F77828AB6C007A3FA69FBEA2C4D8756A720DAA977441FE6FA9DE
- Video: Skyrim Special Edition 2026-10-22 03-59-43.mp4
- Video SHA256: 4F33940E7567D736DB2C0A7651AE656687A9C591F829A0CB7FB3C73D78E655C3
- Video: Skyrim Special Edition 2026-10-22 04-00-52.mp4
- Video SHA256: D9AC682BDA9FA70A4AF5E327F09265A67E47F25F4AC298F7D46E37CA6C317043
- Video: Skyrim Special Edition 2026-10-22 04-01-49.mp4
- Video SHA256: 7A4CF7670BFB5609ECC37A75C3D1148F45094650DAB56CEF145CB3AFC5DAF32A
- Video: Skyrim Special Edition 2026-10-22 04-02-20.mp4
- Video SHA256: 96F01A97A36F4C95E2F91279C4A990E6C447B33DA31E8B633FF7D7F3824813CD
- The media/log hashes describe conversation uploads; this transaction does not claim local copies were archived.


<!-- TOOM_QA1_B1_AIM_AXIS_20261022 -->
## QA1-B1 - Camera aim forward-column correction
- QA1-A2 read gate PASS; original 000/001/source validated.
- Current version stays 0.8.3-dev; no release or DLL bump.
- Magician and Judgement previously sampled the first ROW of NiCamera world rotation.
- Both now sample COLUMN 0: entry[0][0], entry[1][0], entry[2][0].
- Camera local +X and authored beam mesh local +Y are distinct conventions.
- SmoothCam NiCamera-to-camera-node basis mapping supports the selected column.
- Existing normalized direction flows into the marking/fire collision request.
- Gameplay origin, max range, radius, damage, cost and timing remain unchanged.
- This is SOURCE-only geometry correction; no runtime success is claimed.
- Early world hit RefID FE344804 and GPU beam/explosion invisibility remain open.
- No clean build, Skyrim boot or 158-case QA performed during B1.

<!-- TOOM_QA1_B2_FORMATION_SCALE_20261022 -->
## QA1-B2 - Hemisphere size tuning after visible-runtime QA
- The World: user accepted all 12 relative positions, ordering, stage subsets and dome shape.
- The World problem was dome size, not normalized direction-bank topology.
- Increase only the World shell radius from 64u to 128u.
- Preserve R11 D bank, stage I/II/III subsets, bank-slot ownership and original circle scales.
- World outward beam length remains 64u; each explosion endpoint moves from center+D*128u to center+D*192u.
- The endpoint is shared by visual impact and logical explosion damage. No detached decorative endpoint is introduced.
- Temperance shell radius is tuned 64u to 128u; dynamic slot-selection and original eligible target center are preserved.
- Magician charge-circle effective scale is multiplied by 0.65, retaining the eight authored relative scale ratios.
- These tuning values are QA candidates, not visual acceptance or DESIGN LOCK.
- No beam/explosion GPU-visible fix is claimed. No Skyrim runtime retest during B2.



<!-- TOOM_QA1_D2C_NATIVE_BUILD_20261023 -->
## QA1-D2C - ACTI native candidate, recovery and integrated binary
- D2C-A4 recovered pinned D1B candidate CPP/H, build-only DLL and D2A staged ESP.
- D2C-A6/A7 enumerated seven ACTI records, including CircleAnimated.
- FormID and EditorID duplicate counts were zero in the read-only ACTI audit.
- D2C-A9 captured actual current and candidate source in read-only evidence.
- B1 repaired delayed Whole-Lens child readiness, root cache invalidation and FlashCloud readiness.
- B1 candidate CPP SHA256: 1C76A54BBE175FE482B103EDD46B78649C6429B1D26048131445AB809F08E36D.
- B2 separate source-tree MSVC x64 configure/build PASS, DLL 800768 bytes.
- B2 DLL SHA256: 6355DBA95681A39212E7F82C34B06AAC436AECA90579FAC2B0225E07DE747B81.
- The source integration includes earlier staged QA1-B1/B2 geometry changes.
- The historical src 10192000.zip row is retained; missing file was not fabricated.
- The source, build and deploy candidates are committed with 008 in one rollback-protected transaction.
- The existing Ninja build cache was not rebuilt by the isolated build; future current-tree builds require verification.
- ESP SSEEdit Check for Errors and in-game GPU-visible beam/explosion behavior remain unverified.
- No new Skyrim runtime test and no Stage LOCK are claimed.


<!-- TOOM_QA1_D2D_ARTO_20261023 -->
## QA1-D2D - Arcana ARTO visual backend
- D2C-C source and deployment remain the integration baseline.
- Arcana Staffs v0.6.2 was used as read-only technical reference.
- Added a default Whole-Lens ARTO visual companion with 128 slot-specific forms.
- Added an AS_BeamAxis NIF and its separate TOOM texture.
- D2D-B1 local candidate reconstruction and MSVC x64 BUILD PASS.
- B1 DLL SHA256: 37BEC0E8587CEFCAE3AF79475F2753F7EB8FCE44E0AA842280557C89C48D7945.
- Existing ACTI, combat logic, beam timing and other VFX remain independent.
- Static/build/deployment PASS is not GPU-visible runtime PASS.
- SSEEdit Check for Errors and integrated manual One-Boot QA remain pending.
- LOCK=NO.


<!-- TOOM_QA1_D2E_COMMIT_20261023 -->
## QA1-D2E - FlashCloud ARTO integration and build recovery
- Preserve the verified D2D default Whole-Lens ARTO backend.
- Extend slot-specific ARTO acquisition to normal and Tintable FlashCloud.
- Add 128 Flash and 128 FlashTint ARTO forms; existing 128 Beam forms remain.
- Reuse deployed FlashCloud NIFs and their authored animation nodes.
- Keep ACTI as fallback until ARTO visual readiness is established.
- D2E R1 expected CPP hash mismatch was a validator error.
- R2 Configure selected an x86 compiler against x64 vcpkg dependencies.
- R3 VsDevCmd failed because of doubled path quoting.
- R4 Get-Command returned multiple cl.exe paths; the validator expected one.
- R5 selected Hostx64/x64 directly; Configure and Build PASS.
- R5 DLL bytes=810496.
- R5 DLL SHA256=A48BEF60DBAE7EA8E85F8818B23A6A80684880554E6D55C839144DB8FC486606.
- No intermediate Skyrim runtime test was conducted.
- SSEEdit Check for Errors and final visual One-Boot QA remain pending.
- BUILD and DEPLOY are distinct from GPU-visible runtime acceptance.
- LOCK=NO.


<!-- QA1_D2F -->
## QA1-D2F Tintable Whole-Lens ARTO
- Reuse existing ARTO slot lifecycle without introducing polling or gameplay hooks.
- Add 128 Tintable Beam ARTO records; other ARTO forms remain.
- NIF uses AS_BeamAxis, white shader RGB and a separate copy of the TOOM neutral DDS.
- Existing Tintable ACTI remains fallback until ARTO readiness.
- Build DLL SHA256: D3F1DB68723DD1EE254F434876B0FB3A228745BCA643C2062991B6264F7AD6D4.
- Visual appearance and SSEEdit error checks remain unverified.
- One-Boot QA pending. LOCK=NO.


<!-- TOOM_QA1_CP003_D2F_STATIC_20261023 -->
## QA1-CP003 — D2C through D2F retrospective, failures and lessons

### Verified implementation checkpoints
- D2C-C repaired the existing Native ACTI backend. DLL 800768 bytes; BUILD/DEPLOY PASS.
- D2D-B2 introduced an ARTO visual companion for default Whole-Lens beams.
- D2D ARTO model uses the Arcana-inspired AS_BeamAxis and engine-owned ModelReferenceEffect.
- D2E-B2 added ARTO visual paths for normal and Tintable FlashCloud.
- D2F added the Tintable Whole-Lens ARTO path with a separate candidate NIF and DDS.
- Total deployed ARTO forms: 512, in four groups of 128. This does not mean 512 simultaneous effects.
- D2F DLL SHA256: D3F1DB68723DD1EE254F434876B0FB3A228745BCA643C2062991B6264F7AD6D4.
- D2F ESP SHA256: 934D78504EBF75DD3397C3FE1D4296C0A9609F81DC65C7BEB58C81C19E65709A.
- D2F current CPP SHA256: D3FA366D390503C1840567714D208A42B82D683664F048C067368A4727C7016E.
- D2F 10-target atomic commit PASS; rollback not required.
- Final custom structural inspection PASS: 512 ARTO, four referenced model paths.
- User-reported SSEEdit Check for Errors: 538 records processed, errors found 0.

### Error chain and actual causes
1. D2D-B1 PACKAGE_MATCH_COUNT=0: the assistant-created candidate ZIP had not been downloaded to the user's Windows machine. ChatGPT artifacts are not local Windows files.
2. D2D reconstruction succeeded after switching to a fully inline local PowerShell source/asset reconstruction.
3. D2E R1 CPP_CANDIDATE_HASH_MISMATCH: the predicted hash was wrong. The actual seven-edit result was reproducible; the source anchors were not the cause.
4. D2E R2 CONFIGURE_FAILED: CMake selected Hostx86/x86 while vcpkg packages targeted x64-windows-static-md. The x64 spdlog package was rejected.
5. D2E R3 VS_X64_ENV_INITIALIZATION_FAILED: duplicated quotes broke the path to VsDevCmd.bat.
6. D2E R4 COMPILER_NOT_X64: Get-Command returned two cl.exe applications and the script treated both paths as one scalar.
7. D2E R5: explicitly selecting VCToolsInstallDir/bin/Hostx64/x64/cl.exe resolved the environment issue. Configure and BUILD PASS.
8. D2E-B2: eight-target Production/docs/008 commit PASS.
9. D2F: candidate, x64 build, ten-target commit and static inspection PASS.

### Process failure and correction
- Most D2E retries were caused by the assistant's PowerShell, quoting, validation or compiler-selection errors, not demonstrated Skyrim VFX limitations.
- D2D, D2E and D2F were over-separated into build, backup, doc-sync and commit cycles.
- A completed checksum, build or commit must not be rerun without a changed dependency or a real failure.
- Prefer one consolidated VFX integration checkpoint rather than one governance commit per individual visual.
- Read the actual complete current documents after a failure; a previous hash check does not establish renewed reading.
- Preserve useful failed logs, but do not duplicate them as expanded folders plus ZIPs.
- Use the first actual Configure/Build error and fix the smallest failing layer.
- Avoid strict guessed hashes. Expected hashes must be derived from reproduced candidate bytes.
- Distinguish ARTO creation, model acquisition, NIF update and GPU-visible rendering.
- SSEEdit errors=0 confirms its reported record validation, not actual effect visibility or animation correctness.

### Arcana Staffs technical reuse
- Arcana Staffs is a separate independent mod, not TOOM Production authority.
- Its successful runtime VFX implementation motivated the ARTO/ModelReferenceEffect/AS_BeamAxis approach.
- TOOM keeps its own damage, targeting, formation, cast lifetime, audio and ACTI fallback contracts.
- Reuse the working engine-owned visual instance pattern selectively. Do not import unrelated combat hooks.
- Avoid creating more ARTO forms or rewriting completed backends before the current candidate is runtime tested.

### Current acceptance boundary
- Latest actual historical gameplay runtime checkpoint: QA1-CP002.
- D2C through D2F integrated visual changes have not yet received an in-game runtime test.
- Preserve the approved World's twelve-circle positions, relative form and stage order.
- Manual 158-item One-Boot QA remains pending.
- Runtime visual PASS and Stage LOCK are NOT GRANTED.

### Evidence
- QA1_D2C: 011_Commit_Result_20261023_022044.txt.
- QA1_D2D: 006_Commit_Result_20261023_042940.txt.
- QA1_D2E: 015_Commit_Result_20261023_060523.txt.
- QA1_D2F: 003_Result_20261023_063434.txt.
- QA1_D2F: 004_ARTO_Static_20261023_070102.txt.
- SSEEdit output was provided by the user in chat: 538 processed, zero errors.


<!-- TOOM_QA1_CP004_OUTPUT_ATOMIC_WORKFLOW_20261023 -->
## CP004 — Successful working method and presentation lessons

### What worked well
- The user approved the concise Korean development format.
- A clear current task and explicit PASS/FAIL/NOT_TESTED status reduced ambiguity.
- The final checkpoint table made long development conversations recoverable.
- The ✓ / → / ○ progress display made completed and remaining work easy to understand.
- A single file-request line at the end prevented files from being missed.
- PowerShell blocks executed directly on the user's Windows PC without relying on ChatGPT downloads.
- Current-local pins, candidate checks, verified backups, rollback and 008 updates reduced the risk of uncontrolled edits.
- Sequential stage evidence preserved useful diagnostics without flooding the console.
- The completed D2F and CP003 commits demonstrated the usefulness of one combined rollback-protected transaction.

### Mistakes to avoid
- Do not require the user to reinject response formatting and execution policy in every new session.
- Do not assume a good prior answer format will persist without recording it in 000, 006 and 999.
- Do not emit raw UI tags such as <icon .../> that may appear literally in the client.
- Do not label staged code, unexecuted scripts or compiler success as visual runtime success.
- Do not duplicate code, builds, documentation updates or audits solely to create extra checkpoints.
- Do not trust guessed SHA256 values or overstrict validators that reject correct candidate bytes.
- Avoid ambiguous PowerShell native-command quoting and multiple Get-Command compiler matches.
- A connected or uploaded artifact is not automatically a file on the user's Windows drive.
- Do not say the latest local Rules/Lessons were fully read when only their hashes or prior copies were inspected.
- Do not report successful commit or rollback until the executed result establishes it.

### Reuse in all future sessions
- 000 is the normative output/execution rule.
- 001 preserves why the rule was adopted and recurring failure patterns.
- 006 defines the implementation workflow and failure recovery.
- Both active 999 files must explicitly instruct a new assistant to follow this contract.
- Preserve earlier historical checkpoints, but use the newest committed checkpoint as CURRENT.
- The last real gameplay runtime remains QA1-CP002 until new manual QA evidence exists.
- CP004 is documentation/workflow-only; the D2F binary and ESP remain unchanged.


<!-- TOOM_QA1_CP005_VFX_FRAME_BOUNDS -->
## QA1-CP005 — VFX frame and ARTO bounds correction
- Scope: VFXRuntime.cpp and TransientVFXManager.cpp only.
- Active VFX lifetime uses chained PlayerCharacter Update.
- ARTO beam opacity and bounds propagate through the model hierarchy.
- Gameplay, coordinates, ESP and NIF remain unchanged.
- Temperance independent toggle halo is not implemented by this patch.
- Build/deploy evidence is recorded in QA1_CP005 stage evidence.
- Runtime visual acceptance remains NOT_TESTED; LOCK=NO.

<!-- TOOM_QA1_CP006_MAGICIAN_DEPTH -->
## QA1-CP006 — Magician spacing and R1/R2 false-stop recovery
- CP006 R1 PRECHECK FAIL: assistant supplied a truncated 008 hash.
- CP006 R2 PRECHECK FAIL: validator demanded an inventory row for 008 itself.
- Both failures occurred before Production mutation; rollback not required.
- The 008 inventory intentionally excludes its own SHA256.
- Validate 008 externally and do not perform inventory self-lookup.
- Do not delegate partial PowerShell editing to the user.
- Magician kSpacingStart changes from 96u to 28u.
- Magician kSpacingEnd changes from 60u to 12u.
- These values are presentation tuning candidates, not runtime-approved positions.
- Retain scale, tier order, rotation, collision and gameplay authority.
- World twelve-circle formation, Judgement and Temperance remain unchanged.
- Preserve CP005 VFX frame/bounds implementation.
- Runtime NOT_TESTED; LOCK=NO.


<!-- TOOM_QA1_CP007_TEMPERANCE_C2_HALO -->
## QA1-CP007 — Temperance C2 Head Halo
- Reuses the existing CircleOuterBandStatic ACTI and NIF.
- The asset is the outer ring and Hebrew-like inscription band of Magician circle C2.
- Uses an independent temporary reference, not the melee proc visual pool.
- Temperance ON: one small visual behind/above the player's head.
- Temperance OFF and ResetAllState: disable and delete the temporary reference.
- Cell/3D invalidation: retire and recreate only when the head node is valid.
- Player frame hook updates the visual even without active transient effects.
- Initial placement candidates: 0.15 scale, 14u behind head, 5u above.
- No gameplay, damage, mana, Magician spacing, World formation, ESP, NIF or DDS changes.
- Runtime visual positioning and head-node availability remain NOT_TESTED.
- LOCK=NO.


<!-- TOOM_QA1_CP008_SESSION_RECOVERY_20261023 -->
## QA1-CP008 — Cumulative implementation, failures and lessons

### Verified implementation chronology
- QA1-CP003 / D2F: 512 ARTO static checks PASS; SSEEdit 538 records, zero errors (user-reported). Visual runtime acceptance was not established.
- 2026-10-23 gameplay video and OrdinaryOffensiveMagic.log: no observed CTD; some circles, audio and impact visuals appeared, while common beams, Judgement, Temperance and parts of circle presentation remained incomplete.
- Successful Proc, damage, SHOT scheduling, SCENE_READY, BEAM_READY and VisualGeometry logs must not be treated as proof of actual visible GPU output.
- Arcana Staffs v0.6.5 footage/source was used as a working visual reference. Arcana Staffs remains an independent mod, not a TOOM gameplay authority.
- QA1-CP005: PlayerCharacter frame update source and recursive ARTO beam opacity/bounds correction; SOURCE/BUILD/DEPLOY/DOCSYNC PASS. DLL C22F20A12F1E144BE7F49B4E2681B117362AA6E8F62C677FCAA24252F5F2A018. Runtime NOT_TESTED.
- QA1-CP006 R1 PRECHECK FAIL: assistant-provided expected 008 SHA256 was incorrectly truncated to 62 characters. Production unchanged; rollback NOT_REQUIRED.
- The assistant incorrectly instructed the user to edit one line manually, contrary to existing Rules/S01-L04/S02-L02. Correct recovery requires a complete replacement execution block.
- QA1-CP006 R2 PRECHECK FAIL: validator incorrectly required 008_SOURCE_ASSET_INVENTORY.md to appear inside its own ledger. Production unchanged; rollback NOT_REQUIRED.
- QA1-CP006 R3 PASS: Magician visual spacing changed from 96/60u to 28/12u; preserve scale 0.65, tier order, damage, motion, rotation and World twelve-point formation.
- QA1-CP006 R3 Source SHA256 D498D5979F39618074AB27F46E57236BCF04E8B5C57B3A5262ECBC00F97B9764; DLL 173674DBD2022B547086C99B3F1FFB516CECE22CEDE0C2D90C686D9354FDBD48. Runtime NOT_TESTED.
- QA1-CP007 PASS (user Windows RESULT): independent Temperance C2 ring/Hebrew-like inscription band halo; nominal scale 0.15, 14u behind head and 5u above, temporary ACTI lifecycle separate from melee Proc pool.
- QA1-CP007 DLL SHA256 B5F32B33793020B9456D6101DAA9664439207BABEE04F702387467A193B7466D.
- QA1-CP007 inventory SHA256 DAB0141B032A9E12C4E8AB0AA7EEE57DC715F483D325FCC1B7AF42783937DCBC.
- CP007 SOURCE/BUILD/DEPLOY/DOCSYNC/BACKUP PASS. Its in-game runtime remains NOT_TESTED.

### Root causes and prevention
- Treat visibility, coordinates, gameplay hits, sound and actual scene rendering as separate acceptance gates.
- Certain VFX diagnostic records are limited to 16 messages. A stopped diagnostic printout is not proof that VFX execution stopped.
- BEAM_Halo is part of the beam NIF; it is not an independent Temperance ON/OFF halo.
- Use the approved C2 ring and an independent lifecycle for Temperance; verify attachment, movement, toggling, cell change, load and cleanup in game.
- Do not import Arcana Staffs eight-layer placement into World, Judgement or Temperance. Preserve approved World 12-slot topology and stage order.
- The R1/R2 CP006 failures were wrapper-validator defects, not Production corruption.
- Validate 64-character SHA256 pins and inventory row contracts before asking the user to run a complete block.
- Never ask the user to patch old PowerShell by hand. Replace the whole transaction after reading the real failure.
- For CP008, read the actual latest documents, verify current-local CP007 binary and inventory, prepare final bytes, back up originals and 008, commit together, verify POST, rollback fully on failure.
- Do not rebuild or redeploy Production for documentation synchronization.

### Remaining acceptance boundary
- Existing 007 manual One-Boot QA A-J 158 cases are retained.
- Validate beam/circle/impact rendering, Magician eight-layer spacing, World 12-slot formation, Judgement, Temperance C2 halo position and persistence, audio, damage and saves.
- No intermediate game boots or automatic QA collector.
- Current CP007 runtime NOT_TESTED. Stage LOCK=NO.


<!-- TOOM_QA1_CP009_P0A_BEAM_ART_WIDTH -->
## CP009 P0-A — Default ARTO beam width and visibility-owner repair
- Previous CP009 bulk ARTO/NIF candidate was withdrawn without execution.
- Inline atomic PowerShell is the approved delivery path; downloadable .ps1 is not an execution prerequisite.
- Based on Codex 2026-10-24 mesh measurements: original Whole-Lens Halo radius 49.92u, default ARTO radius 1.92u, ratio 26:1.
- Adapt only default BeamWholeLens ARTO transverse width at the renderer boundary.
- Do not modify gameplay width, ACTI fallback width, BeamWidthTransform, longitudinal beam length or endpoint.
- Preserve Tintable mapping until separately measured.
- Separate original input width validation and converted ARTO width validation; Magician T8 converted width 75.4 must remain permitted.
- Default ARTO bounds use measured cross-section rather than the old 256-unit transverse heuristic.
- ARTO readiness is the single display authority; ACTI cannot unhide itself through SetVisible or ApplyActiTransform.
- Restore ACTI fallback authority if its ARTO axis 3D is missing.
- World formation, Judgement, Temperance, Magician gameplay and approved coordinates remain unchanged.
- Visual runtime acceptance remains NOT_TESTED for this candidate; earlier CP007 integrated VFX is PARTIAL_FAIL.
- Stage LOCK=NO. Test default beam visibility and flash-cloud overlap during the next integrated manual QA.


<!-- TOOM_QA1_CP009_P0B_CIRCLE_HALO_POSE -->
## CP009 P0-B — Circle pose and Temperance halo correction
- Current parent: CP009 P0-A BUILD/DEPLOY/DOCSYNC PASS.
- CircleStatic and CircleOuterBandStatic previously converted world rotations to Euler for ACTI SetAngle.
- This candidate neutralizes the ACTI reference rotation and applies the world rotation directly to CC_Root in parent-local space.
- Existing ACTI records and CC_Root meshes are reused. No additional ARTO forms, NIF or DDS.
- Circle pose is retried if engine-owned 3D is not ready. Visuals remain culled while the pose is unresolved.
- Temperance halo behind-head X offset changes to the negative sine of actor yaw; Y remains negative cosine.
- Halo OFF culls scene 3D before disabling and deleting the temporary reference.
- Halo detects 3D root rebuilding and reapplies the transform.
- Actual head-node orientation, instant ON display and final ring normal remain runtime acceptance subjects.
- No gameplay, damage, cost, beam length, World 12-point geometry or audio changes.
- CP009 P0-A default ARTO beam width fix is preserved.
- CP009 P0-B runtime NOT_TESTED; Stage LOCK=NO.


<!-- TOOM_QA1_CP010_PREQA_RECOVERY_20261024 -->
## QA1-CP010 — CP009 P0-B 이후 통합 QA 직전 누적 교훈

### 현 상태와 증거
- CP009 P0-A: 기본 Whole-Lens ARTO 폭 26:1 환산, 변환 전/후 검증, 바운드 및 ACTI/ARTO 표시 권한 교정. SOURCE/BUILD/DEPLOY/DOCSYNC/999/008 PASS.
- CP009 P0-A DLL SHA256 E4DE4D012063A6103C1F21ABB27379E3727BA7735C2C8C5D880E15565A128F67.
- CP009 P0-B: 기존 CircleStatic/OuterBand ACTI의 CC_Root 직접 회전 행렬 적용 후보, 절제 C2 헤일로의 뒤쪽 좌표·3D 숨김·재생성 보정.
- CP009 P0-B는 사용자 제공 Windows 결과로 SOURCE/BUILD/DEPLOY/DOCSYNC/999/008/BACKUP PASS.
- CP009 P0-B TransientVFXManager.cpp SHA256 F8D0DFDE7E06C2F4F5266056E020FB8F58E12299D2640A18D1CE6805123BD56D.
- CP009 P0-B TemperanceRuntime.cpp SHA256 9B736F9A2F36DF6A9BE033A0D15682D584DACC42A69FA45222D98A0B0EF45E16.
- CP009 P0-B DLL SHA256 61A9862F148FF165AD31355DD86CAA7DD9ACB9E2948AD67E27202514BA5E2DA5.
- CP009 P0-B 008 SHA256 EFC8D638761C0C6F9E27D2451CEF987F454EE7FE2E7AEBD3725EDE7575A3EFDE.
- CP009 P0-A/B 통합 후 런타임은 NOT_TESTED. 이전 2026-10-24 영상에서는 마법진 위치·회전 및 헤일로 가시성 FAIL, 기본 빔 판독성 부족, 일부 마법진과 섬광/구름 출력이 관찰되었다.

### 실패, 실수 및 수정 과정
- CP006 R1: 잘못된 62자리 SHA256 상수로 PRECHECK false-stop. 사용자 수동 한 줄 수정을 요구한 지시도 잘못이었다.
- CP006 R2: 008 자기 참조 행을 요구하여 PRECHECK false-stop. R1/R2 모두 Production 변경 없이 종료.
- CP006 R3 PASS는 28/12u 간격 코드 적용을 증명하지만 실제 마법진 배치의 화면 정상화를 증명하지 않는다.
- CP007에서 절제 헤일로를 기존 ACTI에 얹었으나 머리 노드의 월드 기저·시각 3D 수명 검증 없이 14u/5u/0.15배 후보를 적용했다.
- 초기 CP009 대규모 ARTO/Pose 이식 시안은 기존 TOOM 백엔드의 정적 결함을 먼저 확인하지 않고 ARTO 257개와 NIF를 늘리는 과잉 수정 계획이었다. 철회되었으며 실행하지 않았다.
- CP009 외부 다운로드형 .ps1 제공은 과거 C2-B2 R2 SCRIPT NOT FOUND 재발이자 000/001/006 위반이었다. 실행 파일 다운로드 요구 금지.
- 코덱스 실측: 기존 Whole-Lens Halo 반경 49.92u, 기본 ARTO Halo 반경 1.92u. 26:1 단면 환산 누락이 빔 가시성 부족을 설명하는 정적 결함 후보였다.
- 기본 ARTO 폭 보정은 어댑터 경계에만 적용. Magician T8 2.90 x 26 = 75.4를 종전 64 제한으로 잘못 거부하지 않도록 검증 분리. Tintable은 별도 단면 실측 전까지 미변경.
- ACTI와 ARTO의 동일 슬롯 표시 권한 충돌 가능성 확인. 로그의 BEAM_READY, CIRCLE_READY, SPAWN은 실제 픽셀 출력 성공이 아니다.
- F2_ACTI_EULER_NONUNIQUE 경고와 영상의 회전 이상은 연관 조사 대상이나 단독 인과관계가 완전히 확정된 것은 아니다.
- CP009 P0-B는 기존 CC_Root 행렬 적용을 선택했으며, Arcana Staffs의 엔진 소유 ARTO/Pose 전체 이식과 동일하지 않다. 시각적 결과는 수동 검증 전까지 미확정.
- 절제 헤일로 OFF 로그는 Delete 호출 사실만 보여준다. 실제 화면 소멸 시간은 사용자 관찰로 판정한다.
- 빔과 FlashCloud가 겹치면 빔 자체 출력과 착탄 구름의 시야 가림을 분리해 확인한다.

### 유지할 계약과 다음 작업
- 승인된 World 12개 배열, 4/8/12 순서, 128u 반경, 64u 빔 길이, 192u 끝점 및 피해·충돌·Magicka 정책 유지.
- 마법사 T1~T8, 심판, 세계, 절제의 출력·방향·스케일·시각 종료를 구분하여 확인.
- Arcana Staffs v0.6.9는 렌더링 참고 사례이며 TOOM 주문 설계나 런타임 성공 판정을 대체하지 않는다.
- 007 A~J 158개 수동 O/X/- QA 보존. 일부만 시험하면 미시험은 '-'로 남긴다.
- 통합 One-Boot QA와 영상/로그 대조 후 실패 항목만 선택적으로 수정한다.
- CP010은 문서 전용. BUILD/DEPLOY 변경 없음. RUNTIME=CP009_P0B_NOT_TESTED, LOCK=NO.


<!-- TOOM_QA1_CP011_POSTQA_RECOVERY_20261024 -->
## CP011 — 2026-10-24 POST-QA 시행착오, 실수와 교훈

### 실제 구현 권위
- Parent=CP010 PASS, latest Production implementation=CP009 P0-B.
- DLL SHA256 61A9862F148FF165AD31355DD86CAA7DD9ACB9E2948AD67E27202514BA5E2DA5.
- Parent 008 SHA256 2A75A17CBB94D8E8D88930228ADCD99D4C761966D3432A05B93CE6AE3B24BFC8.
- CP009 P0-A: 기본 ARTO 빔 단면 26:1 환산과 ACTI/ARTO 표시 권한 정리.
- CP009 P0-B: CircleStatic/OuterBand CC_Root 행렬 적용과 절제 헤일로 위치·수명 보정.
- 실제 런타임은 빔·소리·착탄 및 절제 헤일로 일부 개선. 전체 성공 아님.

### 사용자 관찰로 등록된 미해결 결함
- Magician 마법진은 손을 따라 움직이는 듯하며 의도 위치에 안정적으로 고정되지 않는다.
- World 마법진은 반복 실행 시 좌표가 달라지거나 아예 표시되지 않는다.
- Judgement 마법진도 정상적인 표시가 불안정하다.
- Temperance 헤일로는 표시되지만 참고 이미지보다 크다. 현재 로그상 Scale=0.15; 목표 수치는 미승인.
- Temperance와 Judgement가 시체에도 반응한다.
- 신규 요구: Judgement 충전 중 사정거리와 표적을 나타내는 가늘고 비피해성인 조준선 빔. 미구현.

### 실제 로그에서 확인한 사실
- ordinaryoffensivemagic(20261023-211733).log: 2026-10-24 06:05:37~06:17:03.
- PlayerCharacterUpdate VFX 프레임 경로와 CoreAssets READY.
- Temperance HALO SPAWN Scale=0.15, 3D_READY 및 TOGGLE_OFF RETIRE 기록.
- World 4->8->12 단계, 12 beams/12 impacts 실행 기록.
- CIRCLE_POSE READY 8회, WAIT 8회; 진단 로그 횟수 제한을 고려해야 한다.
- BEAM_READY, FLASH_READY와 PLAY_REQUEST는 실제 픽셀/청감 성공의 단독 증거가 아니다.
- Judgement FINAL_RELEASE/SHOT는 있으나 명시적 IsDead 진단이 없다.
- BeamWholeLens ACTI에서 F2_ACTI_EULER_NONUNIQUE가 남아 있으나 ARTO 빔 실패 원인으로 단정하지 않는다.
- 로그 시각과 두 영상 프레임의 정확한 일대일 동기화는 미검증.

### 교정할 과거 실수
- 손 노드가 직접 원인이라고 확정적으로 설명했던 답변은 증거 수준을 넘었다. 카메라/배치 프레임, 시전 노드, 월드-로컬 변환, 3D 로드 재시도와 슬롯 소유권을 실제로 감사해야 한다.
- 헤일로 목표 크기를 임의 백분율로 확정하지 않는다. 사용자가 제공한 머리 크기 비교 이미지를 권위로 삼는다.
- 시체 감지는 타게팅/충돌/피해/이펙트의 경로를 분리해 원인을 파악한다.
- CP006 R1 잘못된 62자리 SHA, R2 008 자기참조 false-stop을 재발시키지 않는다.
- CP009 다운로드형 .ps1 제공과 불필요한 257개 ARTO 추가 시안은 철회된 실패 사례다.
- 직전 CP011 첫 입력이 PowerShell >> 프롬프트에 머문 사건: 선택 모드와 미완결 입력을 식별하고, 실행 전후 008·마커로 실제 적용 여부를 확인한다.
- CP011 첫 실행은 적용되지 않았으며 read-only 결과 CP011_MARKERS=0/8, 008=2A75A17CBB94D8E8D88930228ADCD99D4C761966D3432A05B93CE6AE3B24BFC8.
- 긴 블록의 콘솔 입력 상태를 무작정 실행 성공 또는 파일 손상으로 판단하지 않는다.

### 보존과 후속 절차
- World 승인된 12점 배열, 4/8/12 순서, 반경 128u, 빔 길이 64u, 끝점 192u 및 게임플레이·사운드 계약을 보존한다.
- P1-A 시체 감지, P1-B 마법진 프레임/좌표/3D, P1-C 헤일로 축소, P1-D 심판 충전 표시선을 실제 소스 근거로 검토한다.
- 통합 정적 검증/기존 증분 빌드/배포/문서 동기화 후 One-Boot QA를 시행한다.
- 기존 A~J 158개 수동 QA의 판정값은 일괄 변경하지 않는다.
- RUNTIME=PARTIAL_PASS_WITH_OPEN_DEFECTS, LOCK=NO.


## QA1-CP012 VFX-FIRST (2026-10-24)
- 기존 입력 이벤트의 Temperance 예약 갱신을 제거하고 기존 PlayerCharacterUpdate 훅으로 이전.
- 절제: 적중 즉시 결제/스냅샷, 0.15초 후 마법진, 총 0.27초 후 빔 및 피해 판정(런타임 후보값).
- 헤일로 0.095F 축소, 머리 노드 일시 소실 중 숨김 유지. 즉시 ON/OFF 시각 정상화는 미검증.
- CC_Root 위치 이탈 검사 및 재적용은 후보 수정이며 실제 화면 정상화 PASS가 아니다.
- CP012 첫 배포 DLL 잠금, 두 번째 File.Replace 호출 중 경로 형식 오류로 ROLLBACK PARTIAL 발생.
- 검증된 기존 Ninja 빌드 재사용, 바이트 쓰기 및 해시·백업 기반 실패 단계 복구; 게임 부팅 없음.
- P1-A 시체 및 P1-D 심판 조준선 미구현; World 12개 배열, A-J 158개 수동 QA, LOCK=NO 유지.


## QA1-CP013 Magician visual anchor — 2026-10-24
- User video: camera aim movement also moves Magician charge-circle center; existing ResolveChargeAnchor used origin+cameraForward*96u.
- CP013 visual-only candidate: center uses actor GetAngleZ horizontal facing at 96u; camera forward continues to control circle orientation and actual beam aim.
- Historical recurrence prevention: do not invent new file replacement APIs; retain byte-write backup and SHA-based rollback. CP012 prior lock and File.Replace failures preserved.
- World 12-point geometry, Judgement targeting, collision, beam damage and audio source are unchanged. Screen visibility remains an open runtime defect.


## QA1-CP014 Corpse/Halo — 2026-10-24
- CP012 temporary-head-missing branch could hide the halo indefinitely; CP014 retains the last positioned visible ACTI and logs HEAD_WAIT/HEAD_RECOVERED.
- Temperance skips already-dead targets before MP payment; resolved beams skip later dead actors. SE killing blows remain eligible.
- Judgement selects the nearest valid living marking Actor. Damage excludes dead hostile actors; scheduled artillery preserves its last valid aim after target death.
- CP013 Magician, World formation, Trigger Firewall and 158-case manual QA unchanged. Runtime visual PASS not claimed.


## QA1-CP015 Judgement charge aiming guide (2026-10-24)
- Separate damage-free persistent BeamRenderer from post-release 1-damage Judgement markingBeam.
- Keep 20-ft / 426.67u marking range; guide uses 0.06 visual width and does not call beam collision or fixed damage.
- Existing PlayerCharacterUpdate moves visual on aim changes; pause suspends visual; final release, cancellation, equipment change and reset release it.
- Initial render and World/Temperance issues remain runtime OPEN. No new Skyrim QA run or new polling hook.


## QA1-CP016-B R2 Arcana selective VFX recovery — 2026-10-24
- Reference: ArcanaStaffs v0.7.2 and ArcanaSharedAssets v2.7.0.
- Arcana ModelReferenceEffect / AS_CirclePose is a reference, not a direct replacement for TOOM ACTI / CC_Root.
- Halo uses valid ActorBounds top when the head node is unavailable.
- CC_Root parent bound expands only to contain an actual valid child world bound.
- Judgement guide checks live ChargeSession authority before visual updates.
- R1 regression: function used uninitialized $script:anchorCount under StrictMode. R2 uses a pure PatchOne function without shared scope counters.
- Do not repeat script-scope state failures, unverified File.Replace, isolated builds or intermediate Skyrim boots.
- Runtime remains unverified; P1-B/F remain OPEN.


## QA1-CP017 — Halo logical state cleanup (2026-10-24)
- CP016-C found ReleaseHalo returned early when ref was null, leaving headUnavailable, positioned, cellFormID and lastRoot stale.
- CP017 copies/resets the ref, clears logical state first, then returns if no ref. This is one minimal native code change.
- Retain CP016-B fallback, circle bound repair, CP015 Judgement guide, CP014 living-target checks and CP013 Magician anchor.
- Do not claim VFX GPU-visible success from source/build/deploy. ArcanaStaffs v0.7.2 is a mechanism reference only.
- Preserve full 007 A-J 158 manual cases. No mid-stage boot, new QA collector, new Ninja tree or downloadable PS1.


## QA1-CP018-A/B — readiness clock and halo clearance (2026-10-25)
- Current source snapshot: 32/32 hashes validated, ZIP SHA256 6CD7CD5848CE17DFAF7BD1389C4A890A0C0B77B9320699FD042F5C4FF4912069.
- Live QA: corpse suppression PASS; halo visible/toggle/scale PASS but body overlap; Magician, World and Judgement circle visibility FAIL; Temperance proc circles intermittent FAIL.
- CircleStatic WAIT and OuterBand READY logs are each capped at eight; READY proves pose code ran, not pixels. CP016-B BOUND_EXPAND was not recorded in the observed log.
- ArcanaStaffs v0.7.2 CircleReady requires an actual effect 3D and AS_CirclePose; TOOM keeps ACTI and CC_Root, with bounded 0.6s one-time 3D-pose grace in existing transient update.
- The visual-only clock grace delays transient circle growth/retirement until first pose, but times out; no polling hook or actor scan. No claim of all-circle restoration before runtime QA.
- Halo behind-head displacement 14 to 26 units is a screen-fit candidate only; retain scale 0.095, above 5, toggle and backend.
- CP016-B R1 PowerShell scope failure and CP012 File.Replace/lock failures: pure patch function, exact SHA candidate, current source and 008 gates, byte backup, Ninja-consistent rollback.

## QA1-CP019 Arcana 0.7.4 Native/VFX integration — 2026-10-25 08:30:36 +09:00
- Parent CP018-A/B. User 2026-10-25 instruction supersedes older Magician/World visual authority; history above remains preserved.
- Source is embedded Arcana 0.7.4 FlightStateFix, source ZIP D1E507DAC9820740F7E99995F5072B885103DF3DDBFBE84C55E2DFFEF539940F; shared assets 2.7.1.
- One TOOM PlayerCharacter 0xAD and ActorMagicCaster 0x03 authority. Arcana before/original/after order retained; standalone Arcana DLL disabled.
- CircleRenderer now owns engine ModelReferenceEffect / AS_CirclePose handles through EngineCircle. Growth/retirement clock begins after valid 3D pose, separately from gameplay.
- Magician 1..8 circles use Arcana SolarCrown sizes and actor-front (0,74,108) anchor, continuous 22->7 axis compression; beam visual starts at the stack and converges on the unchanged collision endpoint.
- World actual geometry is Arcana Hermit LayoutData at scale .81, circle scale .3564, feet+Z70 frame, about102.465u radius. Previous bounds-center128u is superseded.
- World actual explosion endpoint = new circleWorld + outward normal*64u. This is an explicit spatial gameplay endpoint revision. Damage100/radius85.333333/independent overlap, charge .7/1.4/2.1, 4/8/12 and TOOM FlashCloud remain.
- Judgement damage-free held Whole-Lens guide gets 10Hz living-hostile selection; release revalidates once, preserves one 1-fixed mark and 3/6/9 world-fixed artillery, .45/.12 timing and death continuation.
- TemperanceRuntime.cpp/.h, .15/.27 gameplay delays, damage/cost, geometry, corpse gates and halo scale .095 are preserved; only the circle display backend/lifetime changes.
- Confirmed flight scope: Magician planes/beam and Judgement guide tilt to aim; World dome, fixed artillery origins and Temperance proc retain their spatial policy.
- TOOM names retain priority. Arcana cards: 황제/은둔자/전차/태양/힘/정의/별/탑/광대, adaptive 여사제/달; corresponding card staff and tome names. FormIDs retained.
- New circle ARTO IDs D00..DBF; 192 independent forms. Previous TOOM 537 records byte-identical. Arcana ESP changes are FULL-only, 24 base +8 stance fields.
- Confirmed guide source defects were wrong renderer asset, positive-duration-only ARTO admission and zero held alpha. Fix all together; VISIBLE/READY logs alone were insufficient.
- Candidate MSVC found C2679/C2678/C2088 in the new host adapter before Production: raw callback assignment and !Relocation are unsupported. Read REL/Relocation.h; use uintptr_t and address()==0. Failed log retained; corrected syntax PASS.
- One read-only foreach-pipeline PowerShell ParserError occurred before execution; Production mutation0. Assign the foreach result before piping. No source corruption claim.
- Existing build/msvc x64 Ninja only. Source/backups/POST/deploy/008 validated in one recoverable transaction; exact binary hashes are in stage result/manifest.
- The original 007 A-J 158 cases are byte-preserved. No intermediate Skyrim boot or automatic QA collector.
- RUNTIME=NOT_RUN. GPU visibility, audio, flight/stances/staff and gameplay regression remain for final human One-Boot QA. LOCK=NO.
- AI technical report: evidence/STAGE_EVIDENCE/QA1_CP019_ARCANA074/005_AI_TECHNICAL_HANDOFF.md.

## QA1-CP019 final lifecycle and handoff — 2026-10-25 08:52:13 +09:00
- Bounded Arcana FindCircle: 1.2s/120 searches; finite 2s request +2.2s pending reuse quarantine; valid owned poses renew lifetime. Beams use finite1.5s and pending1.6s quarantine. No unbounded failed request search.
- Cold DataLoaded purges only TOOM's own circle/beam/flash forms after resolution. The host installs hooks after successful single listener registration and rejects unsupported runtimes first.
- First CP019 inventory had correct bytes/hashes but planned timestamps differed from actual writes. Corrected metadata from physical LastWriteTimeUtc; never infer timestamp identity from a hash PASS.
- Candidate-only exact CRLF anchor failed before mutation because Plugin.cpp used LF. Use counted LF/CRLF-aware matching, preserve unrelated bytes. Production was untouched by that precheck failure.
- Final build DLL SHA256 AED5FEE6FDCF92985EC24A7DF5802281828EBFDCA31AB2801E649C77EEC70AA9. Existing build/msvc x64 Ninja link PASS; unchanged-input actual build confirmed no work. Dry-run alone cannot prove no work.
- Final AI technical report: [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP019_ARCANA074\019_AI_TECHNICAL_HANDOFF_FINAL_20261025_085213.md
- SOURCE/STATIC/BUILD/DEPLOY/DOCSYNC are verified separately; final hashes/backup paths in 021_Final_Result_20261025_085213.json. RUNTIME=NOT_RUN, LOCK=NO; original007/158 preserved.
## QA1-CP019 coexistence exception and isolated accounting — 2026-10-25 08:58:23 +09:00
- User authorizes TOOM + Arcana spells/staffs concurrently. This explicitly supersedes only Arcana's entry in the earlier Magician other-hand prohibition; foreign/vanilla spell/ward/staff/scroll restrictions remain.
- Exact plugin-resolved Arcana carrier/enchantment ownership gates the exception. Equipped staff fallback uses its verified formEnchanting field. Base/adaptive names and FormIDs do not change in this refinement.
- Arcana cost writes require current owned spell + matching hand snapshot; opposite-hand ultimate guards reject stale ownership. TOOM ChargeSession/Profile/Arbiter/MagickaTransaction remain byte-identical, so only TOOM's own paid allocations advance TOOM tiers.
- Both modules compete for the same actual player Magicka. No total-resource-delta charging, free tiers, private MP pools, new priority or arbitrary numeric rebalance is introduced.
- Prior vcpkg applocal logged missing dumpbin while returning0 after successful link. Latest build pins Hostx64/x64 on PATH and rejects that diagnostic; clean compile/link/post-build PASS. An exit0 alone is insufficient for a failed post-build helper.
- DLL SHA256 7FE4C816534718FF9D6C403C3897F53E25CF02EA0681278BA37807F7782C4508. Existing Ninja x64 build/deploy PASS; source/008/docs synchronized. RUNTIME=NOT_RUN; LOCK=NO.
- Current AI technical report [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP019_ARCANA074\026_AI_TECHNICAL_HANDOFF_COEXIST_20261025_085823.md. External result [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP019_ARCANA074\028_Coexistence_Result_20261025_085823.json. Original007 A-J158 unchanged; add coexistence focus observations during the same single user QA boot.

## QA1-CP020 PREQA DOCUMENTATION CHECKPOINT — 2026-10-25
<!-- TOOM_QA1_CP020_PREQA_DOCSYNC_20261025 -->
- Parent implementation: QA1-CP019-ARCANA074-COEXIST. Documentation-only checkpoint. DLL/ESP/Native/VFX assets and existing 007 A-J 158 manual questions are unchanged. RUNTIME=NOT_RUN; LOCK=NO.
- User confirms the independent Arcana MO2 mod is unchecked. This is user-observed configuration evidence, not an independent Data-path inspection. Keep startup duplicate-DLL guard and verify actual game bootstrap.
- Source/build/deploy PASS cannot be promoted to pixel, audio, flight, payment or gameplay PASS. First runtime evidence must be from the integrated SE 1.5.97 One-Boot QA.
- MP lesson: shared current Magicka naturally regenerates; before/after AV difference is NOT automatically total cost. TOOM paid charge allocation and Arcana session spending have independent owners. Do not add Arcana payment to a TOOM tier.
- Two MP test lanes: (A) disposable QA save, temporarily set MagickaRate to zero and VERIFY current Magicka is stationary before measuring; (B) original regeneration and buffs restored by reloading without saving. If the stillness check fails, label lane A uncontrolled and do not infer cost from net AV.
- For each lane test right/left owner swap, TOOM-first, Arcana-first, staggered release, low/zero MP, fast equip change, adaptive spells, Arcana staff depletion and flight. Correlate current Magicka with native paid-charge/session logs and observed tier, not with final AV alone.
- The 192 plugin-qualified circle forms represent a bounded reusable REQUEST POOL, not a requirement to render 192 circles simultaneously. Test ordinary reuse, rapid cancel/recast, multiple 4/8/12 World, 3/6/9 Judgement, Temperance normal/power, Arcana concurrent VFX, menu/cell/load/death and cleanup.
- Explicit late-return check: cancel while an engine effect is pending, then watch >2.2 seconds for delayed ghost VFX and retired slots; the bound is from CP019 technical source and is NOT independently runtime-proven. Do not invent a new frame scan or QA collector.
- Log REQUEST/FIRST_POSE/RETIRE/timeout only if those exact event messages exist in current source; inspect actual logging before writing a test command. A successful CPU request does not prove rendered pixels.
- Historical fault: CP013-CP018 repeated offset, CC_Root bounds and 0.6-second grace changes without resolving the ownership/pose route. CP019 replaced circle display with Arcana-derived engine-owned ARTO; do not regress to the old patch-only strategy.
- Historical fault: CP019 candidate compiler C2679/C2678/C2088 REL::Relocation pointer assignment; actual pinned SDK requires uintptr_t assignment and address() test. Candidate-only LF/CRLF patch false-stop and post-build missing dumpbin were corrected and retained as lessons.
- Original 999 is latest-first two-file ENTRYPOINT, not a canonical DEV_ASSETS clone. Preserve old history. Distinguish newest CP020 DOCUMENTATION authority from parent CP019 PRODUCTION authority.

## QA1-CP021 POSTQA CORRECTION + ARCANA BALANCE — 2026-10-25T14:08:51.288579+09:00
<!-- TOOM_QA1_CP021_20261025 -->
- Parent document CP020; parent Production CP019 COEXIST. Previous user runtime is PARTIAL_FAIL. New candidate RUNTIME=NOT_VERIFIED, LOCK=NO.
- Current source/build/deploy hashes and final gate results: [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP021_POSTQA_CORRECTION\051_Result_20261025_140851.json
- Technical implementation and AI recovery report: [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP021_POSTQA_CORRECTION\049_AI_TECHNICAL_REPORT_20261025_140851.md
- Arcana data/ESPs/resources are inside TOOM; standalone Arcana Native remains disabled. Stances addon uses the actual installed Stances master and preserves plugin-qualified local IDs.
- TOOM gameplay MP/damage/tier contracts and original007/158 remain preserved. Arcana grades are fixed per card, not three new tiers per spell.
- User-confirmed Chariot: each actual shot40 cost/64 damage, repeated five-shot total200/320. Justice retains the3sec held beam with at most5x110 contacts. Other balance and billing are in Arcana Balance.inl.
- Staff capacities: normal8x20000, stance2x40000. Native per-instance charge ledger uses the corresponding ordinary card's engine-calculated Destruction reduction; ENCH engine charge cost stays0 to avoid double billing. No staff MP subtraction.
- Root ESPs default English; Language/en and Language/kr each retain the3 ESPs. Arcana - / 아르카나 - names; adaptive staffs (Unbound)/(해방). Language variants differ only in display fields.
-13 private thin-rim NIF derivatives preserve original front meshes, UVs, texture/material values. Sidewall geometry, not extra grace or fake bounds, adds slight depth. Original package/shared2.7.1 files and credits remain byte-preserved.
- Halo .095/back26/up5 retained; only its own CC_Outer/CC_Band scene nodes rotate at .12rad/s (~52.36s/turn). Proc/body/damage/cost paths are unchanged.
- World remains4/8/12 at .7/1.4/2.1 sec. Each new group grows .12s after real first pose. Air policy is snapshotted at cast begin and reuses actual Arcana AirWorldNormal: feet+Z78, radius112, outward beam64, explosion centers radius176. This is an explicit spatial endpoint revision in flight; TOOM explosion asset, damage100/radius85.333333/independent overlap stay.
- Judgement guide has engine-only held ARTO ownership, explicit REQUEST/EFFECT/3D/POSE diagnostics and the actual Arcana Center outlier policy. Older distant marking origin is observed; actor-bound contamination is a hypothesis, not proven runtime causation.
- One final manual SE1.5.97 QA; supplementary FQ in009. No intermediate boot, QA collector, isolated Ninja cache or public upload.

### CP021 failure and prevention record
- Confirmed packaging fault: CP019 merged Native but left Arcana ESPs and Sun/Dome model dependencies only in the independent MO2 mod. That mod was unchecked. Native hooks/TOOM192 forms/17 core probes could pass while Arcana forms and shared circle assets were unavailable. Verify enabled-package dependency closure, not only DLL equality.
- Old ACQUIRE_OR_3D_TIMEOUT merged different failure phases. The new path records request acceptance/failure, exact owned effect acquisition, 3D and first pose separately. None proves pixels.
- Held Judgement ARTO was coupled to ACTI creation/readiness. Held-only presentation now has one engine effect owner; pulses/Temperance/FlashCloud keep their successful path. No timing-grace increase.
- Candidate C2660: guessed3-argument ActorValueOwner::RestoreActorValue was invalid. Actual pinned RestoreActorValue takes2 args and always restores; DamageActorValue takes positive amount and dispatches negative kDamage. Corrected before Production, log007 retained.
- Candidate C2589 repeated S02-L01 Windows min macro failure in halo spin. Corrected to a scalar comparison, without global PCH changes. Failed log028 retained; later actual syntax PASS is distinct.
- Windows CreateProcess206 rejected an oversized command before execution. Complete author source was assembled locally in verified chunks; no user-managed download or partial Production execution.
- The installed Python NIF wrapper referenced a missing getShaderFlags1 export. Its CreateShapeFromData also generated a default shader despite source property IDs. Validation rejected these prototypes. The final author uses current local nifly::CloneShape to copy real shader/alpha children; original fronts and sidewall material bindings are reloaded and compared. No dependency replacement or unverified shader claim.
- Nifly Vector2 uses u/v, not x/y. Actual declaration corrected the candidate utility before asset promotion; failed utility compile032 retained.
- Same-slot hooks are not doubled: existing host0A explicitly dispatches Arcana admission;03/AD remain existing single-owner chains. Stance admission must consult the frozen active mode, not reread a new stance mid-cast.
- Snapshot the complete mutable Ninja state together with artifacts for rollback. Never restore just a DLL and leave its object/dependency state inconsistent. No standalone build directory is created for the plugin.

CP021 linked DLL SHA256 8951AA92C628B0E3693B4EF458275C67867551208FD855B12E1F76465A015A77; bytes 1147392. Final deployment and008 authority are the external verified result above.

## QA1-CP021 실제 ESP 병합 교정 — 2026-10-25T14:48:40.013312+09:00
<!-- TOOM_QA1_CP021_REAL_ESP_MERGE_20261025 -->
- CURRENT=QA1-CP021-ESP-MERGE-CORRECTION; PARENT=CP021 POSTQA CORRECTION(8951AA92...).
- 최신 사용자 지시가 이전 별도 Arcana ESP 유지 및 CP021의3개 ESP 납품 구조를 명시적으로 대체한다. 과거 기록은 삭제하지 않는다.
- 본체:TULL Ordinary Offensive Magic.esp. 기존 TOOM729개 레코드 바이트/로컬ID 보존 + Arcana657개 실제 이관 = TES4 포함1387개.
- 애드온:TULL Ordinary Offensive Magic - Stances Addon.esp. MAST Skyrim.esm / TULL Ordinary Offensive Magic.esp / 실제 설치된 Stances - Dynamic Weapon Movesets SE.esp. 여사제/달 스탠스 레코드와 Native 조회가 이관된 TOOM 소유 폼/모드에 연결됨.
- Arcana 구ID349개가 TOOM과 겹친다. 기존 TOOM은 이동하지 않으며657개 Arcana ID를 미사용 ESL 범위에 명시적으로 매핑한다. IntegratedFormMap.h와 Docs/TOOM_CP021_INTEGRATED_FORM_MAP.json이 현재 매핑이다.
- SPEL/ENCH/MGEF/WEAP/BOOK/PROJ/COBJ/STAT 등의 typed 참조, Native LoadForms·Flight·GunFlight, OAR18파일35조건, EN/KR 언어판까지 함께 이관한다. 마스터 이름만 바꾸는 처리가 아니다.
- TOOM 모드 루트 및 Language/en·kr에서 구 ArcanaStaffs 두 ESP6개를 백업 후 제거. 기본 EN 및 en/kr 보관본은 각각 TOOM 두 ESP.
- CP021 TOOM/Arcana 밸런스·원본 은둔자 방어·자원 소유권·8x20000/2x40000·VFX·비행·헤일로·얇은 측면 자산은 보존한다.
- 사용자는 구 세이브 호환성을 검증/차단 조건에서 명시적으로 제외했다. 세이브 파일 변경/자동 이관은 하지 않는다.
- 기술 인수:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP021_POSTQA_CORRECTION\067_AI_ESP_MERGE_REPORT_20261025_144840.md; 실제 결과/해시:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP021_POSTQA_CORRECTION\069_ESP_MERGE_RESULT_20261025_144840.json; 변경 전후:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP021_POSTQA_CORRECTION\068_ESP_MERGE_MANIFEST_20261025_144840.json.
- 기존 build/msvc Hostx64/x64 Ninja만 사용. 원본007 A~J158 그대로. 실제 게임 부팅 없음; RUNTIME=NOT_RUN_FOR_MERGED_ESP, LOCK=NO.

### 실제 병합 교정 원인·재발 방지
- 이전 CP021은 단일 Native와 같은 MO2 폴더를 실제 단일 ESP 병합과 혼동했다. 사용자 정정 후에는 각 레코드의 실제 소유 플러그인을 기준으로 납품 범위를 확인한다.
- 애드온 MAST/파일명만 바꾸면 원본 데이터의 소유권이 이관되지 않는다. 본체657개 레코드·typed 내부 참조·Native·OAR·언어판을 같은 매핑으로 이관하고 기존 TOOM 레코드 바이트를 비교한다.
- 전체32비트 정수 치환은 금지한다. xEdit 원문 정의의 FormID 필드만 변경하고, 역매핑 후 원본 레코드 바이트와 완전히 같음을 검증한다. 비참조 숫자·문자열·피해·비용은 그대로다.
- 첫 이관 후보는 이전 HEDR count737을 실제 레코드/GRUP 계산과 일치한다고 가정해 PRECHECK에서 멈췄다(실제738). 원본 Arcana도 그룹을 제외한 count657이었다. Production 변경0. R2는 파싱한 실제 수량으로 TES4 HEDR만 정규화하고 그 차이를 기록한다. 통합 본체1399(1386 records+13 groups), 애드온20(14+6).
- 읽기 도구의 기본 cp949 인코딩과 PowerShell rg 다중 경로의 쉼표 오류는 읽기 단계 실패였다. UTF-8을 명시하고 각 경로를 별도 인수로 전달한다. Production 변경으로 오인하지 않는다.
- 이전 CP021 author 스크립트/3-ESP 설치 안내는 역사적 증거다. 최신069 Result/통합 FormMap을 확인하고 재적용하지 않는다.

CP021 merged linked DLL SHA256=E0AECD864CF47ED22C8277F5F39882E1460777198B0FEBF644A9CF59CE3D869C; bytes=1151488. Final008 is in [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP021_POSTQA_CORRECTION\069_ESP_MERGE_RESULT_20261025_144840.json.

## TOOM v0.8.4 — CP022 release layout and dedicated spell tome — 2026-10-25T16:22:44.184533+09:00
<!-- TOOM_CP022_RELEASE_LAYOUT_20261025 -->
- CURRENT=QA1-CP022-RELEASE; PARENT=CP021 ESP-MERGE. User designates this as the release baseline for future development; public folder/archive omit Dev/CP022 Clean suffixes.
- Current source root remains [PROJECT_ROOT]; Native module is src/Arcana, current authoring assets assets/TOOM/Data and assets/TOOM/release_manifest.json. Historical assets/CP019/CP021/parallel/evidence are preserved, outside the install package.
- New MO2 folder:[LOCAL_PATH_REDACTED]
- One Native host SKSE/Plugins/TOOM.dll, settings TOOM.ini. Duplicate old OrdinaryOffensiveMagic.dll/ArcanaStaffs.dll are rejected. Existing OrdinaryOffensiveMagic.log filename and original007/158 remain for QA continuity.
- Main and Stances Addon retain their TOOM filenames and masters. Main1388 records(TES4 included), addon15. Existing FormIDs unchanged. All active EditorIDs use TOOM_. Two original staff first-person STATs and all staff WNAM/EITM/EAMT are preserved; no per-card staff STATs.
- One new shared inventory book STAT01000DC0 is necessary because BOOK INAM previously selected vanilla HighPolyDestructionBook. All15 books now use private world and inventory meshes, with original geometry/UV/collision/controllers/shader parameters retained.
- Book art: plain introductory coursebook; Hebrew title קסם התקפי רגיל; four exact original Hebrew_Band instructions; author מחברת: פרירן. In-universe credit is distinct from real asset authorship. Diffuse1024 BC3,11mips; vanilla plain-back normal; black emission mask.
- Custom meshes/textures/audio use TOOM; OAR uses one TOOM parent. Authored HKX basenames, AS_CirclePose/AS_BeamAxis, CC_* and OOM_ExplosionFlashCloud_Root scene contracts are preserved.
-50 historical/unreferenced/development files are excluded. All797 authored HKX bytes and18 OAR condition/priority sets remain. DDS aliases are deduplicated by complete bytes, not filename.
- TOOM damage/MP/tiers/limiter, Arcana CP021 balance, Hermit defense, staff20000/40000 and VFX placements/lifetimes are unchanged.
- Existing build/msvc Hostx64/x64 Ninja only; CMake deployment destination points to the new TOOM folder, PDB stays in build tree. No new plugin cache, intermediate game boot, automatic QA collector, public upload or save migration.
- Current results and final hashes:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP022_CLEAN_RELEASE\038_RELEASE_RESULT_20261025_162244.json; technical AI handoff:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP022_CLEAN_RELEASE\036_AI_RELEASE_HANDOFF_20261025_162244.md; changed/deployed files:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP022_CLEAN_RELEASE\037_RELEASE_MANIFEST_20261025_162244.json.
- STATIC/BUILD/DEPLOY/DOCSYNC/008 and RUNTIME are separate. RUNTIME=NOT_RUN; LOCK=NO until manual SE1.5.97 QA.

### CP022 technical lessons and rejected candidates
- File names are not dependency evidence. Build closure from ESP typed model/sound fields, Native model/audio literals, NIF texture/behavior fields and OAR routes. Retain vanilla dependencies only after actual installed BSA directory verification.
- Generic MODL is not always a NIF string: ARMO MODL is a4-byte FormID. The first candidate UTF-8 decode stopped before Production. Restrict edits to inspected owner/field types.
- The old VFX nifdump rejected valid staff collision with exit7, including the original unmodified staff. Its source intentionally rejects collision; this is a reader limitation, not corrupted staff geometry. Use the generic nifly Load/IsValid/reference validator for all41 meshes.
- Compilation does not validate scene-node lookup strings. A broad OOM_ EditorID rename would also rename the FlashCloud pose string while leaving the authored node unchanged. The candidate was corrected to preserve OOM_ExplosionFlashCloud_Root and all other authored node contracts before Production.
- Per-card staff STATs and duplicated meshes were unnecessary for the two real staff models. User correction removed8 candidate STATs and16 redundant model copies. Keep display names card-specific and share the original two STATs/four mesh files.
- Book INAM is distinct from world MODL. Original0002FBB5 resolves actual HighPolyDestructionBook STAT and a separate inventory mesh. One shared custom STAT is justified for the15 custom-textured books;15 separate STATs are not.
- Vanilla .nif text in BSFadeNode names is not a resource dependency. Actual nifly AV output identified SpellTomeDestruction*.nif as root node names. Preserve them and verify the inventory Book01Project.hkx in Skyrim - Animations.bsa.
- Repeated TES4 MAST/DATA and multi-effect EFID/EFIT cannot be compared through a plain dictionary: it drops earlier occurrences. The failed candidate whitelist was replaced with ordered subrecord comparison. No gameplay fields changed.
- A missing norm helper affected candidate-only book authoring. Resume the failed metadata phase using verified existing candidate bytes; do not reapply a new STAT to an already-written candidate.
- Absolute-path privacy regex must not mistake https:// for a drive path. Public provenance redacts machine drives while keeping author/source hashes. Candidate scene-binding evidence contained one actual G: path and was sanitized.
- A guessed FlightInput.inl read failed; actual input ownership is in Flight.inl. Enumerate files/search symbols before assuming a file exists.
- Image generation followed user corrections: ornate English cover rejected; plain Hebrew coursebook accepted; original band instructions and Frieren author line added. Only final art is promoted. Built-in image_gen inputs/prompt set and final source are saved under assets/TOOM/Authoring/Books.
- DDS generation fixes actual top-level compressed linear size and complete mip metadata after format encoding. Geometry/UV and every non-path NIF block byte are checked independently; none proves in-game pixels.
- A separate new MO2 folder and exact ZIP are the release handoff. Do not silently overlay the old Dev install or enable both Native hosts. Preserve source/assets/008 and the full mutable Ninja snapshot in a single rollback scope.

Final TOOM.dll SHA256=40CF28F7EF7C4457AA6972C16E5F0C971EB1158A4963111DE428C09DA2026650; bytes=1145344. Release ZIP SHA256=E0667CC2D6F3FB802FCB482ED6CE672E8E2A330F2C1296978A6D6E2B50A56504. Final008 is externally recorded in [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP022_CLEAN_RELEASE\038_RELEASE_RESULT_20261025_162244.json.

## v0.8.5 / QA1-CP023 POSTQA asset and Judgement guide repair — 2026-10-25T17:20:02.989236+09:00
<!-- TOOM_CP023_POSTQA_REPAIR_20261025 -->
- PARENT=CP022 v0.8.4. User's13 screenshots and3:31.27 video establish Runtime VISUAL FAIL: purple book/staff surfaces, opaque square circle planes, halo square, World screen obstruction and bad Judgement guide origin. Earlier hash/ZIP PASS remains byte-identity evidence only.
- Confirmed Native defect: global OOM_ substring replacement also changed already-renamed TOOM_ paths into TTTOOM_/TTOOM_. Runtime log reported10 missing core models. Correct exact10 catalog literals; normalize6 TTOOM circle filenames and typed ESP MODL fields.
- Raw texture path regression: CP022 wrote forward slash strings into40 NIFs. Restore original backslash convention within changed raw texture strings;383 total raw references are checked, with every other byte preserved. No DDS pixel or material/geometry changes.
- A prior nifly Load/IsValid check normalized some BSShaderTextureSet paths in memory via PrepareData -> TrimTexturePaths. It did not validate the exact engine-facing raw bytes. New tools/toom_package.py validates raw paths, actual Native literals and typed ESP paths against the shipped file set. Negative regression checks reject both original bad package and corrected assets paired with old Native source.
- Judgement guide display is separated from moving Actor worldBound center. Use actual active TOOM charge hand's MagicCaster::GetMagicNode world translation, confirmed by pinned SE declaration. A read-only session accessor reports IsHeld; payment/drain/tier code is untouched.
- User specifically requests the CHARGE GUIDE to leave one palm-sized circle. One EngineCircle-backed CircleRenderer follows the same hand node and beam axis; measured child width400u x .03=12u outer diameter. Circle and beam share the start point. Both clean up on release/suspend/menu/equipment/load/reset; no artillery circles are created early.
- Logical marking/selection ray, range, one-time mark, MP, tiers,3/6/9 release artillery and .45/.12 timing remain unchanged. This is display correction; any residual selection-origin behavior requires separate observed evidence.
- All other TOOM/Arcana gameplay, staff20000/40000, Hermit defense, flight, existing halo placement/rotation, authored thickness and World formation/endpoints remain unchanged. Original007 A-J158 is byte-preserved.
- Working assets remain assets/TOOM/Data and module src/Arcana; new current ZIP:[PROJECT_ROOT]\releases\TULL Ordinary Offensive Magic - v0.8.5.zip. MO2:[LOCAL_PATH_REDACTED]
- Evidence/result:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP023_POSTQA_ASSET_REPAIR\019_REPAIR_RESULT_20261025_172002.json; AI technical report:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP023_POSTQA_ASSET_REPAIR\017_AI_REPAIR_REPORT_20261025_172002.md. Parent runtime VISUAL FAIL; corrected runtime NOT_RUN; LOCK NO.

### CP023 recurrence and validator lessons
The assistant's namespace cleanup introduced the broken Native names and engine texture paths; this is not attributed to the user or another mod. File/hash agreement cannot detect a consistently broken package. Verify source literals against actual output paths after all replacements, and inspect raw serialized shader paths before a reader can normalize them.
Candidate R1/R2 incorrectly demanded private copies of verified vanilla texture dependencies; Production unchanged. Corrected validator keeps the exact audited vanilla allowlist. Candidate R3 accidentally reused the old-data variable for a guide patch anchor, causing an int-key error before Production. R4 uses separate guide_old/guide_new names and passes. The first orchestration JS had invalid triple-quote syntax and did not execute. All were candidate/tool failures, not additional Production writes.
Do not infer final runtime restoration from this build. Restore texture resolution before changing exposure, circle dimensions or VFX transforms to compensate for white quads. Keep malformed raw-path and Native mismatch regression cases as permanent packaging rejection gates.

## TOAM v0.8.6 / QA1-CP024 — 2026-10-26T02:06:09.596890+09:00
<!-- TOAM_CP024_STAFF_CTD_ASCENT_NAMESPACE_20261026 -->
- PARENT=TOOM CP023 v0.8.5. Latest user observed textures/Judgement guide-artillery/Fool circles confirmed; stance staff cast CTD. Parent RUNTIME=PARTIAL_PASS_WITH_STAFF_CTD. Corrected RUNTIME=NOT_RUN; LOCK=NO.
- User renames active release to TULL Ordinary Attack Magic / TOAM. Current assets:[PROJECT_ROOT]\assets\TOAM\Data; MO2:[LOCAL_PATH_REDACTED]
- Exact Native CTD cause: wrong pinned RefreshEquippedActorValueCharge SE/AE IDs. SE38752/RVA679000 is a container/index operation; its+96 matches the crash. Local typed RefreshStaffCharge uses native-verified SE37803/RVA6313D0 and paired AE38752. No third-party SDK edits; exact worn staff instance/hand snapshot is used by payment AND admission.
- Existing spell MP/damage/tiers, Hermit defense, Tower cap, normal20000/stance40000 and staff MP0 preserved. Four new ascent-only ARTOsDC1..DC4 and a private outer-band NIF add bounded expanding/fading foot trails during airborne held Jump. Existing takeoff and Temperance visuals stay intact.
- Book prefixes Ordinary Attack Magic: / 일반공격마법:; Fool General Education Magic: / 일반교양마법:. Ring Arcana Limiter / 술식 출력 제한 반지: English latest user direction, Korean original ESP authority.
- Exact token/typed namespace migration covers source, ESP masters/EDIDs/model/audio, NIF raw strings and OAR; previous malformed-prefix and slash regressions remain rejection gates. All797 HKX and45 DDS bytes preserved. Original007/158 SHA00CF6FBC482514684F87DB317EC6A2B433F2286777F25403206BE15BA5F6F0F1.
- Working and packaged air combat credits distinguish authored Native integration from imported flight/firearm HKX and technical references. Preserve original provenance/permissions; PATH_MIGRATION maps prior names.
- AE preparation PARTIAL, not enabled: exact Hermit/Tower precommit AE ABI and audio file/position contracts remain unverified. Do not treat CMake SE+AE options or paired IDs as runtime support. Reject before installing hooks.
- Result:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP024_STAFF_CTD_ASCENT_RING\020_RESULT_20261026_020609.json; AI report:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP024_STAFF_CTD_ASCENT_RING\018_AI_TECHNICAL_REPORT_20261026_020609.md; working report:docs/CP024_NATIVE_AND_MIGRATION_REPORT.md. Full rollback includes mutable Ninja state. Current008 SHA is external in result.

### CP024 prevention
Pinned wrappers can have a reversed SE/AE pair even when their public method signature looks right. Verify the exact native target and crash instruction; removing a refresh or adding cooldowns would not fix this ABI defect.
When a resource resolver starts returning a weapon/instance/hand snapshot, update admission and billing callers together. Candidate MSVC errors are not Production corruption.
Do not infer stored language encoding from PowerShell console mojibake. Decode actual bytes strictly before display-name edits.
Read indexed file sets once; repeated rglob per animation caused a needless stage-only delay. Preserve failed candidate logs, then retry only the corrected layer.
CP024 first transaction built and linked but final validator incorrectly compared legacy `YYYY-MM-DD HH:MM:SS local` rows to new fractional-second timezone strings. Full rollback (all targets and626 Ninja files) passed. Retry keeps historical row bytes and pins their actual pre/post modification time instead; changed rows still require exact new metadata. Historical hashes and normalized-reader success do not validate a renamed package. Recheck actual serialized paths and dependency closure.


<!-- TOAM_CP025_SESSION_DOCSYNC_20261026 -->
## CP025 — cumulative corrections / failed approaches / prevention

1. CP019: Single Native DLL integration does not imply that the
   original ESP, meshes, textures, audio and animations are deployed.
   FORMS ready=false exposed missing Arcana data.

2. CP021: Same-MO2-folder packaging is not a single-ESP merge.
   Real consolidation required typed FormID mapping for 657 records,
   internal references, native form lookups, OAR and Stances MAST.
   String-based 32-bit/global FormID replacement is forbidden.

3. CP022: Renaming OOM_ to TOOM_ using substring replacement
   corrupted already-normalized paths into TTOOM_/TTTOOM_.
   Perform exact-token, idempotent namespace migration and validate
   final raw literal/file pairs.

4. CP022: nifly reader success can hide invalid raw BSEffect shader
   paths because some readers normalize texture strings in memory.
   Inspect shipped raw NIF strings before loader repair; preserve
   backslash conventions, transparency/alpha and authored nodes.

5. CP022: Staffs share silver/gold physical models. Do not create
   per-card STAT or mesh proliferation when only visible names differ.

6. CP023: MODEL_PROBE/FIRST_POSE/3D readiness is not proof of GPU
   pixels. White squares, purple surfaces and fullscreen opaque
   planes require actual screenshot/video acceptance.

7. CP023: Judgement logical actor-center targeting must not be used
   as its visible muzzle. The guide uses the held hand's actual
   MagicCaster node; logical damage/target authority stays separate.

8. CP024: CommonLib's pinned Actor charge-refresh wrapper used
   inverted SE/AE relocation IDs. SE38752 targeted an unrelated
   container/index function, causing stance-staff CTD.
   SE37803 was native-disassembled and a typed local adapter used.
   Never trust wrapper naming/offset pairs without ABI evidence.

9. CP024: Staff admission and payment must use one exact equipped
   instance snapshot: weapon + ExtraCharge + hand + enchantment.
   Do not re-resolve the equipped object midway through payment.

10. CP024: AE compilation proves neither AE hook bytes nor ABI.
    Hermit/Tower SE37523 counterpart and positioned audio calls
    remain blockers. Fail closed rather than silent partial support.

11. CP024: Windows encoding must be determined from actual bytes.
    Console code-page assumptions (cp949 vs UTF-8) are unsafe.

12. CP024: When a function return type changes, inspect and update
    every caller before build; do not fix only the implementation.

13. CP024: Avoid repeated full filesystem traversal per animation
    entry. Index inventory once, then perform bounded lookups.

14. CP024: Historical timestamp-string validators can false-stop
    valid work. Preserve historical row formats and verify actual
    hashes/physical timestamps, not invented textual normalization.

15. CP024: A failed multi-file transaction requires full coherent
    rollback including mutable Ninja state, not a lone old DLL copy.

16. CP025: A handoff ZIP is a snapshot, not the live authority.
    Include the current 000/001/006/008, core documents, both 999
    entrypoints, CP024 reports/results, source snapshot and release
    package. Verify each attachment and ZIP integrity.

17. Always distinguish user-observed old-runtime failures,
    corrected source candidates, and new-runtime verified PASS.
    The latter is not yet available for CP024.

<!-- TOAM_CP026_POSTQA_STANCE_VFX_20261026_065856 -->
## CP026 / TOAM v0.8.7 — 2026-10-26T06:58:56.686504+09:00
- PARENT production CP024 v0.8.6; PARENT docs CP025. Parent user runtime PARTIAL_FAIL_WITH_FREEZE. Corrected runtime NOT_RUN; LOCK NO.
- Actual source/static candidate9 files; main1393 records includingTES4, addon15; EN/root/Language/en and KR/Language/kr verified.
- Staff payment preserves exact worn instance/hand, ExtraCharge, inventory changes and raw charge AV sync. Do not reinstall weapon abilities during caster billing; SE37803 ABI correctness did not prove lifecycle safety. Exact reported freeze instruction remains unconfirmed without a thread stack.
- Adaptive ordinary MP readback checks total and actual damage modifier, never retries or force-sets MP; explicit native God Mode is respected/logged. Its presence in the parent failure is not proven. Other unproven payment rejection still fails closed.
- Unused Stances MASTER removed after typed reference audit; own record/reference index3->2, local IDs unchanged. Actual installed low4251A/mid42519/high42518 perks are dynamically detected. Stances remains needed for stance changes; fallback neutral. Cast mode remains a snapshot.
- World32 Dome ARTO models are uniformly Hermit. Existing stage4/8/12, .7/1.4/2.1, .12 growth, ground/air coordinates and TOAM explosion gameplay/assets are unchanged.
- Temperance halo display only: private ARTO DC5/EngineCircle slot193 and original3-shape band wrapper; scale.095/back26/up5/spin.12 preserved. Proc.15/.27 and all cost/damage/corpse rules unchanged.
- Ascent release stops emission, allows visible .68s tails to finish, and cancels/quarantines unseen pending returns. Off/menu/cell/load/death/reset clear all.
- Tower field ThinRim axial depth3->.06 asset units; actual scale8.5 changes wall-height25.5->.51. Planar art/UV/materials and all gameplay unchanged. Independent mesh comparison passed.
- Limiter MGEF80F description now Ordinary Attack Magic / 일반공격마법 in all language copies. Ring names retained.
- Current physical package contains635 HKX and45 DDS, all byte-preserved; earlier797 counts describe authored source instances, not distinct shipped files.
- Existing build/msvc MSVC Hostx64/x64 Ninja only; same transaction backs up full mutable Ninja state, source/assets/deploy/docs/008. Original007/158 preserved. No game launch or automatic QA collector.
- Evidence/result:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP026_POSTQA_STANCE_VFX\022_RESULT_20261026_065856.json; changes:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP026_POSTQA_STANCE_VFX\021_MANIFEST_20261026_065856.json; AI report:docs/CP026_POSTQA_STANCE_VFX_REPORT.md; release:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - v0.8.7.zip. AE remains blocked on prior native ABI proof.

### CP026 implementation and validation lessons
The source audit proved abrupt ascent clear, mixed Dome model pool and missed MGEF DNAM rename. Do not substitute offset/grace/bounds edits for these defects.
A native function can have the correct ABI yet be unsafe in a particular callback lifecycle. Separate raw charge display from weapon-ability rebuild. Freeze causation is still a runtime question.
Never classify an unchanged MP display as proof of God Mode; read its actual runtime flag and the native modifier. Never retry/force-set MP to conceal a rejected payment.
Use the audited closed xEdit schema for master removal; HEDR includes GRUPs. Candidate header constants were corrected before Production. Typed reference validation compares every field, not just recognizable FormID bytes.
Preflight false stop: the first validator compared CP024 historical008 rows against CP025 files. Current008 explicitly says later rows supersede earlier rows. No Production write occurred. Resolve last row per path, verify physical bytes, and append new rows while preserving old checkpoint content.
A packaging-tool import regenerated a tracked pyc cache during read-only inspection. Its header/timestamp/size and recursively compiled code match the unchanged current source. Accept only this exact classified cache SHA in the atomic008 reconciliation, with backup/readback. Disable bytecode writes for future audit imports; no gameplay file had changed before preflight.
Stage-only tool failures: helper extraction omitted collections import; syntax R1 used unsupported EnumSet-to-int casts; R2 hit the Windows min macro. R3 follows underlying() and the existing ternary style. These failures did not touch Production.
Preserve raw NIF texture strings after library serialization. Independent mesh comparison proves vertices/UVs/normals/colors/tangents and material values; it does not prove GPU pixels.
Tower field depth must be measured after its real scale8.5. An apparently thin asset-space rim became a wall; reduce only the axial depth and preserve planar artwork and gameplay.
Physical archive HKX counts may differ from source-instance counts after byte deduplication. Report the measured current file count rather than repeating historical counts.

<!-- TOAM_CP026_STAFF_SMP_RETRY_20261026_072544 -->
## CP026 supplementary Staff SMP trial — 2026-10-26T07:25:44.020453+09:00
- Parent main/source/build/deploy is TOAM v0.8.7 CP026, DLL D68AB21E9B2757B9DF3DC1879E4880EE660E8844D525D78E2D5511A303AF85AB; this addon does not change that Native DLL, source, main/Stances ESPs or balance.
- User-provided Riilungolz archive SHA478870B28008229E737F88E38B050EBEB7D0BCE77E7A109861C41103E18E505C inspected. It uses skinned weapon + helper armor/XML + equipment script. Technical reference only; none of its assets/scripts/XML/records/binaries is distributed.
- New authored two-model SMP derivatives: 6 skinned shapes, each9-bone chain + root; 2 kinematic/8 dynamic bodies and8 constraints per model. Geometry/UV/normals/colors/tangents preserved; bind reconstruction and installed FSMP XSD passed. First-person STAT/models remain the existing two.
- Private Silver/Gold helpers use slot61/60. Occupied slot preserves foreign equipment and falls back static. Same-model dual wield deliberately falls back static; distinct-model dual wield has independent helpers. Never simulate NPC Root/body or add a ground collider.
- One .20s Papyrus equipment reconciliation; no loop, Native hooks/actor scans, resource/movement/jump mutation or equip locks. Keyword family ownership avoids stale per-card cleanup. Per-session TOAM_SMP_Enabled global default1 applies only if optional ESP is enabled; set0/1 then re-equip for one-boot comparison.
- Base SMP ESP16 records(TES4 included),7 helper/control records +8 original WEAP overrides; stance patch3 records,2 original WEAP overrides. Normal20000/Unbound40000, exactENCH/EITM/WNAM and gameplay bytes are preserved. Root English and Language/en/kr variants.
- Optional authoring:[PROJECT_ROOT]\assets\TOAM\Optional\StaffSMP; MO2 option folders:[LOCAL_PATH_REDACTED]
- Actual installed FSMP log identifies3.1.1; it remains an external dependency. No FSMP/SKSE/compiler DLL is bundled/replaced. Main AE guard remains in place.
- Original FernStaff FS-SMP-001 CTD cause remains UNCONFIRMED: no matching crash log/failed installation hash. Separate FernZoltraak invalid-pointer CTD is not SMP evidence. This trial is not a runtime stability claim.
- Detailed AI report:docs/CP026_STAFF_SMP_RETRY_REPORT.md; result:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP026_POSTQA_STANCE_VFX\048_SMP_DELIVERY_RESULT_20261026_072544.json; manifest:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP026_POSTQA_STANCE_VFX\047_SMP_DELIVERY_MANIFEST_20261026_072544.json. STATIC/PAPYRUS=PASS; RUNTIME=NOT_RUN; LOCK=NO.

### SMP retry tool and ownership lessons
The initial nifly helper compile failed on a guessed Vector3::Length member before authoring. Use pinned declarations or component math. Reacquire skin-instance blocks after partition updates.
Caprica's cwd defines Skyrim namespace interpretation. The first absolute-path compile failed with an invalid namespace; the correct script-directory cwd compiled a Skyrim gameID1 PEX. Preserve both logs.
Do not copy a reference mod's binary/NIF/XML/script as an assumed stability fix. Inspect actual files and compare their activation ownership; own private rigs and validate skin partitions/weights/bind transforms.
An old CTD without a crash log or matching installed bytes has no proven unique cause. STATIC/XSD/compile PASS must never become physics/jump/runtime PASS.
Two identical weapon rigs can share duplicate bone names; use explicit static fallback rather than unverified simultaneous solver ownership. Do not lock/replace foreign slot equipment.

<!-- TOAM_CP026_TOWER_SKY_20261026_074055 -->
## CP026 / TOAM v0.8.8 — Tower sky thickness supplement — 2026-10-26T07:40:55.967917+09:00
- Parent main/source/build/deploy CP026 v0.8.7 and its optional SMP0.1.0-test1 delivery. Previous user failures remain historical; new RUNTIME NOT_RUN; LOCK NO.
- Field band depth was already corrected25.5->.51 world units. Final source/model comparison found a separate sky path: Emperor rim5.16 x RainFootprint(t)/200 gives21.93u at t8.
- One private Tower-sky derivative compresses only CP021_ThinRim axial coordinates by.02: depth.1032 asset units / .4386 world units at t8. Original planar geometry/UV/material preserved; shared Emperor and other common circles byte-identical.
- New8 main ARTOsDC6..DCD own the2 hand x4 bank sky instances. Existing forms.circle grows168->176, normal original168 lookup retained, new local forms loaded directly. Only Rain sky requests kind3.
- Existing exact-ART tickets, finite find/3D pose, retirement/quarantine and forms.circle-wide PurgeArt handle these forms; no new hook, listener, frame loop or scan.
- Main1401(TES4 included), addon15, all rootEN/en/kr. Every pre-existing main/addon record except TES4 remains exact. No FormID moves or damage/MP/timing/range/shot/cap changes. Tower radius850/sky520/cost1600/targetcap630 preserved.
- This supplement changes5 source/config files; total11 distinct files since CP024. Earlier CP026 staff billing, adaptive readback, World consistent model, halo, ascent, limiter and field-rim corrections retained.
- SMP optional folders/archives remain intact, profile activation unchanged. Their WEAP overrides are still exact-compatible because all existing WEAP records and master filenames remain unchanged. Old native freeze and old SMP CTD are not declared runtime-resolved.
- Existing build/msvc Hostx64/x64 Ninja, full mutable Ninja rollback backup, synchronizedsource/assets/deploy/docs/008. All635 HKX/45 DDS/audio bytes and original007/158 preserved.
- New main release:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - v0.8.8.zip; result:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP026_POSTQA_STANCE_VFX\060_RESULT_20261026_074055.json; change manifest:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP026_POSTQA_STANCE_VFX\059_MANIFEST_20261026_074055.json; AI supplement:docs/CP026_TOWER_SKY_REPORT.md. Earlier reports and archives remain historical. AE guard remains; prior ABI blockers unresolved.

### CP026 Tower sky lesson
A field-only thickness correction did not cover the sky owner, which uses a different model and scale function. Audit all actual presentation owners before final acceptance.
Do not shrink all shared circles to repair a large-scale exception. Private owned ARTO/model variants preserve already suitable shared artwork and lifecycle.
Existing forms.circle-wide cleanup must include newly added forms; extending the same array retains the established load/cancel/death/purge authority.
Source/mesh dimensions are not visible-pixel proof. User runtime remains necessary for both Tower rings and the earlier freeze/SMP defects.

<!-- TOAM_CP027_PRESENTATION_BALANCE_20261026_093018 -->
## CP027 / TOAM v0.8.9 — 2026-10-26T09:30:18.977054+09:00
- Parent Production CP026 v0.8.8; new user runtime PARTIAL_PASS_WITH_OPEN_DEFECTS, NO_CTD_REPORTED. Corrected0.8.9 RUNTIME NOT_RUN; LOCK NO.
- Current source11 files, main1402(TES4 included), Stances15, rootEN/Language/en/kr typed parity. New no-damage aimed carrier DCE; original808 World/Temperance carrier exact.
- World private model only: inner Face/NodeGlow1.65, face105->173.25 vs outer band197.8;32 Dome ARTOs reuse existing owners.4/8/12, .7/1.4/2.1,.12 growth, all ground/air origins/endpoints and TOAM explosion gameplay unchanged. Hermit defense untouched.
- Sun one new layer each.8s; actual-first-pose growth.35s from4% scale, alpha.12s.8-second gradual1550 and1520 damage preserved, existing10-second readiness deadline retained.
- Magician/Judgement use Arcana engine aim angles and aimed concentration carrier metadata, zero engine billing/projectiles through existing caster hook, independent TOAM native charge ledger unchanged. No new hook/listener/actor scan.
- Explicit origin authority change for aim consistency: Magician logical collision origin now shared Sun anchor ground(0,74,108), flight uses original FlightCastOffset; Judgement guide/selection/release marking all share fixed right-shoulder local(34,22,126), same flight policy. Latest shoulder instruction overrides old hand guide. Range/radius/damage/MP numbers preserved; spatial origin/reach changes are deliberate and documented.
- Judgement fixed artillery origins/target snapshots/timing3/6/9/.45/.12 retained; circle display scale=.10*clamp(formationRadius/99.84,1,10). Corpses never newly acquired. Temperance gameplay/halo source and assets untouched.
- User approved Arcana-only balance: Chariot100/shot,64damage;500/320 per5. Strength250/s,195damage/s. Star160 activation+50/deployed lane/s(max250), original100 nominal DPS. Tower30/beam,900finale,6300 target/generation cap,1600 activation and180-shot structure retained. Update payload EFIT and native impact/finale/cap together. Vanilla reduction ratio, exact staff chargeMP0,20000/40000 and Hermit unchanged.
- Books use2x2 DDS atlas2048 with original front BC3 blocks preserved through4px mip, generated blank back/inner tiles and corrected integer/negative UV routing. Raw bone weights/partition indices/controller/collision preserved; only4 UV bytes per affected skin-partition vertex change. Front Hebrew title/4lines/Frieren retained.
- SMP0.1.1-test2 consolidates all10 staffs into one18-record ESP, masters Skyrim/main/TOAMStancesAddon only; typed own helper index2->3. Actual active test1 folder and disabled development SMP folder updated, profile bytes unchanged. Old separate disabled patch superseded. Existing two cloth rigs preserved, transition trace added; physics motion still user QA, not STATIC proof.
- Current authoring assets/TOAM/Data; current MO2 remains TULL Ordinary Attack Magic. Existing build/msvc MSVC Hostx64/x64 Ninja only; full mutable Ninja/source/assets/docs/deploy/008 backup and coherent rollback. Original007/158 preserved. No game launch, collector, profile/save edit or public upload. AE guard remains.
- Main ZIP:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - v0.8.9.zip; optional SMP ZIP:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - Staff SMP - 0.1.1-test2.zip; report:docs/CP027_POSTQA_PRESENTATION_BALANCE_REPORT.md; result:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP027_POSTQA_PRESENTATION_BALANCE\036_RESULT_20261026_093018.json; manifest:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP027_POSTQA_PRESENTATION_BALANCE\035_MANIFEST_20261026_093018.json. Final DLL/ZIP/008 SHA values are external in that verified result.

### CP027 prevention and candidate failures
Self-target fire-and-forget metadata and a render-camera axis differ from engine aimed concentration. Keep exact carrier identity, resource owner and geometry origin aligned; never borrow Arcana payment for TOAM tiers. Spatial ray-origin changes must be reported explicitly even if damage/range numbers stay the same.
Changing Tower's design table alone does not change actual projectile EFIT or the native budget nominal; update each real damage owner. Count original independent shot/tick cadence before costs. Displayed MP may regenerate; use the exact payment log/reduction factor.
Negative/integer book UV tiles are valid; one repeated diffuse repeats a title on both covers. Inspect world AND inventory variants. Nifly's default optimizer sorts blocks; disable optimize/sort for scoped UV work and compare raw controller/skin/collision blocks, allowing only actual UV bytes inside NiSkinPartition. Never claim all skin bytes unchanged when UV is stored there.
An enabled base SMP option does not imply its adaptive patch is installed. Audit actual modlist/plugins and typed WEAP/model/VMAD closure. One complete optional ESP removes the missing two-staff route, but does not prove solver movement. Keep unproven normal-staff causes and old CTD causation open until user QA.
Candidate-only failures: inspected actual Chariot payment anchor after a false replacement; imported collections; corrected MGEF disk80/84 versus native padded fields using xEdit; corrected NiShape GetTriangles API; accepted valid negative UVs; accounted for skin-partition UV serialization and disabled default block sorting. No Production file changed in these candidate failures.

<!-- TOAM_CP027_INSTALL_DOC_CLOSE_20261026_093420 -->
## CP027 v0.8.9 — final installation documentation close — 2026-10-26T09:34:20.397008+09:00
- Source/Native/ESP/NIF/DDS/SMP remain the verified CP027093018 bytes; no rebuild. DLL 15E18128804B73E0F4037D3F25C31537D64C8BDCB3D74052BE1ACC116365AA08.
- Final readback found the public English README still contained an obsolete hard Stances master line under historical CP026 text. Remove historical technical prefixes from current EN/KR install READMEs and INSTALL_CURRENT; retain actual only-Skyrim/main addon masters and current0.8.9 instructions. Historical originals remain in verified backups/canonical records.
- Current SMP is one10-staff18-record option, hard masters Skyrim/main/TOAMStancesAddon, no external Stances master. Existing active test1-named folder has0.1.1-test2 bytes, so do not infer installed version from folder label. Old disabled separate patch remains superseded.
- Docs/assets manifest/deployed docs/ZIP/checksum/008 synchronized in one backup-protected transaction; Source/build/Native/profile unchanged. Original007/158 preserved. RUNTIME NOT_RUN; AE not enabled; LOCK NO.
- Latest package SHA256 9B3FC282B30AB15CA8B0414A3BAF7CE2D4A49D88555B1B35C4A753EC00A902A0; final current008 SHA is external in [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP027_POSTQA_PRESENTATION_BALANCE\043_FINAL_RESULT_20261026_093420.json. Parent full build/result remains [PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP027_POSTQA_PRESENTATION_BALANCE\036_RESULT_20261026_093018.json.


<!-- TOAM_RR_CP001_20261026 -->
## TOAM-RR-CP001 — Cross-AI technical research and release lessons

1. TOOM and TOAM are one lineage. Preserve superseded implementation
   history rather than replacing older findings with current versions.

2. CP026 correct ABI and lifecycle separation are distinct concerns:
   item charge settlement must not trigger full weapon-ability
   reinstallation inside ActorMagicCaster update.

3. Resource readback must distinguish actual kDamage modifiers,
   synchronous display values, and God Mode protection. Previous
   failures must not be silently reclassified as fixed.

4. Shared VFX models and real scaled geometry must be audited
   separately. Tower FIELD and SKY required distinct thin-rim paths.
   World pooled artwork selection must not cause random circle scale.

5. MODEL_PROBE, 3D_READY, skin-weight verification, XSD/Papyrus
   compilation and successful game-pixel/FSMP simulation are
   different evidence levels.

6. External SMP references do not grant redistribution rights.
   Slot61/60 helpers, equipment events, solver activation,
   dual-wield fallback, migration and third-person visibility
   require independent runtime testing.

7. The user reported no freeze/CTD in the CP026 partial session.
   This observation is not a proof of permanent fault elimination.
   Reported VFX, targeting, balancing and SMP issues remain open.

8. Initial research indexing: 2997 source entries, 1513 immutable
   snapshots, SHA failures 0, optional missing roots 3.
   This does not prove every historical source was discovered.

9. Hash-matched snapshot preservation is not semantic research.
   A developer must record hypotheses, failed experiments,
   root-cause proof, version, provenance and transferable lessons.

10. Public SOURCE packages must not blindly include internal
    EVIDENCE, logs, local absolute paths, entire SNAPSHOTS,
    private files or third-party binaries.

11. Public AI bootstrap must clearly identify engine/runtime
    applicability, copyright boundaries and unverified claims.

12. Regenerating SOURCE_INDEX after document changes is required.
    The index excludes its own registry subtree to avoid cyclic
    catalog dependencies. Historical snapshots remain immutable.

13. This checkpoint changes documents only; neither build PASS
    nor new runtime acceptance can be inferred from DOCSYNC.


<!-- TOAM_RR_CP002_20261026_SESSION_HANDOFF -->
## RR-CP002 — Handoff, PowerShell and Research Lessons

- CP001 initially false-stopped on a stale hardcoded 008 hash.
  Historical SHA is evidence, not absolute latest-local authority.
  Verify the live file against a coherent current source catalog.
- Interactive PowerShell commands continued after a prior throw.
  Detached try/catch/finally then caused a separate finally error.
  All multi-step work must run as one complete scriptblock.
- CP001 marker validator misclassified three correct documents:
  markers appeared more than once in actual document content.
  Check a dedicated standalone anchor, not arbitrary substring count.
- CP001 closeout verified substantive content12/12, catalog9/9,
  snapshots9/9, new documents3/3, original QA007 preserved.
  The historical failed transaction is not retroactively erased.
- Capture before/after SHA, file bytes, current provenance, complete
  rollback evidence, and independent package verification.
- Source ZIP handoff is a historical snapshot. No source or build
  is promoted to runtime PASS merely by correct packaging.
- Do not use Work mode or other agents without explicit user approval.
- Preserve old technical failures, hypotheses and superseded code
  as history; avoid rewriting earlier root-cause evidence.
- Internal research snapshots and private logs are not automatically
  public-domain assets. Public Research/ needs license/privacy review.
- CP026 v0.8.8 has user partial runtime results and unresolved
  World, Sun/Magician, Judgement, Tower and SMP correction items.
  AE ABI verification is unfinished and AE must remain disabled.

<!-- TOAM_CP027_RESEARCH_AUTHORITY_CLOSE_20261026_120928 -->
## CP027 v0.8.9 — production/research authority reconciliation — 2026-10-26T12:09:28.804754+09:00
- TYPE=DOCUMENTATION_ONLY. Production remains QA1-CP027 TOAM v0.8.9, built/deployed093018 and package-doc close093420. Production parent CP0260.8.8. Later RR-CP001/CP002 prose naming CP026 as current is a superseded pointer, not rollback authority. Preserve those original sections and research policies.
- Read-only audit046 verified all11 source/config files, build/deployed DLL 15E18128804B73E0F4037D3F25C31537D64C8BDCB3D74052BE1ACC116365AA08, main ZIP 9B3FC282B30AB15CA8B0414A3BAF7CE2D4A49D88555B1B35C4A753EC00A902A0 and SMP ZIP E6B1B63EBA25EA55C6EB9B58C357DFE0694B4D4E8DF56EBD9A0B000D1CE9FF0F. RR-CP00213 latest document rows match physical files. No source, asset, build, deployment, profile or save mutation in this close.
- Internal SOURCE_INDEX captured10 changed src files but omitted root CMakeLists.txt. Add its verified current row; refresh existing catalog entries, authority snapshots and resume evidence/package pointers. This bounded refresh does not establish complete historical/optional-root coverage. Immutable snapshots remain intact.
- Pinned installed FSMP3.1.1 commit73900d4f5f40df31d559897c04c6e7ac9ff014d0: BSResourceNiBinaryStream reads VFS paths; SkinnedMeshSystem::valid requires nonempty bones, not collision meshes. Current XML path and no-collider configuration are not proven defects. Reject speculative Data-prefix/collider/wind/mass/equip-lock changes.
- SMP0.1.1-test2 fixes the proven missing two adaptive WEAP routes in the already-enabled all10-staff option. General cloth non-motion, real helper-node presence and solver activation remain unproven. Static ownership/skin/XSD/Papyrus checks do not prove cloth motion or old CTD elimination.
- CP027 World/Sun/aim/Judgement/book/approved Arcana balance changes remain implemented. RUNTIME NOT_RUN_AFTER_CP027; AE NOT_ENABLED; original007/158 preserved; LOCK NO. Next is one user-run consolidated SE1.5.97 QA.
- AI/developer resume report:docs/CP027_RESUME_FSMP_AUTHORITY_REPORT.md; full implementation report:docs/CP027_POSTQA_PRESENTATION_BALANCE_REPORT.md. External doc/index/008 SHA and backup evidence:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP027_POSTQA_PRESENTATION_BALANCE\049_DOC_RECONCILIATION_RESULT_20261026_120928.json. SOURCE_INDEX/snapshots are derived self-excluded catalog outputs; do not embed their hashes in008 and create a dependency cycle.

### CP027 resume validator and dependency lessons
The first046 read-only audit assumed CMake must be indexed and false-stopped with KeyError before production writes. Classify missing catalog coverage and verify actual build input; never change correct source to satisfy a bad validator. The first doc-close preflight also rejected an existing zero-byte stage log. Empty historical logs are valid byte sequences with a normal SHA256; verify length/hash without a nonempty-content assertion. Both false stops had ROLLBACK NOT_REQUIRED and unchanged Production.
A hash-consistent catalog can omit a build input. Resolve newer008 seven-column absolute and three-column relative rows by actual path, last row wins; RESEARCH_REGISTRY relative rows use evidence root. Do not restore old008 for a stale expectation. Pin dependency source to the installed version: dev4.1.1 is not implementation proof for installed3.1.1. Keep proven missing adaptive routes separate from unproven general cloth failure.

<!-- TOAM_CP028_CARRIER_CTD_SMP_20261026_132720 -->
## CP028 / TOAM v0.8.10 — 2026-10-26T13:27:20.002415+09:00
- PARENT Production CP0270.8.9 / SMP0.1.1-test2. New user RUNTIME FAIL:Magician activation CTD plus malformed cloth/no observed SMP motion. Corrected RUNTIME NOT_RUN; AE NOT_ENABLED; LOCK NO.
- Crash SE+00FBA60 reads null EffectSetting.projectileBase at+C8. Input33657 calls MagicItem::GetAVEffect for aimed concentration before caster update; GetAVEffect rejects effects with no projectile. CP027 DCE changed delivery/casting but retained a null projectile from the old self Script carrier. This assistant-introduced metadata regression is confirmed by native call chain and actual ESP, not a third-party-stack blame.
- Minimal repair:existingDCE DATA+72 now references existing empty-model Arcana PROJ A6D in rootEN/en/kr. Each mainESP changes one4-byte field/three differing bytes. SPEL/EFIT, Script/no-hit/no-effect flags, native resource/tier/damage, engine aiming/origins, prior CP027 art/balance and Hermit defense remain. All current Native gameplay source files and hooks are byte-identical. CMake version0.8.10 and packaging validator are the two changed source/config/tool files.
- Persistent release gate validates6 aimed carriers and rejects the actual bad CP027 negative control. A timer suppressed in Update cannot protect earlier input metadata access. Package path closure/BUILD PASS did not catch this semantic prerequisite.
- SMP0.1.2-test3:two private third-person staff NIFs target fixed first cloth bone00 instead of generic weapon root, with equivalent rebased skin/inverse bind. Six moving cloth/ornament geometry/UV/weights and normalized GPU streams verified. Helper/XML/Papyrus/10WEAP/ENCH/capacity20000/40000 and first-person static models unchanged. This is a structural repair candidate; actual visibility and physics are NOT_RUN and general solver failure cause is not uniquely proven.
- Actual current user-installed SMP folder is [LOCAL_PATH_REDACTED]
- Existing build/msvc MSVC Hostx64/x64 Ninja only, full mutable cache/targets backup and coherent rollback. Original007 A-J158 preserved. No game launch, automatic QA, extra hook/frame scan, public upload or dependency DLL replacement.
- Research append-only journal and SOURCE_INDEX/immutable snapshots updated in the same transaction. Derived index/snapshots self-excluded from008 to avoid a cycle. Three optional historical missing scan roots remain incomplete. Source-release public Research edition still requires separate review, not private registry export.
- Report:docs/CP028_CARRIER_CTD_SMP_REPORT.md. Main ZIP:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - v0.8.10.zip; SMP ZIP:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - Staff SMP - 0.1.2-test3.zip; final DLL/ZIP/008/catalog SHA and backup:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP028_CARRIER_CTD_SMP\019_RESULT_20261026_132720.json; manifest:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP028_CARRIER_CTD_SMP\018_MANIFEST_20261026_132720.json. Runtime acceptance remains one consolidated user SE1.5.97 QA.

### CP028 confirmed regression and prevention
Aimed concentration is an engine data contract, not just an aim/camera flag. A no-projectile self Script MGEF cannot be copied to aimed delivery without satisfying input-time GetAVEffect. Preserve the assistant's CP027 defect and failed runtime as history; do not relabel old static/deploy PASS as runtime success. Test a real bad parent artifact as the regression negative control.
Skin rest-shape reconstruction and schema validation did not prove the engine weapon skin-root contract. Match the reference's private first-bone target and prove equivalent binds/hardware weights separately; actual cloth motion remains runtime evidence. Do not add ground collision, mass, wind or waits to conceal unknown activation.
Candidate/audit-only failures:pefile was not installed in the bundled Python; use existing MSVC dumpbin instead of changing dependencies. Reused ESP parser required collections import; corrected before any Production mutation. No rollback was required for those stage-only failures.

<!-- TOAM_CP028_REPORT_NODE_NAME_CLOSE_20261026_133102 -->
## CP028 final report/package close — 2026-10-26T13:31:02.330772+09:00
- Documentation-only correction:final actual Gold NIF audit identifies generic root MillennialStaff. The report incorrectly wrote GrandClassStaff. Correct Pdocs and packaged/deployed TECHNICAL_REPORT; no mesh/node/file rename or Native/gameplay change. The structural repair uses the private TOAM_SMP_Gold_00 anchor, not a hardcoded generic-root string.
- Source/ESP/NIF/SMP/build DLL retain CP028132720 hashes. DLL 00552950DC8088254ED087E360F7CD1A342E1E9E2CA4318803CA4C41DDAD2CF2; SMP ZIP D86308F523C6A2732DDB789B5050A0E911038064F2FDAE3646AD98B321FB0AE3. No rebuild. Update public report provenance/main ZIP/checksum/current008/research catalog in one verified backup transaction. Old report is retained in backup/immutable snapshots; research journal appends the explicit correction.
- Latest final package/doc/index/008 evidence:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP028_CARRIER_CTD_SMP\023_FINAL_RESULT_20261026_133102.json. Main version0.8.10; SMP0.1.2-test3; RUNTIME NOT_RUN; AE NOT_ENABLED; LOCK NO. Original007/158 and profile unchanged.

CP028 report lesson:do not guess a model root label from another model or card name. Read each actual NIF before naming its nodes. Gold generic root is MillennialStaff; private Gold_00 repair and source/binaries are unchanged. Correct factual report strings before final delivery and synchronize package/provenance008/catalog, not just the chat answer.



### 원문 부록: 기존 종합 연구록 전체
원본 SHA256: B4B4CB46F2BA787DB32BD00E48739FD01257B298BB6CB8417B38360A72636757

# TOOM / TOAM — 종합 기술 연구록

문서 성격: 연구 자산 / 누적 기술사 / 다른 AI의 개발 인계용
계보: TULL Ordinary Offensive Magic(TOOM) → TULL Ordinary Attack Magic(TOAM)
프로젝트: [PROJECT_ROOT]
기본 검증 런타임: Skyrim SE 1.5.97

이 문서는 원본 정본의 대체물이 아니다.
실제 코드 변경 전 최신 000/001/006/008 전체 원문 및 소스를 재독한다.
SOURCE_INDEX.csv와 SNAPSHOTS는 증거 보존용이다.

## 1. 연구 목적과 증거 위계

TOOM 초기 연구부터 TOAM 정식 개발까지 모든 확인 가능한
시행착오·기술적 성과·실패·해결책·미해결 과제를 누적한다.

사실을 다음 상태로 명확히 구분한다.

- VERIFIED_SOURCE: 실제 소스/바이트/구현 검사로 확인
- STATIC_PASS: 정적 형식 또는 컴파일 검증
- BUILD_PASS: 실제 빌드 성공
- DEPLOY_PASS: 배포 및 해시 일치
- USER_RUNTIME_PASS: 사용자 게임 실측 정상
- USER_RUNTIME_FAIL: 사용자 게임 실측 실패
- HYPOTHESIS: 원인 또는 해결 가능성에 대한 가설
- NOT_TESTED: 실제 게임 검증 없음
- SUPERSEDED: 후속 설계나 구현으로 대체된 역사적 기술

빌드 성공, NIF 로딩 성공, 로그의 READY와 실제 GPU 표시를
동일한 성공 단계로 취급하지 않는다.

## 2. 전체 개발 계보

### S00 — 개발 환경과 초기 DLL 부트스트랩
Windows, PowerShell, MSVC x64, CMake/Ninja 및 SKSE 플러그인
구축을 통해 Native 개발 기반을 형성했다.
빌드 출력·오류 스트림·PowerShell false-stop과 실제 실패를
구분해야 한다는 운영 교훈을 확립했다.

### S01 — 데이터 및 프로젝트 구조
기존 TULL 기술 자산의 재사용 가능 범위를 감사하고
독립 프로젝트의 소스·ESP·이벤트 권위를 분리했다.
POT와 Magicka Blade는 참고 기술의 출처이지 TOAM 정본은 아니다.

### S02 — 마법 자원 결제
일반 주문 MP 비용, 충전 단계, 우선순위, 결제 스냅샷,
손별 결제 및 MagickaTransaction/ChargeArbiter를 연구했다.
입장 검사와 실제 결제를 분리하되 동일 행동을 중복 결제하지 않는다.
다른 시스템의 실제 결제 delta를 TOAM 충전 단계로 재유입하지 않는다.

### S03 — VFX 기반
TransientVFXManager, VFXInstance, EngineCircle, CircleRenderer,
BeamRenderer, Choreography, 오브젝트 수명·소유권을 발전시켰다.
원본 모델, ARTO, 실제 3D 로딩 및 렌더링 변환의 차이를 구분한다.

### S04 — 전투 판정
Trigger Firewall, 정확한 피해 적용, 표적 형성, BeamCollision,
ActorBounds, WeaponHitSource와 2차 피해 재귀 차단을 정리했다.
논리적 표적 원점·실제 발사 원점·피해 적용 좌표를 구분한다.

### S05 — 마법사
충전 단계별 마법진, 빔·포격, 소모 MP, 조준 및 실제
마법진 배치·연출을 연구하고 반복 조정했다.

### S06 — 심판
표식, 3/6/9 포격, 실제 시전 손 MagicCaster 노드,
마법진 발사점, 표적 선택과 시각적 가이드의 분리 구조를 개발했다.

### S07 — 세계
지상 4/8/12 마법진, 공중 12방향 배치, 돔/빔/폭발,
좌표·반경·스케일·타이밍·잔류 정리를 통합했다.
과거 수치가 최신 좌표 정본을 덮어쓰지 않도록 구분한다.

### S08 — 절제
근접 타격 이벤트, 헤일로, 지연 Proc, 비용 결제,
시체/표적 게이트와 VFX 수명·해제 정책을 개발했다.

### S09/S10 — 통합 QA와 배포
기능별 중간 부팅을 반복하지 않고 통합 One-Boot QA를 사용한다.
출시 자산, 네임스페이스, 언어별 ESP, 의존성 및 원본 출처를 검증한다.

### QA1 CP001~CP018
실게임 문제에 따라 VFX 엔진 처리, ARTO/ACTI,
마법진 좌표, 조준·잔류 수명과 교전 처리의 회귀를 정리했다.
세부 실패·수정 계보는 001 누적 교훈 및 STAGE_EVIDENCE가 권위다.

### CP019 — Arcana Staffs v0.7.4 통합
기존 Arcana Native/ESP/자산과 TOOM을 통합했다.
단일 Native host, 이벤트 순서 및 자원 소유권,
OAR/비행/모션 연결을 분리하여 검증했다.

### CP020~CP021 — 밸런스와 실제 ESP 병합
Arcana 8계열의 비용·피해 및 기존 스탠스 기능을 보존했다.
한 DLL 또는 MO2 폴더 통합과 실제 ESP 레코드 병합을
엄격히 구분한다.
typed FormID 매핑, 참조 재배정, MASTER 및 내부 레코드 검증을
수행했다.

### CP022 — TOOM v0.8.4
정식 배포 구조와 전용 주문서 15종을 정리했다.
이후 실게임에서 텍스처/리소스 경로와 VFX 출력 결함이 발견됐다.

### CP023 — TOOM v0.8.5
NIF 원시 텍스처 경로 및 Native VFX 경로를 교정했다.
심판 지시 가이드를 실제 손의 MagicCaster 위치에 연결했다.
이후 해방 스탠스 지팡이 시전 CTD가 보고됐다.

### CP024 — TOAM v0.8.6
공식 명칭을 TOAM으로 통일했다.
지팡이 SE RelocationID 및 인스턴스 검증,
광대 상승 띠, 주문서 접두어, Arcana Limiter,
외부 모션 출처 표기를 교정했다.
직후 실게임에서 프리징·자원 실패·VFX 결함이 보고됐다.

### CP025 — 문서 인계
DOCUMENTATION_ONLY.
소스·게임 빌드·배포를 변경하지 않은 인계 체크포인트다.

### CP026 — v0.8.7 및 v0.8.8
지팡이 결제 중 전체 무기 능력 갱신 호출을 제거하는 후보,
손별 자원 관측, 세계 돔 통일, 절제 헤일로 표시,
광대 상승 띠 종료, 탑의 FIELD/SKY 전용 얇은 림을 구현했다.
스탠스 애드온 불필요한 외부 MASTER도 typed mapping으로 제거했다.

별도의 Staff SMP 0.1.0-test1 옵션을 개발했다.
SMP는 본체와 분리되며 실제 천 물리 성공이 아직 확정되지 않았다.
v0.8.8 이후 추가 실게임 QA 및 교정 요구가 존재한다.
최종 LOCK 또는 완전한 CTD 해소를 주장하지 않는다.

## 3. 핵심 기술 자산

### Native 및 엔진 ABI
- CommonLibSSE-NG의 함수명과 실제 SE/AE 주소를 별도 검증
- 정확한 함수 시작점, 인자·반환형·레지스터 ABI 검토
- 훅 선점, 원래 호출 순서, 엔진 재진입 및 lifecycle 감시
- 비지원 런타임은 훅 설치 이전에 안전 차단
- AE의 Hermit/Tower precommit 및 오디오 ABI는 미완료

### 마법·지팡이 자원
- 일반 주문은 MP, 지팡이는 해당 장착 인스턴스의 ExtraCharge
- 기존 일반/해방 지팡이 용량 20,000/40,000
- staff 결제는 플레이어 MP를 소비하지 않음
- 결제 검증은 동기 표시값과 실제 modifier를 구분
- God Mode에 따른 AV 보호를 자원 실패로 오판하지 않음
- 결제 과정에서 장착 능력 재설치 같은 고위험 lifecycle 호출 금지

### ESP 및 데이터
- 플러그인 MASTER와 runtime soft dependency의 차이
- 레코드/GRUP/HEDR 계산, typed FormID 및 참조 재배정
- WEAP/ENCH/BOOK/ARMO/ARMA/ARTO/ACTI/MGEF 일관성
- 문자열 전역 치환 금지, namespace/token 경계 검증
- EN/KR 언어별 플러그인, 자산/경로/의존성 확인

### NIF/DDS/VFX
- 직렬화된 shader texture raw path를 원시 바이트로 확인
- 외형 모델 로딩과 GPU 픽셀 표시 성공은 다름
- NiTransform, 좌표계, 모델 스케일과 runtime 스케일 분리
- 생성 요청/3D 준비/first pose/visible/retire/cleanup 분리
- 비동기 요청 취소·quarantine·재출현 금지
- 공유 모델 수정 대신 기능별 전용 ARTO/모델 선호
- 풀 슬롯이 아트워크 선택을 결정하지 않도록 통일

### 비행·모션·OAR
- Arcana FlightStateFix, HKX, OAR 조건 및 프로필
- 상승 중 효과 생성과 키 해제 후 잔존 페이드 분리
- 착륙·메뉴·로드·비행 종료에 맞는 소유 효과 정리
- 모션 출처·외부 저작물·Native 연결의 기여 구분

### SMP 물리
- 외부 FSMP를 사용하며 별도 물리 DLL을 설치하지 않음
- Silver/Gold 독립 리그, XML, NiSkinInstance/weight/partition
- 장착 이벤트 기반 helper ARMO/ARMA 연동
- Silver slot61, Gold slot60
- 동일 모델 양손 장착은 정적 fallback
- 1인칭은 기존 고정 모델
- 기존 슬롯 점유 아이템을 강제로 제거하지 않음
- 리그·Papyrus 컴파일 성공은 물리 동작 또는 CTD 안전의 증거가 아님
- 참고 모드 원본 자산은 허가 없이 재배포하지 않음

## 4. 중요한 실패와 재발 방지 교훈

F01. PowerShell ParserError와 명령 출력 false-stop
- 증상/원인/수정의 분리
- 실행 전 파서, 변수명, 앵커 개수 및 실제 종료 코드 검증
- 콘솔에는 핵심 결과만 출력하고 상세 증거는 파일로 보존

F02. 잘못된 기존 파일 경로 및 샌드박스/로컬 혼동
- [LOCAL_PATH_REDACTED]
- AI가 생성한 파일이 자동으로 사용자 PC에 생긴다고 가정하지 않음

F03. 매크로·PCH·함수 시그니처
- standalone 정적 통과를 production compile로 승격하지 않음
- min/max, EnumSet underlying, 반환형 변경 후 호출자 전수 검사

F04. 검증기 false-stop
- 역사적 timestamp/고정 해시/가상 파일명 추정에 의존 금지
- 실제 파일 내용·정확한 조건·현재 정본을 우선
- 허위 실패에 대응해 수정 범위를 넓히지 않음

F05. 잘못된 문자열 치환
- OOM_ 부분 치환으로 TTOOM_/TTTOOM_ 발생
- 전체 문자열이 아니라 검증된 토큰과 typed reference 기준 수정

F06. NIF 텍스처 경로
- 자동 정규화된 로더의 성공은 원시 경로 무결성을 보증하지 않음
- backslash, shader reference, 실제 게임 GPU 표시 확인

F07. 화면 렌더링 오판
- MODEL_PROBE/3D_READY/FIRST_POSE 성공은 Pixels=UNVERIFIED
- 실제 영상, 여러 각도, 알파·크기·수명 관찰 필수

F08. 심판 시각 원점
- 표적 선택·실제 손 위치·시각적 발사점은 독립적인 권위
- 시각적 발사점 변경이 충돌 판정과 자원 결제에 침투하지 않게 차단

F09. 지팡이 CTD 및 lifecycle 재진입
- CP024에서 잘못된 SE 주소가 확인됨
- SE37803 호출 계약 자체와 호출 위치의 안전성은 별개
- CP026은 결제 중 전체 능력 재설치 경로를 분리
- 프리징의 유일한 원인 확정에는 런타임 스택 또는 재검증 필요

F10. 자원 readback 오판
- 표시 MP가 즉시 변하지 않는다고 실제 결제 실패로 단정 금지
- 실제 modifier/보호상태/결제 소유권을 확인
- 실패 때 무단 반복 결제 또는 강제 MP 설정 금지

F11. 세계 돔 모델과 자산 풀
- 서로 다른 반경 모델이 하나의 풀에서 섞여 시각 편차 발생
- CP026은 동일 모델 사용으로 후보 교정
- 실제 픽셀과 스케일은 게임에서 별도 판정

F12. 탑의 이중 림 두께
- FIELD와 SKY가 다른 모델·스케일 소유자임을 확인
- FIELD만 수정하면 SKY의 벽 같은 두께가 남음
- 독립 ARTO와 전용 모델로 국소 수정

F13. 비행 상승 띠 수명
- 생성 중단과 이미 보이는 이펙트의 제거는 별개
- 키 해제 때 새 생성만 중지, 기존 띠는 잔여 페이드 유지
- 강제 종료/로드에서는 별도 즉시 정리

F14. SMP 정적 성공과 실게임 실패
- weight, XML, PEX 검증은 실물 천의 움직임을 보증하지 않음
- MO2 활성화, helper 착용, 슬롯 충돌, FSMP, 1P/3P를 구분
- 과거 다른 프로젝트의 CTD 원인을 임의로 이식하지 않음

F15. Ninja mutable state 및 롤백
- 빌드 후 옛 DLL만 덮어쓰는 방식 금지
- 원본·후보·문서·008·배포·빌드 출력 상태의 일관성 유지
- 격리 빌드는 사전 승인을 받기 전 수행하지 않음

## 5. 현재 추가 연구 과제

2026-10-26 사용자 보고 시점:
- 태양과 마법사 마법진의 시전 연출·카메라 처리
- 심판 지시 마법진의 위치와 조준성
- 세계 외부 띠와 내부 마법진의 실제 크기 일치
- 탑 다중 빔 피해·실효 HP 변화
- 일반/궁극기 및 지속빔의 비용 대비 효과
- Staff SMP의 실제 활성화 및 물리 미작동 원인
- 프리징·CTD 장시간 회귀 검증
- AE precommit/audio Native ABI 검증

위 항목들은 자동으로 해결 판정을 내리지 않는다.

## 6. 권위 및 재사용 원칙

1. 현재 PC 정본이 과거 ZIP보다 우선한다.
2. 이전 성공·실패 기록은 역사로 영구 보존한다.
3. 검증된 사실과 가설을 별도 기재한다.
4. 변경 전에 실제 000/001/006/008 전체를 다시 읽는다.
5. 관련 소스·자산·유사 실패·현재 해시를 확인한다.
6. 변경은 최소 범위, 원자적, 롤백 가능하게 수행한다.
7. 실제 MSVC/Ninja 빌드와 사용자 One-Boot QA를 구분한다.
8. 007 A~J 원래 158문항을 변경하거나 전수 반복하지 않는다.
9. 기존 gameplay/balance 계약을 임의 변경하지 않는다.
10. 다른 프로젝트에 기술을 재사용할 때 소유권·출처를 유지한다.

## 7. 상세 근거의 위치

evidence\DEV_ASSETS\000_DEVELOPMENT_RULES.md
evidence\DEV_ASSETS\001_DEV_LESSONS.md
evidence\DEV_ASSETS\002_DEVELOPMENT_ROADMAP.md
evidence\DEV_ASSETS\003_REUSE_AUDIT.md
evidence\DEV_ASSETS\004_SPELL_GAMEPLAY_AUTHORITY.md
evidence\DEV_ASSETS\004_TULL_ENGINEERING_REFERENCE.md
evidence\DEV_ASSETS\005_FORMATION_COORDINATE_AUTHORITY.md
evidence\DEV_ASSETS\006_SESSION_WORKFLOW_BOOTSTRAP.md
evidence\DEV_ASSETS\007_ONE_BOOT_QA_CHECKLIST.md
evidence\DEV_ASSETS\008_SOURCE_ASSET_INVENTORY.md

docs\TECHNICAL_REPORT.md 등 실제 검색·등록된 보고서,
evidence\STAGE_EVIDENCE 아래의 모든 검색·등록된 연구 증거,
src 및 tools의 현재 소스는 SOURCE_INDEX.csv에서 추적한다.

이 연구록은 위 자료의 요약·분류층이다.
원문의 개별 사건, 전체 코드, 증거는 SNAPSHOTS와
현재 PC 정본을 함께 읽어 복원한다.

## 8. 갱신 방식

새 기술·실수·교훈은 JOURNAL의 신규 문서로 작성한다.
과거 항목을 삭제하지 않는다.
각 기록에 날짜, 버전, CHECKPOINT, STATUS, 증거 경로·해시,
문제, 확인된 원인, 수정, 회귀 위험, 결과를 넣는다.

SOURCE_INDEX.csv는 등록 스크립트로 최신화한다.
JOURNAL을 정본으로 간주하거나 사용자 승인 없이 Production을
변경하지 않는다.

<!-- TOAM_RR_CP001_20261026 -->
## RR-CP001 — Latest cross-AI research checkpoint

Scope: TOOM initial studies through TOAM CP026 v0.8.8 and the
reported post-v0.8.8 user QA.

The initial registry captured 2997 references and 1513 snapshots,
with 0 hash failures and 3 missing optional search roots.
It is a traceable partial inventory, not evidence that all source
history or technical root causes were exhaustively researched.

CP026 parent correction:
- Staff charge settlement and equip ability lifecycle separation.
- Adaptive casting cost readback and protected AV handling.
- World dome artwork consistency and dedicated halo presentation.
- Ascent emission stop versus already-visible fade lifetime.
- Independent Tower FIELD/SKY thickness owners.
- Optional Silver/Gold FSMP rigs and equipment helper bridge.

CP026 user runtime:
- User reported no freeze/CTD during the observed session.
- Additional corrections requested for World circle scale,
  Sun/Magician emergence and aiming, Judgement guide placement,
  Tower practical damage, overall MP/damage balance and SMP motion.
- These remain open pending actual newer delivery and retest.
- FSMP static validation is not successful in-game cloth physics.

Future reusable knowledge:
Record verified behavior, exact failure condition, rejected attempts,
corrected source/ABI, runtime dependency, regression impact,
ownership and evidence at every new technical milestone.

Publication policy:
The public research edition accompanies the eventual source ZIP.
The internal evidence registry stays separate.


<!-- TOAM_RR_CP002_20261026_SESSION_HANDOFF -->
## RR-CP002 — Cross-AI technical history handoff

RR-CP001 local closeout confirmed all substantive documents and
catalog/snapshots despite repeated marker warnings.
Current user-reported CP026 v0.8.8 gameplay has partial QA and
further requested corrections; SMP remains experimental.

Preserve entire TOOM/TOAM technical lineage, including engine ABI,
ESP typed FormID mapping, NIF raw paths, VFX lifetime, charge and
stance lifecycle, OAR/flight, SMP and failed candidate history.
Do not promote a code or compile result to successful game rendering.

The private package carries latest document/source evidence and
a SHA256 manifest. It is NOT a reviewed public source release.
The public Research/ edition remains separately curated.

<!-- TOAM_CP027_RESEARCH_AUTHORITY_CLOSE_20261026_120928 -->
## CP027 v0.8.9 — production/research authority reconciliation — 2026-10-26T12:09:28.804754+09:00
- TYPE=DOCUMENTATION_ONLY. Production remains QA1-CP027 TOAM v0.8.9, built/deployed093018 and package-doc close093420. Production parent CP0260.8.8. Later RR-CP001/CP002 prose naming CP026 as current is a superseded pointer, not rollback authority. Preserve those original sections and research policies.
- Read-only audit046 verified all11 source/config files, build/deployed DLL 15E18128804B73E0F4037D3F25C31537D64C8BDCB3D74052BE1ACC116365AA08, main ZIP 9B3FC282B30AB15CA8B0414A3BAF7CE2D4A49D88555B1B35C4A753EC00A902A0 and SMP ZIP E6B1B63EBA25EA55C6EB9B58C357DFE0694B4D4E8DF56EBD9A0B000D1CE9FF0F. RR-CP00213 latest document rows match physical files. No source, asset, build, deployment, profile or save mutation in this close.
- Internal SOURCE_INDEX captured10 changed src files but omitted root CMakeLists.txt. Add its verified current row; refresh existing catalog entries, authority snapshots and resume evidence/package pointers. This bounded refresh does not establish complete historical/optional-root coverage. Immutable snapshots remain intact.
- Pinned installed FSMP3.1.1 commit73900d4f5f40df31d559897c04c6e7ac9ff014d0: BSResourceNiBinaryStream reads VFS paths; SkinnedMeshSystem::valid requires nonempty bones, not collision meshes. Current XML path and no-collider configuration are not proven defects. Reject speculative Data-prefix/collider/wind/mass/equip-lock changes.
- SMP0.1.1-test2 fixes the proven missing two adaptive WEAP routes in the already-enabled all10-staff option. General cloth non-motion, real helper-node presence and solver activation remain unproven. Static ownership/skin/XSD/Papyrus checks do not prove cloth motion or old CTD elimination.
- CP027 World/Sun/aim/Judgement/book/approved Arcana balance changes remain implemented. RUNTIME NOT_RUN_AFTER_CP027; AE NOT_ENABLED; original007/158 preserved; LOCK NO. Next is one user-run consolidated SE1.5.97 QA.
- AI/developer resume report:docs/CP027_RESUME_FSMP_AUTHORITY_REPORT.md; full implementation report:docs/CP027_POSTQA_PRESENTATION_BALANCE_REPORT.md. External doc/index/008 SHA and backup evidence:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP027_POSTQA_PRESENTATION_BALANCE\049_DOC_RECONCILIATION_RESULT_20261026_120928.json. SOURCE_INDEX/snapshots are derived self-excluded catalog outputs; do not embed their hashes in008 and create a dependency cycle.

<!-- TOAM_CP028_CARRIER_CTD_SMP_20261026_132720 -->
## CP028 / TOAM v0.8.10 — 2026-10-26T13:27:20.002415+09:00
- PARENT Production CP0270.8.9 / SMP0.1.1-test2. New user RUNTIME FAIL:Magician activation CTD plus malformed cloth/no observed SMP motion. Corrected RUNTIME NOT_RUN; AE NOT_ENABLED; LOCK NO.
- Crash SE+00FBA60 reads null EffectSetting.projectileBase at+C8. Input33657 calls MagicItem::GetAVEffect for aimed concentration before caster update; GetAVEffect rejects effects with no projectile. CP027 DCE changed delivery/casting but retained a null projectile from the old self Script carrier. This assistant-introduced metadata regression is confirmed by native call chain and actual ESP, not a third-party-stack blame.
- Minimal repair:existingDCE DATA+72 now references existing empty-model Arcana PROJ A6D in rootEN/en/kr. Each mainESP changes one4-byte field/three differing bytes. SPEL/EFIT, Script/no-hit/no-effect flags, native resource/tier/damage, engine aiming/origins, prior CP027 art/balance and Hermit defense remain. All current Native gameplay source files and hooks are byte-identical. CMake version0.8.10 and packaging validator are the two changed source/config/tool files.
- Persistent release gate validates6 aimed carriers and rejects the actual bad CP027 negative control. A timer suppressed in Update cannot protect earlier input metadata access. Package path closure/BUILD PASS did not catch this semantic prerequisite.
- SMP0.1.2-test3:two private third-person staff NIFs target fixed first cloth bone00 instead of generic weapon root, with equivalent rebased skin/inverse bind. Six moving cloth/ornament geometry/UV/weights and normalized GPU streams verified. Helper/XML/Papyrus/10WEAP/ENCH/capacity20000/40000 and first-person static models unchanged. This is a structural repair candidate; actual visibility and physics are NOT_RUN and general solver failure cause is not uniquely proven.
- Actual current user-installed SMP folder is [LOCAL_PATH_REDACTED]
- Existing build/msvc MSVC Hostx64/x64 Ninja only, full mutable cache/targets backup and coherent rollback. Original007 A-J158 preserved. No game launch, automatic QA, extra hook/frame scan, public upload or dependency DLL replacement.
- Research append-only journal and SOURCE_INDEX/immutable snapshots updated in the same transaction. Derived index/snapshots self-excluded from008 to avoid a cycle. Three optional historical missing scan roots remain incomplete. Source-release public Research edition still requires separate review, not private registry export.
- Report:docs/CP028_CARRIER_CTD_SMP_REPORT.md. Main ZIP:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - v0.8.10.zip; SMP ZIP:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - Staff SMP - 0.1.2-test3.zip; final DLL/ZIP/008/catalog SHA and backup:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP028_CARRIER_CTD_SMP\019_RESULT_20261026_132720.json; manifest:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP028_CARRIER_CTD_SMP\018_MANIFEST_20261026_132720.json. Runtime acceptance remains one consolidated user SE1.5.97 QA.



### 원문 부록: CP024_NATIVE_AND_MIGRATION_REPORT
원본 SHA256: 85319017FDA213B0A4707A4C94D887F51C9E4BB4B1EC8E41927B5F6717E50907

# TOAM v0.8.6 — CP024 Native crash and namespace migration
Date:2026-10-26T02:06:09.596890+09:00
CURRENT=QA1-CP024-TOAM; PARENT=CP023 v0.8.5.
This report is an AI-readable engineering handoff. Source/static, build, deploy and runtime acceptance are independent.

## Evidence and primary cause
User reports immediate CTD when casting with an Unbound stance staff, while textures, Judgement guide/artillery and Fool circle have now been visually confirmed.
Input crash:000_USER_CRASH.log SHA6FAD162CCDEADE137F4EBFC0F983C375EAEA3E90A9FEF01854BBEE9BC61C860C.
Exception: SkyrimSE1.5.97 RVA679096, mov rcx,[rbx], invalid FFFFFFFFFFFFFFFF.
Stack: TOOM Balance.inl:41 PayResource -> HandUpdate.inl:21; High Priestess Staff(Unbound), ExtraCharge and BEGIN adaptive=true mode3.
The pinned CommonLib Actor::RefreshEquippedActorValueCharge uses RELOCATION_ID(38752,37803). This reverses the SE/AE IDs of the actual weapon-charge/ability update.
Actual installed SE Address Library:38752=>RVA679000;37803=>RVA6313D0.
Read-only disassembly of an existing matching unpacked executable proves SE38752 accepts a container/index-range operation. Passing Actor/weapon/ExtraDataList/left interprets the weapon address as a range index and reaches the precise crash instruction+96.
SE37803 accepts Actor inRCX, form inRDX, extra inR8 and left inR9B, reads ExtraCharge and updates the corresponding charge actor value/HUD.
Packed installed and private-copy SHA match693E5A51EA2680119A68620BCF5080E81745549872B5D06BBD3F51131B67ABAB; unpacked SHA DE92095A18513FCAFFE5A86FD72879D3350C61CDCDB8500B7D31DF2BAE9579CD.
No executable was launched/replaced. 002_NATIVE_ABI.json and the two function disassemblies are the native proof. Packed encrypted-code disassembly was rejected as invalid evidence.

## Correction and resource ownership
Balance.inl uses a local typed RefreshStaffCharge adapter at RelocationID(37803,38752), avoiding the broken pinned wrapper without editing the third-party SDK.
StaffResource holds weapon, exact worn instance and left/right identity from one verified equipped-entry snapshot. Admission and actual payment use the same resolver, with casting-source/staff/enchantment checks. The old second GetEquippedObject dereference is removed.
No billing thresholds or balances changed. Staffs still use ExtraCharge only, retain20000/40000 capacity and vanilla reduction, and require recharge after zero. Ordinary Arcana spells and TOAM paid-tier accounting remain independent. Refresh is skipped at zero-cost consumption exactly as before.
SE's corrected target is native-proven; AE38752 is paired by pinned UpdateWeaponAbility and upstream declarations, not a runtime test:
https://github.com/CharmedBaryon/CommonLibSSE-NG/blob/main/src/RE/A/Actor.cpp
https://ng.commonlib.dev/_offsets_8h.html
Do not remove the inventory change flag or charge HUD/ability refresh to hide a crash.

## Ascent ring presentation
Four new main-plugin ARTOs localDC1..DC4 use TOAM/Arcana/Flight/Arcana_Fool_AscentBand.nif, a private copy of the existing engine-wrapped Sun outer inscription band. It has the same outer/band/thin rim geometry family as Temperance's halo; no full central circle face is added.
Separate ascentBands[4] is updated from the existing FlightFrame. While the flight Jump/Space action is held and not descending, request at most one band every.26s. Band world position snapshots feet-4u, expands from.15 to.31 scale, drifts down22u and fades over.68s after real 3D readiness, giving a dispersing trail. This is expansion/fade, not particle fragments.
Pending requests retain the existing1.2s/120-search bound and2.2s quarantine. Acquired/finished forms may be reused. Menu, flight-off, cell/load/death/reset clear this separate pool. PurgeArt includes the four own forms. No new frame hook/actor scan, gameplay resource cost or flight velocity change.
Original takeoff circle and Temperance halo geometry/.095/spin/placement and Proc timing are preserved.

## Namespace, books and ring
User supersedes TOOM/Ordinary Offensive Magic naming with TULL Ordinary Attack Magic and TOAM.
Main:TULL Ordinary Attack Magic.esp; optional:TULL Ordinary Attack Magic - Stances Addon.esp.
Host TOAM.dll; settings TOAM.ini; log OrdinaryAttackMagic.log.
Active assets:[PROJECT_ROOT]\assets\TOAM\Data; deploy:[LOCAL_PATH_REDACTED]
Existing [PROJECT_ROOT] and build/msvc are retained to preserve the verified incremental cache. Historical source evidence, rollback installations and assets/TOOM remain unchanged; they are not current authoring authority. No profile/checkbox/save edit.
Exact token mapping is applied once to paths/EDIDs, typed ESP fields, Native literals, OAR conditions and fixed-length raw NIF namespace strings. Authored AS_CirclePose/AS_BeamAxis/CC_* and OOM_ExplosionFlashCloud_Root are unchanged. Never reintroduce CP022 substring OOM_ replacement or forward-slash raw shader paths.
All existing FormIDs and gameplay numeric subrecords are unchanged. Main1392 records inclTES4 after+4 ART objects; addon15. Root EN plus Language/en/kr contain both new plugin names and matching addon masters.
15 book display names use Ordinary Attack Magic: Arcana - / 일반공격마법: 아르카나 -; Fool only uses General Education Magic: / 일반교양마법:. Existing art and Hebrew cover remain.
Ring local813: English Arcana Limiter is the user's2026-10-26 correction; Korean 술식 출력 제한 반지 is read from the original CP019 preintegration ESP. No new Arcana - prefix. Earlier candidate English translation Spell Output Limiter Ring was superseded before Production.

## Air combat attribution
Docs/TOAM/CREDITS_AIR_COMBAT.md and docs/CREDITS_AIR_COMBAT.md distinguish TULL design/Codex implementation assistance from imported345 unmodified flight and452 derived firearm HKX. All797 HKX and45 DDS retain parent bytes.
Original author notices/provenance remain verbatim. PATH_MIGRATION.json links prior paths to current names; current ASSET_PROVENANCE and FORM_MAP are regenerated from actual files. Flight Framework is a technical reference, not a bundled runtime/code dependency. Imported animation rights and unresolved per-file authorship are not assigned to the user or AI.
Read the preserved original notices for HydroBirous, Derwan, Redfall6, gp160, Khenta, RavenKZP/QTRModding, a skrub, Ershin and SkyrimThiago. The collected Call to Arms notice has no verified separate author identity. Do not invent one or blanket redistribution permission.

## AE boundary — unfinished
CMake already compiles SE/AE. Engine ArtObject adapters now select(22289,22769), and staff refresh selects(37803,38752).
AE is still NOT ENABLED. Native proof is missing for the Hermit/Tower precommit counterpart toSE37523, its entry/attacker ABI, and the positioned-file audio contract/scalar position call. Installed Address Library contains SE databases only. Vtable/struct source declarations are necessary but not native runtime proof.
The host rejects AE before hooks/listeners rather than losing Hermit defense or the Tower cap. Completing AE remains a required follow-up; do not declare this optional request fulfilled. See docs/RUNTIME_COMPATIBILITY.md.

## Gates, failures and recovery
Static: repeated-subrecord ESP comparison, raw dependency closure({'nifs': 42, 'rawTextureReferences': 386, 'espModels': 3849, 'espSounds': 42, 'nativeLiterals': 22}), independent42-NIF reload,5 Native syntax units,797 HKX/45 DDS byte checks. No rendering PASS is inferred.
First production transaction failed only its legacy timestamp-string validator after build/package checks; full exact-baseline rollback passed. The retry preserves original historical row formats and verifies untouched physical times against PRE, while changed rows retain strict formatted metadata checks.
Candidate failures before Production: R1 assumed Korean cp949 from previous console output; actual active names are UTF8. Strict decode stopped the candidate. R2 StaffInstance return-type change omitted its admission caller; MSVC caught C3535/C2440, corrected both callers. R1 validator repeatedly enumerated a filesystem tree per animation and was stopped as inefficient; R2 indexes once. Failure logs are retained.
Full mutable build/msvc snapshot and exact source/asset/doc/deploy baseline are backed up at[PROJECT_ROOT]\_transactions\QA1_CP024_TOAM\20261026_020609; changed files and008 share rollback. No isolated plugin cache, arbitrary DLL overwrite, intermediate game boot, automated QA collector, public upload or commit/push.
Final build/deploy/hash results:[PROJECT_ROOT]\evidence\STAGE_EVIDENCE\QA1_CP024_STAFF_CTD_ASCENT_RING\020_RESULT_20261026_020609.json. Exact ZIP:[PROJECT_ROOT]\releases\TULL Ordinary Attack Magic - v0.8.6.zip. The report's existence alone does not prove a successful transaction.

## Next human QA: three critical questions
1. Can both Unbound staffs and normal staffs cast, spend the correct hand's charge and recharge without CTD or player MP loss?
2. While airborne, do held/repeated Space actions show the halo-band trail under the feet, then clear on release/menu/landing/load without residual effects?
3. Are EN/KR book prefixes, Fool general-education label, Arcana Limiter names and renamed resource paths correct while original VFX/cost/damage/defense stay intact?
Corrected RUNTIME=NOT_RUN; prior user RUNTIME=PARTIAL_PASS_WITH_STAFF_CTD. LOCK=NO.



### 원문 부록: CP026_POSTQA_STANCE_VFX_REPORT
원본 SHA256: BA0C7ECE01E69A14D8AA5FE84808A3BB78367F86F67108014D8974C8F834DD78

# TOAM v0.8.7 / CP026 — Post-QA stance and VFX correction
Parent production: CP024 v0.8.6; parent documentation: CP025.
This is an AI-readable engineering handoff. Do not promote static, build or deployment evidence to visible/runtime acceptance.

## Evidence and certainty
The user reported World presentation defects, malfunctioning adaptive spells, a freeze during adaptive staff stance switching, abrupt ascent-band removal, old limiter description and an invisible Temperance halo.
User log SHA256 B318F3B826643A801E3B8C7143D45B72A5CFBEADB6DE55D60ECB084BAB0AA321.
Video SHA256 0A440F38CBE94401007AC21A30E99653A66368970668A52896A552500513A078.
The log shows normal spells spending MP successfully before adaptive casts. Later adaptive spells stop on ARCANA_RESOURCE_READBACK_FAIL with unchanged before/after values. This is a confirmed rejection, not proof that the addon MASTER caused it.
Staffs execute several modes and two complete Sun releases. The last log is adaptive Sun BEGIN at03:12:04.845, without a first-payment result. There is no captured frozen-process thread stack. The exact freeze instruction is UNCONFIRMED.
Halo 3D_READY appears while enabled before the supplied video; the log then toggles it off at03:08:14.013. Thus this clip cannot independently prove the earlier halo's invisible-on interval. The user's observed failure is retained.

## Resource and casting ownership
Before: each staff payment changed ExtraCharge then called the full native equip/weapon-ability refresh from inside ActorMagicCaster Update.
CP024 repaired that call's ABI, but a correct ABI is not a correct lifecycle boundary.
Local SE disassembly proves SE37803/RVA6313D0 synchronizes raw item-charge AV, then calls RVA632270 and weapon-ability processing atRVA635290. Calling that during billing can reenter casting/effect handling. This is a concrete unsafe coupling; its responsibility for this particular freeze remains a hypothesis until retest.
After: keep exact StaffResource weapon/instance/hand admission and ExtraCharge debit, inventory change flag and vanilla reduction. Synchronize only that hand's raw item-charge AV through its pinned ActorValueOwner SetActorValue virtual. RVA556780 proves left=82/right=64. No enchantment swap, ability reinstall, cooldown or player-Magicka debit is added.
The native ActorMagicCaster Update itself debits charge through the actor-value owner; equip/unequip remains engine-owned.
Staff capacities remain20000/40000. Ordinary spells retain their own hand ledger, rates and base values.

Before: a displayed MP total that did not change immediately was treated as payment failure and interrupted the adaptive spell.
After: require either the synchronous total or actual kDamage modifier to prove the exact debit; never retry/force-set Magicka. If both reject it, casting still fails closed with both measurements logged. Explicit native God Mode is respected rather than misreported as a failed charge. Native player clamp atRVA6B4670 tests the actual God-Mode byte atRVA2F3AFB6 and blocks protected AV reductions.
God Mode was NOT recorded by the parent log, so its presence in this user's failed casts is UNCONFIRMED. A new ARCANA_RESOURCE_GOD_MODE transition distinguishes it. Zero displayed difference is not sufficient evidence to declare it active.
No paid Arcana resource delta feeds TOAM charge tiers. TOAM ChargeSession/Profile/Arbiter/MagickaTransaction are unchanged. Hermit defense and native billing remain unchanged. Resistance, original intervals, damage and Tower cap are unchanged.
BEGIN records exact spell/staff/stance identity. One enter/return log per cast bounds the native engine-update stage without per-frame logging or new hooks.

## Addon MASTER and typed relocation
The15-record addon has no typed reference to its old master-index2 Stances plugin. Its actual dependency is Native plugin-qualified lookup of the installed three perks.
Remove that unused hard MASTER; preserve Skyrim.esm and the TOAM main master. Remap OWN record index3->2 and all typed references, including BOOK taught spell, EFID, WEAP EITM and recipes. Local IDs and actual Native lookup remain800/801/810/811.
The exact closed schema comes from the CP021 xEdit audit. Validate each field against the expected relocation, not a global four-byte replacement. HEDR counts include GRUPs; do not use a guessed record-only count.
This reduces unnecessary hard dependency; it is NOT proof the old MASTER froze the engine. The installed Stances mod and its own dependencies are still needed for real stance changes. Without it, Native uses neutral mode.
Stance is snapshotted per cast, and affects the next cast. No live enchantment/spell record mutation or free mid-cast switch.

## World presentation
The32 TOAM Dome ARTOs used an i%8 mix of Emperor/Hermit/Chariot/Sun/Strength/Justice/Star/Tower artwork. Actual NIF measurements show differing outer radii (Emperor200, Hermit232), despite one common scale. Resource pool allocation therefore selected the appearance and size.
Use one consistent Hermit model for all32 dome slots. Geometry, matrices, stage4/8/12, .7/1.4/2.1 timing, .12 growth, air/ground coordinates, outward64 beam and frozen damage endpoint remain unchanged.
TOAM FlashCloud assets, 100 fixed damage,85.333333 radius and independent overlap remain unchanged. No Hermit defense is added to World.
This corrects a proven data mismatch. Full visual acceptance still requires actual pixels, including camera angles.

## Temperance display-only change
ACTI spawn/3D readiness was insufficient evidence of a visible correctly posed halo. Replace ONLY the persistent halo's display owner with one dedicated EngineCircle slot/ARTO localDC5 and a private engine wrapper around the same original outer ring, band and thin rim.
The wrapper preserves the three shapes/geometry/UV/textures and colors, adds AS_CirclePose and the actual Arcana OwnEmit flag. Raw texture paths retain original backslashes; no DDS image bytes change.
Engine 3D-first world-to-parent-local pose, finite request/quarantine, renewal and explicit release replace the reference Euler/update path. Preserve scale.095, behind26, up5, head/bounds fallback, vertical yaw and .12rad/s slow spin. The glyph width remains38 world units.
Halo gets a separate slot193; World and proc slots are not borrowed. Pending cancellation cannot reappear; toggle off, cell/load/death/reset dispose owned effects. Existing .15/.27 Proc, corpse gates, damage, MP and paid-pending policy are byte-preserved outside the display block.
The exact old GPU failure is not uniquely proven; this selective engine-display repair is a candidate until user QA.

## Ascent and labels
Before: !ascending called FlightClearAscent, finishing all slots on the release frame.
After: release stops emission. Visible bands keep world snapshots and finish the remaining .68-second lifetime/fade. Pending unseen requests are cancelled/quarantined and finished if returned late. Flight off/menu/cell/load/death/reset still clear immediately. No extra movement/MP/damage changes.
Limiter MGEF local80F DNAM was missed by the previous namespace migration. Update only its description in EN/KR/main root copies. Names remain Arcana Limiter / 술식 출력 제한 반지.

## Tower shallow thickness correction
The user's wall-like Tower perimeter is the FIELD annulus, not the Tower card's ordinary circle. Rain.inl poses CircleForm(kind2) at rainRadius/100 = 850/100 = 8.5. Main ARTO A50..A57 resolve to Arcana_Field_Band.nif.
CP021_ThinRim had 3 asset units of axial depth, which became 25.5 world units after the actual runtime scale. Compress only its axial coordinates about the original center (-.01) by .02: depth .06 asset units / .51 world units. Preserve all planar glyph/boundary vertices, rim X/Z, triangles, UVs, normals, colors, tangents and shader values. Independent native reload comparison passed.
No fence geometry is added. Field height35, radius850, timing, costs, hits and Tower cap630 are unchanged. Actual screen acceptance remains manual.

## Regression gates and useful lessons
Correct ABI does not authorize repeated equip/ability updates from a resource callback. Keep resource settlement, item display and equipped ability lifecycle distinct.
Master deletion needs complete typed remapping even when the removed plugin has no external references.
Pool slot identity must not choose unrelated artwork/scales.
A reference with 3D does not prove correct VFX pixels. Own AS_CirclePose after actual3D and retain the original successful gameplay owner.
Emission stop and retirement are distinct. Cancel unseen pending requests without deleting already-visible tails.
Candidate-only R1 failed on EnumSet-to-int diagnostic casts; R2 on the Windows min macro in Temperance. R3 uses the actual underlying() API and the existing ternary style. Production was not modified by these syntax failures.
No arbitrary runtime cost multiplier, new attack form, independent DLL/listener/frame hook, per-frame actor scan or automatic QA collector is introduced.

## Three final manual QA questions
1. Do both adaptive spells and staffs select all four stances across repeated casts without freeze, unwanted MP, mixed hands or charge loss/recharge faults?
2. Do World4/8/12 and Temperance halo render coherently from multiple camera angles, while existing Proc and explosion behavior remain intact?
3. On Space release do existing bands finish fading without a new late band, and are EN/KR limiter descriptions and menu/cell/load cleanup correct?
RUNTIME=NOT_RUN_AFTER_CORRECTION. Parent runtime=USER_PARTIAL_FAIL_WITH_FREEZE. AE=NOT_ENABLED. LOCK=NO.



### 원문 부록: CP026_STAFF_SMP_RETRY_REPORT
원본 SHA256: B6F49CED3FF3B568074CBAAEA2D359C7AD163B73FBC2B63D5842F1674220B22E

# TOAM / CP026 — Staff SMP retry engineering report
Status: OPTIONAL TEST CANDIDATE. STATIC/PAPYRUS compile verified; RUNTIME NOT_RUN; prior SMP CTD cause UNCONFIRMED; LOCK NO.
Parent Native/main data: TOAM v0.8.7 CP026. Main DLL, balance, existing source and main/stanced ESPs are not modified by this addon.

## Background and evidence
The user requested another attempt at moving the cloth on the two staff models and provided Riilungolz HDT-SMP Staff-97159-1-0-1690732205.7z as a reference.
Archive SHA256: 478870B28008229E737F88E38B050EBEB7D0BCE77E7A109861C41103E18E505C.
Reference page: https://www.nexusmods.com/skyrimspecialedition/mods/97159?tab=description
TheKingofsAss / NanaBeats is credited as the reference author/uploader. Its source shows a skinned weapon and a separate slot61 armor helper linked to a physics XML, equipped/removed by weapon events. These are observations of the supplied files, not our own runtime verification.
No reference NIF, DDS, XML, PSC, PEX, ESP record or binary is redistributed in TOAM. The reference mod's permission terms are retained as a boundary; it supplies technical understanding only.
FSMP installed log: hdtsmp64 v3-1-1-0 (avx2). Its DLL is an external dependency, not bundled/replaced.
Primary FSMP source: https://github.com/DaymareOn/hdtSMP64/tree/v3.1.1
Original SMP technical tutorial: https://www.nexusmods.com/skyrimspecialedition/mods/76813
No downloadable reference script or mesh was copied into this implementation.

## Previous failure: what is known
[LOCAL_PATH_REDACTED]
FernZoltraak's separate NiRefObject CTD has a proven invalid pointer-return cause. That crash must NOT be treated as proof of an SMP root cause.
The old FernStaff package and installation are unchanged. We do not relabel its failed runtime as fixed.

## Current implementation and ownership
| Component | Owner / contract |
| --- | --- |
| Casting, MP, item charge, stance snapshot, damage | Existing TOAM v0.8.7 Native; unchanged |
| WEAP overrides | Same 8 main + 2 stance staff FormIDs, ENCH/EITM/EAMT/WNAM and gameplay payload preserved |
| Model identity | Two physical models only, Silver / Gold; no per-card first-person STATs |
| Moving parts | Each hanging cloth + eyelet/bead + pendant/tassel, 3 skinned shapes per model |
| Rig | 9 skin bones + unweighted root; 2 explicit kinematic bodies, 8 dynamic bodies, 8 constraints |
| Solver | Existing external FSMP; no private physics DLL, Native vtable hook or frame scan |
| Activation | New TOAM_StaffSMP_Bridge Papyrus equipment event; one delayed .20s reconciliation |
| Silver helper | Slot61, ARMO local802 / ARMA804, private keyword800 |
| Gold helper | Slot60, ARMO local803 / ARMA805, private keyword801 |
| Session test switch | TOAM_SMP_Enabled GLOB local806, default1 only in optional ESP |
| First person / unequipped | Original two first-person STATs/models unchanged; no active solver on dropped weapon |
The meshes derive from the existing TOAM Arcana_Staff_Silver/Gold models (FirstClassStaff/MillennialStaff lineage), with geometry/UVs/normals/colors/tangents retained. Original texture assets remain provided by TOAM.
Each family uses a distinct TOAM_SMP_Silver_/Gold_ bone namespace and its own XML/helper.
Only the hanging ribbon and end ornaments are skinned. Shaft, grip, collars, main bindings, gem head and existing model collision are retained.
XML has conservative angular limits, zero linear separation, damping and explicit mass0 anchors. It contains no body/ground collision shape or player-skeleton body binding.

## Why this retry differs
1. Preserve the two current production models and their actual positions; generate chains from the actual ribbon geometry rather than reusing the old failed package.
2. Verify normalized per-vertex weights, actual reloaded NiSkinInstance/partition, and independent bind reconstruction. Maximum measured bind errors: Silver .0174303u; Gold .0132179u (packed weights).
3. Validate both new XML files against the currently installed FSMP XSD, with explicit mass0 root and anchor.
4. No force-removal lock is applied. A foreign item in the helper slot is preserved; that staff falls back to its static bind pose.
5. A keyword identifies the whole Silver/Gold card family, so switching between cards in a family does not let an old per-card script delete a still-needed helper.
6. Same-model dual wield intentionally uses static fallback, avoiding duplicate private bone names competing for one solver. Different-model dual wield has separate helpers/slots; its live behavior remains untested.
7. No while/polling loop, movement, jump, actor-value or charge operation exists in the script. Activation is outside ActorMagicCaster billing.
8. A global permits physics off/on comparison in one user session; no automatic game boot/collector is introduced.
These are prevention mechanisms, not proof of the old CTD's unique cause or of new runtime stability.

## Records, resources and masters
Base: TULL Ordinary Attack Magic - Staff SMP.esp
Masters: Skyrim.esm / TULL Ordinary Attack Magic.esp.
16 records including TES4: 8 exact staff overrides + 2 KYWD + 2 ARMO + 2 ARMA + 1 GLOB.
Stance compatibility: TULL Ordinary Attack Magic - Staff SMP - Stances Patch.esp
Masters: Skyrim.esm / TOAM main / TOAM Stances Addon / TOAM Staff SMP.
3 records including TES4: only the two existing adaptive staff overrides.
Neither addon takes the external Stances plugin as a hard master. Original TOAM Stances Addon still dynamically detects it for actual stance changes.
Normal capacity20000 / Unbound40000, player MP0 and all original Native rates/cost reductions remain the parent contract. Typed checks compare every unchanged WEAP field and keyword prefix; no guessed spell FormID or ENCH remapping.
Exact language/plugin/FormID/helper/keyword/capacity mapping is in FORM_MAP.json. Root ESPs are English; Language/en and Language/kr hold matching variants.

## Install and one-boot QA
Install/enable the optional Staff SMP folder/ESP only if testing this experimental feature. For the two Unbound staffs also install/enable the separate Stances Patch after its listed masters.
The delivery transaction creates both option folders and archives; it does not change MO2 profile checkboxes, plugin activation or saves. Main v0.8.7 works without them.
At the start of a combined session, console: set TOAM_SMP_Enabled to 0. Unequip both staffs and allow .20s; then test the Native/stance correction.
For physics: set TOAM_SMP_Enabled to 1, then unequip/re-equip a single staff. The switch is applied on equipment events, not polled. No file/script edit is required.
Use a fresh staff instance when checking newly added VMAD; old save/script attachment migration is unverified and outside the user's compatibility requirement.
Check single Silver, single Gold, ordinary/Unbound casts, each hand, different-model dual wield, same-model static fallback, occupied helper slots, rapid equip/drop/transfer, draw/sheath, first person, flight, ascent/jump, stance switching, cell/load/death and clean unequip.
Stop using the option if a CTD/jump regression appears; the main production correction remains separate. No runtime PASS is granted here.

## Candidate tool failures and lessons
Initial author-tool compilation used a nonexistent nifly Vector3::Length member; it failed before any mesh authoring. The corrected tool uses explicit component norm.
The skin instance is reacquired after partition maintenance; do not retain a possibly replaced native block pointer across that mutation.
First Papyrus compile failed because Caprica interpreted the entire absolute path from [LOCAL_PATH_REDACTED]
The metadata index disables Python bytecode writes; actual final files and hashes are read back. Schema, model load and script compile are not a substitute for Skyrim/FSMP simulation or rendered cloth motion.



### 원문 부록: CP026_TOWER_SKY_REPORT
원본 SHA256: 37107B53DF1574796A456399504CA94687B1E14177A0C42E5F58EFE6CBD1B4DD

# TOAM v0.8.8 / CP026 — Tower sky rim supplement
Parent: main v0.8.7 CP026 plus separately delivered Staff SMP0.1.0-test1. Runtime NOT_RUN; LOCK NO.
This finishes the Tower thickness correction before final manual QA.

## Confirmed source/asset mechanism
The earlier field-band correction reduced axial depth3 to .06 at scale8.5 (25.5 to .51 world units).
Final cross-check also found the SKY circle uses the shared Emperor circle's CP021_ThinRim: axial depth5.16, scaled by RainFootprint(t)/200.
RainFootprint(t)=100+93.75*max(0,t). At t8, the shared wall depth would be21.93 world units. This is a separate remaining scale path; correcting only the field band did not cover it.
All8 actual side/bank sky ARTOs resolve to Arcana_Emperor.nif. This was verified through IntegratedFormMap and the real main ESP, not guessed from the Tower card name.

## Change and ownership
One private Arcana_Tower_Sky_Thin.nif copies the actual Emperor geometry and compresses ONLY CP021_ThinRim Y coordinates by .02 about its original center. Its depth is .1032 asset units (.4386 world units at t8).
Original planar artwork, rim X/Z, triangles, UVs, normals/colors/tangents and shader values are preserved by independent native comparison.
The shared Emperor circle and every other common circle remain byte-identical. No global thinning of World/Sun/Proc/Halo models.
Eight new main ARTOs DC6..DCD retain original non-model properties, one per2 hands x4 banks; no old FormID moves.
Expand the existing forms.circle array168->176, load the8 local TOAM forms directly, and select kind3 ONLY for Rain's sky slot.
Existing request ticket, exact ART identity, 3D-first pose, retirement/quarantine and forms.circle-wide PurgeArt own these objects. No extra hook, listener or frame pass.
Rain field/sky positions, radius850, height520, rotation/growth, timing, shots, payload/slow/finale, cost1600 and target cap630 remain unchanged. Existing records except TES4 are byte-preserved.
No gameplay, native billing or SMP script change. Staff SMP16-record base/3-record patch are compatible with v0.8.8 because every existing WEAP and master filename remains exact.

## Validation and remaining acceptance
Main1401 records(TES4 included), addon15, defaultEN/Language/en/kr. Original007/158 remains unchanged.
Existing MSVC Hostx64/x64 build/msvc Ninja only; new DLL must be built and deployed with synchronized008.
A shallow edge in mesh/source space is not visible-pixel evidence. User SE1.5.97 manual QA still covers both Tower rings, World, halo, ascent, adaptive freeze/resource and optional SMP.
AE remains guarded for the previously unverified precommit/audio ABI. Neither this data change nor the SMP assets grants AE Native compatibility.

## Development lesson
Audit every scaled presentation owner of an effect. Field and sky have different ARTOs, models and scale functions. Measure thickness after each real transform; a field-only correction cannot certify the sky.
Preserve an already suitable shared asset by giving the exceptional large-scale presentation its own owned ARTO/model. Do not hide this via offsets, bounds, waits, non-orthogonal rotation matrices or mutable shared geometry.



### 원문 부록: CP027_POSTQA_PRESENTATION_BALANCE_REPORT
원본 SHA256: 65B2156B3874604E71B0AE0F1EDB3C31FD5673DD220E6F825A1FAF174F326F89

# TOAM0.8.9 / CP027 — AI technical recovery report
Parent: CP026 v0.8.8. The user reports basic functions working and no CTD in this test, with open presentation/aim/cloth/balance defects. This is PARTIAL runtime evidence, not an all158 PASS. Corrected0.8.9 runtime has NOT_RUN. SE1.5.97 authority; AE remains blocked by the documented precommit/audio ABI audit.

## Evidence
Four user recordings2026-10-26 08-10-11 /08-13-06 /08-16-59 /08-26-40 and OrdinaryAttackMagic.log. Input SHA256, duration and frame contacts are in CP027 stage003_INPUT_MANIFEST.json; preserved log000_USER_RUNTIME.log SHA9E1215931A0B2FACD9801CF227E96CAAF630C48B5A7AB34E742023F99DBC9A2D.
The log proves nominal Tower beam3/finale90/cap630 and actual paid reductions factor0.8023739, not a 100% free-cost assumption. User observations of weak Tower and excessive recurring efficiency are accepted; the selected replacement numbers are explicit user approval, not inferred balancing authority.

## Failure → correction → ownership
World: CP026 unified all32 TOAM Dome slots onto Hermit art but retained mismatched artwork dimensions: inner face halfwidth105 vs inscription band172x1.15=197.8. New private World_Harmonized model scales ONLY Face and NodeGlow local art by1.65 (face173.25). Ring/glow/thin rim/materials/UV remain original. Only the32 World ARTO model paths change. Arcana Hermit and its defense are byte-preserved. World4/8/12, .7/1.4/2.1, .12 growth, ground/air coordinates, outward64 beam, exact explosion endpoints, TOAM explosion assets/damage/overlap remain unchanged.

Sun: CircleCount returned8 immediately and UltimateScale ignored growth; alpha alone cannot provide the requested physical emergence. Request one layer each .8s, start its own .35s scale growth from4% after actual CircleReady/first pose; alpha opens in.12s. Last layer starts5.6s, preserving the8-second1550 budget and1520 payload. All8 posed/grown layers gate release within the existing10s readiness deadline; no extra payment beyond8s. Exact ARTO owner/ticket/retire/quarantine is retained. No bounds/wait workaround or new frame/actor scan.

Magician/Judgement aiming: TOAM read render-camera column0 while Arcana uses Actor GetAimAngle/GetAimHeading (graph aim pitch/heading). TOAM carriers were self-delivery fire-and-forget, unlike Sun aimed concentration. These are confirmed source differences; their responsibility for every user aiming symptom is a runtime hypothesis. Both TOAM carriers now reference new localDCE no-damage Script MGEF, aimed concentration, zero engine cost/projectile/hit events. The shared old808 carrier remains exact for World/Temperance. The existing Arcana caster-update hook suppresses engine projectileTimer/currentSpellCost ONLY for these exact resolved TOAM carriers; Identify/resource/session remain TOAM-owned. No new hook or double payment. Original native charge profile, tiers, MP and fixed damage remain unchanged.
Common HostAimDirection uses the same engine matrix as Sun. Magician centers remain actor-front ground / Arcana FlightCastOffset airborne, and follow that engine heading/axis instead of render-camera free-look.
EXPLICIT SPATIAL CHANGE: Magician collision origin is now the same Sun-style front anchor (ground actor-relative0,74,108; flight uses original Arcana AirCircleOffset), replacing Actor3D worldBound.center. This aligns stack, visible beam and logical ray rather than converging from different heights. Range/radius/damage values stay unchanged; the actual world origin/reach from the feet changes and needs near-wall/air QA. Do not describe this as only cosmetic.
EXPLICIT SPATIAL CHANGE: Judgement selection/release marking and no-damage guide share a fixed right-shoulder emitter (actor-local34,22,126 ground; same Arcana flight-offset policy airborne). It replaces the hand-attached guide and actor-center logical origin. The12u guide circle and thin beam start together, with no animation-hand jitter. The latest user shoulder request supersedes prior hand-origin authority. Range,1-time1-fixed mark,300/900/1800 tiers,3/6/9 release-only artillery,.45/.12 cadence and corpse prevention stay unchanged. Charge guide has no damage.
Judgement large-target artillery: preserve every existing fixed world origin and per-shot target snapshot. Only visible circle scale=.10*clamp(formationRadius/99.84,1,10). Normal baseline40u diameter stays40; radius600 yields~240u. This preserves gameplay positions while restoring angular presence. Fired rays never auto-track; target death does not cancel the committed spatial sequence. Temperance Proc/halo geometry/payment/lifecycle remains unchanged.

## Approved Arcana-only balance
Chariot100 per actual shot/64 damage, repeated5-shot nominal500/320. All independent .36/.39/.42/.45/.40s intervals retained. Strength250/s,195/s with original.1s damage tick. Star activation160 plus50/s per deployed lane (CircleReady and hasPrevious), up to250; committed lanes remain billable between their pulse/fade cycles. Zero undeployed lanes pay no upkeep; no resource charge is attributed to TOAM tiers. Original auto-targeting and lane cadence/damage retained.
Tower30 per beam,900 finale,6300 per target per generation. Carrier1600, original180-shot distribution, radius850/timing/finale actual-contact gate unchanged. Update both payload SPEL EFIT and native impact nominal30, plus finale direct magnitude900 and precommit budget6300. Changing only the design table would leave actual projectile damage stale. No new attack pattern or resistance rule.
ResourceFactor stays CalculateMagickaCost/explicit carrier costOverride. Chariot metadata500 and Strength250 produce the same vanilla skill/perk/gear reduction ratio for their new units. Engine cost is suppressed once for manually billed modes; Hermit's original ordinary engine path is unchanged. Staff exact instance/hand ExtraCharge ledger uses the same reduced unit, never MP. Normal20000/Unbound40000, zero-cost-at100%, recharge/unique-enchantment policy remain. Existing SE charge sync avoids weapon-ability reinstallation inside caster callbacks.
Actual current form map and eight-card table: FORM_MAP.json and ARCANA_BALANCE_089.json. Old CP021 cost numbers are historical and superseded only for these explicitly approved four cards.

## Book back cover
Vanilla UVs repeat across integer tiles (including negativeU/V); assigning one custom front atlas makes both covers repeat its title. The inventory back panel Book01:6 uses negativeU, unlike the world back panel's V+1. A filename/path fix cannot solve this.
Built-in image_gen creates a restrained blank leather back matching the existing Hebrew coursebook. Pack into a2x22048 DDS atlas: original front in tile0/0, blank back/inner panels in the other tiles. UVs divide by2, with outward negativeZ back vertices routed away from the front tile. Duplicate the original normal into the same layout; preserve black emission. Original front BC3 blocks are bit-identical at mips0..8. NIF vertices/normals/colors/tangents/triangle indices, bone weights/partition indices and controller/collision block contents remain unchanged; only4 UV bytes per affected vertex inside NiSkinPartition change; only cover UV data changes. No duplicate per-book STAT/model. The front title/four lines/Frieren author remain. Generated source/prompt metadata are in working Authoring/Books; preview is a UV atlas, not runtime proof.

## Staff SMP coverage
Actual active MO2 profile enables test1 SMP and base/Stances ESP, while the separate SMP Stances Patch is disabled. Thus the two Unbound WEAPs lacked skinned model/keyword/helperVMAD. This definitively explains missing SMP integration for those records; it does not uniquely explain every normal-staff motion observation.
Replace the fragmented option with one same-named SMP ESP covering all10 staffs,7 typed helper/control records,18 total inclTES4. Masters are Skyrim + TOAM main + TOAM Stances Addon (no external Stances hard MASTER). Own helper index2->3 is relocated through exact ARMO/VMAD/KWDA/record fields; integer masks/values are not bulk-replaced. Existing enabled plugin name is retained; active mod folder gets verified updated bytes without changing the user's profile. Old disabled patch is superseded.
Keep the two private rigs, only cloth/ornament skinning, helper slots61/60, no body/root simulation or ground collision, same-model dual-wield static fallback and foreign occupied-slot preservation. Add bounded Papyrus equipment transition trace, not polling. Native hooks/payment/movement are untouched by SMP. Actual cloth movement and previous failure cause remain runtime questions. Do not claim physics PASS from XSD, bind or package checks.

## Invariants and prevention
No new Native hook, listener, global scan, duplicate DLL, actor traversal, cooldown or auto QA collector. Source changes stay within Arcana presentation/balance, two TOAM aim/presentation files and version. TOAM charge ledger/profile/arbiter, fixed damage, World and Temperance source remain byte-identical. Preserve original007/158 SHA00CF6FBC482514684F87DB317EC6A2B433F2286777F25403206BE15BA5F6F0F1.
Candidate failures were not Production failures: exact Chariot payment anchor corrected after inspection; missing collections import fixed; MGEF disk offsets80/84 verified in xEdit rather than padded native108/112; nifly GetTriangles belongs to NiShape; valid negative book UVs rejected a simplistic0..2 guard and were retained. Reuse successful source/syntax and correct only the failed layer. None authorizes silently overwriting current source or 008.
Preflight actual current008 latest-row authority and deployed DLL/package; full source/assets/docs/deploy/Ninja snapshot before writing; existing MSVCx64 build/msvc Ninja only. One transaction updates source, ESP/assets, language variants, DLL/MO2/ZIP/docs/008 with readback or full coherent rollback. No game boot/profile edits or public upload. Build/static/deploy do not prove pixels, camera behavior, cost measurements or SMP motion.

## One consolidated user SE1.5.97 QA
World inside/outside band fit; Sun delayed small-to-large8layers and cancel; Magician/Sun camera/axis in1P/3P ground/flight; Judgement shoulder guide selects the same living target released upon, near walls and corpse rejection; large dragons scaled fixed artillery; Temperance existing halo/proc unchanged. Verify Chariot/Strength/Star 0/50/100% vanilla reduction and exact staff MP0/charge; Tower beam30/finale900/cap6300 before resistance. Re-equip normal and Unbound silver/gold and test cloth during walk/jump/ascent/stance changes. Foreign slot and same-model fallback remain deliberate. Both covers/world and inventory reading animation must show front text and blank back. Runtime NOT_RUN; LOCK NO.



### 원문 부록: CP027_RESUME_FSMP_AUTHORITY_REPORT
원본 SHA256: 39AFD479E257145D47851F566E2FD1B0E074323CCE07B5DB33A0135DE2714F13

# CP027 resume audit — production authority and pinned FSMP3.1.1
STATUS: READ_ONLY_AUDIT, no additional source/asset/build/deployment mutation.

The actual production is TOAM0.8.9 CP027: all11 changed source/config hashes, build/deployed DLL and both final release ZIP hashes match the prior verified results. The later RR-CP002 documentation update preserved these files. Its SOURCE_INDEX captures the10 CP027 src files but omits root CMakeLists.txt; that file was independently verified. Its prose still names CP0260.8.8. That prose is a stale parent description, not an authorized rollback. All13 RR-CP002 current document byte/hash rows match physical files. Preserve these additions, explicitly add the missing build-config index row and append the correct current production pointer. Do not assume a large source catalog covers every build input.

Pinned FSMP: v3.1.1 commit73900d4f5f40df31d559897c04c6e7ac9ff014d0, matching the installed engine version. The earlier dev reference was v4.1.1 and is not binary proof for3.1.1.
1. NetImmerseUtils::readAllFile uses BSResourceNiBinaryStream. The shipped SKSE\Plugins\hdtSkinnedMeshConfigs\TOAM paths are Data-relative VFS paths. A speculative Data-prefix change is rejected.
2. SkinnedMeshSystem::valid requires nonempty bones, not collision meshes. A no-collider bone/constraint system is valid in3.1.1. Adding a ground/body collider to force activation is rejected.
3. Explicit roots are mass0; eight child bones use .04 default, so the XML does not accidentally define all anchors. Original skin/bind/partition checks still apply, not physics motion proof.
4. Exact root NiStringExtraData ownership and file paths match. The Riilungolz reference similarly uses a helper armor/XML and skinned weapon; none of its assets are copied. Our first-person models remain static, third-person cloth/ornaments only.
5. The ten-staff consolidated SMP plugin closes the proven missing two adaptive WEAP routes. The normal-staff non-motion cause is NOT uniquely established. Runtime helper equip timing, actual private weapon nodes under the NPC skeleton, occupied slots, same-model fallback, view and solver activation remain separate questions. Do not mask missing bone/effect proof with stronger wind, larger masses, new colliders, extra waits or equip locks.

No new source or asset repair is justified by this extra static audit. Keep0.8.9 and SMP0.1.1-test2 as the final integrated candidates for one user-run SE1.5.97 QA. Actual cloth, camera/targeting, World/Sun/Judgement/book visibility and approved costs/damage are NOT_TESTED after CP027. AE remains NOT_ENABLED.

Primary source: https://github.com/DaymareOn/hdtSMP64/tree/73900d4f5f40df31d559897c04c6e7ac9ff014d0
Official authoring notes (version-neutral explanatory source): https://github.com/DaymareOn/FSMP-Validator/wiki/27-%E2%80%90-Linking-the-NIF-to-the-XML
Saved pinned source manifest:044_FSMP_PINNED_SOURCE.json and045_FSMP311_* in the CP027 stage; local reference files are research only, not part of the release ZIP.



### 원문 부록: CP028_CARRIER_CTD_SMP_REPORT
원본 SHA256: DFBE5C95E5880C5B953BA9964569F792C448FDE6BD4CDAF3FF2C4FC6BBFEBD1C

# TOAM v0.8.10 / CP028 — aimed-carrier CTD and Staff SMP recovery
Parent:CP027 v0.8.9 / Staff SMP0.1.1-test2. User runtime:Magician activation CTD; third-person cloth malformed and no observed physics. Corrected runtime NOT_RUN, LOCK NO. SE1.5.97 only; existing AE guard remains.

## Confirmed CTD cause and minimal repair
CrashLoggerSSE reports SkyrimSE+00FBA60, mov rax,[rcx+C8], RCX=0; RSI is Arcana - Magician local800. Native log ends immediately after Magician BEGIN at12:58:40.894. Crash filename03:58:41 is consistent with UTC+9, not an unrelated earlier3am event. The supplied video independently documents the staff presentation; its exact frame-to-log time alignment is not established.
Disassembled SE1.5.97 input/caster function33657 first checks casting=2/concentration and delivery=2/aimed, calls MagicItem::GetAVEffect at100DF0, then uses EffectSetting projectile accessor atFBA60. GetAVEffect rejects aimed effects without projectileBase at native+C8 (EffectSetting.data68+projectile60). CP027 cloned self Script MGEF808 intoDCE, changed casting/delivery, but left serialized DATA+72 projectile reference0. Thus GetAVEffect returns null and the unguarded input accessor dereferences it. TOAM+2CF40 maps to FlightAttackProcessButton returning from the original input handler, not a new physics hook.
This is an assistant-introduced CP027 metadata regression. The later caster-update projectileTimer suppression cannot protect an earlier input-time lookup. Do not blame other stack DLLs, disable unrelated mods, add waits, or swallow the original engine input.
Fix:in existing MGEF localDCE only, connect the existing Arcana empty-model carrier PROJ localA6D, already used by Emperor/Sun. Its Script/no-hit/no-effect flags and zero effect magnitude remain; both SPELs, all damage, MP, tiers, beam origins/aim, eight layers, corpse rules, other MGEFs and existing native hooks are unchanged. Each EN/root/KR main ESP changes exactly one4-byte reference (three differing bytes). No new form or attack projectile is authored. The existing exact-carrier timer and cost suppression remain authoritative; no second payment/damage owner.

## Regression gate
The persistent tools/toam_package.py dependency validator now checks both charge carriers in root/en/kr:aimed concentration, exact DCE ownership, zero magnitude/area, Script/no-hit flags, eligible nonnull PROJ and empty model. Negative control:original CP027 package is rejected with SE97 aimed carrier has no eligible projectile effect. The repaired candidate passes6/6. Asset path closure alone did not detect the missing engine semantic prerequisite.
CMake version0.8.10 is the only Native build-input change. Rebuild in existing MSVCx64/Ninja build/msvc; no new hook or gameplay C++ change. Source tool change is packaging validation only.

## Staff SMP structural candidate
The Riilungolz reference's moving weapon NiSkinInstance targets its dedicated kinematic first chain bone. Our test2 moving shapes instead targeted the generic weapon root FirstClassStaff / MillennialStaff while the helper only activates the solver. Static rest-pose reconstruction alone had never tested that engine weapon-root contract.
Test3 targets each private TOAM_SMP_Silver_00 / Gold_00 anchor. Rebase global-to-skin by -anchor and each inverse bind by +anchor. This retains the model-space rest shape and existing nine-bone ownership while avoiding the generic weapon root as the skin root. All original vertices, normals, UVs, colors, tangents, triangles and weights are preserved, including the nonmoving staff and tied wraps. Six moving cloth/attached ornament meshes only; no body, NPC Root, world-ground or global FSMP configuration changes. First-person original static models remain.
Independent saved-file validation:all six normalized hardware vertex-weight streams; original/repaired geometry and skin weights equal; per-vertex bind reconstruction maximum error Silver.01744u / Gold.01322u (stored half precision). XML, helper meshes, Papyrus, ten WEAP/ENCH records and capacities20000/40000 remain byte-identical. No forced equipment locks, polling, frame hooks or actor scans. Existing occupied-slot/same-model fallback policies remain.
This proves the corrected skin-root/bind structure, NOT successful cloth rendering or solver activation. The user's general non-motion cause is not uniquely established. Actual node attachment, helper slot, solver ownership, moving cloth, jump/flight/stance and old CTD behavior remain user runtime checks. Report failures separately; do not relabel test3 physics PASS from XSD/weight/geometry evidence.
Reference-only:https://www.nexusmods.com/skyrimspecialedition/mods/97159. No donor assets/scripts/XML/binaries are copied. FSMP3.1.1 is the installed external dependency, independently identified in the crash module list. Its pinned source uses VFS file paths and does not require collision meshes for a bone-only system, so speculative Data-prefix/collider/wind/mass changes are omitted.

## Preservation and reuse
CP027 World scale fit, Sun sequential growth, engine aiming, shoulder guide, large-target artillery scaling, book blank back and approved Arcana balance remain. TOAM charge ledger/profile/arbiter/fixed damage and Hermit defense are unchanged. Original007 A-J158 is preserved. No profile/save editing, automatic QA collector, game launch, independent plugin build tree or public upload.
Record this incident in cumulative001 and append-only evidence/RESEARCH_REGISTRY/TOOM_TOAM/JOURNAL. Refresh SOURCE_INDEX and immutable snapshots while preserving earlier history. Future public source Research/ is separately reviewed; raw private logs/snapshots are not bundled wholesale.
Evidence:CP028 stage001_INPUT_MANIFEST,003_NATIVE_*,005_CARRIER_RECORDS,007_CARRIER_STATIC,009_*_VERIFY,010_GPU_WEIGHTS and final result/transaction manifest. Candidate-only audit failures:optional pefile Python module unavailable (used existing dumpbin instead); inherited parser helper required collections import (corrected before Production). No source/assets were modified by these preflight failures.

## Consolidated manual SE QA
Magician and Judgement equip/activate/release/cancel in1P/3P, one/both hands, menu/flight; no CTD, no engine damage or double billing. Preserve correct targeting and established presentation. Third-person Silver/Gold normal and Unbound:cloth visible at rest and during walk/jump/ascent/stance, physical swing, helper cleanup, occupied-slot and same-model fallback. Test first-person static fallback separately. Latest corrected runtime NOT_RUN; AE NOT_ENABLED; LOCK NO.



### 원문 부록: Fern 지팡이 초기 이미지기반 제작/정적수치
원본 SHA256: 1F98EE2BE03821146087C06F5FDCACA91F9D29A0EF4C52749AF326295300960F

# Fern Staff 제작 및 정적 검증 보고서

## 1. 제작 기준 및 범위

- 사용자 제공 AGENTS 지침 원문 및 요청을 확인하였습니다. 첨부 이미지는 형태 참고 자료로 사용하였으며, 이미지 속 문구를 작업 지시로 취급하지 않았습니다.
- 기존 프로젝트와 분리된 `[LOCAL_PATH_REDACTED]`에 신규 제작하였습니다.
- 사용자가 선택한 범위는 외형 제작이며, 전용 마법 구현은 포함하지 않았습니다.
- 목재 본체·감개·문양·리본·보석의 메시와 표면 텍스처는 절차적으로 새로 제작하였습니다. 참고 이미지의 픽셀을 텍스처로 전용하지 않았습니다.
- 상단의 비대칭 넓은 목재 끝, 꺾인 몸체, 보라색 감개와 리본, 은색 장식통, 붉은 펜던트를 반영하였습니다. 게임 부착 기준에 맞추기 위해 참고 사진의 세부 비율을 조정하였습니다.

## 2. 바닐라 구조 및 크기 근거

기준 파일은 Skyrim - Meshes1.bsa에 있는 `meshes/weapons/staff01/staff01.nif` 및 `1stpersonstaff01.nif`입니다. 로컬 추출본과 설치 BSA의 해당 멤버를 비교하여 바이트 단위 일치를 확인하였습니다. `evidence/vanilla_archive_proof.json`에 두 멤버의 SHA-256을 기록하였습니다.

- 월드 기준 길이: Y -87.90222168 ~ +46.70283890, 약 134.60506 게임 단위.
- 게임 NIF 장축은 Y, Blender 원본 장축은 Z입니다. 내보내기 변환은 `(x, y, z) → (x, z, -y)`입니다.
- 두 출력 NIF는 같은 제작 메시와 길이를 사용합니다. 각 바닐라 파일의 1·3인칭 INV 회전·줌은 각각 보존하였습니다.
- BSFadeNode, Prn=`WeaponStaff`, BSX=194 및 바닐라 bhkCapsuleShape·강체 설정을 보존하였습니다. 충돌은 원본의 단순 캡슐이며, 리본과 펜던트의 개별 충돌은 없습니다.
- 바닐라 Staff01에는 별도 발사 노드를 추가할 근거가 없으므로 신규 발사 노드를 임의로 만들지 않았습니다. 전용 마법 발사 위치의 런타임 검증은 추후 마법 연결 작업의 범위입니다.

## 3. 파트별 출력 수치

수치는 UV·법선 경계에 따른 정점 분리를 적용한 최종 NIF 기준입니다.

| 파트 | 정점 | 삼각형 |
|---|---:|---:|
| 01_Carved_Wood_Core | 5,125 | 9,840 |
| 02_Lower_Purple_Spiral_Wrap | 6,963 | 13,376 |
| 03_Upper_Purple_Wrap | 2,607 | 4,928 |
| 04_Engraved_Silver_Grip_Sleeve | 1,026 | 1,904 |
| 05_Silver_Collars_and_Rivets | 3,180 | 5,328 |
| 06_Silver_Interlace_and_Eye_Ornaments | 3,912 | 6,412 |
| 07_Hanging_Purple_Ribbon | 910 | 1,816 |
| 08_Pendant_Silver_Eyelet | 231 | 384 |
| 09_Red_Faceted_Pendant | 192 | 76 |
| 합계 | 24,146 | 44,064 |

각 파트의 정점과 삼각형은 모두 15,000 미만입니다. 이 예산과 NIF의 포맷 필드 한계는 별개의 개념입니다. Nifly의 정점·인덱스 구현을 대조하였으며, 15,000을 엔진 공통 한계라고 단정하지 않았습니다.

## 4. 재질 및 플러그인

- 9개 BSTriShape. 각 파트에 BSLightingShaderProperty와 전용 BSShaderTextureSet을 연결하였습니다.
- 불투명 목재·직물·은색 금속·붉은 보석입니다. 발광과 알파 블렌딩은 사용하지 않았습니다.
- 목재·직물 2048², 은색 금속 1024², 보석 512². 재질당 Diffuse 및 Normal+Specular alpha 2개, 전체 8개 DDS입니다.
- 외형 확인용 ESP는 StaffTemplateRestoration(00051B0C)와 1stPersonStaff01(0007A91D)을 기준으로 신규 레코드를 생성했습니다. EITM/EAMT 없음, 마법·인챈트 없음, 기존 레코드 override 0개입니다.
- 신규 아이템 `FS_FernStaff`, 내부 FormID 01000800. 신규 1인칭 STAT `FS_FernStaff1stPerson`, 내부 FormID 01000801. 실제 로드오더 FormID는 게임 콘솔에서 확인해야 합니다.

## 5. 검증 결과 및 경계

| 구분 | 결과 | 근거 |
|---|---|---|
| 바닐라 출처 | PASS | 설치 BSA 멤버와 추출본 SHA-256 일치 |
| NIF 포맷 | PASS | 20.2.0.7 / User 12 / Stream 100, C++ Nifly 저장·재읽기 및 별도 PyNifly DLL 재읽기 |
| 출력 기하 | PASS | 원본 좌표·삼각형 일치, 유효 인덱스, 면적 0 삼각형 없음, UV·법선 유효 |
| 파트 예산 | PASS | 9개 파트 모두 정점·삼각형 각각 15,000 미만 |
| 바운드 | PASS | 모든 정점이 파트별 바운딩 구에 포함됨 |
| 부착·충돌 | STATIC PASS | WeaponStaff 및 바닐라 강체·캡슐 보존 |
| 텍스처 | PASS | 8개 DDS, 모든 참조 파일 존재, DXT1/DXT5 및 전체 밉맵·파일 길이 검사 |
| 플러그인 | STATIC PASS | TES4/STAT/WEAP 직접 재파싱, 신규 ID·마스터·모델 연결 검사. xEdit 실행 검사는 미실시 |
| 외관 | OFFLINE REVIEW | 실제 배포 NIF·DDS 재로딩 후 Blender 렌더 확인. Skyrim 재질의 근사 표현 |
| 기존 원본 보존 | PASS | 기준 NIF 2개 및 Skyrim.esm SHA-256 재확인 |
| 설치·런타임 | UNVERIFIED | MO2 배포·게임 실행 미실시 |

`evidence/validation.json`에 상세 수치와 해시를 기록하였습니다. 바닐라 대비 같은 길이의 비교 이미지는 왼쪽이 원본 Staff01의 중립색 기하, 오른쪽이 새 제작 메시입니다.

## 6. 편집 및 재생성

`source/FernStaff.blend`의 `FERN_STAFF_GAME_MESHES` 컬렉션이 편집 대상입니다. `PRESENTATION_ONLY`의 카메라·조명·가이드는 게임 메시가 아닙니다. 원본 텍스처는 BLEND에 내장되어 있습니다.

재생성 스크립트는 `tools/make_textures.py`, `build_model.py`, `nifassemble.cpp`, `make_plugin.py`입니다. `inspect_sources.py`로 로컬 바닐라 자료를 재확인한 뒤 사용합니다. C++ 도구는 로컬 Nifly 라이브러리에 링크하며, 원본 NIF에서 메타데이터·충돌을 유지하고 제작 기하로 교체합니다. 소스 ZIP에는 Bethesda 원본 NIF와 바닐라 기하를 포함하지 않았습니다. 생성 스크립트 실행은 수동 편집한 BLEND를 재생성할 수 있으므로 별도 작업본에서 수행하십시오.

## 7. 기술 정의 참조

- [Nifly Geometry.cpp](https://github.com/ousnius/nifly/blob/master/src/Geometry.cpp): BSTriShape, 정점·삼각형 및 바운드 구현.
- [PyNifly](https://github.com/BadDogSkyrim/PyNifly): 별도 DLL을 통한 NIF 재읽기.
- [xEdit TES5 정의](https://github.com/TES5Edit/TES5Edit/blob/dev-4.1.5/Core/wbDefinitionsTES5.pas): WEAP의 WNAM, EITM, DNAM 및 플러그인 필드 정의.

## 8. 제작자 관점의 후속 검토 질문

1. 실제 장착 시 바닐라 손잡이 원점과 손가락 위치가 맞으며, 1인칭에서도 리본 간섭이 없는가?
2. 상단 목재·은색 문양·보라색 감개의 비율과 색감이 원하는 참고 이미지에 충분히 가까운가?
3. 추후 지정할 마법의 발사 위치와 리본 물리 적용 여부를 어떤 기준으로 결정할 것인가?



### 원문 부록: ChargeCannon r3 생성/재사용/한계
원본 SHA256: AA6B9F6AAB3C8B692F48EF1EB4FCDB334C8A7D387CAC912C751B209DAC879DB2

# ChargeCannon 에셋 패키지 r3 (= r2 production + Tintable 확장)

r1의 시각 디자인(청백색 마법진, 백색 코어 + 청색 글로우 빔, 청백 충돌 폭발)은 그대로 두고, DLL이 Spawn / Position / Rotation / Scale / Length / Width / Lifetime만 정하면 되도록 구조를 바꾼 패키지입니다. 파일별 변경 내용은 루트의 `CHANGELOG.md`, 좌표/축/변환 규칙은 `Docs/ASSET_COORDINATE_SPEC.md`, 노드별 표는 `Docs/NODE_MAP.md`를 보세요.

## 구성
| 경로 | 내용 |
|---|---|
| `Data/meshes/effects/ChargeCannon/` | **NIF 12개** (Charge_Circle x5, Beam_Body_Unit x2, Beam_Muzzle, Impact x4) |
| `Data/meshes/effects/ChargeCannon/Legacy_r1/` | r1 고정 길이 빔 3개 (참조용, 신규 개발에는 사용 안 함) |
| `Data/textures/effects/ChargeCannon/` | DDS 7장 (기존 6 + `Circle_Small`), DXT5, 밉맵 포함 |
| `Source/png/` | DDS 원본 PNG |
| `Source/nifspec/` | 각 NIF의 노드/메시/셰이더/컨트롤러 정의 (텍스트) |
| `Source/scripts/` | 텍스처, NIF, 프리뷰, 문서 생성 스크립트와 NIF 빌더 소스 |
| `Previews/` | 프리뷰 7장 |
| `Docs/` | 좌표 규격, 노드 맵, 매니페스트 |
| `manifest.json` | 파일 목록, 노드/메시/컨트롤러 요약, SHA-256 |

## 역할 요약
- **마법 A 마법진**: `Charge_Circle_Static` / `Charge_Circle_Core_Static`을 여러 개 인스턴스. 링마다 `CC_Root` 스케일로 크기 리듬, `CC_Outer`/`CC_Band`/`CC_Face`의 로컬 Y축 회전으로 속도·방향 제어.
- **마법 B 발밑/포위 마법진**: `Charge_Circle_Small` (텍스처 1장 + 약한 후광).
- **빔**: `Beam_Body_Unit`(스크롤 포함) 또는 `Beam_Body_Unit_Static`을 `ceil(L/512)`개 이어 붙임. 길이와 폭은 행렬 스케일로 독립 제어 (`ASSET_COORDINATE_SPEC.md` 2절).
- **머즐/폭발**: `Beam_Muzzle`, `Impact_Low/Mid/High`, 신규 `Impact_High_Compact`.

## 요청 14항 자체 검증 결과
| 항목 | 결과 |
|---|---|
| Skyrim SE NIF 포맷 (20.2.0.7, user 12, stream 100) | 15개 전부 nifly로 재로드 성공 |
| 텍스처 참조 경로, 누락 | 참조 7종 전부 `Data/textures/...`에 존재, 누락/미사용 0 |
| 노드 이름이 문서와 일치 | `make_docs.py`가 nifspec와 실제 NIF의 노드 이름을 대조: 불일치 0 |
| Static 계열에 자동 회전 컨트롤러 없음 | 확인 (회전 컨트롤러 Static 0, Core_Static 0, Small 0, Animated 2) |
| Beam Body Unit 세그먼트 이음 | 정점 확인: y 0~512, 양 끝 반지름 동일, UV u 0->1. 텍스처는 512 주기라 u 0 = u 1이고 이음 단차(최대 141)가 내부 단차(최대 141)를 넘지 않음. 프리뷰 `Beam_Seam_Test.png`에서 1/3/8 세그먼트 확인 |
| 길이와 폭 독립 | 구조상 가능: NIF 스케일 1.0, 비균등 행렬은 DLL이 적용. 프리뷰 `Beam_CoreGlow_Split.png`, `Beam_Length_Width_Independence.png`는 **소프트웨어 시뮬레이션**이며 실제 인게임 확인은 아님 |
| DDS 알파/밉맵 | 7장 모두 DXT5, 알파 채널 유효, 전체 밉 체인 |
| 작은 크기 가독성 | `Circle_Small.png`: 110px 3x3 배치에서 Small은 오망성/노드/룬이 읽히고, 축소한 Large보다 선명 (소프트웨어 렌더 기준) |

## 확인하지 못한 것 (인게임/NifSkope 미검증)
1. 인게임에서의 밝기, 가산 블렌딩, Z-정렬. Emissive 값은 시작값입니다.
2. 직접 연결 컨트롤러(루프/1회)가 SE/AE에서 의도대로 동작하는지, 반복 컨트롤러가 전역 시계(App-Time)로 세그먼트 간 위상이 맞는지. 안 맞으면 `Beam_Body_Unit_Static`을 쓰고 UV는 DLL이 구동하세요.
3. UV 스크롤 방향. 반대면 `gen_nifs.py`의 `scroll()` 키 부호를 뒤집으세요.
4. 비균등 행렬 스케일을 SE/AE의 해당 노드에 적용했을 때 렌더링(법선/Falloff 영향 포함).
5. `Impact_*`의 +Y = 표면 법선 가정은 이 패키지의 설계 규약이며, 실제로는 DLL이 정렬해야 합니다.
6. 히브리어 문장은 원어민 검수 전입니다.
7. 실제 왜곡(Distortion) 효과는 없습니다.

## 재생성
```
cd Source/scripts
python3 make_textures.py     # PNG (cairosvg, pillow, numpy 필요)
# DDS: convert X.png -define dds:compression=dxt5 -define dds:mipmaps=<log2 최대변> X.dds   (ImageMagick 6)
g++ -std=c++17 -O2 nifbuild.cpp -I<nifly>/include -I<nifly>/external <nifly>/build/src/libnifly.a -o nifbuild
g++ -std=c++17 -O2 nifinfo.cpp  -I<nifly>/include -I<nifly>/external <nifly>/build/src/libnifly.a -o nifinfo
python3 gen_nifs.py          # nifspec + NIF 15개
python3 previews.py          # Previews/
python3 make_docs.py         # Docs/NODE_MAP.md, Docs/MANIFEST.md, manifest.json (nifinfo 필요)
```
nifly: https://github.com/ousnius/nifly (MPL-2.0, `cmake -DBUILD_TESTING=OFF`). NIF를 수정하려면 `gen_nifs.py`/nifspec을 고치고 재생성하세요. NifSkope로만 고친 수정은 재현되지 않습니다.


---
## r3 추가분: Neutral / Tintable (후방 호환)
- **A. BlueWhite production = r2 그대로.** 모든 r2 파일은 바이트 단위로 같습니다 (`Docs/R2_UNCHANGED.md`, 29개 SHA-256 일치). 현재 기본 마법에 사용하세요.
- **B. Neutral Tintable = 새 파일.** `Data/meshes/effects/ChargeCannon/Tintable/` 의 `*_Tintable.nif` 12개와 `Data/textures/effects/ChargeCannon/Tintable/` 의 Neutral DDS 6장. 향후 DLL 런타임 재색상이나 다른 색 계통에 사용하세요. (`Beam_Streak.dds`는 이미 순백이라 r2 파일을 공유)
- 색 외 동작은 r2와 같습니다: 노드/메시 이름, 지오메트리, 컨트롤러, 좌표 규약 동일 (Static은 회전 컨트롤러 0개 + 펄스 유지, Animated는 회전 유지).
- 사용법과 한계: `Docs/TINTABLE_GUIDE.md`. 메시별 역할/기본 RGB/Tint Safe: `Docs/NODE_MAP.md`. QA 수치: `Docs/TINTABLE_QA.md`. 프리뷰: `Previews/Tintable_Palette_Test.png`, `Tintable_vs_r2_Blue.png`.

### r3 자체 검증 요약
| 항목 | 결과 |
|---|---|
| r2 파일 불변 | 29/29 SHA-256 일치. r2 NIF 15개와 PNG 7장은 패치된 생성기로 다시 만들어도 동일 |
| Neutral 텍스처 | 알파 r2와 동일(최대 차이 0), R=G=B, 화면 기여값 기준 r2 휘도와 최대 약 3/255 차이, 크기/밉맵 수 r2와 동일 (DXT5) |
| Tintable NIF 12개 | nifly 재로드 성공, r2 대응 파일과 색/텍스처 경로 외 전부 동일, 셰이더 기본 RGB 전부 (1,1,1), 메시마다 독립 셰이더 블록 |
| 청색 잔색 (Red/Gold/Green 틴트) | Neutral 0%, r2 청백 단순 곱은 마법진/폭발에서 최대 86% (밝은 픽셀 중 B가 R보다 큰 비율) |
| 흰 코어 유지 | 빔 중심선 평균 RGB (255,255,255) (5개 팔레트 모두) |
| 인게임 | **미검증** |



### 원문 부록: S03 파트분리/최적화/STOP권위
원본 SHA256: F966EA30C20FA0EB0A1E8B33D6AF685E8D6AF92305422F4C5D61EB389CE29431

# Ordinary Offensive Magic — S03 mixed NIF parts / 10-second visual preparation

**최종 불변성 gate: FAIL / STOP.** 완료된 NIF·구성 데이터·미디어의 STATIC QA PASS와 전체 작업 gate를 구분한다. 승인된 기준 이후 외부 변경 228건({'MO2': 26, 'CMakeLists.txt': 1, '_backup': 34, 'build': 71, 'evidence': 68, 'src': 28})이 관측되었다. evidence Markdown 14건은 사용자 지시에 따라 별도 기록했다. 기준을 재작성하거나 production을 복구·덮어쓰지 않았다. 추가 자산 생성/승격은 중단했으며, 완료 결과의 보고서만 정리했다. 상세: reports/V6_EXTERNAL_BUILD_STOP.json, reports/V6_READONLY_VERIFICATION.json, reports/CHATGPT_MAIN_HANDOFF_V6.md.

## Current result

**GENERATED / STATIC QA PASS / RUNTIME UNVERIFIED**. Prepared 3 reusable part types as plain/tintable pairs (6 new NIFs), a mixed T1–T8 composition sheet, a parts comparison sheet and an actual-NIF/DDS software motion GIF. All scale/spacing/growth/timing parameters are CANDIDATE VALUES ONLY. No production integration, deployment or gameplay/runtime code.

Sole output authority: [PROJECT_ROOT]\parallel\S03_NIF_ASSEMBLY.
Final NIF authority: out/Data/meshes/effects/ChargeCannon/Derived.
Reports authority: reports. Canonical candidate data: generator/magician_mixed.candidate.json.

## Immutable source / external authority

Source: [PROJECT_ROOT]\assets\ChargeCannon\r3.
ZIP: [PROJECT_ROOT]\assets\ChargeCannon\source\ChargeCannon_AssetPackage_r3.zip.
ZIP SHA256: C08823E523B3635808048A9DF0EA1ABB95C22D558D09C3F8C7BC82DB62F5265B.

Original r3 107 files and all original DDS remain unchanged. User approved an initial S02D evidence ZIP addition, then separately confirmed the observed 80 Main source/CMake/build/deploy changes and authorized the 1129-file state as the new external baseline. Details: PARTS_V3_RESUME_AUTHORITY.md and work/external-baseline-main-s02d-v4.json. Both earlier STOP records were preserved; no silent rebase or production rollback. Last explicitly approved baseline DLL is 578048 bytes / FDB530C7F9E8D5A7FC51B4698FD2462CFE42A330612BDED8187DDEF10CBD7395.

## New reusable parts

- Charge_Circle_OuterBand_Static (+ Tintable): CC_Root → CC_Outer + CC_Band; 3 nodes, 2 meshes/properties, 0 controllers.
- Charge_Circle_HebrewBand_Static (+ Tintable): CC_Root → CC_Band; 2 nodes, 1 mesh/property, 0 controllers.
- Charge_Circle_OuterRing_Static (+ Tintable): CC_Root → CC_Outer; 2 nodes, 1 mesh/property, 0 controllers.

All new parts omit Face/NodeGlow/Glow/Lens. Origin and +Y/XZ convention retained. Outer Y=0.1 and Band Y=0.2 remain original local offsets. Selected mesh geometry/UV/normals/material/alpha/emissive are unchanged from actual source NIF. Hebrew band geometry radius 150..172; full/outer nominal visible ring radius 190. No automatic diameter normalization and no new textures. Tintable uses original neutral DDS and white RGB, with per-mesh independent properties. Runtime must still establish instance-local mutable state before tinting.

## Mixed composition / timing

C1/C3/C5/C8 FULL, C2/C4/C6/C7 OUTER_BAND. Target scales: 1.00/1.35/0.70/1.55/1.05/0.65/1.40/0.90. C1 remains fixed at origin, scale 1.00. User-requested changes affect only visual template assignment; scales and the 10-second compression/growth timing are unchanged. Hebrew-only and ring-only parts remain prepared but are not placed in this candidate. T8 uses 28 geometry instances; this is a structural count, not measured runtime performance. No tier-specific NIF or monolithic production rig.

Reference tier times for 500 MP/s: 0.2/0.6/1.2/2.4/4/6/8/10 seconds. T2 spacing stays 96. T3 begins a single 8.8-second quintic compression to spacing 60 at 10 seconds. There is no per-tier curve reset. Peak spacing compression speed is 7.670455 units/s at 5.6 seconds; each already active Ci centre speed is (i−1) times spacing speed. Exact per-circle active speed table and formulas: MAGICIAN_10S_TIMING.md/.json. Exact scale/+Y data: MAGICIAN_TIER_COMPOSITION_MATRIX.md and MAGICIAN_TIER_PREVIEW_VALUES.json.

C2..C8 grow from 2% to their own target over candidate 0.24 seconds after their strict threshold; C1 stays fixed scale 1. Compression ends at 10.00s, C8 growth finishes at 10.24s. This visual tail does not delay gameplay firing. S02 source has 500 MP/s per hand; two fully serviced hands nominally reach T8 in 5s. This task uses the requested 10s visual reference and does not alter drain, tiers or charge authority. Variable-rate current-position/velocity retarget belongs to main runtime and remains unimplemented.

## Preview evidence

- MAGICIAN_T1_T8_COMPARISON.png: FRONT/XZ, OBLIQUE 45°/8° elevation, SIDE 78°. Full-size inspection at tier arrival spacing; growth omitted and disclosed.
- CIRCLE_PARTS_COMPARISON.png: outer+band / band only / outer ring only from actual generated plain NIFs.
- MAGICIAN_10S_COMPRESSION_CANDIDATE.gif: 20fps source samples, 0..11s timeline with final hold/growth tail. Actual NIF vertex/UV/DDS software additive render. SIDE contour guides are annotations.
- MAGICIAN_TIER_TRANSITIONS_HALF_SPEED.gif: same frames at 0.5x playback for tier-transition inspection; reference timestamps remain unchanged. SIMULATION_REVIEW.md/.json records continuity measurements.

The original full Static template includes 3 emissive pulse controllers; software preview ignores them. Cameras and exposure are fixed across tiers. HDR, material falloff, sorting/culling, actual rotation, shader instance isolation, attachment, lifetime and performance are not simulated or runtime-verified.

## Final binary list / hashes

Existing EndFade/emitter Common 4 binaries remain unchanged. Old monolithic Magician rig 2 binaries remain **REJECTED_CANDIDATE / DO NOT PROMOTE** and were not regenerated. New 6 parts are current reusable asset candidates. No file has been promoted to production.

| File | Bytes | SHA256 | Review |
|---|---:|---|---|
| out/Data/meshes/effects/ChargeCannon/Derived/Magician/Magician_ChargeRig_8Circle_Static.nif | 106689 | 98A2FC77D2945C6A856C0622E4B8B9D79828F5138AC845BCEFE94A3DD4BA2938 | REJECTED_CANDIDATE |
| out/Data/meshes/effects/ChargeCannon/Derived/Magician/Magician_ChargeRig_8Circle_Static_Tintable.nif | 107058 | 4980FADC50BD4539C382180DB5C2BC878F87F9C127B5165B8EE44663A4761B73 | REJECTED_CANDIDATE |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Beam_EndFade.nif | 3383 | 463FBB711CCCC48C93F56161050A59CEFA12178CE4125F35B1529A7D0602A5FF | KEPT_UNCHANGED |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Beam_EndFade_Tintable.nif | 3419 | 31D98A3391E4EE8A46CC5ABB08BE42FCAE62F5D69BFAEEE31EE868582322C0A5 | KEPT_UNCHANGED |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Emitter_Circle_Small_Static.nif | 1599 | AD0AA794402D17A3C0DBB214A484C80AF954F02D56765A24441157EEF8624272 | KEPT_UNCHANGED |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Emitter_Circle_Small_Static_Tintable.nif | 1617 | B883820D307DB7B7121DE6F044E3B18285BA5809CF73174D4022DCC995EE0E95 | KEPT_UNCHANGED |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Charge_Circle_OuterBand_Static.nif | 9951 | ED296C4CF8ED68095CD691199A3BAA6A5BCA097F6F0EC536A6E5AAB68540BE41 | CURRENT_REUSABLE_PART_CANDIDATE |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Charge_Circle_OuterBand_Static_Tintable.nif | 9969 | B15BF4432BD3D46F786011E4D130FB05DC54349FFC2AD7EBA04596C42D639B54 | CURRENT_REUSABLE_PART_CANDIDATE |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Charge_Circle_HebrewBand_Static.nif | 9417 | 75139F71CCAFC928570372A8275F1A45FB4D3B2258C57E4FCFD5351A62A071A6 | CURRENT_REUSABLE_PART_CANDIDATE |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Charge_Circle_HebrewBand_Static_Tintable.nif | 9426 | DC38D8A74A8F338E7C427035FC4078ABFA59AD9EBFD6B34187A1E656853B806A | CURRENT_REUSABLE_PART_CANDIDATE |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Charge_Circle_OuterRing_Static.nif | 784 | E94E2F98561C983CDC28C7725DFD5FDAAF9FDF9152AD3D3558432A0F8BD8E91A | CURRENT_REUSABLE_PART_CANDIDATE |
| out/Data/meshes/effects/ChargeCannon/Derived/Common/Charge_Circle_OuterRing_Static_Tintable.nif | 793 | 2BCC2D8F7B73F9043E01D0F7E9865914B52379F2CB826E48AE08F75A47C147E9 | CURRENT_REUSABLE_PART_CANDIDATE |

## Generator / source provenance and reproduction

Copied inputs: generator/components/Charge_Circle_Static.nifspec and Charge_Circle_Static_Tintable.nifspec (originals at Source/nifspec/Charge_Circle_Static.nifspec and Source/nifspec/Tintable/Charge_Circle_Static_Tintable.nifspec under immutable r3). Required source Docs README/ASSET_COORDINATE_SPEC/NODE_MAP/TINTABLE_GUIDE/TINTABLE_QA/R2_UNCHANGED, manifest, actual gen_nifs.py/nifbuild.cpp and native NIF/DDS were inspected in this work chain. Source authority details remain in SOURCE_AUTHORITY.json and SOURCE_R3_NATIVE_AUDIT.txt.

New/adapted tools: generator/gen_circle_parts.py, mixed_composition.py, render_mixed_composition.py, finalize_mixed_parts.py, magician_mixed.candidate.json; gen_nifs.py/previews.py route to current entries. Prior v2 finalizer/renderer entry points are blocked from overwriting current outputs. V2/v3/v4/v5 candidates are archived under reports/candidate_history/v2_full_circle, v3_outer_parts_heavy, v4_full_147 and v5_full_13678. Earlier rejected rig history remains under reports/rejected_candidates/r1. Current exposure remains fixed at 0.22 across tiers/views; no asset material/alpha was changed. This V6 revision changes config/preview/documents only and does not regenerate the six parts.

Existing native nifbuild/nifdump tools and pinned Nifly commit cca0a770094bb962fb28ea1fec5ea903e68fda8e were reused. Actual vendor LICENSE is GPL-3.0. No compiler rebuild or production CMake change. Source specs are selected programmatically; final binary files were never manually edited.

Reproduce parts without overwriting finals: python -B generator/gen_circle_parts.py --repro. Preview: python -B generator/render_mixed_composition.py. Slower playback/continuity review: python -B generator/make_review_playback.py. QA/handoff: python -B generator/finalize_mixed_parts.py. Use bundled Python with Pillow/NumPy; TMP/TEMP=work/tmp. Intermediate specs/repro outputs are work/parts_specs and work/parts_repro. Initial half-speed palette requantization caused small RGB differences; exact RGB-to-index conversion corrected them. Diagnostic retained in HALF_SPEED_INITIAL_PALETTE_DIAGNOSTIC.log; decoded frame equality was rerun.

## QA / immutable gate / limits

6 new parts independently built twice with identical hashes; native parse/reload and source provenance passed. 3 plain/tintable pair comparisons passed. All 12 final NIFs freshly reloaded and checked for format, roots/named children/socket, references, property uniqueness, controllers, finite transforms, valid scales and texture paths. Existing 6 binaries and Common preview remained unchanged. Motion math/data/framing and GIF reload verified. Details: DERIVED_NIF_QA.md, PARTS_V3_QA.json, PREVIEW_METHOD.json, DERIVED_NIF_MANIFEST.json.

Original r3 107 files/ZIP and existing 12 derived NIFs match historical bytes. 외부 src/CMake/build/DLL/MO2 및 backup/evidence 변경 때문에 전체 gate는 FAIL / STOP이다. Codex는 production과 기존 문서를 수정하지 않았다. All task-created persistent output remains under S03_NIF_ASSEMBLY. Current inventory and task deltas: TASK_OUTPUT_INVENTORY.json / PARTS_V3_OUTPUTS.json. 별도 ChatGPT Main 인계: reports/CHATGPT_MAIN_HANDOFF_V6.md. Codex 변경 기록: reports/CODEX_V6_CHANGELOG.md. External global filesystem monitoring is not claimed.

Warnings/unresolved: 10s is nominal one-hand reference; dual-hand/rate changes require runtime mapping. C8 strict appearance has a 0.24s visual tail. Source FULL emissive pulse policy and runtime instance-local shader cloning require main review. No native Skyrim validation, CommonLib API assumptions or runtime implementation. Earlier kept-asset reproduction/provenance evidence is reused and distinguished from fresh new-part QA. An initial provenance-check path error for the original Tintable spec was corrected to the observed Source/nifspec/Tintable directory; diagnostic retained in CIRCLE_PARTS_INITIAL_QA_PATH_DIAGNOSTIC.log and QA rerun.

## Main-development review / deferred scope

Review the mixed proportions, continuous compression pacing, appearance duration and six reusable parts before promotion. Runtime loading/spawn/tint/transforms/charge/tier switching/collision/damage/targeting/cleanup/ESP/DLL/deployment remain main-development authority. No baked Red/Purple/Gold/Green families, tier-specific beam/impact, fixed Judgement/Temperance world formations, The World family or gameplay collision were created. No persistent circle+auto muzzle merge. EndFade remains a small spatial cap; emitter retains independent FIRE_ORIGIN centre socket and separate runtime muzzle.

## 프로젝트 책임자 검토 질문 3가지

1. Full/outer+band/band-only/ring-only의 현재 혼합 비율이 시각 의도에 부합하는가?
2. T3부터 10초까지 연속 압축되는 GIF의 속도와 깊이 변화가 적절한가?
3. T8 도달 직후 C8의 0.24초 성장 tail을 유지할지, 더 짧게 조정할지?



### 원문 부록: 비행·모션 출처
원본 SHA256: FF9A706371E882647D13FCE39D0A0965359F26E1573471863376ED4F27DC6A19

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



## CP029 확정 밸런스 표
# Arcana balance — v0.8.11
All costs below are BEFORE vanilla Destruction reductions. Staffs use corresponding charge units and zero player Magicka. Cost and damage have separate owners; discount never multiplies damage.

| Card | Base resource cost | Base damage |
|---|---|---|
| Emperor |105/shot|55/shot|
| Hermit |132/second|Original defense, up to80% reduction;120 release burst|
| Chariot |300/shot;1500 per five|64/shot;320 per five|
| Sun |4650 gradually over8seconds|3040 maximum|
| Strength |750/second|195/second|
| Justice |1500/placement|110 every0.2seconds while aimed, original3second lifetime; no five-contact cap|
| Star |480activation+150/deployed circle/second;max750/second|100 combined nominal/second at full deployment|
| Tower |4800/activation|150/beam,900finale;27900/target/cast cap|

Chariot's five emitters retain independent original periods(.36/.39/.42/.45/.40s); five shots are an accounting total, not a new synchronized burst or five-shot limit. Emperor interval.20s. Justice's alignment, line-of-sight, living-enemy checks and original maximum3 installed emitters remain. Three seconds is measured from placement, so delayed readiness/turning reduces actual contact time. Tower retains180 distributed shots; its cap is a ceiling, not guaranteed damage. Existing resistance, difficulty and hit policies remain.
Latest user direction expressly changes Hermit's cost from44 to132; its defensive code, coverage, timing and burst damage remain unchanged. Previous research saying Hermit resource values are immutable is historical.


## 아트 생성·수정 코드의 증거 위치
ArtTools 폴더에는검토용역사생성기/수정기와원문SHA를보존한다.과거절대경로는비식별화했으므로즉시실행용프로젝트가아니다.원저작물/vanilla입력/외부nifly는각권리조건에맞게별도확보한다.


## CP029 R2 — Arcana 비용 감소80% 상한과 공개 준비 교훈
최신 사용자 지시는 일반·스탠스 Arcana와 지팡이 충전 소모 모두 최대80% 감소,최소20% 지불이다. 이전 본문과 역사 부록의100%무료 허용은 이 범위에서 대체되었다. TOAM 네 주문/리미터는 변경하지 않는다. 공중부양의원래0비용은공격8종기본비용과다르며0에비용을새로만들지않는다.
ResourceFactor는정상유효한엔진할인율을받아하한.20만적용한다. 저주등으로1보다큰비용배율은유지하고NaN/inf/음수는거부한다. Native입장검사와매발/유지/충전/스탠스·지팡이결제는같은함수를쓴다. Hermit일반주문은Native차감이없으므로기존엔진currentSpellCost를clamp한값으로세션초기화/엔진Update전후에설정한다. 스탠스Hermit세션도같은하한을갖고기존snapshot정책을유지한다. 별도MP차감을추가하면이중결제가되므로금지한다.100%장비에서도시작입장검사가무료로통과하지않도록실제최소비용을검사한다.
최소비용:황제21/발,힘150/초,별96발동+진당30/초(5진150/초),전차60/발·300/5발,정의300/설치,은둔자26.4/초,태양930/완충,탑960/발동. 비용의소수부를임의정수반올림하지않으며피해는그대로다. 실제로그/AV·충전량과바닐라메뉴추정표시를구분한다. 이Native정책이변경되지않은MagicMenu비용표시까지후킹했다고주장하지않는다.
정적회귀는실제CapArcanaCostFactor함수에서0/5/20/30/40/100%잔여와비용증가,음수/NaN/inf를검사한다. 실제엔진결제와손/스탠스/매지카보호는사용자게임QA에남는다.
소스준비에서PowerShell의--pathspec-from-file=(식)인수를조립하다경로가별도positional인수로넘어가Git이저장소밖경로로거부했다. 스테이징실패로commit/upload전중단됐으며Production소스변경은없었다. 옵션전체를문자열변수로먼저만들어전달해정확183파일만스테이징했다. 비공개API상태·원격head/tree·각blob SHA와로컬을검증하고공개는Nexus동시출시때까지보류한다. 개인정보/비속어는제외하며라이선스를임의로붙이지않는다.


## CP030 — Common damage,weighted world distribution and manual layout research

The user authorizes one common damage-only multiplier,default1.0,INI range0.01..100,and an optional SKSE Menu Framework3.xpage. Protected:ALL Temperance including Proc/halo/discount,Limiter,Hermit defense,MP/staff charge,80%maximum reduction,cast thresholds,cadence,shot counts and spatial/VFX policies. TOAM Profiles/Charge/FixedDamage and Arcana Design/Balance/RainImpact retain original bytes. Damage scales projectile power or the existing explicit targeted magnitude exactly once;there is no shared SPEL/MGEF mutation,no extra damage hook and no global scaling of foreign damage.

Arcana Hand snapshots the factor. Missiles carry power;detached Sun and installed Justice retain their own factors;Hermit only scales offensive burst;Tower finale retains the Hand factor. Tower's unchanged ledger counts original nominal units,so scaling the delivered damage yields27900×factor while keeping original resistance and per-target/generation policy. TOAM Magician/World snapshot at release;Judgement stores a separate configured factor in each artillery schedule. The original charge-start cost/limiter multiplier and1fixed marking are untouched.

Optional menu uses existing AddSectionItem and six audited C-ABI functions,not GetMainWindow or a new renderer/context/hotkey. Complete export gating precedes registration. The installed older3.0and active3.18Korean DLLs expose the required functions;actual menu pixels are untested. No Framework DLL is replaced/bundled. Atomic<float> isolates render-thread setting edits from game damage reads;INI save/readback must succeed before publishing the new value. Reload rejects invalid input and leaves the old value. Windows cache-flush WritePrivateProfileStringW(null,null,null,path) returned0 in the real temporary-file probe even after successful key write;do not classify that return alone as failure. Verify the key readback. This was caught in candidate validation before Production writes.

Magicka Blade's actual release uses TULL_MagickaBlade_CID.ini and merchant containers,not NPC combat-spell distribution. TOAM reuses CID's documented container/list/location behavior. SPID inventory gifts are not treated as proof of merchant sale stock. SkyPatcher's installed container-LVLI setting is disabled;it is not changed. Five new main LVLI and six addon LVLI select one weighted item per success,using ChanceNone and nested gates for sub-percent rare stance loot. No UseAll flag,no vanilla container override,no custom actor scan,no save reset and no dependency DLL replacement.

Base books cost5..50gold,stance35. All normal/stanced staffs are included in separate pools,with original20000/40000charge,price and enchantments.140 actual vanilla/DLC VENC merchant references;232 broad container bases restricted to actual LocTypeDungeon. These counts establish static target coverage,not immediate stock/reset or third-party merchant coverage. Exact probabilities,conditional card weights,prices,FormIDs and plugin hashes are retained in distribution docs/JSON and Nexus handoff.

### Manual placement editor and original hand-edited coordinates
The source package now includes tools/layout_editor/index.html,baseline.json,original README,manual-layout-20261023-0145.json,current-runtime-layout.json andPROVENANCE. The preserved user export's UTC savedAt converts to2026-10-23 01:45KST. This records deliberate manual XYZ authoring and6independent dual-hand positions in addition to29base circles. Front/side/oblique projections,drag/numeric edits,undo/redo,reference overlays,JSON round-trip/validation andlocalStorage formed the original tool workflow.

The historical editor identifies0.6.6and its source hashes;the Native source comes through0.7.4and TOAM refinements. Preserve both;never silently call the old editor baseline the current runtime. Current LayoutData arrays and owning source hashes are exported separately. Runtime front offsets,Sun layer compression,ground/flight transforms,World endpoints andJudgement/Temperance target spaces must be reviewed together. Copying rawXYZ without ownership/transforms can recreate the earlier aim-following,world/local and endpoint mismatch defects.

The tool is self-contained and does not modify game files. Public research retains exact original generated HTML and manual JSON bytes,relative provenance labels and the current numeric snapshot. Original machine-path author scripts/reference screenshot imports are not blindly published. The user's manual design work is explicitly credited as development input alongside AI-assisted editor/source integration. Existing image-derived circle/staff/book authoring,mesh optimisation,thin-rim/UV/DDS corrections and failed SMP experiments remain in the same unified compendium.

Static parser/finite-boundary C++policy tests and staged source syntax checks passed. These are not runtime claims. Full build/deploy/008/package and private GitHub results are recorded separately in CP030 CURRENT_RESULT. Menu persistence,real multipliedHP,merchant stock,dungeon loot and reset duplication require one consolidated user QA.Original007 A-J158 remains unchanged.


## CP031 —1.0 presentation,flight menu,distribution and reusable formation editor
User confirms the delivered0.8.12 damage-adjustment/Menu Framework feature and NPC merchant sales. Preserve that observed feature PASS;do not claim all158 QA cases or new1.0 runtime. New presentation/config/distribution deltas require focused game QA.

One existing optional menu now has EN/KR preference and a Flight page. Render-thread code validates/writes/readbacks a complete Flight section and only queues an atomic reload flag;the existing game-frame owner applies it. No new hook,actor scan,controller write from UI or Framework binary/font replacement. Existing FlightLoadSettings bounds/defaults and spell/resource ownership remain. UTF8compiler/API exports are verified.
Sun/Magician share8 signed nonuniform layer rates with tier acceleration. Phase is integrated,not reconstructed as currentSpeed×elapsed,so tier changes do not cause angle jumps. TOAM Original/Dome circles and Arcana Hermit gain a short faster reveal spin. Rotation is about the authored+Y normal,leaving source positions/outward collision axes unchanged. Sun/Halo avoid double rotation owners. Magician transfers fired circles to bounded.60s retirement;other existing tails are retained. Cancels/load/cell/death still purge pending/visible owners. Halo.12→.65rad/s;two private halo/ascent models change only baseColorScale×1.65,with every other raw byte preserved. No wall geometry,alpha or texture-path rewrite.
Shop coverage uses143 merchant-specific CONT bases plus6remaining direct refs;replaces134 old refs without overlapping bases/refs. This covers all old140 refs and more reused stock types,including College/inn/spouse/general/food/caravan stores.232 dungeon bases retain location filtering and treasure/boss/ordinary chests. Rates55/15/10%shop,1/.2%stance shop;10/3%dungeon,.1/.02%stance dungeon. Normal WEAP base gold10,Unbound15;ENCH/payment/20000/40000unchanged. Limiter enchantment names exclude Ring;brief DNAM is Ordinary Attack Magic Arcana Limiter / 일반공격마법 술식출력제한. User package excludes Docs and retains short root README/CREDITS;detailed provenance/research stays in source/handoff. Existing installed KR selection,optional-addon omission and personalized INI serialization are legitimate configuration differences,not corruption.

### Reusable editor architecture / 범용 편집기 연구 자산
Original tools/layout_editor HTML/manual export remain preserved. New tools/formation_editor is self-contained EN/KR offline source:engine.js contains pure tested transforms/schema,app.js manages canvas/UI/undo/storage,presets.js avoids file://fetch/CORS and carries reviewed numeric presets. Hierarchy:named library→multiple spells→independent casting variants→assets. Each asset stores explicit hand,actor/target/world anchor,position,base normal,XYZ rotation,scale,source radius/size and group. Counts can be added/deleted per variant;they do not mutate an installed mod's gameplay limits. Named local sets plus versioned JSON exports preserve all spells and variants.
Actual pointer tests prove marquee/group drag and count/rotation edits. Group movement rebases a common WORLD delta into each anchor's local frame. Reanchor/freeze preserves the complete orientation using a basis matrix and RzRyRx decomposition,not just the normal;80random roundtrip tests prevent roll loss. Source preview radius is explicit;rendering-parent inverse scale/rotation remains an integration boundary. Do not confuse feet,engine bounds center and a height/2preview approximation.
Presets retain source hashes and exact imported/manual values,Sun full-charge gap and scales,independent secondary hand coordinates,World ground .81transform and full airborne12-direction sphere including below-feet coordinates,normalized target9-point shell and its radius caveat. Back-swords example demonstrates future event ownership:light-hit create actor-local visuals;heavy attack snapshot world poses/live target then launch;filtering,projectiles,damage,resource and cleanup belong to the future adapter. This is not new TOAM gameplay.
Independent source ZIP/manuals/schema/Node tests can be published separately without Skyrim assets;full private GitHub also includes it and the unified research. Browser/headless math/JSON/UI PASS does not imply Skyrim pixels. Public license choice and imported game-art permission remain separate publication decisions.

### Failure prevention
Initial parent deployment identity rejected user-applied shipped KR root ESP,omitted optional addon and1.000000INI serialization;classify exact allowed configuration instead of forcing EN/optional files back. A reused historical parser preamble reassigned the stage S and misrouted3new evidence records;no Production writes occurred. Move only the verified new files into the actual stage and import AST function definitions in a dedicated namespace thereafter. Preserve failure records and verify actual file paths before calling a phase PASS.

Editor UX supplement:mouse and position/rotation/scale sliders are primary;precise numeric/normal/dimension fields are folded. Named-set hierarchy and independent hand/count variants are preserved in one versioned JSON library.

AI/developer coordinate references: tools/formation_editor/examples/ai-coordinate-examples.json (direct import) and ai-reference-guide.json (machine-readable documentation); actual TOAM source-backed values remain presets/toam-reviewed.json with source hashes.


---
## CP032 /v1.0.0 — static staff cloth orientation correction
# CP032 /TOAM1.0.0 — 프리렌 지팡이 정적 천 장식 교정
작성:2026-10-27T04:52:18.253150+09:00
PARENT=CP0311.0.0. 사용자 실게임 핵심23/23PASS;설정/저장복원/회전가속퇴장/절제/비행/판매·던전/연속시전,관찰된 CTD·프리징 없음. 원158문항 전체 완료로 확대하지 않는다. 이번 천 교정의 RUNTIME=NOT_RUN.

## 증상과 확인 근거
제공된04-06-23영상22~36초 근접 관찰에서 리본이 각도에 따라 사라지고 단면만 보이거나 장식이 분리되어 보이는 현상을 확인했다. 영상만으로 원인을 확정하지 않고 현재 활성 설치본과 기하를 대조했다.04-09-12는전투참고이며 본 교정의 피해/연출 변경 근거가 아니다. 실제 OrdinaryAttackMagic.log1297행/error0/warning16과 일치한다. 이 로그는 물리 성공/형상 정상의 증거가 아니다.
실제 MO2 선택 프로필의 활성 본체는`TULL Ordinary Attack Magic - v1.0.0`,애드온은같은v1.0.0별도폴더다. 이전`TULL Ordinary Attack Magic`폴더와 SMP옵션은 비활성이다. 활성 본체는 CP031 MainZIP932파일과 완전히 일치했다. 구폴더를 실제Production으로 잘못 수정하지 않는다. 프로필/플러그인/세이브는 읽기 전용이다.

## 확인된 원인
Gold/MillennialStaff의`10_Hanging_Red_Ribbon`,BSTriShape block40.300정점/596삼각형의 닫힌 메시지만 앞·뒷면 인덱스 방향을 같게 작성했다. 용접하지 않은 원본 그래프에서도 비다양체/열린모서리는0이나 공유모서리152개가 같은 방향으로 중복 진행한다. 앞/뒤투영면적52.0870/0.0으로,기본single-sided재질이 뒷면을 제거한다. 법선-면 평균 방향 충돌42개도 검출됐다. 누락텍스처나알파문제가 아니다:Gold cloth DDS1024²/11밉,Silver2048²/12밉,디퓨즈알파255,경로와정규맵정상이다.
원본 Arcana0.7.4 FlightStateFix `source/frieren/FrierenStaff.spec`의 리본 위치와 삼각형이 현재CP031와 일치한다. 결함은 SMP시험 이전의 정적 원본에 존재한다. 기본4NIF는SkinInstance없음/추가뼈없음이고,활성프로필의SMP는비활성이다. 이번 결함을 과거SMP solver활성 실패와 혼동하지 않는다.

## 최소 교정
인접삼각형 그래프의방향 제약을 전파하고 닫힌요소의양의부호체적으로 바깥을선택했다.296개삼각형방향을뒤집고300개정점의면적가중법선과UV기반접선/종법선을재계산했다. 공유모서리방향충돌152→0,법선충돌42→0,투영면적26.0435/26.0435. 접선프레임최대내적0.00799653,양자화후길이0.995206..1.004496. 독립nifly Load/IsValid재읽기와재적용동일바이트를확인했다.
삼각형수·정점수·모든위치·UV·스케일·트랜스폼은보존했다. 리본인덱스와법선/접선필드외모든바이트가동일하다. 각NIF2582바이트만달라진다. 셰이더double-sided를켜서가리지않았으며재질·텍스처·경로·BSX·INV/Prn·충돌·헤더·노드·다른13파트·보석/태슬은보존한다. NIF전체재직렬화/경로정규화를하지않는다.

## 페른/Silver 비교
`07_Hanging_Purple_Ribbon`:1153정점/1816삼각형,UV이음새용접후방향충돌0/열린모서리0/비다양체0/법선충돌0,양면투영58.364/58.364. 하단/상단보라감개도방향충돌0이다. 상단감개의고곡률4소면에서정점평균법선과면방향차이가검출되지만동일한인덱스뒤집힘결함은없다. 관찰된페른리본결함근거가없으므로Silver두NIF는전바이트보존했다. 일반적인면평균법선차이를모두토폴로지손상으로단정하지않는다.

## 파일 및 SHA256
|파일|Bytes|변경전|변경후|
|---|---:|---|---|
|meshes/TOAM/Staffs/Arcana_Staff_Gold.nif|487385|09E6C74319F4C7132EA3E962FFDEA85B8BBB7009D801D7CB722AF8BC03BC0196|D376232AAB2952D0133D71B5CB2CEB7B32F4A669E1C10955AD3198F677923F2C|
|meshes/TOAM/Staffs/Arcana_Staff_Gold_FirstPerson.nif|487385|09E6C74319F4C7132EA3E962FFDEA85B8BBB7009D801D7CB722AF8BC03BC0196|D376232AAB2952D0133D71B5CB2CEB7B32F4A669E1C10955AD3198F677923F2C|
각파일은assets/TOAM/Data와실제활성MO2본체에적용한다. 텍스처수정0,Native소스수정0,ESP수정0,XML/Helper/Papyrus추가0. 배포/008최종해시는외부CURRENT_RESULT에서확인한다.

## 참조 영향 범위
Gold일반모델은3인칭·인벤토리·드롭월드공용,1인칭은기존STAT01000A74의Gold_FirstPerson별도참조다. 두NIF는부모단계부터동일바이트였으나두물리경로모두교정한다. 공용모델을쓰는힘/정의/별/탑/달(해방)5지팡이에같이적용된다. 모든WEAP/ENCH/WNAM/MASTER/FormID/가격·충전20000/40000·CID확률은변경하지않는다.
- Arcana - Strength Staff:WEAP0x1000a35,WNAM0x1000a74,ENCH0x1000a25.
- Arcana - Justice Staff:WEAP0x1000a36,WNAM0x1000a74,ENCH0x1000a26.
- Arcana - Star Staff:WEAP0x1000a37,WNAM0x1000a74,ENCH0x1000a27.
- Arcana - Tower Staff:WEAP0x1000a38,WNAM0x1000a74,ENCH0x1000a28.
- Arcana - Moon Staff (Unbound):WEAP0x3000831,WNAM0x1000a74,ENCH0x3000811.

## 최소 사용자 QA
1. 프리렌형 일반/해방 중 사용하던 지팡이를3인칭으로 장착하고정면·뒷면·양측을회전관찰하여리본연속성/본체관통/장식분리를확인한다. 페른은원본유지비교대상이다.
2.1인칭전환·장착해제·재장착·기존시전에서리본/본체표시를확인한다. 전투23항목을다시전부요구하지않는다.
3. 인벤토리회전과월드드롭/재습득에서양면표시·원래충돌을확인한다. 실제게임을자동실행하거나세이브를수정하지않는다.

## 판정과 재사용 교훈
STATIC=PASS;BUILD=NOT_REQUIRED_ASSET_ONLY(parentCP031DLL유지);DEPLOY는트랜잭션결과로판정;CP031USER_RUNTIME=CORE23/23PASS;CP032CLOTH_RUNTIME=NOT_RUN;LOCK=NO. 정적면방향교정은중력/바람물리재도입이아니다.
정상로드/정점수/닫힌모서리만으로렌더가능한양면을검증할수없다. 인접모서리방향·부호체적·앞/뒤투영면적·노멀프레임을별도로검사한다. 정상인Silver를Gold와같이무차별뒤집지않는다. 실제활성MO2폴더를다시찾고바이트를대조한다. 원래모델의확인된결함과SMP활성실패는별개다.
후보도구실패:독립C++리더의external/half.hpp include누락을실제헤더위치로교정했고,새PowerShell호출에서경로변수를재정의하지않은호출과Python가변커서증가식(p+=4*val(...))의평가순서오류,NumPy가bytes를숫자배열로해석하지못한검사오류를수정했다. 모두후보/검사단계의실패이며Production변경0이었다. 부정확한검사결과를모델손상으로해석하지않는다. 실패증거와성공후보는같은Stage에보존한다.
오프라인4방향투영그림은구조진단이며게임셰이더·가시성PASS가아니다. 수정후새런타임결과는사용자실측으로만갱신한다.


## CP033 / TOAM1.0.1 — integrated presentation candidate
Parent is immutable v1.0.0/CP032(actual Registry parentCP031). Preserve core23/23USER_PASS;new1.0.1 USER_RUNTIME NOT_RUN,LOCKNO,SE1.5.97 only. Existing build/msvc Hostx64/x64 Ninja only;full mutable cache backup/rollback. New MO2 v1.0.1 folder only;v1.0.0/CP032 archives/source checkout/mod/profile bytes protected.
Stable catalog18files/14SNDR/17actualevents preserves two audio backends and owned handles,cadence,caps,Gain/fades. Source IDs are append-only;incompatibleIDs fall back. Volume0..1 avoids unsupported amplification. Camera0..3 uses actual existing requests;native attenuation then clamp0..1 means high values can saturate.
Circle audit23NIF/116effectproperties/9DDS;four corresponding decoded alpha planes match. Default early return preserves exact original art. Nondefault reuses existing Arcana Neutral DDS on private property/material clones,one bounded application per ready-root with per-cast snapshots. Actual SE clone/material implementations were disassembled instead of trusting SDK comments. Shared/unknown ownership fails closed;partial assignment restores originals. Live instance independence/pixels remain USER_RUNTIME questions,not static acceptance.
Lessons applied:S03-R11 scalar audio ABI and copied fading ID retained;CP023 raw path closure checks retained;CP028 carrier semantic regression gates retained;CP031 current-active-MO2 identity and exact validators retained. Current candidate failures:incorrect roadmap basename rejected by exists before Production;enumerated exact current basename;Windows cp949 default read rejected UTF8 Korean before writes;explicit UTF8 retry;LF/CRLF anchor mismatch,ambiguous Temperance anchor,wrong syntax-runner source replacement,VS quoting and host-test link dependencies. These were candidate/preflight failures with no Production writes;successful retry validated actual candidate paths. Actual policy1922 and actual Win32 settingsIO27 checks PASS;host stubs cover no game/GPU calls. ExistingFlightHum zero-start must allocate for original fade;initial configured-mute consumes event without endless frame retries. Alpha controllers must be private and target their cloned property. No synthetic FPS or automatic user QA.
Reports:docs/AUDIO_CATALOG,docs/CIRCLE_MATERIAL_AUDIT,docs/COLOR_PRESET_MAPPING,docs/PRESENTATION_SETTINGS_EN_KR,docs/CP033_ONE_BOOT_QA. Final external artifact/008/catalog/build/package hashes and private GitHub sync status:evidence/STAGE_EVIDENCE/QA1_CP033_V101_PRESENTATION/CURRENT_RESULT.json. Publication remains private;no Nexus upload. Existing Formation Editor/generators/AI sample coordinates and original007/158 are unchanged.

See accompanying AUDIO_CATALOG and CIRCLE_MATERIAL_AUDIT for the complete fixed tables and byte evidence.

CP033 recovery R2:the first actual19.61s MSVC build linked AMD64 TOAM.dll SHA6D4C5CFA7D2E3FB9EB44DF2A0AC0133DE03E8F7FCF4B2826CF0B85729D457186. POSTVERIFY wrongly compared the intentionally updated008 to its old guard because derived008/index were absent from the expected-after map. This is a validator regression,not a C++/engine failure. All touched targets and673 mutable Ninja files were restored and hash-verified;all2331 anchor guards match baseline. Correct expected-after metadata includes current008/index and every actual changed Ninja artifact;reuse verified original backups. A new build is required after the coherent Ninja rollback. Preserve the failure and actual success separately.

CP033 final boundary refinement:oversized or non-ASCII AudioSelect values now restore that event default just like invalid numeric IDs,instead of rejecting the whole settings reload. Other malformed volume/camera/color still retain previous valid settings. Actual candidate Win32 host test extended27->31 assertions and passed,including long numeric and Korean IDs. Initial host-test retry exposed ambiguous DirectX Color from forced PCH;fully qualified TOAM::Presentation::Color corrected the test only. No engine/GPU invocation or gameplay change. Final artifact hashes are in CURRENT_RESULT.json.

CP033 final material ownership refinement:actual pinned ModelReferenceEffect owns artObject3D through NiPointer at+C8. Root references must equal that effect owner plus exactly one verified parent child reference when attached,parent child scan is bounded256,and each intermediate NiNode must have one reference. Geometry alone cannot exclude a shared ancestor. These checks precede property/material clone/assignment;extra temporary/cache/active ownership rejects that instance and preserves original appearance. Default remains an immediate return. No model/texture/alpha/geometry/timing change. Live simultaneous engine behavior remains USER_RUNTIME NOT_RUN.
