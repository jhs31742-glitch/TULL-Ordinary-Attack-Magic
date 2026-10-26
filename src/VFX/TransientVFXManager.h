#pragma once

#include "VFX/VFXInstance.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>

namespace TOAM::VFX
{
    struct VFXHandle
    {
        static constexpr
            std::uint16_t kInvalidSlot{
                0xFFFFu
            };

        std::uint16_t slot{
            kInvalidSlot
        };

        std::uint16_t generation{};

        [[nodiscard]]
        bool IsValid() const noexcept
        {
            return
                slot != kInvalidSlot;
        }
    };

    struct SpawnRequest
    {
        AssetID asset{
            AssetID::kCount
        };

        RE::NiNode* parent{};

        RE::NiTransform local{};

        float lifetimeSeconds{
            -1.0F
        };

        bool visible{
            true
        };
    };

    class TransientVFXManager final
    {
    public:
        static constexpr
            std::size_t kCapacity{
                128
            };

        [[nodiscard]]
        static TransientVFXManager&
        Get() noexcept;

        [[nodiscard]]
        VFXHandle Spawn(
            const SpawnRequest& request) noexcept;

        [[nodiscard]]
        bool Release(
            VFXHandle handle) noexcept;

        void ReleaseAll() noexcept;
        [[nodiscard]] std::size_t ReleaseUnbounded() noexcept;
        [[nodiscard]] bool BeginGrowth(VFXHandle handle, float seconds) noexcept;
        [[nodiscard]] bool Retire(VFXHandle handle, float holdSeconds) noexcept;

        void Update(
            float deltaSeconds) noexcept;

        [[nodiscard]]
        bool SetTransform(
            VFXHandle handle,
            const RE::NiTransform& transform) noexcept;

        [[nodiscard]]
        bool SetVisible(
            VFXHandle handle,
            bool visible) noexcept;

        [[nodiscard]]
        RE::NiAVObject* Node(
            VFXHandle handle) noexcept;

        [[nodiscard]]
        std::size_t ActiveCount() const noexcept;

        [[nodiscard]] bool IsAlive(VFXHandle handle) noexcept;

    private:
        struct Slot
        {
            VFXInstance instance{};
            // F2-H2-P0-2B: CircleStatic and CircleOuterBandStatic use ACTI refs.
            RE::NiPointer<RE::TESObjectREFR> actiRef{};
            RE::NiPointer<RE::NiNode> actiParent{};
            RE::NiTransform actiLocal{};
            RE::NiTransform actiLastWorld{};
            RE::NiAVObject* actiLastRoot{};
            // The NIF root is managed by the ACTI reference. Only the
            // three named child nodes can receive non-uniform beam geometry.
            std::array<RE::NiAVObject*, 3> beamActiParts{};
            std::array<RE::NiTransform, 3> beamActiPartOriginals{};
            float beamActiReadyWait{};
            // D2D: independent engine-owned ARTO overlay; ACTI remains fallback.
            RE::NiPointer<RE::ModelReferenceEffect> beamArtEffect{};
            RE::BGSArtObject* beamArtBase{};
            float beamArtAcquireElapsed{};
            bool beamArtRequested{};
            std::chrono::steady_clock::time_point artReuseAfter{};
            bool beamArtReady{};
            bool engineOnly{};
            std::uint8_t artStage{};
            AssetID asset{AssetID::kCount};
            bool actiApplied{};
            bool actiVisible{true};

            std::uint16_t generation{};

            float remainingLifetime{
                -1.0F
            };

            RE::NiTransform growthTarget{};
            float growthElapsed{};
            float growthDuration{};

            // R11: one whole-lens beam owns its width envelope here,
            // surviving BeamRenderer::Detach without another tick source.
            RE::NiTransform beamBaseTransform{};
            float beamElapsed{};
            float beamDuration{};

            // E2-B: mesh references belong to the cloned instance only.
            std::array<RE::NiAVObject*, 32> fadeGeometry{};
            std::size_t fadeGeometryCount{};
            float circleFadeElapsed{};
            float circleFadeDuration{};
            // CP018: capped 3D-pose readiness grace, once per ACTI circle.
            float circleReadyWait{};
            // QA1-C3B: -1 means FlashCloud ACTI 3D is ready.
            float flashActiReadyWait{};
            bool active{};
        };

        [[nodiscard]]
        Slot* Resolve(
            VFXHandle handle) noexcept;
        [[nodiscard]] bool ApplyActiTransform(Slot& slot,
            const RE::NiTransform& local) noexcept;
        void ClearSlotBackend(Slot& slot) noexcept;
        void StartBeamArt(Slot& slot, std::size_t slotIndex) noexcept;
        void UpdateBeamArt(Slot& slot, float deltaSeconds) noexcept;

        std::array<
            Slot,
            kCapacity
        > _slots{};

    };
}