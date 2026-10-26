# TULL Ordinary Attack Magic1.0 — Nexus description draft

A tarot-themed ordinary attack/general-magic collection with charged beams,target-directed artillery,player/target formations,staves,levitation and airborne casting. Core TOAM and embedded Arcana share one Native host with independent cast/resource ownership.

## Files and features
- Main:Magician,Judgement,World,Temperance;Emperor,Hermit,Chariot,Sun,Strength,Justice,Star,Tower;Fool levitation;8normal staffs;Arcana Limiter.
- Optional Stances Addon:High Priestess/Moon adaptive spells and2Unbound staffs. Current stance selects the card;mode/resource owner stays fixed for the cast. Staff capacities20000/Unbound40000;zero playerMagicka;normal soul-gem recharge,unique-enchantment extraction/overwrite protection. No SMP option.
- Optional SKSE Menu Framework UI:common damage-only multiplier and bounded flight controls,English/Korean toggle. INI works without Framework.
- World/Hermit,Judgement and Temperance use their own coordinate/target/lifetime contracts.8Sun/Magician layers have individual spin rates/directions and tier acceleration;reveal spin/fade tails preserve gameplay counts and hit policy. Halo speed/halo-and-ascent brightness are presentation changes.
- Root EN ESPs with retained Language/en and Language/kr variants. Main user documentation consists of short root README.txt and CREDITS.txt;development research is separate.

## Requirements / supported runtime
Supported Native runtime:Skyrim Special Edition1.5.97 only;SKSE64 2.0.20,SE Address Library,BEES(plugin header1.71),Open Animation Replacer,VC++x64 runtime. AE/VR are disabled by the Native guard;CommonLib/BEES or CID's own broader compatibility does not make this DLL AE-compatible.
For supplied stock/loot distribution:Container Item Distributor2.1.3+ and powerofthree Tweaks. Optional settings UI:SKSE Menu Framework3.x;Korean display additionally needs appropriate font glyphs.
Optional Stances Addon additionally requires Stances - Dynamic Weapon Movesets SE.esp and its actual upstream masters:Skyrim.esm,Update.esm,Dawnguard.esm,Dragonborn.esm,Keytrace.esp. Main does not require Stances,standalone Arcana,Flight Framework,SMP,SPID,KID or SkyPatcher. Weapon/firearm compatibility is optional,not a main dependency.

## Acquisition and prices
Tomes and staffs are randomly sold by broad vanilla/DLC merchant stocks,including College,inn/general/food,caravan,spouse and specialist stores. Mods reusing those stocks can inherit coverage. Loot also includes dungeon treasure/ordinary/boss chests,urns,barrels,sacks and similar eligible containers. Existing stock/reset rules apply;not all tomes arrive as a bundle. Base book values5..50gold,stance35;normal staff base10,Unbound15. Actual barter/enchanted-item value is engine-dependent.




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

# TOAM1.0 supplies
CID2.1.3+ and powerofthree Tweaks provide additive weighted one-item lists.149 target identifiers=143 shop-specific container bases+6 explicit merchant refs.134 of the old140 refs are covered by bases;the other6 remain direct. Base/ref sources do not overlap,so switching coverage does not double-add. Extra base types include College,inn,spouse-store,caravan,food/general and other merchant stocks;mods reusing those bases inherit coverage. Unrelated custom stocks are not promised.

|Target|Book|Extra book|Staff|Stance book|Stance staff|
|---|---:|---:|---:|---:|---:|
|Shop|55%|15%|10%|1%|0.2%|
|Dungeon|10%|none|3%|0.1%|0.02%|

232 dungeon bases include treasure/boss/ordinary chests,urns,barrels,sacks and similar loot,restricted to LocTypeDungeon. Not every container guarantees an item. ChanceNone gates a weighted single choice;no UseAll. Stock follows CID/normal resets,not each menu opening. No contents are cleared and no NPC combat spells are taught.

Stance900 book1%;901 staff1%×90320%=0.2%;904 book1%×90210%=0.1%;905 staff10%×9010.2%=0.02%. No list cycles.

|Book|Weight|Base gold|
|---|---:|---:|
|Magician|8|20|
|Judgement|6|25|
|World|8|20|
|Temperance|10|10|
|Emperor|14|8|
|Hermit|10|15|
|Chariot|10|20|
|Sun|4|40|
|Strength|12|10|
|Justice|8|20|
|Star|10|15|
|Tower|3|50|
|Fool|18|5|

Stance books equal weight/base35gold;staff weights remain18/12/12/5/16/12/14/4 for Emperor/Hermit/Chariot/Sun/Strength/Justice/Star/Tower. Staff WEAP base value10/Unbound15;charge20000/40000 and enchantments/payment stay intact. Actual displayed/barter price depends on the engine and barter.
Exact forms/source hashes:DISTRIBUTION.json. Author usage:https://www.nexusmods.com/skyrimspecialedition/mods/99486?tab=description . V0.8.12 shop sales USER_CONFIRMED;V1 broader targets/rates/game loot NOT_RUN.

# Damage / flight menus —1.0
Optional SKSE Menu Framework3.x resolves audited exports and registers Damage - 피해 배율 and Flight - 비행 설정. Korean/한국어 checkbox switches both pages;[Menu] Korean records the preference. Korean glyph coverage depends on the installed Framework font configuration;the installed Korean3.18 is the test target. No Framework DLL/font is bundled or replaced.
DamageMultiplier remains1.0 default,finite.01..100. Excludes ALL Temperance,Limiter,mark1,Hermit defense,resource/80%ceiling,cadence/count. Original cast/release snapshots remain.
Flight page edits Enabled,Speed,BoostSpeed,VerticalSpeed,TakeoffSpeed,TakeoffSeconds,Response,RangedLegPose,BlockMeleeWhileFlying,GunAnimations,HumEnabled,HumVolume. Bounds/defaults exactly match existing FlightLoadSettings. One complete Flight section is written/read back before an atomic reload flag is queued. The existing game frame applies it;the menu never touches Actors/controllers. Reset defaults edits the draft;Apply/save commits it. Missing optional Framework leaves INI available.
V0.8.12 Damage menu/user feature PASS. New bilingual/flight page actual game QA NOT_RUN.

User source/build/deploy result is recorded externally after the guarded transaction;current code/data are candidates until that result.

AI/developer coordinate references: tools/formation_editor/examples/ai-coordinate-examples.json (direct import) and ai-reference-guide.json (machine-readable documentation); actual TOAM source-backed values remain presets/toam-reviewed.json with source hashes.
