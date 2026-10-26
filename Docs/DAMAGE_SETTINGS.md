# TOAM common damage setting / 공통 피해배율

Default: `[Balance]` / `DamageMultiplier=1.0` in `SKSE/Plugins/TOAM.ini`.
Valid finite range:0.01..100.0. Examples:0.5 halves damage;1 preserves the baseline;2 doubles damage. Invalid/NaN/infinite/out-of-range values are rejected. An invalid startup value leaves the default1; an invalid reload retains the active value.

Applies to Magician beam/explosion, World explosion, Judgement artillery beam/explosion and all8 Arcana offensive paths, including ordinary/adaptive spells and corresponding staffs. Hermit burst damage scales; its defense does not. Tower projectile power and final blast use the same cast snapshot; its unchanged nominal budget27900 produces an effective ceiling27900×multiplier.

Excluded:ALL Temperance behavior including its50/80 Proc damage, halo and discount; Arcana Limiter policy; Judgement's1-point marking; flight; Hermit defense; charge thresholds; shot interval/count/range; MP/staff charge and80%maximum cost reduction. Staff capacities remain20000/40000.

Optional SKSE Menu Framework3.x:open its menu, then TULL Ordinary Attack Magic > Damage. Adjust the slider or numeric field and use Apply and save INI. Reset and Reload INI are provided. No GetMainWindow dependency, new hotkey, hook, ImGui context or renderer is created. Missing/unsupported Framework exports disable only the optional page; INI remains available.

Arcana snapshots damage at Begin and carries it through missiles, detached Sun flights, installed Justice and Tower finale. TOAM snapshots at release; scheduled Judgement retains that release value for all delayed shots. Existing projectiles/schedules do not change halfway after a menu edit. TOAM cost/limiter snapshot remains the original charge-start snapshot.

INI edits are read at game start or by Reload INI. Saving through MO2's virtual Data path may place the changed INI in MO2 Overwrite; retain it as your personal settings override. Only this key is saved; flight keys are not rewritten. Plugin descriptions show baseline1x numbers and may not reflect this custom setting.

설정 한 개가 공격 피해에 공통 적용된다. 절제 전체·리미터·심판 표식·은둔자 방어·발사주기/수량·자원 소비와 감소상한은 제외한다. 적용 버튼은 INI 저장과 읽기 검증 성공 후 활성값을 바꾼다. 게임 내 메뉴 및 실제 HP 변화는 사용자 QA 전까지 미검증이다.
