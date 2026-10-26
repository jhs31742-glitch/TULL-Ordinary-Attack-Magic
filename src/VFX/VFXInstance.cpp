#include "VFX/VFXInstance.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cmath>

#include "RE/B/BSGeometry.h"
#include "RE/B/BSShaderProperty.h"
#include "RE/B/BSVisit.h"

namespace TOAM::VFX
{
    void VFXInstance::CaptureBeamBounds() noexcept
    {
        _beamMeshCount = 0;

        if (!_node ||
            (_asset != AssetID::kBeamWholeLens &&
             _asset != AssetID::kBeamWholeLensTintable &&
             _asset != AssetID::kBeamBodyUnit &&
             _asset != AssetID::kBeamBodyUnitStatic)) {
            return;
        }

        bool overflow = false;

        RE::BSVisit::TraverseScenegraphGeometries(
            _node.get(), [this, &overflow](RE::BSGeometry* mesh) {
                const float radius =
                    mesh->GetModelData().modelBound.radius;

                if (!std::isfinite(radius) || radius <= 0.0F) {
                    return RE::BSVisit::BSVisitControl::kContinue;
                }

                if (_beamMeshCount >= _beamMeshes.size()) {
                    overflow = true;
                    return RE::BSVisit::BSVisitControl::kContinue;
                }

                _beamMeshes[_beamMeshCount] = mesh;
                _beamOriginalRadii[_beamMeshCount] = radius;
                ++_beamMeshCount;

                return RE::BSVisit::BSVisitControl::kContinue;
            });

        if (overflow) {
            _beamMeshCount = 0;
            logger::warn(
                "[TOAM][VFX] E3_BOUND_SKIPPED | MeshCapacity");
        }
    }

    void VFXInstance::RepairWholeLensWorldBound() noexcept
    {
        // The whole-lens NIF is authored along local +Y, from 0 to 512u.
        // A NiNode with a stale 32u root bound may be culled before its
        // dynamically stretched child geometries are considered.
        if (!_node || _beamMeshCount == 0 ||
            (_asset != AssetID::kBeamWholeLens &&
             _asset != AssetID::kBeamWholeLensTintable)) {
            return;
        }

        const auto& world = _node->world;
        const auto& matrix = world.rotate;
        const float scale = world.scale;
        if (!std::isfinite(scale) || scale <= 0.0F) {
            return;
        }

        const float yx = matrix.entry[0][1] * scale;
        const float yy = matrix.entry[1][1] * scale;
        const float yz = matrix.entry[2][1] * scale;
        const float xSquared =
            matrix.entry[0][0] * matrix.entry[0][0] +
            matrix.entry[1][0] * matrix.entry[1][0] +
            matrix.entry[2][0] * matrix.entry[2][0];
        const float zSquared =
            matrix.entry[0][2] * matrix.entry[0][2] +
            matrix.entry[1][2] * matrix.entry[1][2] +
            matrix.entry[2][2] * matrix.entry[2][2];
        const float ySquared = yx * yx + yy * yy + yz * yz;

        if (!std::isfinite(ySquared) || ySquared <= 0.0F ||
            !std::isfinite(xSquared) || !std::isfinite(zSquared) ||
            xSquared <= 0.0F || zSquared <= 0.0F) {
            return;
        }

        // The authored outer lens radius is below 50u. A 128u local
        // transverse guard protects the four child shapes while staying
        // proportional to the current width. No gameplay range is changed.
        const float halfLength = 256.0F * std::sqrt(ySquared);
        const float widestSquared =
            xSquared > zSquared ? xSquared : zSquared;
        const float transverseGuard =
            128.0F * scale * std::sqrt(widestSquared);
        const float radius = halfLength + transverseGuard;
        if (!std::isfinite(radius) || radius <= 0.0F ||
            radius > 32768.0F) {
            return;
        }

        const float previousRadius = _node->worldBound.radius;
        _node->worldBound.center = RE::NiPoint3{
            world.translate.x + 256.0F * yx,
            world.translate.y + 256.0F * yy,
            world.translate.z + 256.0F * yz
        };
        _node->worldBound.radius = radius;

        static std::atomic<std::uint32_t> reportCount{0};
        if (reportCount.fetch_add(1, std::memory_order_relaxed) < 12) {
            logger::info(
                "[TOAM][VFX] P03B_ROOT_BOUND | Asset={} | Before={:.2f} | "
                "After={:.2f} | Length={:.2f} | WidthGuard={:.2f}",
                AssetName(_asset), previousRadius, radius,
                2.0F * halfLength, transverseGuard);
        }
    }

    void VFXInstance::RefreshBeamBounds(
        const RE::NiTransform& transform) noexcept
    {
        if (_beamMeshCount == 0) {
            return;
        }

        const auto& m = transform.rotate;
        const float squared =
            m.entry[0][1] * m.entry[0][1] +
            m.entry[1][1] * m.entry[1][1] +
            m.entry[2][1] * m.entry[2][1];

        if (!std::isfinite(squared) || squared <= 0.0F) {
            return;
        }

        const float longitudinalGain = std::sqrt(squared);
        const float gain =
            longitudinalGain > 1.0F ? longitudinalGain : 1.0F;

        for (std::size_t i = 0; i < _beamMeshCount; ++i) {
            const float radius = _beamOriginalRadii[i] * gain;
            if (std::isfinite(radius)) {
                _beamMeshes[i]->GetModelData().modelBound.radius =
                    radius;
            }
        }
    }

    VFXInstance::~VFXInstance()
    {
        Release();
    }

    bool VFXInstance::Spawn(
        const AssetID asset,
        RE::NiNode* parent,
        const RE::NiTransform& local,
        const bool visible) noexcept
    {
        Release();

        if (!parent) {

            logger::error(
                "[TOAM][VFX] Spawn rejected | Parent=null | Asset={}",
                AssetName(asset));

            return false;
        }

        const char* path =
            AssetPath(asset);

        if (!path || path[0] == '\0') {

            logger::error(
                "[TOAM][VFX] Spawn rejected | Invalid asset={}",
                static_cast<std::uint32_t>(asset));

            return false;
        }

        RE::NiPointer<RE::NiNode>
            prototype;

        RE::BSModelDB::DBTraits::ArgsType
            args{};

        const auto loadResult =
            RE::BSModelDB::Demand(
                path,
                prototype,
                args
            );

        if (
            loadResult !=
                RE::BSResource::ErrorCode::kNone ||
            !prototype
        ) {

            logger::error(
                "[TOAM][VFX] Demand failed | Asset={} | Path={} | Error={}",
                AssetName(asset),
                path,
                static_cast<std::int32_t>(loadResult));

            return false;
        }

        auto* cloneObject =
            prototype->Clone();

        if (!cloneObject) {

            logger::error(
                "[TOAM][VFX] Clone returned null | Asset={}",
                AssetName(asset));

            return false;
        }

        auto* cloneNode =
            cloneObject->AsNode();

        if (!cloneNode) {

            logger::error(
                "[TOAM][VFX] Clone is not NiNode | Asset={}",
                AssetName(asset));

            return false;
        }

        _node.reset(
            cloneNode
        );

        _asset =
            asset;

        CaptureBeamBounds();

        _node->local =
            local;
        RefreshBeamBounds(local);

        _node->SetAppCulled(
            !visible
        );

        parent->AttachChild(
            _node.get(),
            true
        );

        // Keep the dynamically attached clone's world data and bounds
        // in sync with its assigned local transform before rendering.
        RE::NiUpdateData update{};
        update.flags.set(RE::NiUpdateData::Flag::kDirty);
        _node->Update(update);
        RepairWholeLensWorldBound();

        // E3: bounded read-only snapshot for each asset, after attachment.
        // CPU-side state is diagnostic only; this cannot prove GPU pixels.
        static std::array<std::atomic<std::uint32_t>,
            static_cast<std::size_t>(AssetID::kCount)> renderSnapshotCount{};
        const auto assetIndex = static_cast<std::size_t>(asset);
        if (assetIndex < renderSnapshotCount.size() &&
            renderSnapshotCount[assetIndex].fetch_add(
                1, std::memory_order_relaxed) == 0) {
            std::size_t meshCount{};
            std::size_t hiddenMeshes{};
            std::size_t missingShader{};
            std::size_t zeroFade{};
            std::size_t invalidBounds{};
            RE::BSVisit::TraverseScenegraphGeometries(
                _node.get(), [&](RE::BSGeometry* geometry) {
                    ++meshCount;
                    hiddenMeshes += geometry->GetAppCulled() ? 1 : 0;
                    missingShader +=
                        geometry->GetGeometryRuntimeData().shaderProperty.get() ? 0 : 1;
                    zeroFade += (std::isfinite(geometry->fadeAmount) &&
                        geometry->fadeAmount <= 0.0F) ? 1 : 0;
                    invalidBounds +=
                        (!std::isfinite(geometry->worldBound.radius) ||
                         geometry->worldBound.radius <= 0.0F) ? 1 : 0;
                    return RE::BSVisit::BSVisitControl::kContinue;
                });
            logger::info(
                "[TOAM][VFX] E3_RENDER_TREE | Asset={} | ParentHidden={} | "
                "RootHidden={} | RootFade={:.3f} | RootBound={:.3f} | "
                "Meshes={} | HiddenMeshes={} | NoShader={} | "
                "ZeroFade={} | InvalidBounds={}",
                AssetName(asset), parent->GetAppCulled(),
                _node->GetAppCulled(), _node->fadeAmount,
                _node->worldBound.radius, meshCount, hiddenMeshes,
                missingShader, zeroFade, invalidBounds);
        }

        // Bounded event-driven evidence; no per-frame diagnostic loop.
        static std::atomic<std::uint32_t> logged{0};
        if (logged.fetch_add(1, std::memory_order_relaxed) < 16) {
            logger::info(
                "[TOAM][VFX] SCENE_READY | Asset={} | Attached={} | Culled={} | World=({:.1f},{:.1f},{:.1f}) | BoundRadius={:.1f}",
                AssetName(asset),
                _node->parent == parent,
                _node->GetAppCulled(),
                _node->world.translate.x,
                _node->world.translate.y,
                _node->world.translate.z,
                _node->worldBound.radius);
        }

        // E2-A R3: compare NIF prototype and clone ownership without
        // changing shader properties, materials, or alpha.
        if (asset == AssetID::kBeamWholeLens ||
            asset == AssetID::kBeamWholeLensTintable) {
            static std::atomic<std::uint32_t> ownerLogCount{0};
            if (ownerLogCount.fetch_add(1, std::memory_order_relaxed) < 12) {
                struct OwnershipSnapshot
                {
                    std::array<RE::BSShaderProperty*, 16> shaders{};
                    std::array<RE::BSShaderMaterial*, 16> materials{};
                    std::size_t count{};
                };

                const auto capture = [](RE::NiAVObject* root) {
                    OwnershipSnapshot result{};
                    RE::BSVisit::TraverseScenegraphGeometries(
                        root, [&result](RE::BSGeometry* geometry) {
                            if (result.count < result.shaders.size()) {
                                auto* shader = geometry->GetGeometryRuntimeData()
                                    .shaderProperty.get();
                                result.shaders[result.count] = shader;
                                result.materials[result.count] =
                                    shader ? shader->GetBaseMaterial() : nullptr;
                            }
                            ++result.count;
                            return RE::BSVisit::BSVisitControl::kContinue;
                        });
                    return result;
                };

                const auto original = capture(prototype.get());
                const auto cloned = capture(_node.get());
                bool missingShader{};
                bool sharedShader{};
                bool sharedMaterial{};
                for (std::size_t i = 0;
                     i < original.count && i < original.shaders.size(); ++i) {
                    missingShader |= original.shaders[i] == nullptr;
                    for (std::size_t j = 0;
                         j < cloned.count && j < cloned.shaders.size(); ++j) {
                        if (original.shaders[i] &&
                            original.shaders[i] == cloned.shaders[j]) {
                            sharedShader = true;
                        }
                        if (original.materials[i] &&
                            original.materials[i] == cloned.materials[j]) {
                            sharedMaterial = true;
                        }
                    }
                }
                for (std::size_t j = 0;
                     j < cloned.count && j < cloned.shaders.size(); ++j) {
                    missingShader |= cloned.shaders[j] == nullptr;
                }

                const bool complete =
                    original.count == cloned.count &&
                    original.count > 0 &&
                    original.count <= original.shaders.size() &&
                    cloned.count <= cloned.shaders.size() &&
                    !missingShader;
                logger::info(
                    "[TOAM][VFX] E2_SHADER_OWNER | Asset={} | PrototypeMeshes={} | CloneMeshes={} | Complete={} | SharedShader={} | SharedMaterial={} | AlphaMutation=false",
                    AssetName(asset), original.count, cloned.count,
                    complete, sharedShader, sharedMaterial);
            }
        }

        return true;
    }

    void VFXInstance::Release() noexcept
    {
        _beamMeshCount = 0;
        if (!_node) {

            _asset =
                AssetID::kCount;

            return;
        }

        if (
            auto* parent =
                _node->parent
        ) {
            parent->DetachChild(
                _node.get()
            );
        }

        _node.reset();

        _asset =
            AssetID::kCount;
    }

    bool VFXInstance::SetLocalTransform(
        const RE::NiTransform& transform) noexcept
    {
        if (!_node) {
            return false;
        }

        _node->local =
            transform;
        RefreshBeamBounds(transform);

        RE::NiUpdateData update{};
        update.flags.set(RE::NiUpdateData::Flag::kDirty);
        _node->Update(update);
        RepairWholeLensWorldBound();

        if (_beamMeshCount > 0) {
            static std::atomic<std::uint32_t> evidenceCount{0};
            if (evidenceCount.fetch_add(
                    1, std::memory_order_relaxed) < 12) {
                const auto& m = transform.rotate;
                const float yGain = std::sqrt(
                    m.entry[0][1] * m.entry[0][1] +
                    m.entry[1][1] * m.entry[1][1] +
                    m.entry[2][1] * m.entry[2][1]);

                logger::info(
                    "[TOAM][VFX] E3_BOUND_CONTRACT | Asset={} | "
                    "YGain={:.3f} | UniformScale={:.5f} | "
                    "Meshes={} | RootBound={:.2f}",
                    AssetName(_asset), yGain, transform.scale,
                    _beamMeshCount, _node->worldBound.radius);
            }
        }

        return true;
    }

    bool VFXInstance::SetVisible(
        const bool visible) noexcept
    {
        if (!_node) {
            return false;
        }

        _node->SetAppCulled(
            !visible
        );

        return true;
    }

    bool VFXInstance::IsActive() const noexcept
    {
        return
            static_cast<bool>(_node);
    }

    AssetID VFXInstance::Asset() const noexcept
    {
        return _asset;
    }

    RE::NiAVObject*
    VFXInstance::Node() const noexcept
    {
        return _node.get();
    }
}