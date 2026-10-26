#pragma once

#include "VFX/EngineCircle.h"

#include "RE/Skyrim.h"

#include <array>
#include <cstddef>

namespace TOAM::VFX
{
    struct CirclePose
    {
        RE::NiPoint3 translation{
            0.0F,
            0.0F,
            0.0F
        };

        RE::NiPoint3 rotationRadians{
            0.0F,
            0.0F,
            0.0F
        };

        float scale{
            1.0F
        };

        bool visible{
            true
        };
    };

    class CircleRenderer final
    {
    public:
        static constexpr
            std::size_t kCapacity{
                16
            };

        CircleRenderer() = default;
        void SetStyle(EngineCircle::Style style) noexcept { _style=style; }

        CircleRenderer(
            const CircleRenderer&) = delete;

        CircleRenderer& operator=(
            const CircleRenderer&) = delete;

        ~CircleRenderer();

        [[nodiscard]]
        bool Spawn(
            std::size_t slot,
            RE::NiNode* parent,
            AssetID asset,
            const CirclePose& pose) noexcept;

        [[nodiscard]]
        bool SpawnTransform(
            std::size_t slot,
            RE::NiNode* parent,
            AssetID asset,
            const RE::NiTransform& transform,
            bool visible) noexcept;

        [[nodiscard]]
        bool SetPose(
            std::size_t slot,
            const CirclePose& pose) noexcept;

        [[nodiscard]]
        bool SetTransform(
            std::size_t slot,
            const RE::NiTransform& transform,
            bool visible) noexcept;

        [[nodiscard]]
        bool SetVisible(
            std::size_t slot,
            bool visible) noexcept;

        [[nodiscard]]
        bool Release(
            std::size_t slot) noexcept;

        void ReleaseAll() noexcept;
        [[nodiscard]] bool BeginGrowth(std::size_t slot, float seconds) noexcept;
        [[nodiscard]] bool Retire(std::size_t slot, float holdSeconds) noexcept;
        void RetireAll(float holdSeconds) noexcept;

        [[nodiscard]]
        bool IsActive(
            std::size_t slot) const noexcept;

        [[nodiscard]]
        RE::NiAVObject* Node(
            std::size_t slot) noexcept;

        [[nodiscard]]
        std::size_t ActiveCount() const noexcept;

        [[nodiscard]]
        static bool IsCircleAsset(
            AssetID asset) noexcept;

        [[nodiscard]]
        static RE::NiTransform MakeTransform(
            const CirclePose& pose) noexcept;

    private:
        EngineCircle::Style _style{EngineCircle::Style::Original};
        std::array<RE::NiPointer<RE::NiNode>,kCapacity> _parents{};
        std::array<
            VFXHandle,
            kCapacity
        > _handles{};
    };
}