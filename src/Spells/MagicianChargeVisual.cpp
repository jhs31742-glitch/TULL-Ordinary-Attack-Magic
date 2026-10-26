#include "Arcana/Integration.h"
#include "VFX/CircleMotion.h"
#include "Spells/MagicianChargeVisual.h"

#include <array>
#include <cmath>

namespace TOAM::Spells::Magician
{
    namespace
    {
        constexpr std::array<float, kTierCount>
            kTargetScaleByCircle{{
                1.00F,
                1.35F,
                0.70F,
                1.55F,
                1.05F,
                0.65F,
                1.40F,
                0.90F
            }};

        constexpr float kCompressionStartCharge{
            600.0F
        };

        constexpr float kCompressionEndCharge{
            5000.0F
        };

        constexpr float kSpacingStart{
            22.0F
        };

        constexpr float kSpacingEnd{
            7.0F
        };

        constexpr float kNewCircleStartScaleFactor{
            0.02F
        };

        constexpr float kAppearanceSeconds{
            0.24F
        };

        constexpr float kBaseRotationRadiansPerSecond{
            0.55F
        };

        constexpr float kPerCircleRotationIncrement{
            0.08F
        };

        constexpr float kTierRotationAcceleration{
            0.12F
        };


        [[nodiscard]]
        constexpr bool ValidateCandidate() noexcept
        {
            if (
                kTargetScaleByCircle.size() !=
                    kTierCount ||
                kCompressionStartCharge <=
                    0.0F ||
                kCompressionEndCharge <=
                    kCompressionStartCharge ||
                kSpacingStart <=
                    kSpacingEnd ||
                kSpacingEnd <=
                    0.0F ||
                kNewCircleStartScaleFactor <=
                    0.0F ||
                kNewCircleStartScaleFactor >=
                    1.0F ||
                kAppearanceSeconds <=
                    0.0F
            ) {
                return false;
            }

            for (
                std::size_t index = 0;
                index < kTargetScaleByCircle.size();
                ++index
            ) {
                if (
                    kTargetScaleByCircle[index] <=
                        0.0F
                ) {
                    return false;
                }
            }

            return true;
        }


        static_assert(
            ValidateCandidate(),
            "Magician V6 visual candidate invalid"
        );


        [[nodiscard]]
        float Clamp01(
            const float value) noexcept
        {
            if (value <= 0.0F) {
                return 0.0F;
            }

            if (value >= 1.0F) {
                return 1.0F;
            }

            return value;
        }


        [[nodiscard]]
        float QuinticSmootherStep(
            const float progress) noexcept
        {
            const float value =
                Clamp01(
                    progress
                );

            return
                value *
                value *
                value *
                (
                    value *
                    (
                        value *
                        6.0F -
                        15.0F
                    ) +
                    10.0F
                );
        }


        [[nodiscard]]
        VFX::AssetID CircleAssetForIndex(
            const std::size_t index) noexcept
        {
            switch (index) {
            case 1:
            case 3:
            case 5:
            case 6:
                return
                    VFX::AssetID::
                        kCircleOuterBandStatic;

            case 0:
            case 2:
            case 4:
            case 7:
            default:
                return
                    VFX::AssetID::
                        kCircleStatic;
            }
        }
    }


    ChargeVisual::~ChargeVisual()
    {
        ReleaseAll();
    }


    float ChargeVisual::TuningSpacingForCharge(
        const float effectiveCharge) noexcept
    {
        if (
            !std::isfinite(
                effectiveCharge
            ) ||
            effectiveCharge <=
                kCompressionStartCharge
        ) {
            return kSpacingStart;
        }

        if (
            effectiveCharge >=
            kCompressionEndCharge
        ) {
            return kSpacingEnd;
        }

        const float rawProgress =
            (
                effectiveCharge -
                kCompressionStartCharge
            ) /
            (
                kCompressionEndCharge -
                kCompressionStartCharge
            );

        const float progress =
            QuinticSmootherStep(
                rawProgress
            );

        return
            kSpacingStart +
            (
                kSpacingEnd -
                kSpacingStart
            ) *
            progress;
    }


    float ChargeVisual::TuningSpacingForTier(
        const std::uint8_t tier) noexcept
    {
        if (
            tier == 0 ||
            tier >
                kTierCount
        ) {
            return 0.0F;
        }

        if (tier == 1) {
            return 0.0F;
        }

        const auto* profile =
            GetTierProfile(
                tier
            );

        if (!profile) {
            return 0.0F;
        }

        return
            TuningSpacingForCharge(
                profile->chargeThreshold
            );
    }


    float ChargeVisual::TuningTargetScale(
        const std::size_t circleIndex) noexcept
    {
        if (
            circleIndex >=
            kCircleCapacity
        ) {
            return 0.0F;
        }

        return
            Arcana::HostSunScale(circleIndex);
    }


    float ChargeVisual::TuningRotationSpeed(
        const std::size_t circleIndex,
        const std::uint8_t tier) noexcept
    {
        if (
            circleIndex >=
                kCircleCapacity ||
            tier == 0 ||
            tier >
                kTierCount
        ) {
            return 0.0F;
        }

        return VFX::CircleMotion::LayerSpeed(circleIndex,static_cast<float>(tier));
    }


    VFX::CirclePose ChargeVisual::BuildTargetPose(
        const std::size_t circleIndex,
        const std::uint8_t tier,
        const float effectiveCharge) noexcept
    {
        VFX::CirclePose pose{};

        if (
            circleIndex >=
                kCircleCapacity ||
            tier == 0 ||
            tier >
                kTierCount ||
            circleIndex >=
                static_cast<std::size_t>(
                    tier
                )
        ) {
            pose.visible =
                false;

            return pose;
        }

        const float spacing =
            TuningSpacingForCharge(
                effectiveCharge
            );

        pose.translation =
            RE::NiPoint3{
                0.0F,
                circleIndex == 0 ?
                    0.0F :
                    spacing *
                    static_cast<float>(
                        circleIndex
                    ),
                0.0F
            };

        pose.rotationRadians =
            RE::NiPoint3{
                0.0F,
                0.0F,
                0.0F
            };

        pose.scale =
            TuningTargetScale(
                circleIndex
            );

        pose.visible =
            true;

        return pose;
    }


    bool ChargeVisual::Begin(
        RE::NiNode* parent) noexcept
    {
        ReleaseAll();

        if (!parent) {
            return false;
        }

        _renderer.SnapshotColor(Presentation::Spell::Magician);
        _renderer.SetStyle(VFX::EngineCircle::Style::SunFull);
        _parent =
            parent;

        return true;
    }


    bool ChargeVisual::SetAnchorTransform(
        const RE::NiTransform& anchorLocal) noexcept
    {
        if (!_parent) {
            return false;
        }

        _anchorLocal =
            anchorLocal;

        _anchorReady =
            true;

        return true;
    }


    bool ChargeVisual::SpawnNewCircle(
        const std::size_t circleIndex,
        const VFX::CirclePose& target) noexcept
    {
        if (
            !_parent ||
            !_anchorReady ||
            circleIndex >=
                kCircleCapacity ||
            !target.visible ||
            !(target.scale > 0.0F)
        ) {
            return false;
        }

        VFX::CirclePose start =
            target;

        if (circleIndex != 0) {
            start.scale =
                target.scale *
                kNewCircleStartScaleFactor;
        }

        if (!(start.scale > 0.0F)) {
            return false;
        }

        const auto asset =
            CircleAssetForIndex(
                circleIndex
            );

        const RE::NiTransform local =
            _anchorLocal *
            VFX::CircleRenderer::
                MakeTransform(
                    start
                );

        if (
            !_renderer.SpawnTransform(
                circleIndex,
                _parent,
                asset,
                local,
                start.visible
            )
        ) {
            return false;
        }

        static_cast<void>(_renderer.BeginGrowth(circleIndex,kAppearanceSeconds));
        _active[circleIndex] =
            true;

        _poses[circleIndex] =
            start;

        _rotationRadians[circleIndex] =
            0.0F;

        _tweens[circleIndex].
            Snap(
                start
            );

        // C1 is the fixed full-size anchor.
        if (circleIndex == 0) {
            return true;
        }

        if (
            !_tweens[circleIndex].
                Begin(
                    start,
                    target,
                    kAppearanceSeconds,
                    VFX::TransitionCurve::
                        kSmootherStep
                )
        ) {
            static_cast<void>(
                _renderer.Release(
                    circleIndex
                )
            );

            _active[circleIndex] =
                false;

            return false;
        }

        return true;
    }


    bool ChargeVisual::SetTier(
        const std::uint8_t tier) noexcept
    {
        if (
            !_parent ||
            tier >
                kTierCount
        ) {
            return false;
        }

        if (tier == 0) {
            ReleaseAll();
            return true;
        }

        const std::size_t newCount =
            static_cast<std::size_t>(
                tier
            );

        for (
            std::size_t index = newCount;
            index < kCircleCapacity;
            ++index
        ) {
            if (!_active[index]) {
                continue;
            }

            static_cast<void>(
                _renderer.Release(
                    index
                )
            );

            _tweens[index] =
                {};

            _poses[index] =
                {};

            _rotationRadians[index] =
                0.0F;

            _active[index] =
                false;
        }

        for (
            std::size_t index = 0;
            index < newCount;
            ++index
        ) {
            if (_active[index]) {
                continue;
            }

            const VFX::CirclePose target =
                BuildTargetPose(
                    index,
                    tier,
                    _effectiveCharge
                );

            if (
                !target.visible ||
                !SpawnNewCircle(
                    index,
                    target
                )
            ) {
                return false;
            }
        }

        _tier =
            tier;

        return true;
    }


    bool ChargeVisual::SetEffectiveCharge(
        const float effectiveCharge) noexcept
    {
        if (
            !std::isfinite(
                effectiveCharge
            ) ||
            effectiveCharge <
                0.0F
        ) {
            return false;
        }

        _effectiveCharge =
            effectiveCharge;

        const std::uint8_t resolvedTier =
            ResolveTierNumber(
                effectiveCharge
            );

        if (resolvedTier == 0) {

            // Preserve Begin()/anchor state before T1.
            if (_tier != 0) {
                ReleaseAll();
            }

            return true;
        }

        if (resolvedTier == _tier) {
            return true;
        }

        return
            SetTier(
                resolvedTier
            );
    }


    bool ChargeVisual::Update(
        const float deltaSeconds) noexcept
    {
        if (
            !_parent ||
            !_anchorReady ||
            !std::isfinite(
                deltaSeconds
            ) ||
            deltaSeconds <=
                0.0F
        ) {
            return false;
        }

        if (_tier == 0) {
            return true;
        }

        const std::size_t visibleCount =
            static_cast<std::size_t>(
                _tier
            );

        for (
            std::size_t index = 0;
            index < visibleCount;
            ++index
        ) {
            if (!_active[index]) {
                return false;
            }

            VFX::CirclePose pose =
                _tweens[index].
                    IsActive() ?
                    _tweens[index].
                        Advance(
                            deltaSeconds
                        ) :
                    _tweens[index].
                        Current();

            const VFX::CirclePose target =
                BuildTargetPose(
                    index,
                    _tier,
                    _effectiveCharge
                );

            if (!target.visible) {
                return false;
            }

            // Compression is driven continuously by current effective MP,
            // never restarted at tier boundaries.
            pose.translation =
                target.translation;

            if (
                !_tweens[index].
                    IsActive()
            ) {
                pose.scale =
                    target.scale;
            }

            _rotationRadians[index] =
                VFX::AdvanceWrappedRadians(
                    _rotationRadians[index],
                    TuningRotationSpeed(
                        index,
                        _tier
                    ),
                    deltaSeconds
                );

            pose.rotationRadians.y =
                _rotationRadians[index];

            pose.visible =
                true;

            const RE::NiTransform local =
                _anchorLocal *
                VFX::CircleRenderer::
                    MakeTransform(
                        pose
                    );

            if (
                !_renderer.SetTransform(
                    index,
                    local,
                    pose.visible
                )
            ) {
                return false;
            }

            _poses[index] =
                pose;
        }

        return true;
    }


    void ChargeVisual::RetireAll(float seconds) noexcept
    {
        _renderer.RetireAll(seconds);
        ReleaseAll(); // Handles already transferred to bounded EngineCircle retirement.
    }


    void ChargeVisual::ReleaseAll() noexcept
    {
        _renderer.ReleaseAll();

        for (
            std::size_t index = 0;
            index < kCircleCapacity;
            ++index
        ) {
            _tweens[index] =
                {};

            _poses[index] =
                {};

            _rotationRadians[index] =
                0.0F;

            _active[index] =
                false;
        }

        _anchorLocal =
            {};

        _parent =
            nullptr;

        _anchorReady =
            false;

        _effectiveCharge =
            0.0F;

        _tier =
            0;
    }


    bool ChargeVisual::IsActive() const noexcept
    {
        return
            _parent != nullptr &&
            _tier != 0;
    }


    std::uint8_t ChargeVisual::Tier() const noexcept
    {
        return _tier;
    }


    std::size_t ChargeVisual::VisibleCircleCount() const noexcept
    {
        std::size_t count{};

        for (
            const bool active :
            _active
        ) {
            if (active) {
                ++count;
            }
        }

        return count;
    }
}