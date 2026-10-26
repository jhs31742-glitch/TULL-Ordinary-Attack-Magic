#include "Arcana/Integration.h"
#include "VFX/EngineCircle.h"
#include "PCH.h"
#include "Core/Runtime.h"
#include "Core/DamageSettings.h"

namespace logger = SKSE::log;

namespace
{
    void OnSKSEMessage(
        SKSE::MessagingInterface::Message* message)
    {
        if (!message) {
            return;
        }

        if(message->type==SKSE::MessagingInterface::kDataLoaded ||
           message->type==SKSE::MessagingInterface::kPreLoadGame ||
           message->type==SKSE::MessagingInterface::kPostLoadGame ||
           message->type==SKSE::MessagingInterface::kNewGame) {
            TOAM::VFX::EngineCircle::Reset();
        }
        Arcana::Message(message);
        switch (message->type) {
        case SKSE::MessagingInterface::kPostPostLoad:
            TOAM::Core::DamageSettings::RegisterMenu();
            break;
        case SKSE::MessagingInterface::kDataLoaded:
            logger::info("[TOAM][Plugin] Message=DataLoaded");
            TOAM::Core::Runtime::OnDataLoaded();
            break;

        case SKSE::MessagingInterface::kPreLoadGame:
            logger::info("[TOAM][Plugin] Message=PreLoadGame");
            TOAM::Core::Runtime::OnPreLoadGame();
            break;

        case SKSE::MessagingInterface::kPostLoadGame:
            logger::info("[TOAM][Plugin] Message=PostLoadGame");
            TOAM::Core::Runtime::OnPostLoadGame();
            break;

        case SKSE::MessagingInterface::kNewGame:
            logger::info("[TOAM][Plugin] Message=NewGame");
            TOAM::Core::Runtime::OnNewGame();
            break;

        default:
            break;
        }
    }
}
SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    SKSE::AllocTrampoline(16);

    auto logDir = logger::log_directory();
    if (!logDir) {
        return false;
    }

    *logDir /= "OrdinaryAttackMagic.log";

    auto sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            logDir->string(), true);

    auto log =
        std::make_shared<spdlog::logger>(
            "OrdinaryAttackMagic", std::move(sink));

    spdlog::set_default_logger(std::move(log));
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);

    logger::info("Ordinary Attack Magic loaded");
    logger::info("DLL bootstrap PASS");

    if(GetFileAttributesW(L"Data\\SKSE\\Plugins\\ArcanaStaffs.dll") != INVALID_FILE_ATTRIBUTES ||
       GetFileAttributesW(L"Data\\SKSE\\Plugins\\OrdinaryOffensiveMagic.dll") != INVALID_FILE_ATTRIBUTES ||
       GetFileAttributesW(L"Data\\SKSE\\Plugins\\TOOM.dll") != INVALID_FILE_ATTRIBUTES ||
       GetFileAttributesW(L"Data\\SKSE\\Plugins\\OrdinaryAttackMagic.dll") != INVALID_FILE_ATTRIBUTES) {
        logger::critical("[TOAM][CP019] Legacy ArcanaStaffs.dll and OrdinaryAttackMagic.dll must be disabled; TOAM.dll is the only host");
        return false;
    }
    Arcana::ConfigureFormSource("TULL Ordinary Attack Magic.esp",0);
    if(REL::Module::get().version()!=SKSE::RUNTIME_SSE_1_5_97) {
        logger::critical("[TOAM][Compatibility] AE preparation is incomplete: Hermit/Tower precommit ABI and audio file contract need native verification. No hooks installed. Runtime={}",REL::Module::get().version().string());
        return false;
    }
    auto* messaging =
        SKSE::GetMessagingInterface();

    if (!messaging) {
        logger::critical(
            "[TOAM][Plugin] MessagingInterface unavailable");
        return false;
    }

    if (!messaging->RegisterListener(OnSKSEMessage)) {
        logger::critical(
            "[TOAM][Plugin] RegisterListener failed");
        return false;
    }

    static_cast<void>(TOAM::Core::DamageSettings::Reload());
    if(!Arcana::InstallHooks()) {return false;}

    logger::info(
        "[TOAM][Plugin] Lifecycle listener registered");

    return true;
}
