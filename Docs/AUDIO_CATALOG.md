# TOAM 1.0.1 AUDIO_CATALOG / 효과음 카탈로그
Baseline:1.0.0 CP032. Stable IDs are append-only. Source:actual shipped RIFF/XWM files,14 actual SNDR records,and every Native sound call. No SNDR is referenced by another ESP record;14 are loaded by Native form lookup. There are17 actual event settings;unused catalog sounds do not create new events.
WAV duration is frames/sample-rate;XWM duration uses the final RIFF dpds decoded PCM byte count divided by decoded bytes/second. It is not an audible engine measurement. WAV peak/rms are normalized PCM;XWM loudness is unmeasured. No new audio asset is authored.

|ID|EN / KR|Original Data-relative file|SNDR local|Class|Seconds|PCM peak / rms|Original event users|
|---|---|---|---|---|---:|---|---|
|001|Magician launch / 마법사 발사|sound/fx/TOAM/Spells/Arcana_Magician_Launch.xwm|0|One-shot|1.160998|UNMEASURED / UNMEASURED|MagicianLaunch|
|002|Magician heavy launch / 마법사 강한 발사|sound/fx/TOAM/Spells/Arcana_Magician_HeavyLaunch.xwm|0|One-shot|1.718277|UNMEASURED / UNMEASURED|MagicianHeavyLaunch, WorldRelease|
|003|Judgement launch / 심판 발사|sound/fx/TOAM/Spells/Arcana_Judgement_Launch.xwm|0|One-shot|1.393197|UNMEASURED / UNMEASURED|JudgementShot|
|004|Temperance proc / 절제 발동|sound/fx/TOAM/Spells/Arcana_Temperance_Proc.xwm|0|One-shot|1.532517|UNMEASURED / UNMEASURED|TemperanceProc|
|005|Emperor shot / 황제 발사|sound/fx/TOAM/Arcana/Arcana_Emperor_Shot.wav|0xA51|One-shot|0.780000|0.501190185546875 / 0.05142858580555348|EmperorShot, ChariotShot, StarShot|
|006|Strength loop / 힘 지속음|sound/fx/TOAM/Arcana/Arcana_Strength_Loop.wav|0xA52|Loop|8.000000|0.712371826171875 / 0.17499466224541507|StrengthLoop, JusticeLoop|
|007|Formation (silent) / 전개음 (무음 원본)|sound/fx/TOAM/Arcana/Arcana_Formation.wav|0xA53|One-shot|0.050000|0.0 / 0.0|Catalog only;no active baseline call|
|008|Charge loop (silent) / 충전 지속음 (무음 원본)|sound/fx/TOAM/Arcana/Arcana_Charge_Loop.wav|0xA54|Loop|0.050000|0.0 / 0.0|Catalog only;no active baseline call|
|009|Sun release / 태양 발사 원본|sound/fx/TOAM/Arcana/Arcana_Sun_Release.wav|0xA55|One-shot|0.320000|0.8599853515625 / 0.24435392774058023|Catalog only;no active baseline call|
|010|Sun charge / 태양 충전음|sound/fx/TOAM/Arcana/Arcana_Sun_Charge.wav|0xA56|Loop|0.470000|0.65997314453125 / 0.18563965192432114|Catalog only;no active baseline call|
|011|Strength start / 힘 시작음|sound/fx/TOAM/Arcana/Arcana_Strength_Start.wav|0xA57|One-shot|0.320000|0.5 / 0.1372601045497764|StrengthStart|
|012|Strength end / 힘 종료음|sound/fx/TOAM/Arcana/Arcana_Strength_End.wav|0xA58|One-shot|0.500000|0.6199951171875 / 0.136800137318042|SunCancel|
|013|Tower shot / 탑 발사|sound/fx/TOAM/Arcana/Arcana_Tower_Shot.wav|0xA59|One-shot|0.064989|0.50115966796875 / 0.146344607961288|TowerShot|
|014|Hermit detonation / 은둔자 폭발 원본|sound/fx/TOAM/Arcana/Arcana_Hermit_Detonation.wav|0xC77|One-shot|0.500000|0.89996337890625 / 0.39248253307430087|Catalog only;no active baseline call|
|015|Hermit aftershock / 은둔자 여진 원본|sound/fx/TOAM/Arcana/Arcana_Hermit_Aftershock.wav|0xC78|One-shot|0.240000|0.6199951171875 / 0.251742609710313|Catalog only;no active baseline call|
|016|Sun impact / 태양 충격음|sound/fx/TOAM/Arcana/Arcana_Sun_Impact.wav|0xC85|One-shot|5.800000|0.91998291015625 / 0.15024796978116464|SunRelease|
|017|Burst explosion / 공통 폭발|sound/fx/TOAM/Arcana/Arcana_Burst_Explosion.wav|0xC88|One-shot|5.352000|1.0 / 0.3587915933895953|HermitExplosion, TowerExplosion|
|018|Fool hover / 광대 부유음|sound/fx/TOAM/Arcana/Arcana_Fool_Hover.wav|0xCA9|Loop|12.000000|0.399993896484375 / 0.19758209167434146|FoolHum|

## Original event policy / 원본 이벤트 정책
Every volume factor defaults1.00. Effective gain=original gain x Audio/Master x AudioSpell/<spell> x AudioEvent/<event>. All factors are finite0..1. Master/event0 skips new sound allocation. Original direct-file flags0x10/priority0x80,2D output with original3D fallback,and SE scalar SetPosition adapter are retained. Engine SetVolume does not establish guaranteed amplification above1;the shipped Burst PCM already peaks at1.0. No >1 control is offered.

|Event / INI key|KR|Spell|Default ID|Original Gain and lifetime / cleanup|
|---|---|---|---|---|
|MagicianLaunch|마법사 발사|Magician|001|tiers1..4:.85/.90/.95/1;1.161s,cap3,60ms fade|
|MagicianHeavyLaunch|마법사 강한 발사|Magician|002|tiers5..8:.95/1/1/1;1.719s,shared Magician cap3,60ms fade|
|JudgementShot|심판 포격|Judgement|003|.95;1.394s,cap3,60ms fade;only shotIndex%3==0 and >=80ms catch-up gate|
|WorldRelease|세계 해제|World|002|stages.85/.95/1;1.719s,cap3,60ms fade|
|TemperanceProc|절제 발동|Temperance|004|.95;1.533s,cap2,100ms fade|
|EmperorShot|황제 발사|Emperor|005|.68;per-hand16 shot slots;existing completion stops|
|ChariotShot|전차 발사|Chariot|005|.54;per-hand16 shot slots;existing five independent cadences|
|StrengthStart|힘 시작|Strength|011|.65;per-hand16 shot slots;one start event|
|StrengthLoop|힘 지속|Strength|006|.78;one sustain per hand;existing FadeSustain|
|SunRelease|태양 발사|Sun|016|.90;four ultimate voices;existing natural tail/clear|
|SunCancel|태양 취소|Sun|012|.50;four ending slots;existing clear|
|StarShot|별 발사|Star|005|.47;per-hand16 shot slots;one request for an actual lane dispatch|
|JusticeLoop|정의 지속|Justice|006|.42;one loop per installed sentry;existing release|
|TowerShot|탑 발사|Tower|013|.34;per-hand16 shot slots;actual queued-shot launch only|
|HermitExplosion|은둔자 폭발|Hermit|017|.94;existing burst voice side*4;full tail policy|
|TowerExplosion|탑 폭발|Tower|017|1.00;existing burst voice side*4;full tail policy|
|FoolHum|광대 부유|Fool|018|starts0 then Flight/HumVolume(default.35) x original.30s fade-in;one hover loop;existing sustain fade|

Direct file owners expire at original event duration+100ms,even if the replacement file is longer. Replacement may cut a longer sound;it never extends the event or adds attacks. Each cap includes its retiring policy exactly as before. Arcana retains its own shot/loop/ultimate/ending/burst/fading owners;no handle is shared with the direct-file path. FadeSustain keeps a copied original handle ID and four bounded fade slots. Cancellation/load/reset ownership is retained. Descriptor base attenuation/category/output and flags remain original bytes.
Direct events accept all catalog one-shots. Descriptor events accept only matching one-shot/loop IDs with an actual SNDR;001..004 cannot be routed through an invented descriptor. Loop set=006/008/010/018. SNDR LNAM0x801 establishes the shipped loop class;one-shots use0x1. IDs007/008 are genuinely silent .05s original WAVs and labelled as such. All wrong/out-of-range/malformed/incompatible IDs restore that event's original ID.
Initially muted Fool consumes its start event without allocating/retrying a sound every frame. Unmuting such a session begins at the next flight. An already active hover handle retains its original follow/fade updates and cached-volume multiplication;changing source never swaps that live handle. No file/INI reads occur in playback.

청감·실제 믹싱·음원 교체 재생은 USER_RUNTIME 미검증입니다. 잘못된 ID는 원본으로 복구하고,단발/루프 및 직접파일/SNDR 호환 경계를 유지합니다. 미리듣기는 없습니다.
