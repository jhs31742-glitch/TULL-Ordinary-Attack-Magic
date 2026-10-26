# JSON contract / JSON 구조
Library schema: TULL_FORMATION_LIBRARY_1. Project schema: TULL_FORMATION_PROJECT_1.
- library.name: named preset set / 세트 이름.
- library.spells[]: {id,name,project}; activeSpell selects one / 마법 목록·현재 마법.
- project.anchors.actor/target/world: {position:[x,y,z],offset:[x,y,z],yaw:degrees,height:gameUnits,pivot:feet|center}.
- project.formations: key → {name,assets:[]}; activeFormation selects one cast-context variant. Counts are independent.
- asset: {id,name,kind:circle|sword|marker,hand:left|right|shared,anchor:actor|target|world,position:[x,y,z],normal:[x,y,z],rotation:[x,y,z],scale,radius,size:[x,y,z],group,attachment:follow|snapshot,modelPath}.
Names may be a string or {en,kr}. Model paths are authoring labels. No file/URL is fetched. IDs are unique inside a variant. Hand labels do not automatically create or mirror assets. Variant keys are developer-chosen context identifiers, not engine FormIDs.
이름은 문자열 또는{en,kr}입니다. 모델 경로는 기록용이며 자동 로드하지 않습니다. 손 표시는 자동 복제·대칭을 실행하지 않습니다. 시전 변형 키는 개발자가 선택하는 문맥 이름이며 FormID가 아닙니다.

Bounds: at most64 spells,16 variants/project,512 assets/variant and4096 assets/project. Imported JSON is limited to8MiB. Coordinates/angles/dimensions must be finite and bounded;scale .001..100;dimensions/radius positive;normal nonzero. Invalid imports/edits retain the prior workspace.

Transform authority: BaseNormalFrame*Rz*Ry*Rx. Source circle lies on localXZ with+Y normal. Refer to the tested engine implementation for Skyrim yaw and anchor rebasing. Target-center pivot's height/2 is a preview approximation, not proof of valid ActorBounds.

Recommended adapter sequence / 권장 연동 순서:
1. Resolve cast context → pick spell and one formation.
2. Resolve actual actor/target frame and source asset bounds.
3. Evaluate local→world pose once; snapshot when firing if required.
4. Apply inverse rendering-parent transform after valid3D/pose readiness.
5. Keep event,resource,damage,render and lifetime owners explicit.
6. Cancel pending requests and retire visible effects without ghost reappearance.

Backward compatibility: unknown schema versions are rejected. The preserved original manual editor JSON is reference data;use its converted presets here or the original editor for direct historical import. Do not silently pretend schemas match.
