# Build and reproduce
Use Windows x64 Visual Studio C++ tools, Windows SDK, CMake3.25+ and Ninja. The local verified reference build uses existing build/msvc, MSVC14.51.36231, x64-windows-static-md and pinned dependencies below. Compiling is not runtime validation.

Clone recursively. CommonLibSSE-NG is pinned to8c4025b01fac2bea1bbe73a3a9da7b4fde338343 (alandtse lineage); vcpkg to4bc07e3eb00c5a9539a5a7a83415150a9260f8db. The vcpkg manifest baseline matches. Run git submodule update --init --recursive after extracting a source ZIP with git initialization or obtain the exact pins manually. A plain ZIP does not contain submodule content.

In an x64 Developer PowerShell:
```powershell
cmake --preset release-windows
cmake --build --preset release-windows --target ordinaryattackmagic
```
Output:build/msvc/TOAM.dll. The build does not install or enable plugins. Explicit deploy uses cmake --build --preset deploy and only copies the DLL; prepare a complete reviewed asset package separately. DEPLOY_DIR_OVERRIDE controls that destination. Never blindly use a machine-specific path from old research.
The circle adapter is optional and OFF by default. Enable TOAM_BUILD_ASSET_TOOLS only with TOAM_NIFLY_ROOT and TOAM_NIFLY_LIBRARY explicitly set. Historical ArtTools depend on their documented templates/font/nifly/Blender/Python environment and are not automatic build dependencies. Legacy Linux/test presets are inherited templates, not validated support promises; PLUGIN_TESTS_ONLY has no shipped example test source.
DataSources/PluginRecords JSON is byte-preserving ordered subrecords. tools/esp_records.py reconstructs each ESP to a chosen output directory and verifies its stored SHA256. It preserves typed references and existing IDs; never replace arbitrary32bit patterns.
Use release manifests to compare source/package hashes. Paths and module names must be validated in serialized NIF bytes as well as loader views. No game EXE or Address Library databases are supplied. AE stays rejected until exact ABI/hook proof and separate manual QA.
