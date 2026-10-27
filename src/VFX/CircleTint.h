#pragma once
#include "Core/PresentationSettings.h"
#include <memory>
namespace RE {class NiAVObject;class ModelReferenceEffect;}
namespace TOAM::VFX::CircleTint {
struct Lease;
struct State { Presentation::Color color{Presentation::Color::Default};RE::NiAVObject* attemptedRoot{};std::shared_ptr<Lease> lease{}; };
void Prepare() noexcept;
// One bounded attempt per prepared effect/root. Known pointers of that effect are counted exactly; unknown sharing rejects.
void Apply(State&,RE::ModelReferenceEffect*) noexcept;
}
