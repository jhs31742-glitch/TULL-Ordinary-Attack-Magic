#include "VFX/PresentationMath.h"
#include "VFX/BeamRenderer.h"

#include <cmath>

namespace TOAM::VFX
{
    namespace
    {
        constexpr float
            kDirectionEpsilon{
                0.000001F
            };

        constexpr float
            kMinimumWidthScale{
                0.001F
            };

        [[nodiscard]]
        float LengthSquared(
            const RE::NiPoint3& value) noexcept
        {
            return
                value.x * value.x +
                value.y * value.y +
                value.z * value.z;
        }

        [[nodiscard]]
        bool Normalize(
            const RE::NiPoint3& value,
            RE::NiPoint3& result) noexcept
        {
            const float lengthSquared =
                LengthSquared(value);

            if (
                !std::isfinite(lengthSquared) ||
                lengthSquared <=
                    kDirectionEpsilon
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
                std::isfinite(result.x) &&
                std::isfinite(result.y) &&
                std::isfinite(result.z);
        }
    }

    BeamRenderer::~BeamRenderer()
    {
        Release();
    }

    bool BeamRenderer::IsWholeBeamAsset(const AssetID asset) noexcept
    {
        return asset == AssetID::kBeamWholeLens || asset == AssetID::kBeamWholeLensTintable;
    }

    bool BeamRenderer::IsBeamBodyAsset(
        const AssetID asset) noexcept
    {
        return
            IsWholeBeamAsset(asset) ||
            asset ==
                AssetID::kBeamBodyUnit ||
            asset ==
                AssetID::kBeamBodyUnitStatic;
    }

    bool BeamRenderer::IsPoseValid(
        const BeamPose& pose) noexcept
    {
        return
            IsBeamBodyAsset(
                pose.asset
            ) &&
            std::isfinite(
                pose.length
            ) &&
            pose.length >
                0.0F &&
            pose.length <=
                kMaxLength &&
            std::isfinite(
                pose.widthScale
            ) &&
            pose.widthScale >
                0.0F &&
            std::isfinite(
                pose.lifetimeSeconds
            ) &&
            pose.lifetimeSeconds !=
                0.0F &&
            std::isfinite(
                pose.origin.x
            ) &&
            std::isfinite(
                pose.origin.y
            ) &&
            std::isfinite(
                pose.origin.z
            ) &&
            std::isfinite(
                pose.direction.x
            ) &&
            std::isfinite(
                pose.direction.y
            ) &&
            std::isfinite(
                pose.direction.z
            );
    }

    std::size_t BeamRenderer::RequiredSegments(
        const float length) noexcept
    {
        if (
            !std::isfinite(length) ||
            length <= 0.0F
        ) {
            return 0;
        }

        const auto count =
            static_cast<std::size_t>(
                std::ceil(
                    length /
                    kUnitLength
                )
            );

        if (
            count >
            kMaxSegments
        ) {
            return 0;
        }

        return count;
    }

    bool BeamRenderer::BuildBasis(
        const RE::NiPoint3& direction,
        Basis& result) noexcept
    {
        RE::NiPoint3 forward{};

        if (
            !Normalize(
                direction,
                forward
            )
        ) {
            return false;
        }

        // Authored beam forward axis is local +Y.
        result.y =
            forward;

        // Prefer world +Z as the reference up axis.
        // For near-vertical beams use +X to avoid a degenerate cross.
        const RE::NiPoint3 reference =
            std::fabs(
                result.y.z
            ) < 0.999F ?
                RE::NiPoint3{
                    0.0F,
                    0.0F,
                    1.0F
                } :
                RE::NiPoint3{
                    1.0F,
                    0.0F,
                    0.0F
                };

        RE::NiPoint3 right{};

        if (
            !Normalize(
                result.y.Cross(
                    reference
                ),
                right
            )
        ) {
            return false;
        }

        result.x =
            right;

        RE::NiPoint3 up{};

        if (
            !Normalize(
                result.x.Cross(
                    result.y
                ),
                up
            )
        ) {
            return false;
        }

        result.z =
            up;

        return true;
    }

    RE::NiTransform
    BeamRenderer::MakeSegmentTransform(
        const RE::NiPoint3& segmentOrigin,
        const Basis& basis,
        const float longitudinalScale,
        const float widthScale) noexcept
    {
        RE::NiTransform transform{};

        const float safeWidth =
            widthScale >
                kMinimumWidthScale ?
                widthScale :
                kMinimumWidthScale;

        // NiTransform exposes one uniform scalar.
        //
        // Width uses transform.scale.
        //
        // To keep authored +Y length independent from width,
        // the Y basis column carries:
        //
        //     longitudinalScale / widthScale
        //
        // Effective result after uniform scale:
        //
        //     X = widthScale
        //     Y = longitudinalScale
        //     Z = widthScale
        //
        // This deliberately uses NiMatrix3 as the linear basis
        // instead of assuming it remains orthonormal.
        const float compensatedLongitudinal =
            longitudinalScale /
            safeWidth;

        transform.translate =
            segmentOrigin;

        transform.scale =
            safeWidth;

        transform.rotate.entry[0][0] =
            basis.x.x;

        transform.rotate.entry[1][0] =
            basis.x.y;

        transform.rotate.entry[2][0] =
            basis.x.z;


        transform.rotate.entry[0][1] =
            basis.y.x *
            compensatedLongitudinal;

        transform.rotate.entry[1][1] =
            basis.y.y *
            compensatedLongitudinal;

        transform.rotate.entry[2][1] =
            basis.y.z *
            compensatedLongitudinal;


        transform.rotate.entry[0][2] =
            basis.z.x;

        transform.rotate.entry[1][2] =
            basis.z.y;

        transform.rotate.entry[2][2] =
            basis.z.z;

        return transform;
    }

    bool BeamRenderer::SetBeam(
        RE::NiNode* parent,
        const BeamPose& pose) noexcept
    {
        if (
            !parent ||
            !IsPoseValid(pose)
        ) {

            logger::error(
                "[TOAM][BeamRenderer] SetBeam rejected | Parent={} | Length={} | Width={} | Asset={}",
                parent != nullptr,
                pose.length,
                pose.widthScale,
                AssetName(
                    pose.asset
                ));

            return false;
        }

        Basis basis{};

        if (
            !BuildBasis(
                pose.direction,
                basis
            )
        ) {

            logger::error(
                "[TOAM][BeamRenderer] Invalid direction");

            return false;
        }

        const std::size_t requiredSegments =
            Presentation::InstanceCount(pose.length,IsWholeBeamAsset(pose.asset));

        if (
            requiredSegments == 0
        ) {

            logger::error(
                "[TOAM][BeamRenderer] Segment count invalid | Length={}",
                pose.length);

            return false;
        }

        Release();

        _parent =
            parent;

        _asset =
            pose.asset;

        auto& manager =
            TransientVFXManager::Get();

        for (
            std::size_t index = 0;
            index < requiredSegments;
            ++index
        ) {

            const float segmentStart =
                static_cast<float>(
                    index
                ) *
                kUnitLength;

            float segmentLength =
                pose.length -
                segmentStart;

            if (
                !IsWholeBeamAsset(pose.asset) &&
                segmentLength > kUnitLength
            ) {
                segmentLength =
                    kUnitLength;
            }

            if (
                segmentLength <= 0.0F
            ) {

                Release();

                return false;
            }

            const float longitudinalScale =
                segmentLength /
                kUnitLength;

            const RE::NiPoint3 segmentOrigin =
                pose.origin +
                basis.y *
                segmentStart;

            SpawnRequest request{};

            request.asset =
                pose.asset;

            request.parent =
                parent;

            request.local =
                MakeSegmentTransform(
                    segmentOrigin,
                    basis,
                    longitudinalScale,
                    pose.widthScale
                );

            request.lifetimeSeconds =
                pose.lifetimeSeconds;

            request.visible =
                pose.visible;

            const auto handle =
                manager.Spawn(
                    request
                );

            if (!handle.IsValid()) {

                logger::error(
                    "[TOAM][BeamRenderer] Segment spawn failed | Index={} | Count={}",
                    index,
                    requiredSegments);

                Release();

                return false;
            }

            _segments[index] =
                handle;

            _segmentCount =
                index + 1;
        }

        logger::debug(
            "[TOAM][BeamRenderer] Spawned | Length={} | Width={} | Segments={}",
            pose.length,
            pose.widthScale,
            _segmentCount);

        return true;
    }

    bool BeamRenderer::ApplyExisting(
        const BeamPose& pose,
        const Basis& basis,
        const std::size_t requiredSegments) noexcept
    {
        if (
            requiredSegments !=
                _segmentCount ||
            pose.asset !=
                _asset
        ) {
            return false;
        }

        auto& manager =
            TransientVFXManager::Get();

        for (
            std::size_t index = 0;
            index < requiredSegments;
            ++index
        ) {

            if (
                !_segments[index].IsValid()
            ) {
                return false;
            }

            const float segmentStart =
                static_cast<float>(
                    index
                ) *
                kUnitLength;

            float segmentLength =
                pose.length -
                segmentStart;

            if (
                !IsWholeBeamAsset(pose.asset) &&
                segmentLength > kUnitLength
            ) {
                segmentLength =
                    kUnitLength;
            }

            if (
                segmentLength <= 0.0F
            ) {
                return false;
            }

            const float longitudinalScale =
                segmentLength /
                kUnitLength;

            const RE::NiPoint3 segmentOrigin =
                pose.origin +
                basis.y *
                segmentStart;

            const auto transform =
                MakeSegmentTransform(
                    segmentOrigin,
                    basis,
                    longitudinalScale,
                    pose.widthScale
                );

            if (
                !manager.SetTransform(
                    _segments[index],
                    transform
                )
            ) {
                return false;
            }

            if (
                !manager.SetVisible(
                    _segments[index],
                    pose.visible
                )
            ) {
                return false;
            }
        }

        return true;
    }

    bool BeamRenderer::UpdateBeam(
        const BeamPose& pose) noexcept
    {
        if (
            !_parent ||
            !IsPoseValid(pose)
        ) {
            return false;
        }

        Basis basis{};

        if (
            !BuildBasis(
                pose.direction,
                basis
            )
        ) {
            return false;
        }

        const std::size_t requiredSegments =
            Presentation::InstanceCount(pose.length,IsWholeBeamAsset(pose.asset));

        if (
            requiredSegments == 0
        ) {
            return false;
        }

        if (
            requiredSegments !=
                _segmentCount ||
            pose.asset !=
                _asset
        ) {
            return
                SetBeam(
                    _parent,
                    pose
                );
        }

        if (
            !ApplyExisting(
                pose,
                basis,
                requiredSegments
            )
        ) {

            logger::warn(
                "[TOAM][BeamRenderer] Existing segment chain invalid; rebuilding");

            return
                SetBeam(
                    _parent,
                    pose
                );
        }

        return true;
    }

    void BeamRenderer::Detach() noexcept
    {
        _segments = {};
        _segmentCount = 0;
        _asset = AssetID::kCount;
        _parent = nullptr;
    }

    void BeamRenderer::Release() noexcept
    {
        auto& manager =
            TransientVFXManager::Get();

        for (
            auto& handle :
            _segments
        ) {

            if (!handle.IsValid()) {
                continue;
            }

            static_cast<void>(
                manager.Release(
                    handle
                )
            );

            handle =
                {};
        }

        _parent =
            nullptr;

        _asset =
            AssetID::kCount;

        _segmentCount =
            0;
    }

    bool BeamRenderer::IsActive() const noexcept
    {
        return
            _segmentCount >
                0;
    }

    std::size_t
    BeamRenderer::SegmentCount() const noexcept
    {
        return
            _segmentCount;
    }
}