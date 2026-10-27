# CP035 | Verified Source ZIP Publication Plan

The following plan is copied from the CP034-D3 Source ZIP.
The original root path was NOT present in CP035-R1.

# Publication and handoff policy
User decision2026-10-26:upload this repository PRIVATE now. Make it public together with the Nexus Mods launch, after the launch review. No automatic visibility change, scheduled task or Nexus upload is authorized by this document.
Before public launch: review imported animation/Bethesda-derived assets, choose the license for owned code/research without relicensing external work, confirm CP029 manual gameplay QA, verify exact source/DLL/ZIP checksums, draft Nexus credits/dependencies/install/version notes, then publish the matching source tag and Nexus files together.
Distribution archives, source ZIP, GitHub checkout and chat handoff are kept in separate delivery folders. The private source repository excludes the runtime binary asset package and private logs/evidence registry. It is not a secret-bearing personal backup.
MO2 instruction:deploy files ONLY to the mod folder. The user owns ALL mod/plugin activation, including disabling prior SMP and enabling the stance addon. Do not edit profiles/modlist.txt/plugins.txt or enablement from a future session without a new direct instruction.

CP033:1.0.1 final LOCK and Nexus/public visibility require separate user approval after new audio/visual/runtime QA. CP031 core23/23 does not auto-pass1.0.1.


# CP035 | Release Upload Matrix

Status: STAGING PLAN ONLY. LOCK=NO; PUBLIC_UPLOAD=NOT_RUN.

| Category | File | Status |
|---|---|---|
| Main | TULL Ordinary Attack Magic - v1.0.3.zip | Existing verified distribution |
| Optional | TULL Ordinary Attack Magic - Stances Addon - v1.0.3.zip | Existing verified distribution |
| Source | TULL Ordinary Attack Magic - Source - v1.0.3.zip | Regenerate after CP035 docs update |
| Documentation | TOAM Engineering & Art Research - v1.0.3.zip | NOT BUILT; privacy and rights review pending |
| Optional Tool | TULL Formation Editor 1.0.zip | NOT BUILT; offline test and licensing pending |
| Private | CP034-D3 internal handoff / EVIDENCE / backup | NEVER PUBLIC |

Source ZIP contains the existing Research/TOAM_RESEARCH_COMPENDIUM.md and 16 files under tools/formation_editor.
The editor is a standalone browser-based formation layout tool, not an in-game NIF editor.
Do not advertise playable NIF export or game-save editing unless separately verified.

Release gates:
1. Review third-party/derived assets and distribution permissions.
2. Review licenses and redact private paths or credentials.
3. Synchronize source and research documentation.
4. Package standalone research and editor releases.
5. Check EN/KR Nexus description, requirements, credits and optional addon masters.
6. Verify all publication ZIP SHA256 values.
7. Obtain explicit final LOCK and publication approval.
