#include "VFX/Choreography.h"

#include <cmath>

namespace TOAM::VFX
{
    namespace
    {
        constexpr float kPi{
            3.14159265358979323846F
        };

        constexpr float kTau{
            6.28318530717958647692F
        };

        constexpr float kVectorEpsilon{
            0.000001F
        };

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
        bool IsFinitePoint(
            const RE::NiPoint3& value) noexcept
        {
            return
                std::isfinite(value.x) &&
                std::isfinite(value.y) &&
                std::isfinite(value.z);
        }

        [[nodiscard]]
        bool IsFiniteCirclePose(
            const CirclePose& pose) noexcept
        {
            return
                IsFinitePoint(
                    pose.translation
                ) &&
                IsFinitePoint(
                    pose.rotationRadians
                ) &&
                std::isfinite(
                    pose.scale
                ) &&
                pose.scale >
                    0.0F;
        }

        [[nodiscard]]
        bool IsFiniteBeamPose(
            const BeamPose& pose) noexcept
        {
            return
                IsFinitePoint(
                    pose.origin
                ) &&
                IsFinitePoint(
                    pose.direction
                ) &&
                std::isfinite(
                    pose.length
                ) &&
                pose.length >
                    0.0F &&
                std::isfinite(
                    pose.widthScale
                ) &&
                pose.widthScale >
                    0.0F &&
                BeamRenderer::
                    IsBeamBodyAsset(
                        pose.asset
                    );
        }

        [[nodiscard]]
        float Lerp(
            const float start,
            const float target,
            const float progress) noexcept
        {
            return
                start +
                (
                    target -
                    start
                ) *
                progress;
        }

        [[nodiscard]]
        RE::NiPoint3 LerpPoint(
            const RE::NiPoint3& start,
            const RE::NiPoint3& target,
            const float progress) noexcept
        {
            return RE::NiPoint3{
                Lerp(
                    start.x,
                    target.x,
                    progress
                ),
                Lerp(
                    start.y,
                    target.y,
                    progress
                ),
                Lerp(
                    start.z,
                    target.z,
                    progress
                )
            };
        }

        [[nodiscard]]
        float WrapSignedRadians(
            const float radians) noexcept
        {
            if (!std::isfinite(radians)) {
                return 0.0F;
            }

            float wrapped =
                std::fmod(
                    radians + kPi,
                    kTau
                );

            if (wrapped < 0.0F) {
                wrapped +=
                    kTau;
            }

            return
                wrapped -
                kPi;
        }

        [[nodiscard]]
        float LerpAngle(
            const float start,
            const float target,
            const float progress) noexcept
        {
            const float delta =
                WrapSignedRadians(
                    target -
                    start
                );

            return
                WrapSignedRadians(
                    start +
                    delta *
                    progress
                );
        }

        [[nodiscard]]
        RE::NiPoint3 LerpEuler(
            const RE::NiPoint3& start,
            const RE::NiPoint3& target,
            const float progress) noexcept
        {
            return RE::NiPoint3{
                LerpAngle(
                    start.x,
                    target.x,
                    progress
                ),
                LerpAngle(
                    start.y,
                    target.y,
                    progress
                ),
                LerpAngle(
                    start.z,
                    target.z,
                    progress
                )
            };
        }

        [[nodiscard]]
        float VectorLengthSquared(
            const RE::NiPoint3& value) noexcept
        {
            return
                value.x * value.x +
                value.y * value.y +
                value.z * value.z;
        }

        [[nodiscard]]
        bool NormalizeDirection(
            const RE::NiPoint3& value,
            RE::NiPoint3& result) noexcept
        {
            const float lengthSquared =
                VectorLengthSquared(
                    value
                );

            if (
                !std::isfinite(
                    lengthSquared
                ) ||
                lengthSquared <=
                    kVectorEpsilon
            ) {
                return false;
            }

            const float inverseLength =
                1.0F /
                std::sqrt(
                    lengthSquared
                );

            result =
                value *
                inverseLength;

            return
                IsFinitePoint(
                    result
                );
        }

        [[nodiscard]]
        RE::NiPoint3 LerpDirection(
            const RE::NiPoint3& start,
            const RE::NiPoint3& target,
            const float progress) noexcept
        {
            const RE::NiPoint3 blended =
                LerpPoint(
                    start,
                    target,
                    progress
                );

            RE::NiPoint3 normalized{};

            if (
                NormalizeDirection(
                    blended,
                    normalized
                )
            ) {
                return normalized;
            }

            if (
                NormalizeDirection(
                    target,
                    normalized
                )
            ) {
                return normalized;
            }

            if (
                NormalizeDirection(
                    start,
                    normalized
                )
            ) {
                return normalized;
            }

            return RE::NiPoint3{
                0.0F,
                1.0F,
                0.0F
            };
        }

        [[nodiscard]]
        CirclePose InterpolateCirclePose(
            const CirclePose& start,
            const CirclePose& target,
            const float progress) noexcept
        {
            CirclePose result{};

            result.translation =
                LerpPoint(
                    start.translation,
                    target.translation,
                    progress
                );

            result.rotationRadians =
                LerpEuler(
                    start.rotationRadians,
                    target.rotationRadians,
                    progress
                );

            result.scale =
                Lerp(
                    start.scale,
                    target.scale,
                    progress
                );

            if (progress >= 1.0F) {

                result.visible =
                    target.visible;
            }
            else {

                result.visible =
                    start.visible ||
                    target.visible;
            }

            return result;
        }

        [[nodiscard]]
        BeamPose InterpolateBeamPose(
            const BeamPose& start,
            const BeamPose& target,
            const float progress) noexcept
        {
            BeamPose result{};

            result.origin =
                LerpPoint(
                    start.origin,
                    target.origin,
                    progress
                );

            result.direction =
                LerpDirection(
                    start.direction,
                    target.direction,
                    progress
                );

            result.length =
                Lerp(
                    start.length,
                    target.length,
                    progress
                );

            result.widthScale =
                Lerp(
                    start.widthScale,
                    target.widthScale,
                    progress
                );

            if (progress >= 1.0F) {

                result.visible =
                    target.visible;
            }
            else {

                result.visible =
                    start.visible ||
                    target.visible;
            }

            result.asset =
                target.asset;

            return result;
        }
    }


    float EvaluateTransitionCurve(
        const TransitionCurve curve,
        const float progress) noexcept
    {
        const float clamped =
            Clamp01(
                progress
            );

        switch (curve) {
        case TransitionCurve::kLinear:
            return clamped;

        case TransitionCurve::kSmootherStep:
            return
                clamped *
                clamped *
                clamped *
                (
                    clamped *
                    (
                        clamped *
                        6.0F -
                        15.0F
                    ) +
                    10.0F
                );

        case TransitionCurve::kSmoothStep:
        default:
            return
                clamped *
                clamped *
                (
                    3.0F -
                    2.0F *
                    clamped
                );
        }
    }


    float AdvanceWrappedRadians(
        const float currentRadians,
        const float radiansPerSecond,
        const float deltaSeconds) noexcept
    {
        if (
            !std::isfinite(
                currentRadians
            ) ||
            !std::isfinite(
                radiansPerSecond
            ) ||
            !std::isfinite(
                deltaSeconds
            ) ||
            deltaSeconds <=
                0.0F
        ) {
            return
                WrapSignedRadians(
                    currentRadians
                );
        }

        return
            WrapSignedRadians(
                currentRadians +
                radiansPerSecond *
                deltaSeconds
            );
    }


    void Timeline::Begin(
        const float durationSeconds) noexcept
    {
        _elapsed =
            0.0F;

        if (
            std::isfinite(
                durationSeconds
            ) &&
            durationSeconds >
                0.0F
        ) {

            _duration =
                durationSeconds;

            _active =
                true;

            return;
        }

        _duration =
            0.0F;

        _active =
            false;
    }


    void Timeline::Reset() noexcept
    {
        _duration =
            0.0F;

        _elapsed =
            0.0F;

        _active =
            false;
    }


    float Timeline::Advance(
        const float deltaSeconds) noexcept
    {
        if (!_active) {
            return 1.0F;
        }

        if (
            std::isfinite(
                deltaSeconds
            ) &&
            deltaSeconds >
                0.0F
        ) {

            _elapsed +=
                deltaSeconds;
        }

        if (
            _elapsed >=
            _duration
        ) {

            _elapsed =
                _duration;

            _active =
                false;
        }

        return
            Progress();
    }


    float Timeline::Progress() const noexcept
    {
        if (
            _duration <=
            0.0F
        ) {
            return 1.0F;
        }

        return
            Clamp01(
                _elapsed /
                _duration
            );
    }


    bool Timeline::IsActive() const noexcept
    {
        return _active;
    }


    bool Timeline::IsComplete() const noexcept
    {
        return !_active;
    }


    bool CirclePoseTween::Begin(
        const CirclePose& current,
        const CirclePose& target,
        const float durationSeconds,
        const TransitionCurve curve) noexcept
    {
        if (
            !IsFiniteCirclePose(
                current
            ) ||
            !IsFiniteCirclePose(
                target
            ) ||
            !std::isfinite(
                durationSeconds
            )
        ) {
            return false;
        }

        _start =
            current;

        _target =
            target;

        _current =
            current;

        _curve =
            curve;

        _timeline.Begin(
            durationSeconds
        );

        if (
            !_timeline.IsActive()
        ) {

            _current =
                target;
        }

        return true;
    }


    void CirclePoseTween::Snap(
        const CirclePose& pose) noexcept
    {
        _start =
            pose;

        _target =
            pose;

        _current =
            pose;

        _timeline.Reset();
    }


    CirclePose CirclePoseTween::Advance(
        const float deltaSeconds) noexcept
    {
        if (
            !_timeline.IsActive()
        ) {
            return _current;
        }

        const float rawProgress =
            _timeline.Advance(
                deltaSeconds
            );

        const float curvedProgress =
            EvaluateTransitionCurve(
                _curve,
                rawProgress
            );

        _current =
            InterpolateCirclePose(
                _start,
                _target,
                curvedProgress
            );

        if (
            _timeline.IsComplete()
        ) {

            _current =
                _target;
        }

        return _current;
    }


    const CirclePose&
    CirclePoseTween::Current() const noexcept
    {
        return _current;
    }


    bool CirclePoseTween::IsActive() const noexcept
    {
        return
            _timeline.IsActive();
    }


    bool CirclePoseTween::IsComplete() const noexcept
    {
        return
            _timeline.IsComplete();
    }


    bool BeamPoseTween::Begin(
        const BeamPose& current,
        const BeamPose& target,
        const float durationSeconds,
        const TransitionCurve curve) noexcept
    {
        if (
            !IsFiniteBeamPose(
                current
            ) ||
            !IsFiniteBeamPose(
                target
            ) ||
            !std::isfinite(
                durationSeconds
            ) ||
            current.asset !=
                target.asset
        ) {
            return false;
        }

        _start =
            current;

        _target =
            target;

        _current =
            current;

        _curve =
            curve;

        _timeline.Begin(
            durationSeconds
        );

        if (
            !_timeline.IsActive()
        ) {

            _current =
                target;
        }

        return true;
    }


    void BeamPoseTween::Snap(
        const BeamPose& pose) noexcept
    {
        _start =
            pose;

        _target =
            pose;

        _current =
            pose;

        _timeline.Reset();
    }


    BeamPose BeamPoseTween::Advance(
        const float deltaSeconds) noexcept
    {
        if (
            !_timeline.IsActive()
        ) {
            return _current;
        }

        const float rawProgress =
            _timeline.Advance(
                deltaSeconds
            );

        const float curvedProgress =
            EvaluateTransitionCurve(
                _curve,
                rawProgress
            );

        _current =
            InterpolateBeamPose(
                _start,
                _target,
                curvedProgress
            );

        if (
            _timeline.IsComplete()
        ) {

            _current =
                _target;
        }

        return _current;
    }


    const BeamPose&
    BeamPoseTween::Current() const noexcept
    {
        return _current;
    }


    bool BeamPoseTween::IsActive() const noexcept
    {
        return
            _timeline.IsActive();
    }


    bool BeamPoseTween::IsComplete() const noexcept
    {
        return
            _timeline.IsComplete();
    }
}