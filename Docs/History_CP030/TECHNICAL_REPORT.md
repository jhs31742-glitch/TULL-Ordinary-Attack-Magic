# CP029 / v0.8.11 release preparation
Parent:CP028 v0.8.10, user-reported core runtime PASS; SMP no motion.
New:Arcana8 costs x3; Justice continuous110/.2s during3s; Sun3040; Tower150/beam,900finale,27900target cap. TOAM four spells, resource ownership, limiter, VFX geometry, Hermit defense and flight unchanged.
Restore actual Stances hard MASTER using typed own-record/reference index2->3 mapping. Preserve local IDs, all ENCH/WEAP capacity, enchantment and first-person connections. Normalize TES4 author/description toTULL/TOAM. Rename English Fool book prefix toOrdinary General Magic.
SMP withdrawn: no SMP plugin, helper, XML, PEX or simulated mesh in release. Existing static staff meshes remain.
Separate Main and optional Stances archives, English and Korean variants. Separate source, private GitHub checkout and chat handoff folders. Single unified technical/art research compendium contains cumulative mistakes, successes and precautions.
SE1.5.97 only enabled. The current changes are STATIC/BUILD/DEPLOY tested separately; game results for them remain NOT_RUN. The user's earlier core acceptance is retained and does not automatically test these new numbers.
GitHub policy:PRIVATE now; publish together with Nexus later after rights review. No Nexus upload or visibility change is performed here.

# Arcana balance — v0.8.11
All costs below are BEFORE vanilla Destruction reductions, capped at80% for Arcana spells and staff charge. Staffs use corresponding charge units and zero player Magicka. Cost and damage have separate owners; discount never multiplies damage.

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

## CP029 R2 — Arcana maximum cost reduction80%
User-approved2026-10-26:applies to all8 ordinary/adaptive Arcana spells and staff charge. Final resource factor=max(.20,engineCalculateMagickaCost/baseCost). TOAM cost/limiter/damage unchanged. Hermit stays engine-billed through currentSpellCost with the same floor, never a second Native debit. Staff player MP remains0. God Mode remains the explicit engine cheat exception. The unchanged vanilla MagicMenu estimate may differ from the actual capped Native payment; use actual payment/charge and this policy for QA.
