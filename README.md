# TULL Ordinary Attack Magic v1.0.3

Public source, engineering/art research and the offline TULL Formation Editor.
Repository: https://github.com/jhs31742-glitch/TULL-Ordinary-Attack-Magic

- [Build instructions](BUILDING.md) and [pinned dependencies](DEPENDENCIES.lock.json)
- [Nexus description EN/KR - plain text](Docs/TOAM_v1.0.3_Nexus_Description_EN_KR_Plain.txt)
- [Nexus description EN/KR - BBCode](Docs/TOAM_v1.0.3_Nexus_Description_EN_KR_BBCode.txt)
- [Current v1.0.3 implementation](Docs/CP034_D3_RECAST_UI_TAROT_REPORT.md)
- [User stress-test evidence and limits](Docs/CP035_V103_STRESS_RELEASE_READINESS.md)
- [Presentation settings](Docs/PRESENTATION_SETTINGS_EN_KR.md), [audio catalog](Docs/AUDIO_CATALOG.md), [circle materials](Docs/CIRCLE_MATERIAL_AUDIT.md)
- [Unified engineering and art research](Research/TOAM_RESEARCH_COMPENDIUM.md)
- [Formation Editor EN](tools/formation_editor/README_EN.md) / [한국어](tools/formation_editor/README_KR.md)
- [Publication scope and provenance](Docs/GITHUB_PUBLICATION.md)

## Runtime and verification
Skyrim SE 1.5.97 is the supported runtime. AE/VR are not enabled.
CP031 core user QA passed 23/23. The v1.0.3 multi-enemy, multi-color stress session was user-reported PASS with 553 recorded color applications. Blue and Default were not rechecked in that latest session; a historical isolated CTD remains unexplained. These observations do not certify unconditional stability or a final release LOCK.

The source contains 112 byte-exact Native C++/header/include files from the verified v1.0.3 build and six ordered plugin-record sources. Runtime DLLs, game assets, imported animations/audio and private logs/handoffs are excluded. Source publication does not authorize redistribution of excluded assets.

## Formation Editor
Open `tools/formation_editor/index.html` locally. It edits visual layouts and exports JSON; it does not edit saves or write production NIF files. Actual source-backed presets and illustrative AI examples are identified separately.

## Licenses and credits
Original code and editor material owned by TULL or authorized for licensing: MIT. Original owned research prose/diagrams: CC BY 4.0. Third-party material retains its original terms. See [license scope](LICENSING.md), [credits](Docs/CREDITS.txt) and [third-party notices](Docs/THIRD_PARTY_NOTICES.md).

GitHub public publication was explicitly authorized by the user on 2026-10-27. Nexus upload and final runtime LOCK remain separate; this operation does not perform either. MO2 profiles, activation, saves and the immutable v1.0.0/CP032 anchor are preserved. SMP remains excluded.
