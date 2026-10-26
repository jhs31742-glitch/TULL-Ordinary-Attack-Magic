# Runtime compatibility — TOAM v0.8.11
SE 1.5.97 remains the runtime authority. The CP024 staff CTD correction uses a native-disassembled SE function and the pinned SE/AE relocation pair (37803,38752). Engine ArtObject paths now use (22289,22769); no pointer is fabricated from the SE BOOL return.

AE support is PREPARED IN PART, NOT ENABLED and NOT RUNTIME TESTED. CMake compiles both SE and AE CommonLib code paths. This is not equivalent to a supported AE release.
Unresolved requirements:
1. Hermit/Tower actor-value precommit hook: current source has only verified SE37523 and its 5-byte entry/attacker ABI. No exact AE counterpart or entry bytes were found in the pinned SDK or installed Address Library (which has SE databases only). Guessing an adjacent AE ID is prohibited.
2. Audio positioned-file creation flags and scalar SetPosition ABI: SE is native-proven; AE file contract is not. The SDK's SetPosition(NiPoint3) is already known incorrect for SE and cannot prove AE's ABI.
3. Weapon-hit call-site offsets, projectile/vtable slots and runtime struct access have SE/AE source declarations, but still require exact AE native validation and manual QA.

The host rejects unsupported runtimes BEFORE registering listeners or installing hooks. It does not silently disable Hermit defense, remove the Tower damage cap, or call a SE-only ID on AE.
To finish: obtain a legitimately owned local AE executable and matching Address Library, verify these concrete call sites and ABI/entry bytes, add paired adapters and guarded hook checks, then build in the existing build/msvc and run separate AE manual QA. No download, executable replacement or game launch is performed by this task.
Dependencies: matching SKSE and Address Library for the target runtime, OAR and the original Stances plugin for the optional addon. ESP loading support alone does not establish Native ABI support.
