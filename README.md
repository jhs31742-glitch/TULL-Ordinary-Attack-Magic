# TULL Ordinary Attack Magic v1.0.5 — Experimental

Public source, engineering/art research and the offline TULL Formation Editor.

- [1.0.5 experimental release and install files](https://github.com/jhs31742-glitch/TULL-Ordinary-Attack-Magic/releases/tag/v1.0.5-experimental)
- [1.0.5 release notes / 변경 사항](Docs/RELEASE_NOTES_1.0.5.md)
- [Build instructions](BUILDING.md) and [pinned dependencies](DEPENDENCIES.lock.json)
- [Unified engineering and art research](Research/TOAM_RESEARCH_COMPENDIUM.md)
- [Formation Editor EN](tools/formation_editor/README_EN.md) / [한국어](tools/formation_editor/README_KR.md)
- [Licenses](LICENSING.md), [credits](Docs/CREDITS.txt) and [third-party notices](Docs/THIRD_PARTY_NOTICES.md)

## Supported runtimes
- SE **1.5.97**, with SKSE64 **2.0.20**.
- **Experimental:** AE **1.6.1170**, using matching SKSE64 and Address Library packages.
- Other AE runtimes, Skyrim 1.7.x and VR are blocked.
- Static checks and MSVC x64 compile/link passed. **AE gameplay, Experience XP awards, VFX and stability are untested. Final LOCK: NO.**

## Changes in 1.0.5
The same DLL selects the audited SE/AE damage and scalar audio adapters. Exact runtime and hook preflight checks reject unsupported cases. Includes the 1.0.4 kill-attribution change and approved reduced dungeon drops; existing merchant distribution and CID INIs are unchanged.

The repository carries 113 byte-exact native source files for the verified local 1.0.5 candidate. Historical DataSources/PluginRecords and runtime asset manifests remain the 1.0.3 authoring snapshot; current low-drop ESPs are provided in the install archives. See the release notes for hashes, installation and limits.

## Prior v1.0.3 release record
The following describes earlier verification only; it does not grant runtime acceptance to 1.0.5.

CP031 core user QA passed 23/23. The v1.0.3 multi-enemy, multi-color stress session was user-reported PASS with 553 recorded color applications. Blue and Default were not rechecked in that latest session; a historical isolated CTD remains unexplained. These observations do not certify unconditional stability or a final release LOCK.

The source contains 112 byte-exact Native C++/header/include files from the verified v1.0.3 build and six ordered plugin-record sources. Runtime DLLs, game assets, imported animations/audio and private logs/handoffs are excluded. Source publication does not authorize redistribution of excluded assets.

## Formation Editor
Open `tools/formation_editor/index.html` locally. It edits visual layouts and exports JSON; it does not edit saves or write production NIF files. Actual source-backed presets and illustrative AI examples are identified separately.

## Licenses and credits
Original code and editor material owned by TULL or authorized for licensing: MIT. Original owned research prose/diagrams: CC BY 4.0. Third-party material retains its original terms. See [license scope](LICENSING.md), [credits](Docs/CREDITS.txt) and [third-party notices](Docs/THIRD_PARTY_NOTICES.md).

GitHub public publication was explicitly authorized by the user on 2026-10-27. Nexus upload and final runtime LOCK remain separate; this operation does not perform either. MO2 profiles, activation, saves and the immutable v1.0.0/CP032 anchor are preserved. SMP remains excluded.
