# TOAM world distribution / 월드 배포 사양

Container Item Distributor(CID)2.1.3 or newer compatible version is required for automatic stock/loot. Its Address Library,powerofthree Tweaks and SKSE requirements remain external. No dependency DLL is bundled or replaced. SPID/KID/SkyPatcher are not required for these configs.

Main:TOAM_WorldDistribution_CID.ini. Stances addon:TOAM_Stances_CID.ini, shipped only in the optional addon archive. Disable/remove that config with the addon; its pools require the addon ESP and actual Stances master.

| Eligible container | Regular book | Additional regular book | Regular staff | Stance book | Stance staff |
| --- | ---: | ---: | ---: | ---: | ---: |
| Merchant stock |85%|35%|20%|5%|1%|
| Dungeon loot |10%|none|3%|0.5%|0.2%|

Each successful roll selects ONE item by the weights below. Merchant books have two independent lists; they can be different or repeat. Never give every tome as one collection. Each normal staff is charge20000; stance staff40000. Staff gold values remain unchanged. ChanceNone on each LVLI supplies the probability. Dungeon stance rates use nested1%×50%=0.5% and1%×20%=0.2% lists; fractions are not rounded to integer CID chance.

Targets:140 unique merchant-container references resolved from actual vanilla/Update/Dawnguard/HearthFires/Dragonborn faction VENC fields, including mages,general stores,caravans,fences,blacksmiths,alchemists and other vanilla/DLC services.232 container bases(chests,urns,barrels,sacks and similar containers) additionally require the actual LocTypeDungeon keyword on their current location. This does not promise coverage of unrelated custom merchant chests or unclassified mod-added dungeons. Exact IDs,source hashes and weights are in DISTRIBUTION.json.

These are probabilities per eligible list evaluation,not a guarantee for each visit. CID governs initial distribution and reset handling; reopening a menu/cell is not a supported reroll. Previously generated stock may need its normal reset; no reset/save/console operation is forced by TOAM. Existing contents and other mods' items are not cleared. NPCs are not taught combat spells.

## Books
| Card | Relative weight | Conditional share | Base gold value |
| --- | ---: | ---: | ---: |
|Magician|8|6.612%|20|
|Judgement|6|4.959%|25|
|World|8|6.612%|20|
|Temperance|10|8.264%|10|
|Emperor|14|11.570%|8|
|Hermit|10|8.264%|15|
|Chariot|10|8.264%|20|
|Sun|4|3.306%|40|
|Strength|12|9.917%|10|
|Justice|8|6.612%|20|
|Star|10|8.264%|15|
|Tower|3|2.479%|50|
|Fool|18|14.876%|5|

Base value is discounted/marked up by the existing barter system; it is not a fixed shop sale price. Stance books High Priestess/Moon each cost35 base gold and have equal weight within their rare book pool.

## Staffs
| Card | Relative weight | Conditional share |
| --- | ---: | ---: |
|Emperor|18|19.355%|
|Hermit|12|12.903%|
|Chariot|12|12.903%|
|Sun|5|5.376%|
|Strength|16|17.204%|
|Justice|12|12.903%|
|Star|14|15.054%|
|Tower|4|4.301%|

High Priestess/Moon Unbound staffs have equal weight within the rare stance-staff pool. Normal and stance staff gold values,damage,unique enchantment protection and soul-gem recharge remain unchanged.

## Editing probabilities
The five main LVLI locals DCF..DD3 own merchant book,extra book,staff,dungeon book,staff rolls. ChanceNone=100-minus-probability;LVLF does not use UseAll. Repeated LVLO entries encode relative weights,not extra simultaneous copies. Addon900/901 own merchant rolls;902/903 are dungeon leaves;904/905 are1%gates. Editing CID ?chance instead adds another independent gate and changes the effective rate. Keep each list below255 entries and avoid cycles.

Source documentation:[CID author usage](https://www.nexusmods.com/skyrimspecialedition/mods/99486?tab=description). Merchant sale visibility,reset cadence and real loot appearance are RUNTIME NOT_RUN.
