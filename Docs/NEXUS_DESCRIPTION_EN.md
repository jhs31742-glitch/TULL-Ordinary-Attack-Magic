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

## Controls and settings
Learn Fool from its Ordinary General Magic tome. Toggle levitation with its power;Jump ascends,Sneak descends,Sprint boosts. Flight settings expose movement/boost/vertical/takeoff speeds,takeoff duration,response,leg pose,melee block,firearm animations and hover-sound controls. Saving queues a game-thread reload.
DamageMultiplier default1.0/range.01..100 multiplies chosen offensive paths only;2doubles baseline attacks. ALL Temperance,Limiter,mark1,Hermit defense,resource costs,80%reduction ceiling,rate/count and charge thresholds remain fixed. Existing in-flight/scheduled attacks retain their snapshot.

## Actual runtime evidence
User-confirmed on the delivered v0.8.12 lineage:core stable/no CTD in the reported session;Limiter bands/numeric results;charge-start ring/Temperance snapshot;Arcana spending does not advance TOAM tiers;TOAM+Arcana coexistence and foreign-spell exclusion;Damage adjustment/Menu Framework page;NPC merchant sales. This does not certify all158 original QA cases,all new vendor types,dungeon probabilities,all per-spell damage multipliers or every stress/save scenario.
New1.0 bilingual/flight page,spin/fade/brightness changes,staff displayed prices and broader/reduced stock distributions require focused user game QA. Source/static/build/deploy gates must be read from final handoff;do not label them runtime PASS. The standalone formation editor has independent browser/JSON/math tests,not game-runtime proof.

## AI use disclosure and credits
TULL supplied design,art direction,manual coordinates and game testing. OpenAI Codex assisted with C++integration/debugging,procedural asset authoring,editor,tests and documentation. The custom Hebrew tome cover used AI image generation. Circle/staff work used supplied reference images with code-assisted mesh construction,optimisation and manual correction;AI assistance does not replace imported authors' credits.
Embedded Arcana Staffs0.7.4 FlightStateFix/shared2.7.1;flight motions by HydroBirous/Hydro Birous,Derwan,Redfall6,gp160,Khenta,RavenKZP,QTRModding;firearm sources Call to Arms and a skrub's Skrub's Guns3rdPerson. Flight Framework by SkyrimThiago/RavenKZP/QTR-Modding was a technical reference,not a bundled DLL/code dependency. Ershin(OAR),ThirdEyeSqueegee(CID),powerofthree(Tweaks),SKSE/CommonLib/Address Library/Menu Framework authors and Bethesda retain original rights. Full source research preserves attribution gaps rather than guessing them.
Source/research:https://github.com/[LOCAL_USER]742-glitch/TULL-Ordinary-Attack-Magic . Currently PRIVATE;public launch should be coordinated with Nexus publication. Imported animation/Bethesda-asset redistribution conditions and final software/editor license must be resolved before public upload. This draft does not perform that upload or grant new third-party permissions.

## Separate developer tool
TULL Formation Editor is a separate offline source download with EN/KR UI/manuals,named sets,multiple spells,independent cast variants,left/right ownership,asset add/remove,marquee/group movement,XYZ rotation/scale,actor/target/world frames,airborne full-sphere presets and JSON import/export. No TOAM/game installation is required. The back-blade example is an authoring example for other mods,not a TOAM attack feature.
