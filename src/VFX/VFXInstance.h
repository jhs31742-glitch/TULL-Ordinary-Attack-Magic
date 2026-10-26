#pragma once

#include "VFX/AssetCatalog.h"

#include "RE/Skyrim.h"
#include "RE/B/BSGeometry.h"

#include <array>
#include <cstddef>

namespace TOAM::VFX
{
    class VFXInstance final
    {
    public:
        VFXInstance() = default;

        VFXInstance(
            const VFXInstance&) = delete;

        VFXInstance& operator=(
            const VFXInstance&) = delete;

        VFXInstance(
            VFXInstance&&) = default;

        VFXInstance& operator=(
            VFXInstance&&) = default;

        ~VFXInstance();

        [[nodiscard]]
        bool Spawn(
            AssetID asset,
            RE::NiNode* parent,
            const RE::NiTransform& local,
            bool visible = true) noexcept;

        void Release() noexcept;

        [[nodiscard]]
        bool SetLocalTransform(
            const RE::NiTransform& transform) noexcept;

        [[nodiscard]]
        bool SetVisible(
            bool visible) noexcept;

        [[nodiscard]]
        bool IsActive() const noexcept;

        [[nodiscard]]
        AssetID Asset() const noexcept;

        [[nodiscard]]
        RE::NiAVObject* Node() const noexcept;

    private:
        void CaptureBeamBounds() noexcept;
        void RefreshBeamBounds(
            const RE::NiTransform& transform) noexcept;
        void RepairWholeLensWorldBound() noexcept;

        // Per-clone data; original NIF assets remain unchanged.
        std::array<RE::BSGeometry*, 32> _beamMeshes{};
        std::array<float, 32> _beamOriginalRadii{};
        std::size_t _beamMeshCount{};

        RE::NiPointer<RE::NiAVObject>
            _node{};

        AssetID _asset{
            AssetID::kCount
        };
    };
}