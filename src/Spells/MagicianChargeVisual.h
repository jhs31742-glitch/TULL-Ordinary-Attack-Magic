#pragma once

#include "Spells/MagicianProfile.h"

#include "VFX/Choreography.h"
#include "VFX/CircleRenderer.h"

#include "RE/Skyrim.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace TOAM::Spells::Magician
{
    class ChargeVisual final
    {
    public:
        static constexpr std::size_t kCircleCapacity{
            kTierCount
        };

        static float TuningBeamDepth(float charge) noexcept {
            const auto tier=ResolveTierNumber(charge);
            return TuningSpacingForCharge(charge)*float(tier>0?tier-1:0)+18.0F;
        }
        ChargeVisual() = default;
        ChargeVisual(const ChargeVisual&) = delete;
        ChargeVisual& operator=(const ChargeVisual&) = delete;
        ~ChargeVisual();

        [[nodiscard]]
        bool Begin(
            RE::NiNode* parent) noexcept;

        [[nodiscard]]
        bool SetAnchorTransform(
            const RE::NiTransform& anchorLocal) noexcept;

        [[nodiscard]]
        bool SetEffectiveCharge(
            float effectiveCharge) noexcept;

        [[nodiscard]]
        bool SetTier(
            std::uint8_t tier) noexcept;

        [[nodiscard]]
        bool Update(
            float deltaSeconds) noexcept;

        void ReleaseAll() noexcept;
        void RetireAll(float seconds) noexcept;

        [[nodiscard]]
        bool IsActive() const noexcept;

        [[nodiscard]]
        std::uint8_t Tier() const noexcept;

        [[nodiscard]]
        std::size_t VisibleCircleCount() const noexcept;

        [[nodiscard]]
        static float TuningSpacingForTier(
            std::uint8_t tier) noexcept;

        [[nodiscard]]
        static float TuningTargetScale(
            std::size_t circleIndex) noexcept;

        [[nodiscard]]
        static float TuningRotationSpeed(
            std::size_t circleIndex,
            std::uint8_t tier) noexcept;

    private:
        [[nodiscard]]
        static float TuningSpacingForCharge(
            float effectiveCharge) noexcept;

        [[nodiscard]]
        static VFX::CirclePose BuildTargetPose(
            std::size_t circleIndex,
            std::uint8_t tier,
            float effectiveCharge) noexcept;

        [[nodiscard]]
        bool SpawnNewCircle(
            std::size_t circleIndex,
            const VFX::CirclePose& target) noexcept;

        VFX::CircleRenderer _renderer{};

        std::array<
            VFX::CirclePoseTween,
            kCircleCapacity
        > _tweens{};

        std::array<
            VFX::CirclePose,
            kCircleCapacity
        > _poses{};

        std::array<
            float,
            kCircleCapacity
        > _rotationRadians{};

        std::array<
            bool,
            kCircleCapacity
        > _active{};

        RE::NiTransform _anchorLocal{};

        RE::NiNode* _parent{};

        bool _anchorReady{};

        float _effectiveCharge{};

        std::uint8_t _tier{};
    };
}