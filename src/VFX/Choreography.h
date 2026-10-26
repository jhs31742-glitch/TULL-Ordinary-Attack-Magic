#pragma once

#include "VFX/BeamRenderer.h"
#include "VFX/CircleRenderer.h"

#include <cstdint>

namespace TOAM::VFX
{
    enum class TransitionCurve : std::uint8_t
    {
        kLinear = 0,
        kSmoothStep,
        kSmootherStep
    };

    [[nodiscard]]
    float EvaluateTransitionCurve(
        TransitionCurve curve,
        float progress) noexcept;

    [[nodiscard]]
    float AdvanceWrappedRadians(
        float currentRadians,
        float radiansPerSecond,
        float deltaSeconds) noexcept;

    class Timeline final
    {
    public:
        void Begin(
            float durationSeconds) noexcept;

        void Reset() noexcept;

        [[nodiscard]]
        float Advance(
            float deltaSeconds) noexcept;

        [[nodiscard]]
        float Progress() const noexcept;

        [[nodiscard]]
        bool IsActive() const noexcept;

        [[nodiscard]]
        bool IsComplete() const noexcept;

    private:
        float _duration{};
        float _elapsed{};
        bool _active{};
    };


    class CirclePoseTween final
    {
    public:
        [[nodiscard]]
        bool Begin(
            const CirclePose& current,
            const CirclePose& target,
            float durationSeconds,
            TransitionCurve curve =
                TransitionCurve::kSmoothStep) noexcept;

        void Snap(
            const CirclePose& pose) noexcept;

        [[nodiscard]]
        CirclePose Advance(
            float deltaSeconds) noexcept;

        [[nodiscard]]
        const CirclePose& Current() const noexcept;

        [[nodiscard]]
        bool IsActive() const noexcept;

        [[nodiscard]]
        bool IsComplete() const noexcept;

    private:
        CirclePose _start{};
        CirclePose _target{};
        CirclePose _current{};

        Timeline _timeline{};

        TransitionCurve _curve{
            TransitionCurve::kSmoothStep
        };
    };


    class BeamPoseTween final
    {
    public:
        [[nodiscard]]
        bool Begin(
            const BeamPose& current,
            const BeamPose& target,
            float durationSeconds,
            TransitionCurve curve =
                TransitionCurve::kSmoothStep) noexcept;

        void Snap(
            const BeamPose& pose) noexcept;

        [[nodiscard]]
        BeamPose Advance(
            float deltaSeconds) noexcept;

        [[nodiscard]]
        const BeamPose& Current() const noexcept;

        [[nodiscard]]
        bool IsActive() const noexcept;

        [[nodiscard]]
        bool IsComplete() const noexcept;

    private:
        BeamPose _start{};
        BeamPose _target{};
        BeamPose _current{};

        Timeline _timeline{};

        TransitionCurve _curve{
            TransitionCurve::kSmoothStep
        };
    };
}