cmake_minimum_required(VERSION 3.21)

project(
    ordinaryattackmagic
    VERSION 1.0.1
    DESCRIPTION "Ordinary Attack Magic SKSE plugin"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PLUGIN_TESTS_ONLY "Build unit tests instead of the plugin" OFF)

if(PLUGIN_TESTS_ONLY)

    find_package(Catch2 3 CONFIG REQUIRED)

    include(CTest)
    include(Catch)

    add_executable(${PROJECT_NAME}Tests test/ExampleTests.cpp)

    target_include_directories(${PROJECT_NAME}Tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    target_link_libraries(${PROJECT_NAME}Tests PRIVATE Catch2::Catch2WithMain)

    catch_discover_tests(${PROJECT_NAME}Tests)

else()

    # Fail fast with an actionable message if the submodule wasn't checked out.
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/commonlibsse-ng/CMakeLists.txt")
        message(FATAL_ERROR "lib/commonlibsse-ng is not checked out. Clone with --recurse-submodules, "
                            "or run: git submodule update --init --recursive")
    endif()

    # Dependencies

    find_package(spdlog CONFIG REQUIRED)
    # directxtk must be found before add_commonlibsse_plugin (CommonLibSSE-NG links
    # Microsoft::DirectXTK transitively).
    find_package(directxtk CONFIG REQUIRED)
    # rapidcsv must be in vcpkg.json (CommonLibSSE-NG unconditionally adds it as an include dir)

    # CommonLibSSE-NG

    set(ENABLE_SKYRIM_SE
        ON
        CACHE BOOL "" FORCE)
    set(ENABLE_SKYRIM_AE
        ON
        CACHE BOOL "" FORCE)
    set(ENABLE_SKYRIM_VR
        OFF
        CACHE BOOL "" FORCE)
    set(BUILD_TESTS
        OFF
        CACHE BOOL "" FORCE)
    set(SKSE_SUPPORT_XBYAK
        OFF
        CACHE BOOL "" FORCE)

    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/commonlibsse-ng" "${CMAKE_CURRENT_BINARY_DIR}/commonlibsse-ng"
                     EXCLUDE_FROM_ALL)

    include("${CMAKE_CURRENT_SOURCE_DIR}/lib/commonlibsse-ng/cmake/CommonLibSSE.cmake")

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/src/version.rc" @ONLY)

    # Plugin ??auto-generates SKSEPlugin_Version; Plugin.cpp only needs SKSEPlugin_Load
    add_commonlibsse_plugin(
        ${PROJECT_NAME}
        NAME
        "TOAM"
        TULL
        "TULL"
        SOURCES
        src/Plugin.cpp
        src/Arcana/Plugin.cpp
        src/VFX/EngineCircle.cpp
        src/Audio/SpellAudio.cpp
        src/Core/DataCache.cpp
        src/Core/Runtime.cpp
        src/Core/DamageSettings.cpp
        src/Core/PresentationSettings.cpp
        src/VFX/CircleTint.cpp
        src/Core/MagickaTransaction.cpp
        src/Charge/ChargeProfile.cpp
        src/Charge/ChargeSession.cpp
        src/Charge/MagickaChargeArbiter.cpp
        src/Charge/ChargeRuntime.cpp
        src/VFX/AssetCatalog.cpp
        src/VFX/VFXInstance.cpp
        src/VFX/TransientVFXManager.cpp
        src/VFX/CircleRenderer.cpp
        src/VFX/BeamRenderer.cpp
        src/VFX/VFXRuntime.cpp
        src/VFX/Choreography.cpp
        src/Combat/FixedDamage.cpp
        src/Combat/ActorBounds.cpp
        src/Combat/TargetFormation.cpp
        src/Combat/BeamCollision.cpp
        src/Spells/MagicianProfile.cpp
        src/Spells/MagicianChargeVisual.cpp
        src/Spells/MagicianRuntime.cpp
        src/Spells/JudgementProfile.cpp
        src/Spells/JudgementRuntime.cpp
        src/Spells/WorldProfile.cpp
        src/Spells/WorldFormation.cpp
        src/Spells/WorldRuntime.cpp
        src/Spells/TemperanceRuntime.cpp
        src/Spells/ArcanaLimiterRuntime.cpp
        src/Combat/WeaponHitSource.cpp
        "${CMAKE_CURRENT_BINARY_DIR}/src/version.rc"
        USE_ADDRESS_LIBRARY)

    source_group("Resources" FILES "${CMAKE_CURRENT_BINARY_DIR}/src/version.rc")

    # cmake_llvm_rc preprocesses RC files using the target's INCLUDE_DIRECTORIES, not CMAKE_C_FLAGS.
    # The xwin SDK paths are in CMAKE_C_FLAGS_INIT as /imsvc flags and are invisible to cmake_llvm_rc,
    # so we add them explicitly here for the RC compiler to find winres.h.
    if(DEFINED XWIN_DIR)
        target_include_directories(
            ${PROJECT_NAME} SYSTEM PRIVATE "$<$<COMPILE_LANGUAGE:RC>:${XWIN_DIR}/sdk/include/um>"
                                           "$<$<COMPILE_LANGUAGE:RC>:${XWIN_DIR}/sdk/include/shared>")
    endif()

    set_source_files_properties(src/Arcana/Plugin.cpp PROPERTIES
        SKIP_PRECOMPILE_HEADERS ON COMPILE_DEFINITIONS "ARCANA_EMBEDDED=1")
    file(GLOB ARCANA074_PARTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Arcana/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Arcana/*.inl")
    set_property(SOURCE src/Arcana/Plugin.cpp APPEND PROPERTY OBJECT_DEPENDS "${ARCANA074_PARTS}")
    target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

    # Asset conversion is an optional developer tool, not a plugin build dependency.
    option(TOAM_BUILD_ASSET_TOOLS "Build optional nifly circle converter" OFF)
    if(TOAM_BUILD_ASSET_TOOLS)
        set(TOAM_NIFLY_ROOT "" CACHE PATH "nifly source root")
        set(TOAM_NIFLY_LIBRARY "" CACHE FILEPATH "compiled nifly library")
        if(NOT EXISTS "${TOAM_NIFLY_ROOT}/include/NifFile.hpp" OR NOT EXISTS "${TOAM_NIFLY_LIBRARY}")
            message(FATAL_ERROR "Set TOAM_NIFLY_ROOT and TOAM_NIFLY_LIBRARY for optional asset tools")
        endif()
        add_executable(TOAMCircleAdapter tools/circle_adapter.cpp)
        target_include_directories(TOAMCircleAdapter PRIVATE "${TOAM_NIFLY_ROOT}/include" "${TOAM_NIFLY_ROOT}/external")
        target_link_libraries(TOAMCircleAdapter PRIVATE "${TOAM_NIFLY_LIBRARY}")
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
                                                       "${CMAKE_CURRENT_BINARY_DIR}/src")

    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES OUTPUT_NAME TOAM
                   PREFIX ""
                   SUFFIX ".dll")

    # Compiler/linker flags: /Zi emits PDB in all configs; Release gets full optimization + /DEBUG:FULL
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /Zi "$<$<CONFIG:Release>:/Zc:inline;/JMC->")
        target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
                            "$<$<CONFIG:Release>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>")
    endif()

    # LTO/IPO ??guarded: unconditionally setting IPO causes a fatal error on toolchains that don't support it
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _ipo_supported OUTPUT _ipo_output)
    if(_ipo_supported)
        set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    else()
        message(VERBOSE "IPO not supported: ${_ipo_output}")
    endif()

    # Deploy: copies DLL + PDB to SKYRIM_MODS_FOLDER, SKYRIM_FOLDER/Data, or build/deploy/ as fallback

    if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
        set(_default_deploy "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
    elseif(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
        set(_default_deploy "$ENV{SKYRIM_FOLDER}/Data")
    else()
        set(_default_deploy "${CMAKE_BINARY_DIR}/deploy/${PROJECT_NAME}")
    endif()

    set(DEPLOY_DIR_OVERRIDE
        ""
        CACHE
            PATH
            "Override deploy destination (leave empty to derive from SKYRIM_MODS_FOLDER or SKYRIM_FOLDER in the environment, optionally sourced from .env)"
    )

    # DEPLOY_DIR was previously written to the cache by the project and is no longer supported.
    # Clear it so stale cached values don't shadow the env-derived path. Users who want a
    # fixed deploy path should use -DDEPLOY_DIR_OVERRIDE=... instead.
    if(DEFINED CACHE{DEPLOY_DIR})
        message(WARNING "DEPLOY_DIR is no longer a cache variable and has been removed. "
                        "To pin a fixed deploy path, set -DDEPLOY_DIR_OVERRIDE=... instead.")
        unset(DEPLOY_DIR CACHE)
    endif()

    # Always recompute so environment changes take effect on every configure.
    # DEPLOY_DIR_OVERRIDE (if set) wins over the env-derived default.
    unset(DEPLOY_DIR CACHE)
    if(NOT DEPLOY_DIR_OVERRIDE STREQUAL "")
        set(DEPLOY_DIR "${DEPLOY_DIR_OVERRIDE}")
    else()
        set(DEPLOY_DIR "${_default_deploy}")
    endif()

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${DEPLOY_DIR}/SKSE/Plugins")

    # Debug symbols stay in build/msvc; the clean runtime install contains TOAM.dll only.

    # `cmake --build --preset deploy`  (or  cmake --workflow --preset deploy)
    add_custom_target(
        deploy
        COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}"
        DEPENDS ${PROJECT_NAME}
        COMMENT "Deploying to ${DEPLOY_DIR}/SKSE/Plugins")

endif() # PLUGIN_TESTS_ONLY
