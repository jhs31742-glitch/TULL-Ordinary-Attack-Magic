# TULL Ordinary Attack Magic0.8.12 — Nexus upload/handoff data

## Requirements and installation
| Dependency | Requirement |
| --- | --- |
|Skyrim Special Edition1.5.97|Only enabled Native runtime;AE/VR remain disabled|
|SKSE64 2.0.20|Matching1.5.97 build|
|Address Library for SKSE Plugins|SE1.5.97 database|
|Backported Extended ESL Support(BEES)|SE1.5.97 needs support for this plugin's1.71 header;not AE DLL compatibility|
|Open Animation Replacer|Shipped flight/combat animation conditions|
|Microsoft Visual C++x64 runtime|Native runtime prerequisite|
|Container Item Distributor2.1.3+|Required for supplied merchant/dungeon distribution,not for direct console access|
|powerofthree Tweaks|CID prerequisite|
|SKSE Menu Framework3.x|Optional in-game damage settings;INI works without it|
|Stances - Dynamic Animation Sets|Optional Stances Addon only;actual master Stances - Dynamic Weapon Movesets SE.esp|

The Stances master chain also requires its actual installed Update.esm,Dawnguard.esm,Dragonborn.esm and Keytrace.esp dependencies. Follow the upstream package's complete instructions. No independent Arcana ESP/DLL or SMP option is part of the new release.

Install Main into MO2's TULL Ordinary Attack Magic folder. Install the optional TOAM Stances Addon only with its masters. Root ESPs are English;Language/en and Language/kr retain matching variants. Only the user changes MO2 mod/plugin checkboxes. This delivery does not alter profile enablement or saves. Earlier SMP options must remain disabled by the user.

GitHub remains private now. Public source visibility is planned together with the Nexus release. Binary distribution and public license decisions remain subject to the already documented art/animation provenance and permissions review. No Nexus upload or public visibility change is performed here.

## Spell specifications at DamageMultiplier1.0
All listed Arcana resource costs are before vanilla reductions. Ordinary/adaptive MP and staff charge share the maximum80%reduction/minimum20%payment policy. Staff playerMP is always0;capacities20000/Unbound40000. Damage customisation does not change any resource cost,rate,count,charge tier,limiter or defense rule.

|Arcana card|Base resource cost|Base attack damage|Original behavior|
|---|---|---|---|
|Emperor|105/shot|55/shot|Original0.20s cadence|
|Strength|750/second|195/second|19.5per0.10s nominal tick|
|Star|480activation+150/deployed circle/second,max750/second|100combined nominal/second|Five original independent auto-attack lanes|
|Chariot|300/shot;1500/five|64/shot;320/five|Five independent emitters;not a five-shot limit|
|Justice|1500/placement|110per0.20s contact|Original3s emitter;continuous aligned/clear living-target damage;no five-contact cap|
|Hermit|132/second|120burst|Original ward,up to80%reduction,defense unchanged|
|Sun|4650gradually over8s|3040maximum|Original8s charge;no upfront full billing|
|Tower|4800activation|150/beam,900finale,target nominal cap27900|180distributed beams;damage multiplier also reflected in effective cap|
|Fool|Existing levitation policy|No attack damage|Ordinary General Magic;flight/ascent trails unchanged|

Adaptive High Priestess:neutral Emperor/low Hermit/mid Chariot/high Sun. Adaptive Moon:neutral Strength/low Justice/mid Star/high Tower. Cast mode/payment/damage snapshot is retained through the cast;stance changes affect the next cast. No separate invented adaptive attack is added.

|Original TOAM spell|Charge/resource policy|Base damage|
|---|---|---|
|Magician|500MP/s per hand;thresholds100/300/600/1200/2000/3000/4000/5000;T8stat cap,continued held drain|Beam100/300/600/1200/2000/3000/4000/5000;explosion20/60/150/300/600/900/1400/2000|
|Judgement|500MP/s per hand;300/900/1800 thresholds;3/6/9 artillery circles|Guide0;mark1 fixed;each artillery150beam+50explosion;first0.45s,next0.12s|
|World|500MP/s;350/700/1050 thresholds;0.7/1.4/2.1s,4/8/12 circles|100each independent explosion;TOAM VFX retained;no ward ability|
|Temperance|Original normal50/power150Proc base costs;toggle/halo/corpse block unchanged|Normal50,power3×80;ALL Temperance excluded from common damage multiplier|

Original TOAM charge-start Limiter/Temperance cost-and-output snapshot remains unchanged;the common damage-only multiplier is an additional factor for Magician/World/Judgement artillery. Arcana does not use that TOAM resource/limiter policy. Current costs and source-backed specs are in BALANCE.md,ARCANA_BALANCE_089.json and the implementation report.

## Distribution settings for the Nexus description
| Eligible container | Regular book | Additional regular book | Regular staff | Stance book | Stance staff |
| --- | ---: | ---: | ---: | ---: | ---: |
| Merchant stock |85%|35%|20%|5%|1%|
| Dungeon loot |10%|none|3%|0.5%|0.2%|

Each successful roll selects ONE item by the weights below. Merchant books have two independent lists; they can be different or repeat. Never give every tome as one collection. Each normal staff is charge20000; stance staff40000. Staff gold values remain unchanged. ChanceNone on each LVLI supplies the probability. Dungeon stance rates use nested1%×50%=0.5% and1%×20%=0.2% lists; fractions are not rounded to integer CID chance.

Targets:140 unique merchant-container references resolved from actual vanilla/Update/Dawnguard/HearthFires/Dragonborn faction VENC fields, including mages,general stores,caravans,fences,blacksmiths,alchemists and other vanilla/DLC services.232 container bases(chests,urns,barrels,sacks and similar containers) additionally require the actual LocTypeDungeon keyword on their current location. This does not promise coverage of unrelated custom merchant chests or unclassified mod-added dungeons. Exact IDs,source hashes and weights are in DISTRIBUTION.json.

These are probabilities per eligible list evaluation,not a guarantee for each visit. CID governs initial distribution and reset handling; reopening a menu/cell is not a supported reroll. Previously generated stock may need its normal reset; no reset/save/console operation is forced by TOAM. Existing contents and other mods' items are not cleared. NPCs are not taught combat spells.


See DISTRIBUTION.md for every card's weight,base gold price and editing instructions;DISTRIBUTION.json contains the exact140merchant/232dungeon targets and source hashes. All15books are cheap(base5..50gold;stance35);all10staffs are included in their corresponding regular/rare pools. No all-tomes bundle. Dungeon appearance is lower;stance books/staffs are particularly rare.

## Configuration and QA handoff
# TOAM common damage setting / 공통 피해배율

Default: `[Balance]` / `DamageMultiplier=1.0` in `SKSE/Plugins/TOAM.ini`.
Valid finite range:0.01..100.0. Examples:0.5 halves damage;1 preserves the baseline;2 doubles damage. Invalid/NaN/infinite/out-of-range values are rejected. An invalid startup value leaves the default1; an invalid reload retains the active value.

Applies to Magician beam/explosion, World explosion, Judgement artillery beam/explosion and all8 Arcana offensive paths, including ordinary/adaptive spells and corresponding staffs. Hermit burst damage scales; its defense does not. Tower projectile power and final blast use the same cast snapshot; its unchanged nominal budget27900 produces an effective ceiling27900×multiplier.

Excluded:ALL Temperance behavior including its50/80 Proc damage, halo and discount; Arcana Limiter policy; Judgement's1-point marking; flight; Hermit defense; charge thresholds; shot interval/count/range; MP/staff charge and80%maximum cost reduction. Staff capacities remain20000/40000.

Optional SKSE Menu Framework3.x:open its menu, then TULL Ordinary Attack Magic > Damage. Adjust the slider or numeric field and use Apply and save INI. Reset and Reload INI are provided. No GetMainWindow dependency, new hotkey, hook, ImGui context or renderer is created. Missing/unsupported Framework exports disable only the optional page; INI remains available.

Arcana snapshots damage at Begin and carries it through missiles, detached Sun flights, installed Justice and Tower finale. TOAM snapshots at release; scheduled Judgement retains that release value for all delayed shots. Existing projectiles/schedules do not change halfway after a menu edit. TOAM cost/limiter snapshot remains the original charge-start snapshot.

INI edits are read at game start or by Reload INI. Saving through MO2's virtual Data path may place the changed INI in MO2 Overwrite; retain it as your personal settings override. Only this key is saved; flight keys are not rewritten. Plugin descriptions show baseline1x numbers and may not reflect this custom setting.

설정 한 개가 공격 피해에 공통 적용된다. 절제 전체·리미터·심판 표식·은둔자 방어·발사주기/수량·자원 소비와 감소상한은 제외한다. 적용 버튼은 INI 저장과 읽기 검증 성공 후 활성값을 바꾼다. 게임 내 메뉴 및 실제 HP 변화는 사용자 QA 전까지 미검증이다.

## Remaining manual game checks
1. Default1x and changed0.5/2x damage,including Tower finale/cap and delayed Justice/Judgement.
2. INI startup,reload,save,persistence and Framework optional absence/page rendering.
3. Temperance Proc/halo,Limiter,80%resource ceiling,Hermit defense,rate/count and MP/staff payment unchanged.
4. Several mages,general stores,caravans and other merchant types show varied stock,not all tomes.
5. Dungeon chest/urn/barrel loot and rare stance pools;normal reset does not duplicate supplies.
6. Original007 A-J158 remains unchanged;this is a focused supplement,not an automatic QA tool.

Core CP028 runtime was user-confirmed.CP029R2 and CP030 changes require user game QA. Static/C++build/deployment do not prove runtime appearance or correct HP changes. AE/VR remain disabled.
