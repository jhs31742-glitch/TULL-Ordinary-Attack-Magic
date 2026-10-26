#pragma once

#include <cstdint>

namespace TOAM::Combat
{
    enum class FirewallOrigin : std::uint8_t
    {
        kNone = 0,
        kMagicianBeam,
        kMagicianExplosion,
        kJudgement,
        kWorld,
        kTemperanceAssist
    };

    namespace detail
    {
        inline thread_local std::uint32_t g_firewallDepth{};
        inline thread_local FirewallOrigin g_firewallOrigin{
            FirewallOrigin::kNone
        };
    }

    class ScopedTriggerFirewall final
    {
    public:
        explicit ScopedTriggerFirewall(
            FirewallOrigin origin) noexcept :
            previousOrigin_(detail::g_firewallOrigin)
        {
            ++detail::g_firewallDepth;

            if (origin != FirewallOrigin::kNone) {
                detail::g_firewallOrigin = origin;
            }
        }

        ~ScopedTriggerFirewall()
        {
            if (detail::g_firewallDepth > 0) {
                --detail::g_firewallDepth;
            }

            detail::g_firewallOrigin =
                detail::g_firewallDepth == 0 ?
                    FirewallOrigin::kNone :
                    previousOrigin_;
        }

        ScopedTriggerFirewall(
            const ScopedTriggerFirewall&) = delete;

        ScopedTriggerFirewall& operator=(
            const ScopedTriggerFirewall&) = delete;

    private:
        FirewallOrigin previousOrigin_{
            FirewallOrigin::kNone
        };
    };

    [[nodiscard]] inline bool
        IsTriggerSuppressed() noexcept
    {
        return detail::g_firewallDepth != 0;
    }

    [[nodiscard]] inline FirewallOrigin
        GetFirewallOrigin() noexcept
    {
        return detail::g_firewallOrigin;
    }

    inline void ResetTransientState() noexcept
    {
        detail::g_firewallDepth = 0;
        detail::g_firewallOrigin = FirewallOrigin::kNone;
    }
}