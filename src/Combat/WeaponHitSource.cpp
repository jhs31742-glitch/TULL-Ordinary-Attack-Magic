#include "Combat/WeaponHitSource.h"

namespace TOAM::Combat::WeaponHitSource
{
    namespace
    {
        Callback g_callback{};
        bool g_installed{};

        void Observe(
            RE::TESObjectREFR* targetRef,
            RE::Actor* aggressor,
            RE::TESObjectWEAP* weapon,
            const RE::HitData& hitData) noexcept
        {
            auto* player =
                RE::PlayerCharacter::GetSingleton();

            if (!player ||
                aggressor != player ||
                !targetRef ||
                !weapon) {
                return;
            }

            auto* target =
                targetRef->As<RE::Actor>();

            if (!target) {
                return;
            }

            const bool powerAttack =
                hitData.flags.any(
                    RE::HitData::Flag::kPowerAttack);

            if (g_callback) {
                g_callback(
                    target,
                    weapon,
                    hitData,
                    powerAttack);
            }
        }

        namespace Hook
        {
            static inline constexpr REL::RelocationID kTargetID{
                37633,
                38586
            };

            static inline constexpr REL::VariantOffset kTargetOffset{
                0x16A,
                0xFA,
                0x0
            };

            namespace SE
            {
                using func_t = void (*)(
                    RE::ScriptEventSourceHolder*,
                    RE::NiPointer<RE::TESObjectREFR>&,
                    RE::NiPointer<RE::TESObjectREFR>&,
                    RE::FormID,
                    RE::FormID,
                    RE::HitData&);

                static inline func_t func{};

                void thunk(
                    RE::ScriptEventSourceHolder* holder,
                    RE::NiPointer<RE::TESObjectREFR>& target,
                    RE::NiPointer<RE::TESObjectREFR>& aggressor,
                    RE::FormID source,
                    RE::FormID projectile,
                    RE::HitData& hitData)
                {
                    auto* aggressorRef = aggressor.get();

                    Observe(
                        target.get(),
                        aggressorRef ?
                            aggressorRef->As<RE::Actor>() :
                            nullptr,
                        hitData.weapon,
                        hitData);

                    func(
                        holder,
                        target,
                        aggressor,
                        source,
                        projectile,
                        hitData);
                }
            }

            namespace AE
            {
                using func_t = void (*)(
                    RE::AIProcess*,
                    RE::HitData&);

                static inline func_t func{};

                void thunk(
                    RE::AIProcess* targetProcess,
                    RE::HitData& hitData)
                {
                    func(targetProcess, hitData);

                    auto* target =
                        targetProcess ?
                            targetProcess->GetUserData() :
                            nullptr;

                    auto aggressorPtr =
                        hitData.aggressor.get();

                    Observe(
                        target,
                        aggressorPtr.get(),
                        hitData.weapon,
                        hitData);
                }
            }
        }
    }

    void SetCallback(Callback callback) noexcept
    {
        g_callback = callback;
    }

    bool IsInstalled() noexcept
    {
        return g_installed;
    }

    void Install()
    {
        if (g_installed) {
            return;
        }

        REL::Relocation<std::uintptr_t> target{
            Hook::kTargetID,
            Hook::kTargetOffset
        };

        auto& trampoline = SKSE::GetTrampoline();

        if (REL::Module::IsAE()) {
            const auto original =
                trampoline.write_call<5>(
                    target.address(),
                    Hook::AE::thunk);

            Hook::AE::func =
                reinterpret_cast<Hook::AE::func_t>(
                    original);
        } else {
            const auto original =
                trampoline.write_call<5>(
                    target.address(),
                    Hook::SE::thunk);

            Hook::SE::func =
                reinterpret_cast<Hook::SE::func_t>(
                    original);
        }

        g_installed = true;
    }
}