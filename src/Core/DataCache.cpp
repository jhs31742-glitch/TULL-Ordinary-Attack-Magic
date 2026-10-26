#include "Core/DataCache.h"

#include <array>
#include <string_view>

namespace TOAM::Core::DataCache
{
    namespace
    {
        using SpellArray = std::array<
            RE::SpellItem*,
            static_cast<std::size_t>(SpellID::kCount)>;

        SpellArray g_spells{};
        bool g_ready{};

        constexpr std::array<std::string_view, 4>
            kEditorIDs{
                "TOAM_Magician",
                "TOAM_Judgement",
                "TOAM_TheWorld",
                "TOAM_Temperance"
            };

        [[nodiscard]] constexpr std::size_t
            ToIndex(SpellID id) noexcept
        {
            return static_cast<std::size_t>(id);
        }
    }

    void Reset() noexcept
    {
        g_spells.fill(nullptr);
        g_ready = false;
    }

    bool ResolveAll() noexcept
    {
        Reset();

        bool allResolved = true;

        for (std::size_t i = 0; i < kEditorIDs.size(); ++i) {
            g_spells[i] =
                RE::TESForm::LookupByEditorID<RE::SpellItem>(
                    kEditorIDs[i]);

            if (!g_spells[i]) {
                logger::error(
                    "[TOAM][DataCache] Missing form | EditorID={}",
                    kEditorIDs[i]);

                allResolved = false;
            }
        }

        g_ready = allResolved;

        if (g_ready) {
            logger::info(
                "[TOAM][DataCache] READY | Forms=4/4");
        } else {
            std::size_t resolved{};

            for (auto* spell : g_spells) {
                if (spell) {
                    ++resolved;
                }
            }

            logger::error(
                "[TOAM][DataCache] NOT_READY | Forms={}/4",
                resolved);
        }

        return g_ready;
    }

    bool IsReady() noexcept
    {
        return g_ready;
    }

    RE::SpellItem* GetSpell(SpellID id) noexcept
    {
        const auto index = ToIndex(id);

        if (index >= g_spells.size()) {
            return nullptr;
        }

        return g_spells[index];
    }

    RE::SpellItem* GetMagician() noexcept
    {
        return GetSpell(SpellID::kMagician);
    }

    RE::SpellItem* GetJudgement() noexcept
    {
        return GetSpell(SpellID::kJudgement);
    }

    RE::SpellItem* GetTheWorld() noexcept
    {
        return GetSpell(SpellID::kTheWorld);
    }

    RE::SpellItem* GetTemperance() noexcept
    {
        return GetSpell(SpellID::kTemperance);
    }
}