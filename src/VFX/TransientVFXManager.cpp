#include "VFX/PresentationMath.h"
#include "VFX/TransientVFXManager.h"

#include "RE/B/BSGeometry.h"
#include "RE/B/BSVisit.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace TOAM::VFX
{
    namespace
    {
        constexpr std::uint16_t
            kGenerationCeiling{
                0xFFFFu
            };

        // Approved R11 real-time presentation, not the slowed preview video.
        constexpr float kBeamEnvelopeSeconds{0.48F};
        constexpr float kBeamStepSeconds{0.06F};
        constexpr float kBeamWidthValues[9]{
            0.10F, 1.00F, 0.98F, 0.80F, 0.60F,
            0.36F, 0.18F, 0.05F, 0.00F
        };

        [[nodiscard]]
        float BeamWidthFactor(const float elapsed) noexcept
        {
            if (elapsed <= 0.0F) { return kBeamWidthValues[0]; }
            if (elapsed >= kBeamEnvelopeSeconds) { return 0.0F; }

            const float position = elapsed / kBeamStepSeconds;
            const auto index = static_cast<std::size_t>(position);
            if (index >= 8) { return 0.0F; }
            const float fraction = position - static_cast<float>(index);
            return kBeamWidthValues[index] +
                (kBeamWidthValues[index + 1] - kBeamWidthValues[index]) * fraction;
        }

        [[nodiscard]]
        float BeamAlphaFactor(const float elapsed) noexcept
        {
            if (elapsed <= 0.12F) { return 1.0F; }
            if (elapsed >= kBeamEnvelopeSeconds) { return 0.0F; }
            return (kBeamEnvelopeSeconds - elapsed) / 0.36F;
        }

        void ApplyArtBeamBounds(RE::NiAVObject* object,
            const RE::NiPoint3& center, float radius, float opacity) noexcept
        {
            if (!object) { return; }
            object->fadeAmount = opacity;
            object->worldBound.center = center;
            object->worldBound.radius = radius;
            if (auto* node = object->AsNode()) {
                for (auto& child : node->GetChildren()) {
                    if (child) {
                        ApplyArtBeamBounds(child.get(), center, radius, opacity);
                    }
                }
            }
        }
        void ApplyInstanceFade(
            RE::NiAVObject* root,
            const std::array<RE::NiAVObject*, 32>& geometry,
            const std::size_t count,
            const float opacity) noexcept
        {
            if (!root) { return; }
            root->fadeAmount = opacity;
            for (std::size_t i = 0; i < count && i < geometry.size(); ++i) {
                if (geometry[i]) { geometry[i]->fadeAmount = opacity; }
            }
        }

        [[nodiscard]]
        RE::NiTransform BeamWidthTransform(
            const RE::NiTransform& authored,
            const float widthFactor) noexcept
        {
            // The beam's Y basis column already compensates the single
            // NiTransform scale. Applying only scale would also shorten Y.
            // Divide Y column by factor to retain identical world endpoints.
            const float safeFactor = widthFactor > 0.001F ? widthFactor : 0.001F;
            auto result = authored;
            result.scale *= safeFactor;
            for (std::size_t row = 0; row < 3; ++row) {
                result.rotate.entry[row][1] /= safeFactor;
            }
            return result;
        }
    }

    bool TransientVFXManager::ApplyActiTransform(
        Slot& slot, const RE::NiTransform& local) noexcept
    {
        if (!slot.actiRef || !slot.actiParent) { return false; }
        const RE::NiTransform world = slot.actiParent->world * local;
        const auto finite = [](float value) { return std::isfinite(value); };
        if (!finite(world.translate.x) || !finite(world.translate.y) ||
            !finite(world.translate.z) || !finite(world.scale) ||
            world.scale <= 0.0F) { return false; }

        const bool beamActi = slot.asset == AssetID::kBeamWholeLens ||
            slot.asset == AssetID::kBeamWholeLensTintable;
        const bool circleActi = slot.asset == AssetID::kCircleStatic ||
            slot.asset == AssetID::kCircleOuterBandStatic;
        auto actorRotation = world.rotate;
        float beamLongitudinal = 1.0F;
        if (beamActi) {
            // Keep ACTI rotation orthonormal. The NIF BEAM_Root child owns
            // longitudinal deformation and the ACTI scalar owns width.
            const float y2 = actorRotation.entry[0][1] * actorRotation.entry[0][1] +
                actorRotation.entry[1][1] * actorRotation.entry[1][1] +
                actorRotation.entry[2][1] * actorRotation.entry[2][1];
            if (!finite(y2) || y2 <= 0.000001F) { return false; }
            beamLongitudinal = std::sqrt(y2);
            if (!finite(beamLongitudinal) || beamLongitudinal > 100000.0F) { return false; }
            for (std::size_t row = 0; row < 3; ++row) {
                actorRotation.entry[row][1] /= beamLongitudinal;
            }
            // Reject a sheared or degenerate basis instead of feeding a
            // non-rotation matrix to ToEulerAnglesXYZ/SetAngle.
            for (std::size_t col = 0; col < 3; ++col) {
                float length2 = 0.0F;
                for (std::size_t row = 0; row < 3; ++row) {
                    length2 += actorRotation.entry[row][col] * actorRotation.entry[row][col];
                }
                if (!finite(length2) || std::fabs(length2 - 1.0F) > 0.03F) {
                    return false;
                }
            }
            for (std::size_t a = 0; a < 3; ++a) {
                for (std::size_t b = a + 1; b < 3; ++b) {
                    float dot = 0.0F;
                    for (std::size_t row = 0; row < 3; ++row) {
                        dot += actorRotation.entry[row][a] * actorRotation.entry[row][b];
                    }
                    if (!finite(dot) || std::fabs(dot) > 0.03F) { return false; }
                }
            }
        }

        RE::NiPoint3 euler{};
        const bool uniqueEuler = circleActi ?
            true : actorRotation.ToEulerAnglesXYZ(euler);
        if (!finite(euler.x) || !finite(euler.y) || !finite(euler.z)) {
            static std::uint32_t rotationFailureLogCount{};
            if (rotationFailureLogCount++ < 8) {
                logger::warn("[TOAM][VFX] F2_ACTI_EULER_REJECT | Asset={}",
                    AssetName(slot.asset));
            }
            return false;
        }
        if (!uniqueEuler) {
            static std::uint32_t singularLogCount{};
            if (singularLogCount++ < 8) {
                logger::warn("[TOAM][VFX] F2_ACTI_EULER_NONUNIQUE | Asset={}",
                    AssetName(slot.asset));
            }
        }
        const bool moved = !slot.actiApplied || world != slot.actiLastWorld;
        if (moved) {
            slot.actiRef->SetAngle(euler);
            // Keep the engine-owned ACTI reference at stable unit scale.
            // Beam width and axial length are applied to named NIF children.
            slot.actiRef->SetScale(
                (beamActi || circleActi) ? 1.0F : world.scale);
            slot.actiRef->SetPosition(world.translate);
            slot.actiRef->Update3DPosition(true);
            slot.actiLastWorld = world;
            slot.actiApplied = true;
        }
        auto* root = slot.actiRef->Get3D();
        if (!root && slot.actiLastRoot) {
            // A lost/rebuilt 3D root invalidates all raw child pointers.
            slot.actiLastRoot = nullptr;
            slot.beamActiParts.fill(nullptr);
            slot.fadeGeometryCount = 0;
            if (beamActi) { slot.beamActiReadyWait = 0.0F; }
        }
        const bool newRoot = root && root != slot.actiLastRoot;
        const bool retryParts = beamActi && root &&
            (!slot.beamActiParts[0] || !slot.beamActiParts[1] ||
             !slot.beamActiParts[2]);
        if (newRoot || retryParts) {
            slot.fadeGeometryCount = 0;
            slot.actiLastRoot = root;
            slot.beamActiParts.fill(nullptr);
            // Whole-Lens NIF block 0 is BEAM_Root itself. It MUST NOT be
            // deformed; use only its verified direct child NiNodes.
            if (beamActi) {
                constexpr std::array<const char*, 3> kPartNames{{
                    "BEAM_Core", "BEAM_Glow", "BEAM_Halo"
                }};
                auto* modelRoot = root->GetObjectByName(
                    RE::BSFixedString("BEAM_Root"));
                bool foundAll = modelRoot != nullptr;
                for (std::size_t part = 0;
                     foundAll && part < kPartNames.size(); ++part) {
                    auto* found = root->GetObjectByName(
                        RE::BSFixedString(kPartNames[part]));
                    if (!found || found == modelRoot ||
                        found->parent != modelRoot) {
                        foundAll = false;
                        break;
                    }
                    slot.beamActiParts[part] = found;
                    slot.beamActiPartOriginals[part] = found->local;
                }
                if (!foundAll) {
                    slot.beamActiParts.fill(nullptr);
                    static std::atomic<std::uint32_t> missingCount{0};
                    if (missingCount.fetch_add(1, std::memory_order_relaxed) < 16) {
                        logger::warn("[TOAM][VFX] BEAM_ACTI_PARTS_MISSING | Asset={}",
                            AssetName(slot.asset));
                    }
                }
            }
            bool overflow{};
            RE::BSVisit::TraverseScenegraphGeometries(
                root, [&slot, &overflow](RE::BSGeometry* geometry) {
                    if (slot.fadeGeometryCount >= slot.fadeGeometry.size()) {
                        overflow = true;
                        return RE::BSVisit::BSVisitControl::kStop;
                    }
                    slot.fadeGeometry[slot.fadeGeometryCount++] = geometry;
                    return RE::BSVisit::BSVisitControl::kContinue;
                });
            if (overflow) { slot.fadeGeometryCount = 0; }
        }
        // Existing active-effect tick: repair a circle pose only if it drifts.
        // No new frame hook, full actor scan or change to World formation.
        if (circleActi && root && !moved && !newRoot && slot.actiApplied) {
            auto* observed = root->GetObjectByName(
                RE::BSFixedString("CC_Root"));
            if (observed && observed->parent) {
                const auto gap = observed->world.translate - world.translate;
                const float error2 = gap.x * gap.x + gap.y * gap.y + gap.z * gap.z;
                if (finite(error2) && error2 > 4.0F) {
                    slot.actiApplied = false;
                    static std::atomic<std::uint32_t> driftCount{0};
                    if (driftCount.fetch_add(1, std::memory_order_relaxed) < 16) {
                        logger::warn("[TOAM][CIRCLE_POSE] DRIFT_REPAIR | Asset={} | ErrorSq={:.3f}",
                            AssetName(slot.asset), error2);
                    }
                }
            }
        }
        if (circleActi && root && (moved || newRoot || !slot.actiApplied)) {
            auto* pose = root->GetObjectByName(
                RE::BSFixedString("CC_Root"));

            if (pose && pose->parent) {
                const auto& parent = pose->parent->world;
                bool validParent =
                    finite(parent.scale) &&
                    std::fabs(parent.scale) >= 0.001F;

                for (std::size_t row = 0; row < 3; ++row) {
                    for (std::size_t col = 0; col < 3; ++col) {
                        if (!finite(world.rotate.entry[row][col])) {
                            validParent = false;
                        }
                    }
                }

                if (validParent) {
                    RE::NiUpdateData update{};
                    update.flags.set(RE::NiUpdateData::Flag::kDirty);
                    root->Update(update);

                    const auto inverse = parent.rotate.Transpose();
                    pose->local.translate =
                        inverse * (world.translate - parent.translate) /
                        parent.scale;
                    pose->local.rotate = inverse * world.rotate;
                    pose->local.scale = world.scale / parent.scale;

                    root->Update(update);
                    slot.actiApplied = true;

                    const auto& childBound = pose->worldBound;
                    auto& rootBound = root->worldBound;
                    const auto finiteBound = [](const RE::NiPoint3& p) {
                        return std::isfinite(p.x) &&
                            std::isfinite(p.y) &&
                            std::isfinite(p.z);
                    };
                    if (finiteBound(childBound.center) &&
                        finiteBound(rootBound.center) &&
                        std::isfinite(childBound.radius) &&
                        childBound.radius > 0.0F &&
                        std::isfinite(rootBound.radius) &&
                        rootBound.radius >= 0.0F) {
                        const auto offset =
                            childBound.center - rootBound.center;
                        const float distanceSquared =
                            offset.x * offset.x +
                            offset.y * offset.y +
                            offset.z * offset.z;
                        if (std::isfinite(distanceSquared) &&
                            distanceSquared >= 0.0F) {
                            const float requiredRadius =
                                std::sqrt(distanceSquared) +
                                childBound.radius;
                            if (std::isfinite(requiredRadius) &&
                                requiredRadius > rootBound.radius &&
                                requiredRadius <= 32768.0F) {
                                rootBound.radius = requiredRadius;
                                static std::atomic<std::uint32_t>
                                    repairs{0};
                                if (repairs.fetch_add(
                                        1, std::memory_order_relaxed) < 12) {
                                    logger::info(
                                        "[TOAM][CIRCLE_POSE] BOUND_EXPAND | Asset={} | Radius={:.2f}",
                                        AssetName(slot.asset),
                                        requiredRadius);
                                }
                            }
                        }
                    }

                    static std::atomic<std::uint32_t> readyCount{0};
                    if (readyCount.fetch_add(
                            1, std::memory_order_relaxed) < 8) {
                        logger::info(
                            "[TOAM][CIRCLE_POSE] READY | Asset={}",
                            AssetName(slot.asset));
                    }
                } else {
                    slot.actiApplied = false;
                }
            } else {
                // The reference can exist before its NIF is ready.
                // Retry when the normal active-effect update runs.
                slot.actiApplied = false;
                static std::atomic<std::uint32_t> waitCount{0};
                if (waitCount.fetch_add(
                        1, std::memory_order_relaxed) < 8) {
                    logger::warn(
                        "[TOAM][CIRCLE_POSE] WAIT | Asset={}",
                        AssetName(slot.asset));
                }
            }
        }
        const bool beamPartsReady = slot.beamActiParts[0] &&
            slot.beamActiParts[1] && slot.beamActiParts[2];
        if (beamActi && root && beamPartsReady &&
            (moved || newRoot || retryParts)) {
            // Deform the three actual child nodes, never the engine-owned root.
            // ACTI remains scale=1, avoiding 0.006-scale reference quantization.
            const float transverse = world.scale;
            const float longitudinal = beamLongitudinal * world.scale;
            if (!finite(longitudinal) || longitudinal <= 0.0F) { return false; }
            for (std::size_t part = 0; part < slot.beamActiParts.size(); ++part) {
                auto deform = slot.beamActiPartOriginals[part];
                for (std::size_t row = 0; row < 3; ++row) {
                    deform.rotate.entry[row][0] *= transverse;
                    deform.rotate.entry[row][1] *= longitudinal;
                    deform.rotate.entry[row][2] *= transverse;
                }
                slot.beamActiParts[part]->local = deform;
            }
            RE::NiUpdateData update{};
            update.flags.set(RE::NiUpdateData::Flag::kDirty);
            root->Update(update);
            // Mirror the existing whole-lens 512u +Y root bound guard.
            const float halfLength = 256.0F * longitudinal;
            const float transverseGuard = 128.0F * world.scale;
            const float boundRadius = halfLength + transverseGuard;
            if (finite(boundRadius) && boundRadius > 0.0F &&
                boundRadius <= 32768.0F) {
                root->worldBound.center = RE::NiPoint3{
                    world.translate.x + 256.0F * world.scale * world.rotate.entry[0][1],
                    world.translate.y + 256.0F * world.scale * world.rotate.entry[1][1],
                    world.translate.z + 256.0F * world.scale * world.rotate.entry[2][1]
                };
                root->worldBound.radius = boundRadius;
            }
        }
        if (root) {
            // One visual owner, including ARTO readiness transitions.
            root->SetAppCulled(
                !slot.actiVisible || slot.beamArtReady ||
                (circleActi && !slot.actiApplied));
        }
        if (root && (moved || newRoot || retryParts ||
                     slot.circleFadeDuration > 0.0F)) {
            const float remaining = slot.circleFadeDuration > 0.0F
                ? 1.0F - slot.circleFadeElapsed / slot.circleFadeDuration
                : (slot.circleFadeElapsed > 0.0F ? 0.0F : 1.0F);
            const float circleFade = remaining > 0.0F ? remaining : 0.0F;
            const float fade = beamActi && slot.beamDuration > 0.0F
                ? BeamAlphaFactor(slot.beamElapsed) : circleFade;
            ApplyInstanceFade(root, slot.fadeGeometry,
                slot.fadeGeometryCount, fade);
        }
        // A valid transform may precede 3D child readiness.
        // Update owns the bounded readiness check and retirement.
        return true;
    }

    void TransientVFXManager::StartBeamArt(
        Slot& slot, const std::size_t slotIndex) noexcept
    {
        const bool supportedArt = slot.asset == AssetID::kBeamWholeLens ||
            slot.asset == AssetID::kBeamWholeLensTintable ||
            slot.asset == AssetID::kFlashCloud ||
            slot.asset == AssetID::kFlashCloudTintable;
        if (!supportedArt || slotIndex >= kCapacity ||
            REL::Module::get().version() != SKSE::RUNTIME_SSE_1_5_97) {
            return;
        }
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) { return; }

        std::array<char, 40> editorID{};
        const char* artPrefix = slot.asset == AssetID::kBeamWholeLens ?
            "TOAM_ART_Beam_" :
            slot.asset == AssetID::kBeamWholeLensTintable ?
            "TOAM_ART_BeamTint_" :
            slot.asset == AssetID::kFlashCloud ?
            "TOAM_ART_Flash_" : "TOAM_ART_FlashTint_";
        const int written = std::snprintf(editorID.data(), editorID.size(),
            "%s%03zu", artPrefix, slotIndex);
        if (written <= 0 ||
            static_cast<std::size_t>(written) >= editorID.size()) {
            return;
        }
        auto* art = RE::TESForm::LookupByEditorID<RE::BGSArtObject>(
            editorID.data());
        if (!art) {
            static std::atomic<std::uint32_t> missing{0};
            if (missing.fetch_add(1, std::memory_order_relaxed) < 8) {
                logger::warn("[TOAM][ARTO] FORM_MISSING | Slot={}", slotIndex);
            }
            return;  // ACTI backend remains authoritative and untouched.
        }

        // A unique ARTO form per transient slot prevents ambiguous reacquisition.
        using Apply = bool(*)(RE::TESObjectREFR*, RE::BGSArtObject*, float,
            RE::TESObjectREFR*, bool, bool, RE::NiAVObject*, bool);
        static REL::Relocation<Apply> apply{REL::RelocationID(22289,22769)};
        auto* root = player->Get3D(false);
        if (!root || !apply(player, art, 1.5F, nullptr, false, false,
                            root, false)) {
            return;
        }
        slot.beamArtBase = art;
        slot.beamArtRequested = true;
        slot.beamArtReady = false;
        slot.beamArtAcquireElapsed = 0.0F;
        if(slot.engineOnly)logger::info("[TOAM][CP021GUIDE] REQUEST_ACCEPTED | Form={:08X} | Gen={} | ACTI=false | Pixels=UNVERIFIED",art->GetFormID(),slot.generation);
    }

    void TransientVFXManager::UpdateBeamArt(
        Slot& slot, const float deltaSeconds) noexcept
    {
        if (!slot.beamArtRequested || !slot.beamArtBase ||
            !slot.actiParent) {
            return;
        }
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) { return; }

        if ((!slot.beamArtEffect || slot.beamArtEffect->finished) &&
            slot.beamArtAcquireElapsed >= 1.25F) {
            if(slot.engineOnly&&slot.artStage<4){
                logger::warn("[TOAM][CP021GUIDE] EFFECT_ACQUIRE_TIMEOUT | Form={:08X} | Gen={}",slot.beamArtBase->GetFormID(),slot.generation);slot.artStage=4;
            }
            return;
        }
        if (!slot.beamArtEffect || slot.beamArtEffect->finished) {
            slot.beamArtEffect.reset();
            if (auto* lists = RE::ProcessLists::GetSingleton()) {
                lists->ForEachModelEffect([&](RE::ModelReferenceEffect* effect) {
                    if (effect->finished ||
                        effect->artObject != slot.beamArtBase ||
                        effect->target.get().get() != player) {
                        return RE::BSContainer::ForEachResult::kContinue;
                    }
                    slot.beamArtEffect.reset(effect);
                    if(slot.engineOnly&&slot.artStage<1){slot.artStage=1;logger::info("[TOAM][CP021GUIDE] EFFECT_ACQUIRED | Form={:08X} | Gen={}",slot.beamArtBase->GetFormID(),slot.generation);}
                    return RE::BSContainer::ForEachResult::kStop;
                });
            }
        }

        auto* effect = slot.beamArtEffect.get();
        if(effect && !effect->finished) {effect->lifetime=effect->age+1.5F;}
        auto* root = effect && !effect->finished ? effect->Get3D() : nullptr;
        if(slot.engineOnly&&root&&slot.artStage<2){slot.artStage=2;logger::info("[TOAM][CP021GUIDE] MODEL_3D_READY | Form={:08X} | Gen={} | Pixels=UNVERIFIED",slot.beamArtBase->GetFormID(),slot.generation);}
        const bool isFlashArt = slot.asset == AssetID::kFlashCloud ||
            slot.asset == AssetID::kFlashCloudTintable;
        if (isFlashArt) {
            // Leave the animated EXP_Flash child untouched.
            static const RE::BSFixedString kFlashPose{"OOM_ExplosionFlashCloud_Root"};
            auto* pose = root ? root->GetObjectByName(kFlashPose) : nullptr;
            if (!pose || !pose->parent) {
                if (std::isfinite(deltaSeconds) && deltaSeconds > 0.0F) {
                    slot.beamArtAcquireElapsed += deltaSeconds;
                }
                return; // ACTI remains the fallback.
            }
            const auto world = slot.actiParent->world * slot.actiLocal;
            const auto& parent = pose->parent->world;
            if (!std::isfinite(world.scale) || world.scale <= 0.0F ||
                !std::isfinite(parent.scale) || parent.scale <= 0.001F ||
                !std::isfinite(world.translate.x) ||
                !std::isfinite(world.translate.y) ||
                !std::isfinite(world.translate.z)) {
                return;
            }
            const auto inverse = parent.rotate.Transpose();
            pose->local.translate = inverse *
                (world.translate - parent.translate) / parent.scale;
            pose->local.rotate = inverse * world.rotate;
            pose->local.scale = world.scale / parent.scale;
            RE::NiUpdateData update{};
            update.time = slot.growthElapsed;
            update.flags.set(RE::NiUpdateData::Flag::kDirty);
            root->Update(update);
            const float proposedBound = 384.0F * world.scale + 32.0F;
            const float bound = proposedBound < 2048.0F ?
                proposedBound : 2048.0F;
            if (!std::isfinite(bound) || bound <= 0.0F) { return; }
            root->worldBound.center = world.translate;
            root->worldBound.radius = bound;
            root->fadeAmount = slot.actiVisible ? 1.0F : 0.0F;
            root->SetAppCulled(!slot.actiVisible);
            if (slot.actiRef && slot.actiRef->Get3D()) {
                slot.actiRef->Get3D()->SetAppCulled(true);
            }
            if (!slot.beamArtReady) {
                slot.beamArtReady = true;
                static std::atomic<std::uint32_t> readyLogs{0};
                if (readyLogs.fetch_add(1, std::memory_order_relaxed) < 16) {
                    logger::info("[TOAM][ARTO] FLASH_READY | Asset={}",
                        AssetName(slot.asset));
                }
            }
            return;
        }
        static const RE::BSFixedString kAxis{"AS_BeamAxis"};
        auto* axis = root ? root->GetObjectByName(kAxis) : nullptr;
        if (!axis || !axis->parent) {
            // ARTO 3D unavailable: fallback regains display authority.
            slot.beamArtReady = false;
            if (std::isfinite(deltaSeconds) && deltaSeconds > 0.0F) {
                slot.beamArtAcquireElapsed += deltaSeconds;
            }
            if(slot.engineOnly&&slot.beamArtAcquireElapsed>=1.25F&&slot.artStage<4){
                logger::warn("[TOAM][CP021GUIDE] {} | Form={:08X} | Gen={}",root?"POSE_NODE_OR_PARENT_MISSING":"MODEL_3D_TIMEOUT",slot.beamArtBase->GetFormID(),slot.generation);slot.artStage=4;
            }
            return;
        }

        // The TOAM beam transform embeds longitudinal scale in the +Y
        // basis column. Move it to the private ARTO NIF axis, as in Arcana.
        const auto world = slot.actiParent->world * slot.actiLocal;
        const auto finite = [](const float v) { return std::isfinite(v); };
        if (!finite(world.scale) || world.scale <= 0.0F ||
            !finite(world.translate.x) || !finite(world.translate.y) ||
            !finite(world.translate.z)) {
            return;
        }
        float ySquared = 0.0F;
        for (std::size_t row = 0; row < 3; ++row) {
            const auto y = world.rotate.entry[row][1];
            if (!finite(y)) { return; }
            ySquared += y * y;
        }
        if (!finite(ySquared) || ySquared < 0.000001F) { return; }

        const float longitudinal = std::sqrt(ySquared) * world.scale;
        const float authoredTransverse = world.scale;
        if (!finite(longitudinal) || longitudinal > 64.0F ||
            !finite(authoredTransverse) || authoredTransverse <= 0.0F ||
            authoredTransverse > 64.0F) { return; }

        // CP009: the default ARTO mesh has 1/26 of the legacy
        // Whole-Lens transverse dimensions. Do not multiply
        // ACTI, gameplay widths or BeamWidthTransform.
        // Tintable keeps its prior mapping pending NIF measurement.
        constexpr float kDefaultArtTransverseRatio{26.0F};
        const float transverse = slot.asset == AssetID::kBeamWholeLens ?
            authoredTransverse * kDefaultArtTransverseRatio :
            authoredTransverse;
        if (!finite(transverse) || transverse <= 0.0F ||
            transverse > 2048.0F) { return; }

        auto basis = world.rotate;
        const float yLength = std::sqrt(ySquared);
        for (std::size_t row = 0; row < 3; ++row) {
            basis.entry[row][1] /= yLength;
        }
        const auto& parent = axis->parent->world;
        if (!finite(parent.scale) || std::fabs(parent.scale) < 0.001F) {
            return;
        }

        auto stretch = basis;
        for (std::size_t row = 0; row < 3; ++row) {
            stretch.entry[row][0] *= transverse;
            stretch.entry[row][1] *= longitudinal;
            stretch.entry[row][2] *= transverse;
        }
        const auto inverse = parent.rotate.Transpose();
        axis->local.translate =
            inverse * (world.translate - parent.translate) / parent.scale;
        axis->local.rotate = inverse * stretch;
        axis->local.scale = 1.0F / parent.scale;
        RE::NiUpdateData update{};
        root->Update(update);

        const float length = 512.0F * longitudinal;
        const auto direction = basis * RE::NiPoint3{0.0F, 1.0F, 0.0F};
        const auto center = world.translate + direction * (length * 0.5F);
        // The measured default ARTO maximum outer radius is 1.92.
        // Tintable retains its existing conservative bound.
        const float boundTransverse = slot.asset == AssetID::kBeamWholeLens ?
            transverse * 2.0F : transverse * 256.0F;
        const float radius = length * 0.5F + boundTransverse + 8.0F;
        if (!finite(radius) || radius <= 0.0F ||
            radius > 32768.0F) { return; }
        const float fade = slot.beamDuration > 0.0F ?
            BeamAlphaFactor(slot.beamElapsed) : 1.0F;
        ApplyArtBeamBounds(root, center, radius,
            slot.actiVisible ? fade : 0.0F);
        root->SetAppCulled(!slot.actiVisible || fade <= 0.0F);

        if(slot.engineOnly&&slot.artStage!=3){slot.artStage=3;logger::info("[TOAM][CP021GUIDE] FIRST_POSE | Form={:08X} | Gen={} | Origin=({},{},{}) | Length={} | Pixels=UNVERIFIED",slot.beamArtBase->GetFormID(),slot.generation,world.translate.x,world.translate.y,world.translate.z,length);}
        // Show the engine-owned art instead of potentially duplicated ACTI.
        if (slot.actiRef && slot.actiRef->Get3D()) {
            slot.actiRef->Get3D()->SetAppCulled(true);
        }
        if (!slot.beamArtReady) {
            slot.beamArtReady = true;
            static std::atomic<std::uint32_t> ready{0};
            if (ready.fetch_add(1, std::memory_order_relaxed) < 16) {
                logger::info("[TOAM][ARTO] BEAM_READY | Length={:.2f}", length);
            }
        }
    }

    void TransientVFXManager::ClearSlotBackend(Slot& slot) noexcept
    {
        if(slot.beamArtRequested && !slot.beamArtEffect) {
            slot.artReuseAfter=std::chrono::steady_clock::now()+std::chrono::milliseconds(1700);
        }
        if (slot.beamArtBase) {
            // Retire even if ProcessLists did not yet yield an effect pointer.
            if (auto* player = RE::PlayerCharacter::GetSingleton()) {
                if (auto* lists = RE::ProcessLists::GetSingleton()) {
                    lists->ForEachModelEffect([&](RE::ModelReferenceEffect* effect) {
                        if (!effect->finished &&
                            effect->artObject == slot.beamArtBase &&
                            effect->target.get().get() == player) {
                            effect->finished = true;
                        }
                        return RE::BSContainer::ForEachResult::kContinue;
                    });
                }
            }
        }
        if (slot.beamArtEffect) {
            slot.beamArtEffect->finished = true;
            slot.beamArtEffect.reset();
        }
        slot.beamArtBase = nullptr;
        slot.beamArtReady = false;
        slot.beamArtRequested = false;
        slot.beamArtAcquireElapsed = 0.0F;
        if (slot.actiRef) {
            auto retained = slot.actiRef;
            slot.actiRef.reset();
            slot.actiParent.reset();
            slot.actiLastRoot = nullptr;
            slot.beamActiParts.fill(nullptr);
            slot.actiApplied = false;
            retained->SetCollision(false);
            retained->Disable();
            retained->SetDelete(true);
        }
        slot.actiParent.reset();slot.engineOnly=false;slot.artStage=0;
        slot.instance.Release();
        slot.asset = AssetID::kCount;
    }

    TransientVFXManager&
    TransientVFXManager::Get() noexcept
    {
        static TransientVFXManager
            manager{};

        return manager;
    }





    VFXHandle
    TransientVFXManager::Spawn(
        const SpawnRequest& request) noexcept
    {
        if (
            request.asset ==
                AssetID::kCount ||
            !request.parent
        ) {
            return {};
        }

        for (
            std::size_t index = 0;
            index < _slots.size();
            ++index
        ) {
            auto& slot =
                _slots[index];

            if (slot.active || std::chrono::steady_clock::now() < slot.artReuseAfter) {
                continue;
            }

            if (
                slot.generation ==
                kGenerationCeiling
            ) {
                slot.generation =
                    1;
            }
            else {

                ++slot.generation;

                if (
                    slot.generation == 0
                ) {
                    slot.generation =
                        1;
                }
            }

            const bool beamActi = request.asset == AssetID::kBeamWholeLens ||
                request.asset == AssetID::kBeamWholeLensTintable;
            const bool useActi = request.asset == AssetID::kCircleStatic ||
                request.asset == AssetID::kCircleOuterBandStatic ||
                request.asset == AssetID::kFlashCloud ||
                request.asset == AssetID::kFlashCloudTintable ||
                (beamActi && request.lifetimeSeconds >= 0.0F);
            slot.engineOnly = beamActi && request.lifetimeSeconds < 0.0F;
            slot.artStage = 0;
            slot.asset = request.asset;
            slot.actiVisible = request.visible;
            slot.actiLocal = request.local;
            slot.actiLastRoot = nullptr;
            slot.beamActiParts.fill(nullptr);
            slot.beamActiReadyWait = 0.0F;
            slot.actiApplied = false;
            slot.flashActiReadyWait = 0.0F;
            slot.fadeGeometryCount = 0;
            slot.circleFadeDuration = 0.0F;
            slot.circleFadeElapsed = 0.0F;
            if (slot.engineOnly) {
                slot.actiParent.reset(request.parent);
            } else if (useActi) {
                auto* player = RE::PlayerCharacter::GetSingleton();
                const char* actiEditorID =
                    request.asset == AssetID::kCircleOuterBandStatic
                    ? "TOAM_VFX_CircleOuterBandStatic_ACTI"
                    : request.asset == AssetID::kFlashCloud
                    ? "TOAM_VFX_FlashCloud_ACTI"
                    : request.asset == AssetID::kFlashCloudTintable
                    ? "TOAM_VFX_FlashCloudTintable_ACTI"
                    : request.asset == AssetID::kBeamWholeLens
                    ? "TOAM_VFX_BeamWholeLens_ACTI"
                    : request.asset == AssetID::kBeamWholeLensTintable
                    ? "TOAM_VFX_BeamWholeLensTintable_ACTI"
                    : "TOAM_VFX_CircleStatic_ACTI";
                auto* base = RE::TESForm::LookupByEditorID<RE::TESObjectACTI>(
                    actiEditorID);
                if (!player || !base) {
                    logger::error("[TOAM][VFX] F2_ACTI_BASE_MISSING | Player={} | Form={}",
                        player != nullptr, base != nullptr);
                    slot.asset = AssetID::kCount;
                    return {};
                }
                auto placed = player->PlaceObjectAtMe(base, false);
                if (!placed) {
                    logger::error("[TOAM][VFX] F2_ACTI_SPAWN_FAILED");
                    slot.asset = AssetID::kCount;
                    return {};
                }
                slot.actiRef = placed;
                slot.actiParent.reset(request.parent);
                placed->SetTemporary();
                placed->SetCollision(false);
                if (!ApplyActiTransform(slot, request.local)) {
                    ClearSlotBackend(slot);
                    return {};
                }
            } else if (!slot.instance.Spawn(
                request.asset, request.parent, request.local, request.visible)) {
                slot.asset = AssetID::kCount;
                return {};
            }

            slot.growthDuration=0.0F;
            slot.growthElapsed=0.0F;
            slot.beamDuration=0.0F;
            slot.beamElapsed=0.0F;
            slot.remainingLifetime =
                request.lifetimeSeconds;

            if (!useActi) { slot.fadeGeometryCount = 0; }
            slot.circleFadeElapsed = 0.0F;
            slot.circleFadeDuration = 0.0F;
            slot.circleReadyWait = 0.0F;
            // Cache only geometry owned by the cloned scene graph.
            // Neither the prototype nor shared shader/material is touched.
            if (!useActi && !slot.engineOnly) {
                slot.instance.Node()->fadeAmount = 1.0F;
                bool fadeOverflow = false;
                RE::BSVisit::TraverseScenegraphGeometries(
                    slot.instance.Node(),
                    [&slot, &fadeOverflow](RE::BSGeometry* geometry) {
                        if (slot.fadeGeometryCount >= slot.fadeGeometry.size()) {
                            fadeOverflow = true;
                            return RE::BSVisit::BSVisitControl::kStop;
                        }
                        geometry->fadeAmount = 1.0F;
                        slot.fadeGeometry[slot.fadeGeometryCount++] = geometry;
                        return RE::BSVisit::BSVisitControl::kContinue;
                    });
                if (fadeOverflow) {
                    slot.fadeGeometryCount = 0;
                    static std::uint32_t overflowLogCount{};
                    if (overflowLogCount++ < 8) {
                        logger::warn("[TOAM][VFX] FADE_MESH_OVERFLOW | Asset={}",
                            AssetName(request.asset));
                    }
                }
            }

            slot.active =
                true;

            // Only the R11 whole-lens presentation receives the 0.48s
            // width envelope. Leave marking/segmented beams unchanged.
            if ((request.asset == AssetID::kBeamWholeLens ||
                 request.asset == AssetID::kBeamWholeLensTintable) &&
                std::isfinite(request.lifetimeSeconds) &&
                request.lifetimeSeconds >= kBeamEnvelopeSeconds) {
                slot.beamBaseTransform = request.local;
                slot.beamDuration = kBeamEnvelopeSeconds;
                const auto initial = BeamWidthTransform(
                    slot.beamBaseTransform, kBeamWidthValues[0]);
                if (useActi) { slot.actiLocal = initial; }
                if (!(useActi ? ApplyActiTransform(slot, initial) :
                      slot.instance.SetLocalTransform(initial))) {
                    ClearSlotBackend(slot);
                    slot.remainingLifetime = -1.0F;
                    slot.beamDuration = 0.0F;
                    slot.active = false;
                    return {};
                }
                static std::uint32_t startLogCount{};
                if (startLogCount < 16) {
                    ++startLogCount;
                    logger::info(
                        "[TOAM][VFX] BEAM_ENVELOPE_START | Asset={} | Duration={:.2f} | BaseWidth={:.4f}",
                        AssetName(request.asset), slot.beamDuration, request.local.scale);
                }
            }

            // D2E: slot-specific ARTO for default beam and both flashes.
            // ACTI remains visible until the ARTO model is ready.
            const bool flashArt = request.asset == AssetID::kFlashCloud ||
                request.asset == AssetID::kFlashCloudTintable;
            const bool wholeBeamArt =
                request.asset == AssetID::kBeamWholeLens ||
                request.asset == AssetID::kBeamWholeLensTintable;
            if (flashArt || wholeBeamArt) {
                StartBeamArt(slot, index);
                if(slot.engineOnly&&!slot.beamArtRequested){
                    logger::warn("[TOAM][CP021GUIDE] REQUEST_FAILED | Slot={} | Gen={}",index,slot.generation);
                    ClearSlotBackend(slot);slot.active=false;return {};
                }
            }

            return VFXHandle{
                static_cast<
                    std::uint16_t>(index),
                slot.generation
            };
        }

        logger::error(
            "[TOAM][VFX] Transient capacity exhausted | Capacity={}",
            kCapacity);

        return {};
    }

    bool TransientVFXManager::BeginGrowth(VFXHandle handle, float seconds) noexcept
    {
        auto* slot=Resolve(handle);
        if (!slot || !std::isfinite(seconds) || seconds<=0.0F) {return false;}
        slot->growthTarget = slot->actiRef ? slot->actiLocal :
            slot->instance.Node()->local;
        slot->growthElapsed=0.0F;slot->growthDuration=seconds;
        auto initial=slot->growthTarget;initial.scale*=0.02F;
        return slot->actiRef ? ApplyActiTransform(*slot, initial) :
            slot->instance.SetLocalTransform(initial);
    }
    bool TransientVFXManager::Retire(VFXHandle handle, float holdSeconds) noexcept
    {
        auto* slot=Resolve(handle);
        if (!slot || !std::isfinite(holdSeconds) || holdSeconds<=0.0F) {return false;}
        slot->remainingLifetime=holdSeconds;
        switch (slot->asset) {
        case AssetID::kCircleStatic:
        case AssetID::kCircleCoreStatic:
        case AssetID::kCircleAnimated:
        case AssetID::kCircleCoreAnimated:
        case AssetID::kCircleSmall:
        case AssetID::kCircleOuterBandStatic:
            slot->circleFadeElapsed = 0.0F;
            slot->circleFadeDuration = holdSeconds;
            break;
        default:
            break;
        }
        static std::uint32_t retireLogCount{};
        if (slot->circleFadeDuration > 0.0F && retireLogCount++ < 16) {
            logger::info("[TOAM][VFX] CIRCLE_FADE_START | Seconds={:.2f}", holdSeconds);
        }
        return true;
    }

    bool TransientVFXManager::Release(
        const VFXHandle handle) noexcept
    {
        auto* slot =
            Resolve(handle);

        if (!slot) {
            return false;
        }



        ClearSlotBackend(*slot);

        slot->remainingLifetime =
            -1.0F;
        slot->beamDuration = 0.0F;
        slot->beamElapsed = 0.0F;
        slot->growthDuration = 0.0F;
        slot->circleFadeDuration = 0.0F;
        slot->fadeGeometryCount = 0;

        slot->active =
            false;

        return true;
    }

    void TransientVFXManager::ReleaseAll() noexcept
    {
        std::size_t released{};

        for (auto& slot : _slots) {

            if (!slot.active) {
                continue;
            }

            ClearSlotBackend(slot);

            slot.remainingLifetime =
                -1.0F;
            slot.beamDuration = 0.0F;
            slot.beamElapsed = 0.0F;
            slot.growthDuration = 0.0F;
            slot.circleFadeDuration = 0.0F;
            slot.fadeGeometryCount = 0;

            slot.active =
                false;

            ++released;
        }

        if (released > 0) {

            logger::info(
                "[TOAM][VFX] ReleaseAll | Released={}",
                released);
        }
    }

    std::size_t TransientVFXManager::ReleaseUnbounded() noexcept
    {
        std::size_t released{};
        for (std::size_t index = 0; index < _slots.size(); ++index) {
            const auto& slot = _slots[index];
            if (!slot.active ||
                (std::isfinite(slot.remainingLifetime) && slot.remainingLifetime > 0.0F)) {
                continue;
            }
            if (Release(VFXHandle{static_cast<std::uint16_t>(index), slot.generation})) {
                ++released;
            }
        }
        return released;
    }

    void TransientVFXManager::Update(
        const float deltaSeconds) noexcept
    {

        if (
            !std::isfinite(deltaSeconds) ||
            deltaSeconds <= 0.0F
        ) {
            return;
        }

        for (std::size_t slotIndex = 0; slotIndex < _slots.size(); ++slotIndex) {
            auto& slot = _slots[slotIndex];
            if (!slot.active) {continue;}
            // The existing bounded transient tick owns the ARTO companion.
            if (slot.beamArtRequested) {
                UpdateBeamArt(slot, deltaSeconds);
                if(slot.engineOnly&&slot.artStage==4){
                    ClearSlotBackend(slot);slot.active=false;continue;
                }
            }

            // QA1-C3B: ACTI 3D may be deferred after PlaceObjectAtMe().
            // The normal effect clock starts only after 3D is ready.
            // On timeout use the existing retirement branch, not a new sink.
            const bool waitingFlashActi = slot.actiRef &&
                (slot.asset == AssetID::kFlashCloud ||
                 slot.asset == AssetID::kFlashCloudTintable) &&
                slot.flashActiReadyWait >= 0.0F;
            if (waitingFlashActi) {
                if (slot.beamArtReady ||
                    (slot.actiRef->Get3D() &&
                     ApplyActiTransform(slot, slot.actiLocal))) {
                    static std::atomic<std::uint32_t> readyLogs{0};
                    if (readyLogs.fetch_add(1, std::memory_order_relaxed) < 16) {
                        logger::info("[TOAM][VFX] FLASH_ACTI_3D_READY | Asset={} | Wait={:.3f}",
                            AssetName(slot.asset), slot.flashActiReadyWait);
                    }
                    slot.flashActiReadyWait = -1.0F;
                } else {
                    slot.flashActiReadyWait += deltaSeconds;
                    if (slot.flashActiReadyWait <
                        (slot.beamArtRequested ? 1.0F : 0.50F)) {
                        continue;
                    }
                    static std::atomic<std::uint32_t> timeoutLogs{0};
                    if (timeoutLogs.fetch_add(1, std::memory_order_relaxed) < 16) {
                        logger::warn("[TOAM][VFX] FLASH_ACTI_3D_TIMEOUT | Asset={}",
                            AssetName(slot.asset));
                    }
                    slot.remainingLifetime = deltaSeconds;
                }
            }

            // QA1-C3C-2: do not advance the beam width/lifetime until the
            // ACTI model and its named deformation root are both available.
            if (slot.actiRef &&
                (slot.asset == AssetID::kBeamWholeLens ||
                 slot.asset == AssetID::kBeamWholeLensTintable) &&
                slot.beamActiReadyWait >= 0.0F) {
                if (slot.actiRef->Get3D() &&
                    ApplyActiTransform(slot, slot.actiLocal) &&
                    slot.beamActiParts[0] &&
                    slot.beamActiParts[1] && slot.beamActiParts[2]) {
                    static std::atomic<std::uint32_t> readyCount{0};
                    if (readyCount.fetch_add(1, std::memory_order_relaxed) < 16) {
                        logger::info("[TOAM][VFX] BEAM_ACTI_READY | Asset={} | Wait={:.3f}",
                            AssetName(slot.asset), slot.beamActiReadyWait);
                    }
                    slot.beamActiReadyWait = -1.0F;
                } else {
                    slot.beamActiReadyWait += deltaSeconds;
                    if (slot.beamArtReady) {
                        slot.beamActiReadyWait = -1.0F;
                    } else if (slot.beamActiReadyWait < (
                        slot.beamArtRequested ? 1.0F : 0.50F)) {
                        continue;
                    } else {
                    static std::atomic<std::uint32_t> timeoutCount{0};
                    if (timeoutCount.fetch_add(1, std::memory_order_relaxed) < 16) {
                        logger::warn("[TOAM][VFX] BEAM_ACTI_READY_TIMEOUT | Asset={}",
                            AssetName(slot.asset));
                    }
                    ClearSlotBackend(slot);
                    slot.beamDuration = 0.0F;
                    slot.remainingLifetime = -1.0F;
                    slot.active = false;
                    continue;
                    }
                }
            }

            // CP018: a newly placed ACTI circle can exist before its
            // CC_Root is present.  Delay presentation clocks briefly so
            // short-lived proc circles cannot expire before first pose.
            // No additional hook or Actor/ProcessLists scan is introduced.
            const bool circleActi = slot.actiRef &&
                (slot.asset == AssetID::kCircleStatic ||
                 slot.asset == AssetID::kCircleOuterBandStatic);
            if (circleActi && slot.circleReadyWait >= 0.0F) {
                if (!slot.actiRef->Get3D() || !slot.actiApplied) {
                    auto pose = slot.actiLocal;
                    if (slot.growthDuration > 0.0F) {
                        pose = slot.growthTarget;
                        pose.scale *= Presentation::GrowthFactor(
                            slot.growthElapsed, slot.growthDuration);
                    }
                    static_cast<void>(ApplyActiTransform(slot, pose));
                }
                if (slot.actiRef->Get3D() && slot.actiApplied) {
                    if (slot.circleReadyWait > 0.0F) {
                        static std::atomic<std::uint32_t> resumeLogs{0};
                        if (resumeLogs.fetch_add(1, std::memory_order_relaxed) < 16) {
                            logger::info(
                                "[TOAM][CIRCLE_POSE] CLOCK_RESUME | Asset={} | Wait={:.3f}",
                                AssetName(slot.asset), slot.circleReadyWait);
                        }
                    }
                    slot.circleReadyWait = -1.0F;
                } else if (slot.circleReadyWait < 0.60F) {
                    slot.circleReadyWait += deltaSeconds;
                    // Only the visual clock is deferred, and at most 0.6 s.
                    // A failed/missing NIF cannot hold an effect indefinitely.
                    continue;
                } else {
                    static std::atomic<std::uint32_t> timeoutLogs{0};
                    if (timeoutLogs.fetch_add(1, std::memory_order_relaxed) < 16) {
                        logger::warn(
                            "[TOAM][CIRCLE_POSE] CLOCK_GRACE_EXPIRED | Asset={} | Wait={:.3f}",
                            AssetName(slot.asset), slot.circleReadyWait);
                    }
                    slot.circleReadyWait = -1.0F;
                }
            }

            // P0-4C-B1: FlashCloud shader alpha starts at zero.
            // Advance NIF controllers using this instance's elapsed time.
            // Reuse the existing bounded transient update; no new poller.
            RE::NiAVObject* flashRoot = nullptr;
            if (slot.asset == AssetID::kFlashCloud ||
                slot.asset == AssetID::kFlashCloudTintable) {
                flashRoot = slot.beamArtReady && slot.beamArtEffect &&
                    !slot.beamArtEffect->finished ?
                    slot.beamArtEffect->Get3D() :
                    (slot.actiRef ? slot.actiRef->Get3D() :
                     slot.instance.Node());
            }
            if (flashRoot &&
                slot.growthDuration <= 0.0F &&
                slot.remainingLifetime > 0.0F) {
                const float previous = slot.growthElapsed;
                const float next = previous + deltaSeconds;
                if (std::isfinite(next)) {
                    slot.growthElapsed = next < 2.20F ? next : 2.20F;
                    RE::NiUpdateData animation{};
                    animation.time = slot.growthElapsed;
                    animation.flags.set(RE::NiUpdateData::Flag::kDirty);
                    flashRoot->Update(animation);

                    if (previous < 0.12F && slot.growthElapsed >= 0.12F) {
                        static std::atomic<std::uint32_t> samples{0};
                        if (samples.fetch_add(1, std::memory_order_relaxed) < 16) {
                            logger::info(
                                "[TOAM][VFX] P04C_FLASH_TIME | Asset={} | Time={:.3f} | RootBound={:.3f}",
                                AssetName(slot.asset), slot.growthElapsed,
                                flashRoot->worldBound.radius);
                        }
                    }
                }
            }

            if (slot.growthDuration>0.0F) {
                slot.growthElapsed+=deltaSeconds;
                auto transform=slot.growthTarget;
                transform.scale*=Presentation::GrowthFactor(slot.growthElapsed,slot.growthDuration);
                if (slot.actiRef) {
                    static_cast<void>(ApplyActiTransform(slot, transform));
                } else {
                    static_cast<void>(slot.instance.SetLocalTransform(transform));
                }
                if(slot.growthElapsed>=slot.growthDuration) {slot.growthDuration=0.0F;}
            }
            if (slot.beamDuration > 0.0F) {
                slot.beamElapsed += deltaSeconds;
                if (slot.fadeGeometryCount > 0) {
                    ApplyInstanceFade(slot.actiRef ? slot.actiRef->Get3D() :
                        slot.instance.Node(), slot.fadeGeometry,
                        slot.fadeGeometryCount, BeamAlphaFactor(slot.beamElapsed));
                }
                const float widthFactor = BeamWidthFactor(slot.beamElapsed);
                if (widthFactor > 0.0F) {
                    const auto transform = BeamWidthTransform(
                        slot.beamBaseTransform, widthFactor);
                    if (slot.actiRef) {
                        slot.actiLocal = transform;
                        static_cast<void>(ApplyActiTransform(slot, transform));
                    } else {
                        static_cast<void>(slot.instance.SetLocalTransform(transform));
                    }
                } else {
                    // Last sampled instant: no zero-scale/Inf matrix; life
                    // owner releases this slot at its original expiry.
                    if (slot.actiRef) {
                        slot.actiVisible = false;
                        if (auto* root = slot.actiRef->Get3D()) {
                            root->SetAppCulled(true);
                        }
                    } else {
                        static_cast<void>(slot.instance.SetVisible(false));
                    }
                    slot.beamDuration = 0.0F;
                }
            }
            if (slot.circleFadeDuration > 0.0F) {
                slot.circleFadeElapsed += deltaSeconds;
                if (slot.fadeGeometryCount > 0) {
                    const float remaining = slot.circleFadeDuration - slot.circleFadeElapsed;
                    const float fade = remaining > 0.0F
                        ? remaining / slot.circleFadeDuration : 0.0F;
                    ApplyInstanceFade(slot.actiRef ? slot.actiRef->Get3D() :
                        slot.instance.Node(), slot.fadeGeometry,
                        slot.fadeGeometryCount, fade);
                }
                if (slot.circleFadeElapsed >= slot.circleFadeDuration) {
                    slot.circleFadeDuration = 0.0F;
                }
            }
            if (slot.actiRef && slot.growthDuration <= 0.0F) {
                static_cast<void>(ApplyActiTransform(slot, slot.actiLocal));
            }
            if (slot.beamArtReady && slot.beamArtEffect &&
                !slot.beamArtEffect->finished && slot.actiRef) {
                if (auto* oldRoot = slot.actiRef->Get3D()) {
                    oldRoot->SetAppCulled(true);
                }
            }
            if (slot.remainingLifetime<=0.0F) {continue;}

            slot.remainingLifetime -=
                deltaSeconds;

            if (
                slot.remainingLifetime >
                0.0F
            ) {
                continue;
            }


            ClearSlotBackend(slot);

            slot.remainingLifetime =
                -1.0F;
            slot.beamDuration = 0.0F;
            slot.beamElapsed = 0.0F;
            slot.growthDuration = 0.0F;
            slot.circleFadeDuration = 0.0F;
            slot.fadeGeometryCount = 0;

            slot.active =
                false;
        }
    }

    bool TransientVFXManager::SetTransform(
        const VFXHandle handle,
        const RE::NiTransform& transform) noexcept
    {
        auto* slot =
            Resolve(handle);

        if(!slot) {return false;}
        if(slot->engineOnly){slot->actiLocal=transform;return true;}
        if (slot->actiRef) {
            if (slot->beamDuration > 0.0F) {
                slot->beamBaseTransform = transform;
                const auto widened = BeamWidthTransform(
                    transform, BeamWidthFactor(slot->beamElapsed));
                slot->actiLocal = widened;
                return ApplyActiTransform(*slot, widened);
            }
            slot->actiLocal = transform;
            auto effective = transform;
            if (slot->growthDuration > 0.0F) {
                slot->growthTarget = transform;
                effective.scale *= Presentation::GrowthFactor(
                    slot->growthElapsed, slot->growthDuration);
            }
            return ApplyActiTransform(*slot, effective);
        }
        if (slot->beamDuration > 0.0F) {
            slot->beamBaseTransform = transform;
            return slot->instance.SetLocalTransform(
                BeamWidthTransform(transform, BeamWidthFactor(slot->beamElapsed)));
        }
        auto effective=transform;
        if(slot->growthDuration>0.0F) {
            slot->growthTarget=transform;
            effective.scale*=Presentation::GrowthFactor(slot->growthElapsed,slot->growthDuration);
        }
        return slot->instance.SetLocalTransform(effective);
    }

    bool TransientVFXManager::SetVisible(
        const VFXHandle handle,
        const bool visible) noexcept
    {
        auto* slot =
            Resolve(handle);

        if (!slot) { return false; }
        if(slot->engineOnly){slot->actiVisible=visible;return true;}
        if (slot->actiRef) {
            slot->actiVisible = visible;
            if (auto* root = slot->actiRef->Get3D()) {
                const bool pendingCirclePose =
                    (slot->asset == AssetID::kCircleStatic ||
                     slot->asset == AssetID::kCircleOuterBandStatic) &&
                    !slot->actiApplied;
                root->SetAppCulled(
                    !visible || slot->beamArtReady || pendingCirclePose);
            }
            return true;
        }
        return slot->instance.SetVisible(visible);
    }

    RE::NiAVObject*
    TransientVFXManager::Node(
        const VFXHandle handle) noexcept
    {
        auto* slot =
            Resolve(handle);

        return slot ? (slot->engineOnly ? (slot->beamArtEffect ? slot->beamArtEffect->Get3D() : nullptr) :
            (slot->actiRef ? slot->actiRef->Get3D() : slot->instance.Node())) : nullptr;
    }

    bool TransientVFXManager::IsAlive(VFXHandle handle) noexcept
    {
        return Resolve(handle) != nullptr;
    }

    std::size_t
    TransientVFXManager::ActiveCount() const noexcept
    {
        std::size_t count{};

        for (const auto& slot : _slots) {

            if (slot.active) {
                ++count;
            }
        }

        return count;
    }

    TransientVFXManager::Slot*
    TransientVFXManager::Resolve(
        const VFXHandle handle) noexcept
    {
        if (
            !handle.IsValid() ||
            handle.slot >=
                _slots.size()
        ) {
            return nullptr;
        }

        auto& slot =
            _slots[handle.slot];

        if (
            !slot.active ||
            slot.generation !=
                handle.generation
        ) {
            return nullptr;
        }

        return
            &slot;
    }
}