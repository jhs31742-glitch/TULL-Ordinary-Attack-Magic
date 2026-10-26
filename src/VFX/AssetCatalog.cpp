#include "VFX/AssetCatalog.h"

#include "RE/Skyrim.h"

#include <array>
#include <cstddef>

namespace TOAM::VFX
{
    namespace
    {
        struct AssetSpec
        {
            const char* name;
            const char* path;
        };

        constexpr std::array<
            AssetSpec,
            static_cast<std::size_t>(
                AssetID::kCount
            )
        > kAssets{{
            {
                "CircleStatic",
                "meshes\\TOAM\\VFX\\Circles\\TOAM_Circle_Static.nif"
            },
            {
                "CircleCoreStatic",
                "meshes\\TOAM\\VFX\\Circles\\TOAM_Circle_Core_Static.nif"
            },
            {
                "CircleAnimated",
                "meshes\\TOAM\\VFX\\Circles\\TOAM_Circle_Animated.nif"
            },
            {
                "CircleCoreAnimated",
                "meshes\\TOAM\\VFX\\Circles\\TOAM_Circle_Core_Animated.nif"
            },
            {
                "CircleSmall",
                "meshes\\TOAM\\VFX\\Circles\\TOAM_Circle_Small.nif"
            },
            {
                "BeamBodyUnit",
                "meshes\\TOAM\\VFX\\Beams\\Beam_Body_Unit.nif"
            },
            {
                "BeamBodyUnitStatic",
                "meshes\\TOAM\\VFX\\Beams\\Beam_Body_Unit_Static.nif"
            },
            {
                "BeamMuzzle",
                "meshes\\TOAM\\VFX\\Beams\\Beam_Muzzle.nif"
            },
            {
                "ImpactLow",
                "meshes\\TOAM\\VFX\\Explosions\\Impact_Low.nif"
            },
            {
                "ImpactMid",
                "meshes\\TOAM\\VFX\\Explosions\\Impact_Mid.nif"
            },
            {
                "ImpactHigh",
                "meshes\\TOAM\\VFX\\Explosions\\Impact_High.nif"
            },
            {
                "ImpactHighCompact",
                "meshes\\TOAM\\VFX\\Explosions\\Impact_High_Compact.nif"
            },
            {
                "CircleOuterBandStatic",
                "meshes\\TOAM\\VFX\\Circles\\TOAM_Circle_OuterBand_Static.nif"
            },
            {"BeamWholeLens", "meshes\\TOAM\\VFX\\Beams\\TOAM_Beam_WholeLens.nif"},
            {"BeamWholeLensTintable", "meshes\\TOAM\\VFX\\Beams\\TOAM_Beam_WholeLens_Tintable.nif"},
            {"FlashCloud", "meshes\\TOAM\\VFX\\Explosions\\TOAM_Explosion_BlueWhite_FlashCloud.nif"},
            {"FlashCloudTintable", "meshes\\TOAM\\VFX\\Explosions\\TOAM_Explosion_BlueWhite_FlashCloud_Tintable.nif"}
        }};

        [[nodiscard]]
        constexpr std::size_t ToIndex(
            const AssetID id) noexcept
        {
            return static_cast<std::size_t>(id);
        }
    }

    const char* AssetName(
        const AssetID id) noexcept
    {
        const auto index =
            ToIndex(id);

        if (index >= kAssets.size()) {
            return "Invalid";
        }

        return kAssets[index].name;
    }

    const char* AssetPath(
        const AssetID id) noexcept
    {
        const auto index =
            ToIndex(id);

        if (index >= kAssets.size()) {
            return "";
        }

        return kAssets[index].path;
    }

    bool ProbeAsset(
        const AssetID id) noexcept
    {
        const char* path =
            AssetPath(id);

        if (!path || path[0] == '\0') {
            return false;
        }

        RE::NiPointer<RE::NiNode>
            model;

        RE::BSModelDB::DBTraits::ArgsType
            args{};

        const auto result =
            RE::BSModelDB::Demand(
                path,
                model,
                args
            );

        const bool ready =
            result ==
                RE::BSResource::ErrorCode::kNone &&
            static_cast<bool>(model);

        if (!ready) {

            logger::error(
                "[TOAM][VFX] Asset probe failed | Asset={} | Path={} | Error={}",
                AssetName(id),
                path,
                static_cast<std::int32_t>(result));
        }

        return ready;
    }

    bool ProbeCoreAssets() noexcept
    {
        bool allReady =
            true;

        for (
            std::size_t index = 0;
            index < static_cast<std::size_t>(AssetID::kCount);
            ++index
        ) {
            const auto id =
                static_cast<AssetID>(index);

            if (!ProbeAsset(id)) {
                allReady = false;
            }
        }

        logger::info(
            "[TOAM][VFX] Core asset probe | Ready={} | Count={}",
            allReady,
            static_cast<std::size_t>(AssetID::kCount));

        return allReady;
    }
}