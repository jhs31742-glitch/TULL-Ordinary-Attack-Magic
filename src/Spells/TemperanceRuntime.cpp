#include "Audio/SpellAudio.h"

#include "Spells/TemperanceRuntime.h"

#include "Combat/ActorBounds.h"
#include "Combat/BeamCollision.h"
#include "Combat/FixedDamage.h"
#include "Combat/TargetFormation.h"
#include "Combat/TriggerFirewall.h"
#include "Combat/WeaponHitSource.h"
#include "Core/DataCache.h"
#include "Core/MagickaTransaction.h"
#include "Core/Runtime.h"
#include "VFX/AssetCatalog.h"
#include "VFX/BeamRenderer.h"
#include "VFX/CircleRenderer.h"

#include "RE/Skyrim.h"
#include "SKSE/Events.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace TOAM::Spells::Temperance::Runtime
{
    namespace
    {
        // TOAM_S08_B2_VFX

        inline constexpr float kProcDelaySeconds{ 0.27F };
        inline constexpr float kCircleDelaySeconds{ 0.15F };
        inline constexpr float kNormalCost{ 50.0F };
        inline constexpr float kNormalDamage{ 50.0F };
        inline constexpr float kPowerCost{ 150.0F };
        inline constexpr float kPowerBeamDamage{ 80.0F };

        inline constexpr float
            kShellRadiusWorldUnits{
                128.0F
            };

        inline constexpr float
            kCircleVisualScale{
                0.10F
            };

        inline constexpr float
            kBeamVisualWidth{
                0.08F
            };

        inline constexpr float
            kBeamVisualLifetimeSeconds{
                0.48F
            };

        inline constexpr float
            kDirectionEpsilon{
                0.000001F
            };

        inline constexpr std::size_t
            kNormalPoolCapacity{
                3
            };

        inline constexpr std::size_t
            kPowerPoolCapacity{
                2
            };

        inline constexpr float
            kBeamCollisionRadiusWorldUnits{
                0.0F
            };

        inline constexpr std::size_t
            kPowerVisualCount{
                3
            };

        inline constexpr std::size_t
            kVisualSlotCount{
                kNormalPoolCapacity +
                kPowerPoolCapacity * kPowerVisualCount
            };

        inline constexpr std::size_t
            kPowerVisualBaseSlot{
                kNormalPoolCapacity
            };

        inline constexpr std::size_t
            kInvalidVisualSlot{
                kVisualSlotCount
            };

        static_assert(
            kVisualSlotCount <=
                TOAM::VFX::CircleRenderer::
                    kCapacity
        );


        struct PendingProc final
        {
            RE::ActorHandle target{};

            std::array<
                RE::NiPoint3,
                kPowerVisualCount
            > circleWorld{};

            RE::NiPoint3
                targetCenter{};

            std::size_t
                visualBaseSlot{
                    kInvalidVisualSlot
                };

            std::size_t
                visualCount{};

            float elapsedSeconds{};
            bool circlesPresented{};

            bool visualGeometryValid{};
            bool active{};
        };


        struct State final
        {
            std::array<
                PendingProc,
                kNormalPoolCapacity
            > normal{};

            // Each Power proc owns three independent visual slots.
            std::array<PendingProc, kPowerPoolCapacity> power{};

            // Distinct from each already-paid proc's circleWorld snapshot.
            std::array<std::size_t, 2> recentNormalSlots{{
                TOAM::Combat::kUpperHemisphereSlotCount,
                TOAM::Combat::kUpperHemisphereSlotCount
            }};
            std::uint16_t lastPowerMask{};
            bool enabled{};
            bool spellSinkInstalled{};
        };


        struct VisualState final
        {
            TOAM::VFX::CircleRenderer
                circles{};

            std::array<
                TOAM::VFX::BeamRenderer,
                kVisualSlotCount
            > beams{};
        };


        struct VisualBasis final
        {
            RE::NiPoint3 x{};
            RE::NiPoint3 y{};
            RE::NiPoint3 z{};
        };


        [[nodiscard]]
        State& GetState() noexcept
        {
            static State state{};
            return state;
        }


        [[nodiscard]]
        VisualState& GetVisualState() noexcept
        {
            static VisualState state{};
            return state;
        }


        [[nodiscard]]
        bool IsFinitePoint(
            const RE::NiPoint3& point) noexcept
        {
            return
                std::isfinite(point.x) &&
                std::isfinite(point.y) &&
                std::isfinite(point.z);
        }


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
            result =
                {};

            const float lengthSquared =
                LengthSquared(
                    value
                );

            if (
                !std::isfinite(
                    lengthSquared
                ) ||
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
                IsFinitePoint(
                    result
                );
        }


        [[nodiscard]]
        bool IsFiniteTransform(
            const RE::NiTransform& transform) noexcept
        {
            if (
                !IsFinitePoint(
                    transform.translate
                ) ||
                !std::isfinite(
                    transform.scale
                ) ||
                transform.scale <=
                    kDirectionEpsilon
            ) {
                return false;
            }

            for (
                std::size_t row = 0;
                row < 3;
                ++row
            ) {
                for (
                    std::size_t column = 0;
                    column < 3;
                    ++column
                ) {
                    if (
                        !std::isfinite(
                            transform.rotate.
                                entry[row][column]
                        )
                    ) {
                        return false;
                    }
                }
            }

            return true;
        }


        [[nodiscard]]
        bool BuildVisualBasis(
            const RE::NiPoint3& direction,
            VisualBasis& result) noexcept
        {
            result =
                {};

            // ChargeCannon circle/beam authored forward axis is local +Y.
            if (
                !Normalize(
                    direction,
                    result.y
                )
            ) {
                return false;
            }

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

            if (
                !Normalize(
                    result.y.Cross(
                        reference
                    ),
                    result.x
                )
            ) {
                return false;
            }

            if (
                !Normalize(
                    result.x.Cross(
                        result.y
                    ),
                    result.z
                )
            ) {
                return false;
            }

            return true;
        }


        void ApplyVisualBasis(
            RE::NiTransform& transform,
            const VisualBasis& basis) noexcept
        {
            transform.rotate.entry[0][0] =
                basis.x.x;
            transform.rotate.entry[1][0] =
                basis.x.y;
            transform.rotate.entry[2][0] =
                basis.x.z;

            transform.rotate.entry[0][1] =
                basis.y.x;
            transform.rotate.entry[1][1] =
                basis.y.y;
            transform.rotate.entry[2][1] =
                basis.y.z;

            transform.rotate.entry[0][2] =
                basis.z.x;
            transform.rotate.entry[1][2] =
                basis.z.y;
            transform.rotate.entry[2][2] =
                basis.z.z;
        }


        [[nodiscard]]
        bool ResolveVisualRoot(
            RE::NiNode*& parent,
            RE::NiTransform& rootInverse) noexcept
        {
            parent =
                nullptr;

            rootInverse =
                {};

            auto* worldRoot =
                RE::Main::
                    WorldRootNode();

            if (
                !worldRoot ||
                !std::isfinite(
                    worldRoot->world.scale
                ) ||
                std::fabs(
                    worldRoot->world.scale
                ) <=
                    kDirectionEpsilon
            ) {
                return false;
            }

            rootInverse =
                worldRoot->
                    world.
                    Invert();

            if (
                !IsFiniteTransform(
                    rootInverse
                )
            ) {
                return false;
            }

            parent =
                worldRoot;

            return true;
        }


        [[nodiscard]]
        bool ResolveTargetFrame(
            RE::Actor* target,
            RE::NiPoint3& center,
            RE::NiPoint3& forwardAxis) noexcept
        {
            center =
                {};

            forwardAxis =
                {};

            if (!target) {
                return false;
            }

            TOAM::Combat::ActorBounds
                bounds{};

            if (
                !TOAM::Combat::
                    ResolveActorBounds(
                        target,
                        bounds
                    ) ||
                !TOAM::Combat::
                    IsUsableActorBounds(
                        bounds
                    )
            ) {
                return false;
            }

            auto* target3D =
                target->Get3D();

            if (!target3D) {
                return false;
            }

            const auto actorWorld =
                target3D->world;

            if (
                !IsFiniteTransform(
                    actorWorld
                )
            ) {
                return false;
            }

            const RE::NiPoint3
                localOrigin{
                    0.0F,
                    0.0F,
                    0.0F
                };

            const RE::NiPoint3
                localForward{
                    0.0F,
                    1.0F,
                    0.0F
                };

            const RE::NiPoint3
                worldOrigin =
                    actorWorld *
                    localOrigin;

            const RE::NiPoint3
                worldForward =
                    actorWorld *
                    localForward;

            const RE::NiPoint3
                forwardCandidate =
                    worldForward -
                    worldOrigin;

            if (
                !Normalize(
                    forwardCandidate,
                    forwardAxis
                )
            ) {
                return false;
            }

            center =
                bounds.center;

            return
                IsFinitePoint(
                    center
                );
        }


        void ResetProcVisualGeometry(
            PendingProc& proc) noexcept
        {
            proc.circleWorld =
                {};

            proc.targetCenter =
                {};

            proc.visualBaseSlot =
                kInvalidVisualSlot;

            proc.visualCount =
                0;

            proc.visualGeometryValid =
                false;
        }


        [[nodiscard]]
        bool BuildProcVisualGeometry(
            PendingProc& proc,
            RE::Actor* target,
            const bool powerAttack,
            const std::size_t visualBaseSlot) noexcept
        {
            ResetProcVisualGeometry(
                proc
            );

            if (
                !target ||
                visualBaseSlot >=
                    kVisualSlotCount
            ) {
                return false;
            }

            RE::NiPoint3 center{};
            RE::NiPoint3 forwardAxis{};

            if (
                !ResolveTargetFrame(
                    target,
                    center,
                    forwardAxis
                )
            ) {
                return false;
            }

            TOAM::Combat::TargetFormation
                formation{};

            if (
                !TOAM::Combat::
                    BuildUpperHemisphereReference(
                        center,
                        forwardAxis,
                        kShellRadiusWorldUnits,
                        formation
                    ) ||
                !formation.valid ||
                formation.count !=
                    TOAM::Combat::
                        kUpperHemisphereSlotCount
            ) {
                return false;
            }

            auto& selectionState = GetState();
            std::uint16_t avoid = powerAttack ? selectionState.lastPowerMask : 0;
            if (!powerAttack) {
                for (const auto slot : selectionState.recentNormalSlots) {
                    if (slot < TOAM::Combat::kUpperHemisphereSlotCount) {
                        avoid |= std::uint16_t{1} << slot;
                    }
                }
            }
            std::array<std::size_t, TOAM::Combat::kUpperHemisphereSlotCount> chosen{};
            if (!TOAM::Combat::SelectDistributedUpperHemisphereSlots(
                    powerAttack ? kPowerVisualCount : 1, avoid, chosen)) {
                return false;
            }

            proc.targetCenter =
                formation.center;

            proc.visualBaseSlot =
                visualBaseSlot;

            if (powerAttack) {

                proc.visualCount =
                    kPowerVisualCount;

                if (
                    proc.visualBaseSlot +
                        proc.visualCount >
                    kVisualSlotCount
                ) {
                    ResetProcVisualGeometry(
                        proc
                    );
                    return false;
                }

                for (
                    std::size_t index = 0;
                    index <
                        kPowerVisualCount;
                    ++index
                ) {
                    const auto slot =
                        chosen[index];

                    if (
                        slot >=
                            formation.count
                    ) {
                        ResetProcVisualGeometry(
                            proc
                        );
                        return false;
                    }

                    proc.circleWorld[index] =
                        formation.points[slot];

                    if (
                        !IsFinitePoint(
                            proc.circleWorld[
                                index
                            ]
                        )
                    ) {
                        ResetProcVisualGeometry(
                            proc
                        );
                        return false;
                    }
                }
            }
            else {

                proc.visualCount =
                    1;

                const auto slot =
                    chosen[0];

                proc.circleWorld[0] =
                    formation.points[slot];

                if (
                    !IsFinitePoint(
                        proc.circleWorld[0]
                    )
                ) {
                    ResetProcVisualGeometry(
                        proc
                    );
                    return false;
                }
            }

            // Successful geometry only: keep other pending procs immutable.
            if (powerAttack) {
                selectionState.lastPowerMask = 0;
                for (std::size_t i = 0; i < kPowerVisualCount; ++i) {
                    selectionState.lastPowerMask |= std::uint16_t{1} << chosen[i];
                }
            } else {
                selectionState.recentNormalSlots[1] = selectionState.recentNormalSlots[0];
                selectionState.recentNormalSlots[0] = chosen[0];
            }
            proc.visualGeometryValid =
                true;

            return true;
        }


        [[nodiscard]]
        std::size_t SpawnProcCircles(
            const PendingProc& proc) noexcept
        {
            if (
                !proc.visualGeometryValid ||
                proc.visualCount == 0 ||
                proc.visualBaseSlot >=
                    kVisualSlotCount ||
                proc.visualBaseSlot +
                    proc.visualCount >
                    kVisualSlotCount
            ) {
                return 0;
            }

            RE::NiNode* parent{};
            RE::NiTransform rootInverse{};

            if (
                !ResolveVisualRoot(
                    parent,
                    rootInverse
                )
            ) {
                return 0;
            }

            auto& visual =
                GetVisualState();

            std::size_t spawned{};

            for (
                std::size_t index = 0;
                index <
                    proc.visualCount;
                ++index
            ) {
                const std::size_t rendererSlot =
                    proc.visualBaseSlot +
                    index;

                const RE::NiPoint3
                    worldPosition =
                        proc.circleWorld[
                            index
                        ];

                const RE::NiPoint3
                    inward =
                        proc.targetCenter -
                        worldPosition;

                VisualBasis basis{};

                if (
                    !BuildVisualBasis(
                        inward,
                        basis
                    )
                ) {
                    continue;
                }

                RE::NiTransform
                    worldTransform{};

                worldTransform.translate =
                    worldPosition;

                worldTransform.scale =
                    kCircleVisualScale;

                ApplyVisualBasis(
                    worldTransform,
                    basis
                );

                const RE::NiTransform
                    localTransform =
                        rootInverse *
                        worldTransform;

                if (
                    !IsFiniteTransform(
                        localTransform
                    )
                ) {
                    continue;
                }

                if (
                    visual.circles.
                        SpawnTransform(
                            rendererSlot,
                            parent,
                            TOAM::VFX::
                                AssetID::
                                    kCircleStatic,
                            localTransform,
                            true
                        )
                ) {
                    static_cast<void>(visual.circles.BeginGrowth(rendererSlot,0.12F));
                    ++spawned;
                }
            }

            return spawned;
        }


        void ReleaseProcCircles(
            const PendingProc& proc) noexcept
        {
            if (
                proc.visualCount == 0 ||
                proc.visualBaseSlot >=
                    kVisualSlotCount ||
                proc.visualBaseSlot +
                    proc.visualCount >
                    kVisualSlotCount
            ) {
                return;
            }

            auto& circles =
                GetVisualState().
                    circles;

            for (
                std::size_t index = 0;
                index <
                    proc.visualCount;
                ++index
            ) {
                static_cast<void>(
                    circles.Retire(
                        proc.visualBaseSlot + index, 0.60F
                    )
                );
            }
        }


        [[nodiscard]]
        bool ShowBeamVisual(
            TOAM::VFX::BeamRenderer& renderer,
            const RE::NiPoint3& worldOrigin,
            const RE::NiPoint3& worldEnd) noexcept
        {
            if (
                !IsFinitePoint(
                    worldOrigin
                ) ||
                !IsFinitePoint(
                    worldEnd
                )
            ) {
                return false;
            }

            RE::NiNode* parent{};
            RE::NiTransform rootInverse{};

            if (
                !ResolveVisualRoot(
                    parent,
                    rootInverse
                )
            ) {
                return false;
            }

            const RE::NiPoint3
                localOrigin =
                    rootInverse *
                    worldOrigin;

            const RE::NiPoint3
                localEnd =
                    rootInverse *
                    worldEnd;

            const RE::NiPoint3
                localVector =
                    localEnd -
                    localOrigin;

            const float
                localLengthSquared =
                    LengthSquared(
                        localVector
                    );

            if (
                !std::isfinite(
                    localLengthSquared
                ) ||
                localLengthSquared <=
                    kDirectionEpsilon
            ) {
                return false;
            }

            const float localLength =
                std::sqrt(
                    localLengthSquared
                );

            RE::NiPoint3
                localDirection{};

            if (
                !Normalize(
                    localVector,
                    localDirection
                )
            ) {
                return false;
            }

            const float localWidth =
                kBeamVisualWidth *
                std::fabs(
                    rootInverse.scale
                );

            if (
                !std::isfinite(
                    localWidth
                ) ||
                localWidth <=
                    0.0F
            ) {
                return false;
            }

            TOAM::VFX::BeamPose
                pose{};

            pose.origin =
                localOrigin;

            pose.direction =
                localDirection;

            pose.length =
                localLength;

            pose.widthScale =
                localWidth;

            pose.lifetimeSeconds =
                kBeamVisualLifetimeSeconds;

            pose.visible =
                true;

            pose.asset =
                TOAM::VFX::
                    AssetID::
                        kBeamWholeLens;

            const bool spawned=renderer.SetBeam(parent,pose);
            if(spawned) {renderer.Detach();}
            return spawned;
        }


        [[nodiscard]]
        std::size_t ShowProcBeams(
            const PendingProc& proc) noexcept
        {
            if (
                !proc.visualGeometryValid ||
                proc.visualCount == 0 ||
                proc.visualBaseSlot >=
                    kVisualSlotCount ||
                proc.visualBaseSlot +
                    proc.visualCount >
                    kVisualSlotCount
            ) {
                return 0;
            }

            auto& visual =
                GetVisualState();

            std::size_t spawned{};

            for (
                std::size_t index = 0;
                index <
                    proc.visualCount;
                ++index
            ) {
                const std::size_t rendererSlot =
                    proc.visualBaseSlot +
                    index;

                if (
                    ShowBeamVisual(
                        visual.beams[
                            rendererSlot
                        ],
                        proc.circleWorld[
                            index
                        ],
                        proc.targetCenter
                    )
                ) {
                    ++spawned;
                }
            }

            return spawned;
        }


        void ReleaseAllVisuals() noexcept
        {
            auto& visual =
                GetVisualState();

            visual.circles.
                ReleaseAll();

            for (
                auto& beam :
                visual.beams
            ) {
                beam.Release();
            }
        }


        void ClearPending() noexcept
        {
            auto& state =
                GetState();

            state.normal =
                {};

            state.power =
                {};
        }


        [[nodiscard]]
        bool AnyPending() noexcept
        {
            const auto& state =
                GetState();

            for (const auto& proc : state.power) {
                if (proc.active) {
                    return true;
                }
            }

            for (
                const auto& proc :
                state.normal
            ) {
                if (proc.active) {
                    return true;
                }
            }

            return false;
        }


        [[nodiscard]]
        PendingProc* FindNormalVacancy(
            std::size_t& resultIndex) noexcept
        {
            auto& state =
                GetState();

            resultIndex =
                kNormalPoolCapacity;

            for (
                std::size_t index = 0;
                index <
                    state.normal.size();
                ++index
            ) {
                auto& proc =
                    state.normal[
                        index
                    ];

                if (proc.active) {
                    continue;
                }

                resultIndex =
                    index;

                return
                    &proc;
            }

            return nullptr;
        }


        // CP026: display-only migration to the already-integrated engine ARTO owner.
        // Proc .15/.27, .095 scale, head offsets, toggle and paid damage stay unchanged.
        constexpr float kHaloScale{0.095F},kHaloBehindHead{26.0F},kHaloAboveHead{5.0F};
        constexpr float kHaloSpinRadiansPerSecond{0.65F};
        struct HaloState final {
            TOAM::VFX::CircleRenderer renderer{};
            RE::NiAVObject* lastRoot{};
            std::uint32_t cellFormID{};
            bool positioned{},headUnavailable{};
            float spinPhase{};
            std::array<RE::NiAVObject*,2> spinParts{};
            std::array<RE::NiMatrix3,2> spinBase{};
        };
        HaloState& GetHaloState() noexcept {static HaloState state{};return state;}
        void ReleaseHalo(const char* reason) noexcept {
            auto& state=GetHaloState();const bool active=state.renderer.IsActive(0);
            state.renderer.ReleaseAll();state.lastRoot=nullptr;state.cellFormID=0;
            state.positioned=false;state.headUnavailable=false;state.spinPhase=0;state.spinParts.fill(nullptr);
            if(active)logger::info("[TOAM][HALO] RETIRE | Reason={}",reason);
        }
        void UpdateHaloInternal(float spinDelta=0.0F) noexcept {
            if(!GetState().enabled){ReleaseHalo("TOGGLE_OFF");return;}
            auto* player=RE::PlayerCharacter::GetSingleton();
            if(!player||player->IsDead()){ReleaseHalo("PLAYER_UNAVAILABLE");return;}
            auto* cell=player->GetParentCell();
            auto* skeleton=player->Get3D(false);
            auto* head=skeleton?skeleton->GetObjectByName(RE::BSFixedString{"NPC Head [Head]"}):nullptr;
            if(!cell){ReleaseHalo("CELL_UNAVAILABLE");return;}
            auto& halo=GetHaloState();
            if(halo.cellFormID&&halo.cellFormID!=cell->GetFormID())ReleaseHalo("CELL_CHANGED");
            RE::NiPoint3 fallbackHeadPosition{};
            bool fallbackReady{};
            if (!head) {
                TOAM::Combat::ActorBounds bounds{};
                if (TOAM::Combat::ResolveActorBounds(player, bounds) &&
                    TOAM::Combat::IsUsableActorBounds(bounds)) {
                    fallbackHeadPosition = bounds.center;
                    fallbackHeadPosition.z += bounds.radius;
                    fallbackReady = std::isfinite(fallbackHeadPosition.z);
                }
            }
            if (!head && !fallbackReady) {
                if (!halo.headUnavailable) {
                    logger::warn("[TOAM][HALO] HEAD_WAIT | Existing={} | Positioned={}",
                        halo.renderer.IsActive(0), halo.positioned);
                    halo.headUnavailable = true;
                }
                if (halo.renderer.IsActive(0)&&halo.positioned) {
                    static_cast<void>(halo.renderer.SetVisible(0,true));
                }
                return;
            }
            if (!head && fallbackReady) {
                if (!halo.headUnavailable) {
                    logger::info("[TOAM][HALO] BOUNDS_FALLBACK | Source=ActorBoundTop");
                    halo.headUnavailable = true;
                }
            } else if (halo.headUnavailable) {
                logger::info("[TOAM][HALO] HEAD_RECOVERED");
                halo.headUnavailable = false;
                halo.positioned = false;
            }

            const float yaw = player->GetAngleZ();
            const RE::NiPoint3 headPosition = head ? head->world.translate : fallbackHeadPosition;
            const RE::NiPoint3 wanted{
                headPosition.x - std::sin(yaw) * kHaloBehindHead,
                headPosition.y - std::cos(yaw) * kHaloBehindHead,
                headPosition.z + kHaloAboveHead
            };

            if (!std::isfinite(yaw) ||
                !std::isfinite(wanted.x) ||
                !std::isfinite(wanted.y) ||
                !std::isfinite(wanted.z)) {
                ReleaseHalo("NONFINITE_POSE");
                return;
            }


            auto& state=GetHaloState();
            auto* parent=RE::Main::WorldRootNode();
            if(!parent||!std::isfinite(parent->world.scale)||std::abs(parent->world.scale)<.001F)return;
            RE::NiTransform world{};world.translate=wanted;world.scale=kHaloScale;
            // XZ artwork normal is +Y; actor yaw alone controls the vertical halo plane.
            const float c=std::cos(yaw),sn=std::sin(yaw);
            world.rotate.entry[0][0]=c;world.rotate.entry[0][1]=sn;world.rotate.entry[0][2]=0;
            world.rotate.entry[1][0]=-sn;world.rotate.entry[1][1]=c;world.rotate.entry[1][2]=0;
            world.rotate.entry[2][0]=0;world.rotate.entry[2][1]=0;world.rotate.entry[2][2]=1;
            const auto local=parent->world.Invert()*world;
            state.renderer.SetStyle(TOAM::VFX::EngineCircle::Style::Halo);
            if(!state.renderer.IsActive(0)){
                state.lastRoot=nullptr;state.spinParts.fill(nullptr);
                if(!state.renderer.SpawnTransform(0,parent,TOAM::VFX::AssetID::kCircleOuterBandStatic,local,true))return;
                logger::info("[TOAM][HALO] REQUEST | Scale={} | EngineARTO=true | Pixels=UNVERIFIED",kHaloScale);
            }else static_cast<void>(state.renderer.SetTransform(0,local,true));
            state.cellFormID=cell->GetFormID();state.positioned=true;
            auto* root=state.renderer.Node(0);if(!root)return;
            if(root!=state.lastRoot){
                state.lastRoot=root;
                static const std::array<RE::BSFixedString,2> names{{"CC_Outer","CC_Band"}};
                for(std::size_t i=0;i<2;++i){state.spinParts[i]=root->GetObjectByName(names[i]);if(state.spinParts[i])state.spinBase[i]=state.spinParts[i]->local.rotate;}
                logger::info("[TOAM][HALO] FIRST_POSE | SlowSpin={} | Pixels=UNVERIFIED",state.spinParts[0]&&state.spinParts[1]);
            }
            if(std::isfinite(spinDelta)&&spinDelta>0)state.spinPhase=std::fmod(state.spinPhase+(spinDelta<.1F?spinDelta:.1F)*kHaloSpinRadiansPerSecond,6.28318530718F);
            RE::NiMatrix3 spin;spin.MakeYRotation(state.spinPhase);
            for(std::size_t i=0;i<2;++i)if(state.spinParts[i])state.spinParts[i]->local.rotate=state.spinBase[i]*spin;
            RE::NiUpdateData update{};root->Update(update);
        }
        void ToggleEnabled() noexcept
        {
            auto& state =
                GetState();

            state.enabled =
                !state.enabled;
            if (state.enabled) {
                UpdateHaloInternal();
            } else {
                ReleaseHalo("TOGGLE_OFF");
            }

            // Already-paid pending procs intentionally finish after toggle OFF.
            logger::info(
                "[TOAM][S08] Temperance toggle | Enabled={} | PendingPreserved={}",
                state.enabled,
                AnyPending());
        }


        void ResolvePending(
            PendingProc& proc,
            RE::PlayerCharacter* player,
            const float deltaSeconds,
            const float damagePerBeam,
            const char* branch) noexcept
        {
            if (!proc.active || !player ||
                !std::isfinite(deltaSeconds) || deltaSeconds <= 0.0F) {
                return;
            }

            proc.elapsedSeconds += deltaSeconds;
            if (!proc.circlesPresented &&
                proc.elapsedSeconds >= kCircleDelaySeconds) {
                proc.circlesPresented = true;
                if (proc.visualGeometryValid) {
                    const auto count = SpawnProcCircles(proc);
                    TOAM::Audio::TemperanceProc(proc.targetCenter);
                    logger::info("[TOAM][S08] CIRCLES_DELAYED | Branch={} | Count={} | Time={:.3f}",
                        branch, count, proc.elapsedSeconds);
                }
                // Keep at least one actual rendered frame between circle and beam.
                return;
            }
            if (proc.elapsedSeconds < kProcDelaySeconds) {
                return;
            }

            std::size_t resolvedBeams{};
            std::size_t intersectedActors{};
            std::size_t appliedHits{};

            if (proc.visualGeometryValid &&
                proc.visualCount > 0 &&
                proc.visualCount <= kPowerVisualCount &&
                std::isfinite(damagePerBeam) && damagePerBeam > 0.0F) {
                for (std::size_t beamIndex = 0;
                     beamIndex < proc.visualCount; ++beamIndex) {
                    const auto origin = proc.circleWorld[beamIndex];
                    const auto delta = proc.targetCenter - origin;
                    RE::NiPoint3 direction{};
                    if (!Normalize(delta, direction)) {
                        continue;
                    }
                    const float distanceSquared = LengthSquared(delta);
                    if (!std::isfinite(distanceSquared) ||
                        distanceSquared <= kDirectionEpsilon) {
                        continue;
                    }

                    TOAM::Combat::BeamSegmentQuery query{};
                    query.origin = origin;
                    query.direction = direction;
                    query.maxDistance = std::sqrt(distanceSquared);
                    query.radius = kBeamCollisionRadiusWorldUnits;

                    TOAM::Combat::BeamCollisionResult collision{};
                    if (!TOAM::Combat::ResolveBeamCollision(
                            query, player, collision) || !collision.resolved) {
                        continue;
                    }
                    ++resolvedBeams;

                    // Deduplicate only within this beam, without a new allocation.
                    for (std::size_t hitIndex = 0;
                         hitIndex < collision.actorHits.size(); ++hitIndex) {
                        auto actor = collision.actorHits[hitIndex].actor.get();
                        // The proc remains paid and its visuals complete, but
                        // later dead actors receive no additional damage.
                        if (!actor || actor->IsDead()) {
                            continue;
                        }
                        const auto actorId = actor->GetFormID();
                        bool duplicate{};
                        for (std::size_t earlier = 0;
                             earlier < hitIndex; ++earlier) {
                            auto prior = collision.actorHits[earlier].actor.get();
                            if (prior && prior->GetFormID() == actorId) {
                                duplicate = true;
                                break;
                            }
                        }
                        if (duplicate) {
                            continue;
                        }
                        ++intersectedActors;
                        if (TOAM::Combat::ApplyFixedHealthDamage(
                                player, actor.get(), damagePerBeam,
                                TOAM::Combat::FirewallOrigin::kTemperanceAssist)) {
                            ++appliedHits;
                        }
                    }
                }
            }

            // Gameplay and presentation are separately accounted for.
            ReleaseProcCircles(proc);
            const std::size_t visualBeams = ShowProcBeams(proc);
            logger::info(
                "[TOAM][S08B2B] Proc resolved | Branch={} | DamagePerBeam={:.3f} | ResolvedBeams={} | Intersections={} | Applied={} | VisualBeams={}",
                branch, damagePerBeam, resolvedBeams, intersectedActors,
                appliedHits, visualBeams);
            proc = {};
        }

        // S08-B1 diagnostic only: these are last-observed swing boundaries,
        // NOT an authoritative per-animation StrikeID.
        struct StrikeTraceLane final
        {
            std::uint64_t inputGeneration{};
            std::uint64_t swingSerial{};
            RE::FormID swingWeapon{};
            std::uint32_t contacts{};
            bool held{};
        };

        struct StrikeTraceState final
        {
            StrikeTraceLane left{};
            StrikeTraceLane right{};
            std::uint64_t nextSerial{};
            std::uint32_t logged{};
        };

        StrikeTraceState& GetStrikeTraceState() noexcept
        {
            static StrikeTraceState state{};
            return state;
        }

        bool AllowStrikeTraceLog() noexcept
        {
            auto& state = GetStrikeTraceState();
            if (state.logged >= 512) {
                if (state.logged == 512) {
                    logger::warn("[TOAM][S08TRACE] TRACE_LIMIT_REACHED | Limit=512");
                    ++state.logged;
                }
                return false;
            }
            ++state.logged;
            return true;
        }

        class StrikeInputSink final :
            public RE::BSTEventSink<RE::InputEvent*>
        {
        public:
            RE::BSEventNotifyControl ProcessEvent(
                RE::InputEvent* const* events,
                RE::BSTEventSource<RE::InputEvent*>*) override
            {
                if (!events || !*events ||
                    !TOAM::Core::Runtime::IsGameReady()) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                auto* names = RE::UserEvents::GetSingleton();
                if (!names) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                for (auto* input = *events; input; input = input->next) {
                    auto* button = input->AsButtonEvent();
                    if (!button) {
                        continue;
                    }
                    const auto& name = button->QUserEvent();
                    const bool left = name == names->leftAttack;
                    const bool right = name == names->rightAttack;
                    if (!left && !right) {
                        continue;
                    }
                    auto& lane = left ? GetStrikeTraceState().left :
                                        GetStrikeTraceState().right;
                    if (button->IsDown() && !lane.held) {
                        lane.held = true;
                        ++lane.inputGeneration;
                        if (AllowStrikeTraceLog()) {
                            logger::info("[TOAM][S08TRACE] DOWN Hand={} InputGen={}",
                                left ? "L" : "R", lane.inputGeneration);
                        }
                    } else if (button->IsUp() && lane.held) {
                        lane.held = false;
                        if (AllowStrikeTraceLog()) {
                            logger::info("[TOAM][S08TRACE] UP Hand={} InputGen={}",
                                left ? "L" : "R", lane.inputGeneration);
                        }
                    }
                }
                return RE::BSEventNotifyControl::kContinue;
            }
        };

        class StrikeActionSink final :
            public RE::BSTEventSink<SKSE::ActionEvent>
        {
        public:
            RE::BSEventNotifyControl ProcessEvent(
                const SKSE::ActionEvent* event,
                RE::BSTEventSource<SKSE::ActionEvent>*) override
            {
                if (!event || !TOAM::Core::Runtime::IsGameReady() ||
                    event->actor != RE::PlayerCharacter::GetSingleton() ||
                    event->type.get() != SKSE::ActionEvent::Type::kWeaponSwing) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                const auto slot = event->slot.get();
                const bool left = slot == SKSE::ActionEvent::Slot::kLeft;
                const bool right = slot == SKSE::ActionEvent::Slot::kRight;
                if (!left && !right) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                auto* weapon = event->sourceForm ?
                    event->sourceForm->As<RE::TESObjectWEAP>() : nullptr;
                if (!weapon || !weapon->IsMelee()) {
                    return RE::BSEventNotifyControl::kContinue;
                }
                auto& trace = GetStrikeTraceState();
                auto& lane = left ? trace.left : trace.right;
                lane.swingSerial = ++trace.nextSerial;
                lane.contacts = 0;
                lane.swingWeapon = weapon->GetFormID();
                if (AllowStrikeTraceLog()) {
                    logger::info("[TOAM][S08TRACE] SWING Hand={} Serial={} InputGen={} Weapon={:08X}",
                        left ? "L" : "R", lane.swingSerial,
                        lane.inputGeneration, lane.swingWeapon);
                }
                return RE::BSEventNotifyControl::kContinue;
            }
        };

        StrikeInputSink& GetStrikeInputSink() noexcept
        {
            static StrikeInputSink sink{};
            return sink;
        }

        StrikeActionSink& GetStrikeActionSink() noexcept
        {
            static StrikeActionSink sink{};
            return sink;
        }

        bool g_traceInputRegistered{};
        bool g_traceActionRegistered{};

        void OnWeaponHit(
            RE::Actor* target,
            RE::TESObjectWEAP* weapon,
            const RE::HitData& hitData,
            const bool powerAttack) noexcept
        {
            auto& state =
                GetState();

            // Trace all genuine weapon callbacks even when Temperance is OFF.
            if (TOAM::Core::Runtime::IsGameReady() && target && weapon) {
                const bool left = hitData.flags.any(
                    RE::HitData::Flag::kLeftHand);
                auto& lane = left ? GetStrikeTraceState().left :
                                    GetStrikeTraceState().right;
                ++lane.contacts;
                if (AllowStrikeTraceLog()) {
                    logger::info("[TOAM][S08TRACE] HIT Hand={} LastSwing={} Contact={} InputGen={} SwingWeapon={:08X} HitWeapon={:08X} Target={:08X} Power={} Melee={} Bash={} TimedBash={}",
                        left ? "L" : "R", lane.swingSerial, lane.contacts,
                        lane.inputGeneration, lane.swingWeapon,
                        weapon->GetFormID(), target->GetFormID(), powerAttack,
                        hitData.flags.any(RE::HitData::Flag::kMeleeAttack),
                        hitData.flags.any(RE::HitData::Flag::kBash),
                        hitData.flags.any(RE::HitData::Flag::kTimedBash));
                }
            }

            if (!state.enabled) {
                return;
            }
            if (TOAM::Combat::IsTriggerSuppressed()) {
                if (AllowStrikeTraceLog()) {
                    logger::info("[TOAM][S08] Proc skipped | Reason=TRIGGER_FIREWALL");
                }
                return;
            }

            auto* player =
                RE::PlayerCharacter::
                    GetSingleton();

            if (
                !player ||
                !target ||
                !weapon
            ) {
                return;
            }

            // SE weapon callback is observed before the vanilla hit path.
            // A corpse already dead before contact must not buy a new proc.
            // A killing blow remains eligible at the pre-vanilla snapshot.
            if (target->IsDead()) {
                if (AllowStrikeTraceLog()) {
                    logger::info("[TOAM][S08] Proc skipped | Reason=TARGET_ALREADY_DEAD | Target={:08X}",
                        target->GetFormID());
                }
                return;
            }

            if (
                hitData.flags.any(
                    RE::HitData::Flag::
                        kBash
                ) ||
                hitData.flags.any(
                    RE::HitData::Flag::
                        kTimedBash
                )
            ) {
                if (AllowStrikeTraceLog()) {
                    logger::info("[TOAM][S08] Proc skipped | Reason=BASH_OR_TIMED_BASH | Bash={} | TimedBash={}",
                        hitData.flags.any(RE::HitData::Flag::kBash),
                        hitData.flags.any(RE::HitData::Flag::kTimedBash));
                }
                return;
            }

            if (!weapon->IsMelee()) {
                if (AllowStrikeTraceLog()) {
                    logger::info("[TOAM][S08] Proc skipped | Reason=NON_MELEE");
                }
                return;
            }

            PendingProc* proc =
                nullptr;

            float cost =
                0.0F;

            std::size_t visualBaseSlot =
                kInvalidVisualSlot;

            if (powerAttack) {
                std::size_t freeIndex = kPowerPoolCapacity;
                for (std::size_t index = 0;
                     index < state.power.size(); ++index) {
                    if (!state.power[index].active) {
                        freeIndex = index;
                        break;
                    }
                }
                if (freeIndex == kPowerPoolCapacity) {
                    logger::info(
                        "[TOAM][S08B2B] Proc skipped | Branch=POWER | Reason=POWER_POOL_FULL");
                    return;
                }
                proc = &state.power[freeIndex];
                cost = kPowerCost;
                visualBaseSlot = kPowerVisualBaseSlot +
                    freeIndex * kPowerVisualCount;
            }
            else {

                std::size_t normalIndex{};

                proc =
                    FindNormalVacancy(
                        normalIndex
                    );

                if (!proc) {
                    logger::info(
                        "[TOAM][S08] Proc skipped | Branch=NORMAL | Reason=NORMAL_POOL_FULL");

                    return;
                }

                cost =
                    kNormalCost;

                visualBaseSlot =
                    normalIndex;
            }

            const auto spend =
                TOAM::Core::
                    TrySpendMagicka(
                        player,
                        cost
                    );

            if (!spend.paid) {

                logger::info(
                    "[TOAM][S08] Proc skipped | Branch={} | Reason=INSUFFICIENT_MAGICKA | Required={:.3f} | Before={:.3f} | After={:.3f} | ToggleRemains={}",
                    powerAttack ?
                        "POWER" :
                        "NORMAL",
                    cost,
                    spend.before,
                    spend.after,
                    state.enabled);

                return;
            }

            proc->target =
                target->GetHandle();

            proc->elapsedSeconds =
                0.0F;

            proc->active =
                true;

            const bool
                visualGeometryReady =
                    BuildProcVisualGeometry(
                        *proc,
                        target,
                        powerAttack,
                        visualBaseSlot
                    );

            logger::info(
                "[TOAM][S08] Proc committed | Branch={} | Target={:08X} | Cost={:.3f} | Magicka={:.3f}->{:.3f} | CircleDelay={:.3f} | BeamDelay={:.3f} | VisualGeometry={}",
                powerAttack ? "POWER" : "NORMAL", target->GetFormID(),
                cost, spend.before, spend.after, kCircleDelaySeconds,
                kProcDelaySeconds, visualGeometryReady);
        }


        class SpellCastSink final :
            public RE::BSTEventSink<
                RE::TESSpellCastEvent>
        {
        public:
            RE::BSEventNotifyControl
            ProcessEvent(
                const RE::TESSpellCastEvent*
                    event,
                RE::BSTEventSource<
                    RE::TESSpellCastEvent>*) override
            {
                if (
                    !event ||
                    !TOAM::Core::Runtime::
                        IsGameReady() ||
                    !TOAM::Core::DataCache::
                        IsReady()
                ) {
                    return
                        RE::BSEventNotifyControl::
                            kContinue;
                }

                auto* player =
                    RE::PlayerCharacter::
                        GetSingleton();

                if (!player) {
                    return
                        RE::BSEventNotifyControl::
                            kContinue;
                }

                if (
                    event->object.get() !=
                        player
                ) {
                    return
                        RE::BSEventNotifyControl::
                            kContinue;
                }

                auto* temperance =
                    TOAM::Core::DataCache::
                        GetTemperance();

                if (
                    !temperance ||
                    event->spell !=
                        temperance->
                            GetFormID()
                ) {
                    return
                        RE::BSEventNotifyControl::
                            kContinue;
                }

                ToggleEnabled();

                return
                    RE::BSEventNotifyControl::
                        kContinue;
            }
        };


        [[nodiscard]]
        SpellCastSink&
        GetSpellCastSink() noexcept
        {
            static SpellCastSink
                sink{};

            return sink;
        }
    }


    void Install() noexcept
    {
        auto& state =
            GetState();

        if (!g_traceActionRegistered) {
            if (auto* source = SKSE::GetActionEventSource()) {
                source->AddEventSink(&GetStrikeActionSink());
                g_traceActionRegistered = true;
            }
        }
        if (!g_traceInputRegistered) {
            if (auto* source = RE::BSInputDeviceManager::GetSingleton()) {
                source->AddEventSink(&GetStrikeInputSink());
                g_traceInputRegistered = true;
            }
        }
        logger::info("[TOAM][S08TRACE] Sinks | Action={} Input={}",
            g_traceActionRegistered, g_traceInputRegistered);

        if (
            !state.spellSinkInstalled
        ) {
            auto* holder =
                RE::ScriptEventSourceHolder::
                    GetSingleton();

            if (holder) {

                holder->AddEventSink(
                    &GetSpellCastSink()
                );

                state.spellSinkInstalled =
                    true;
            }
            else {

                logger::error(
                    "[TOAM][S08] ScriptEventSourceHolder unavailable");
            }
        }

        TOAM::Combat::WeaponHitSource::
            SetCallback(
                &OnWeaponHit
            );

        if (
            !TOAM::Combat::WeaponHitSource::
                IsInstalled()
        ) {
            try {
                TOAM::Combat::WeaponHitSource::
                    Install();
            }
            catch (...) {
                logger::critical(
                    "[TOAM][S08] WeaponHitSource installation threw");
            }
        }

        logger::info(
            "[TOAM][S08] Runtime install | SpellSink={} | WeaponHitSource={}",
            state.spellSinkInstalled,
            TOAM::Combat::WeaponHitSource::
                IsInstalled());
    }


    void ResetAllState() noexcept
    {
        auto& state =
            GetState();

        const bool hadState =
            state.enabled ||
            AnyPending();

        ReleaseAllVisuals();
        ReleaseHalo("RESET_ALL_STATE");

        GetStrikeTraceState() = {};

        state.enabled =
            false;

        ClearPending();
        state.recentNormalSlots = {{
            TOAM::Combat::kUpperHemisphereSlotCount,
            TOAM::Combat::kUpperHemisphereSlotCount
        }};
        state.lastPowerMask = 0;

        if (hadState) {
            logger::info(
                "[TOAM][S08] Runtime state reset");
        }
    }


    void UpdateHalo() noexcept
    {
        UpdateHaloInternal(RE::Main::QFrameAnimTime());
    }

    bool IsInstalled() noexcept
    {
        return
            GetState().
                spellSinkInstalled &&
            TOAM::Combat::WeaponHitSource::
                IsInstalled();
    }


    bool IsEnabled() noexcept
    {
        return
            GetState().
                enabled;
    }


    bool HasPending() noexcept
    {
        return
            AnyPending();
    }


    void UpdatePending(
        const float deltaSeconds) noexcept
    {
        if (
            !AnyPending() ||
            !std::isfinite(
                deltaSeconds
            ) ||
            deltaSeconds <=
                0.0F
        ) {
            return;
        }

        auto* player =
            RE::PlayerCharacter::
                GetSingleton();

        if (!player) {

            ReleaseAllVisuals();
            ClearPending();

            return;
        }

        auto& state =
            GetState();

        for (
            auto& proc :
            state.normal
        ) {
            ResolvePending(
                proc,
                player,
                deltaSeconds,
                kNormalDamage,
                "NORMAL"
            );
        }

        for (auto& proc : state.power) {
            ResolvePending(
                proc, player, deltaSeconds, kPowerBeamDamage, "POWER");
        }
    }
}