# TOAM 1.0.1 COLOR_PRESET_MAPPING
All13 keys are independent. Default leaves each original1.0.0 material and texture untouched. Nondefault RGB palette is multiplied by max(original RGB) per geometry,retaining original alpha/baseColorScale/other fields. Blue(.30,.60,1),Red(1,.16,.12),Gold(1,.67,.18),Purple(.68,.25,1). These are shader factors,not guaranteed final screen pixel RGB;lighting/blending/tone mapping remain engine-owned. Different original brightness ratios remain.

|CircleColor key|Owned roles|Snapshot authority|Original models / mapping|
|---|---|---|---|
|Magician|8-layer charge + its retirement|MagicianChargeVisual::Begin|TOAM_Engine_Circle / original per-layer model|
|Judgement|charge guide +3/6/9 delayed artillery|BeginChargeGuide;copied to artillery generation|TOAM_Engine_Circle|
|World|4/8/12 deployment + delayed display|BeginCharge;World state|Arcana_World_Harmonized|
|Temperance|halo + each paid Proc|halo Toggle ON;Proc paid commit|Arcana_Temperance_Halo_Engine;TOAM_Engine_Circle|
|Emperor|existing per-hand Arcana circles,retired owners|Arcana Begin;copied into detached owner|existing Emperor circle model|
|Hermit|existing per-hand Arcana circles,retired owners|Arcana Begin;copied into detached owner|existing Hermit circle model|
|Chariot|existing per-hand Arcana circles,retired owners|Arcana Begin;copied into detached owner|existing Chariot circle model|
|Sun|existing per-hand Arcana circles,retired owners|Arcana Begin;copied into detached owner|existing Sun circle model|
|Strength|existing per-hand Arcana circles,retired owners|Arcana Begin;copied into detached owner|existing Strength circle model|
|Justice|existing per-hand Arcana circles,retired owners;installed sentry|Arcana Begin;copied into detached owner|existing Justice circle model|
|Star|existing per-hand Arcana circles,retired owners|Arcana Begin;copied into detached owner|existing Star circle model|
|Tower|existing per-hand Arcana circles,retired owners;queued sky/field muzzle|Arcana Begin;copied into detached owner|existing Tower circle model|
|Fool|takeoff + ascent bands|flight Toggle ON;copied at sigil creation|Arcana_Field_Band;Arcana_Fool_AscentBand|

Already created/pending/retired circles retain their stored color when settings change. Next cast/effect owner snapshots new settings. Halo keeps its Toggle-ON snapshot across cell/3D rebuilds;flight ascent inherits the flight session snapshot until toggled off/on. Both hands have separate Hand/slot state. Delayed World/Judgement/Proc/sentry/Tower requests carry the original snapshot rather than reading a later setting. Finish/reset/new-slot clears attempted-root state. Pool size and request/quarantine/generation policies are unchanged. Beams,explosions,projectiles and unrelated spells are excluded.
Formation Editor/manual JSON/numeric layout and historical authoring generators retain original coordinates and references. Runtime color is an additional presentation policy after those transforms;no editor source or sample geometry is rewritten. Both active shared ARTO model paths and preserved legacy aliases remain available.
