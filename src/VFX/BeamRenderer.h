#pragma once

#include "VFX/TransientVFXManager.h"

#include "RE/Skyrim.h"

#include <array>
#include <cstddef>

namespace TOAM::VFX
{
    struct BeamPose
    {
        RE::NiPoint3 origin{
            0.0F,
            0.0F,
            0.0F
        };

        RE::NiPoint3 direction{
            0.0F,
            1.0F,
            0.0F
        };

        float length{
            0.0F
        };

        float widthScale{
            1.0F
        };

        float lifetimeSeconds{
            -1.0F
        };

        bool visible{
            true
        };

        AssetID asset{
            AssetID::kBeamBodyUnitStatic
        };
    };

    class BeamRenderer final
    {
    public:
        static constexpr
            float kUnitLength{
                512.0F
            };

        static constexpr
            std::size_t kMaxSegments{
                32
            };

        static constexpr
            float kMaxLength{
                kUnitLength *
                static_cast<float>(
                    kMaxSegments
                )
            };

        BeamRenderer() = default;

        BeamRenderer(
            const BeamRenderer&) = delete;

        BeamRenderer& operator=(
            const BeamRenderer&) = delete;

        ~BeamRenderer();

        [[nodiscard]]
        bool SetBeam(
            RE::NiNode* parent,
            const BeamPose& pose) noexcept;

        [[nodiscard]]
        bool UpdateBeam(
            const BeamPose& pose) noexcept;

        void Release() noexcept;

        // Transfer positive-lifetime visuals to the existing transient manager.
        void Detach() noexcept;

        [[nodiscard]] static bool IsWholeBeamAsset(AssetID asset) noexcept;

        [[nodiscard]]
        bool IsActive() const noexcept;

        [[nodiscard]]
        std::size_t SegmentCount() const noexcept;

        [[nodiscard]]
        static std::size_t RequiredSegments(
            float length) noexcept;

        [[nodiscard]]
        static bool IsBeamBodyAsset(
            AssetID asset) noexcept;

    private:
        struct Basis
        {
            RE::NiPoint3 x{};
            RE::NiPoint3 y{};
            RE::NiPoint3 z{};
        };

        [[nodiscard]]
        static bool BuildBasis(
            const RE::NiPoint3& direction,
            Basis& result) noexcept;

        [[nodiscard]]
        static bool IsPoseValid(
            const BeamPose& pose) noexcept;

        [[nodiscard]]
        static RE::NiTransform MakeSegmentTransform(
            const RE::NiPoint3& segmentOrigin,
            const Basis& basis,
            float longitudinalScale,
            float widthScale) noexcept;

        [[nodiscard]]
        bool ApplyExisting(
            const BeamPose& pose,
            const Basis& basis,
            std::size_t requiredSegments) noexcept;

        std::array<
            VFXHandle,
            kMaxSegments
        > _segments{};

        RE::NiNode* _parent{};

        AssetID _asset{
            AssetID::kCount
        };

        std::size_t _segmentCount{};
    };
}