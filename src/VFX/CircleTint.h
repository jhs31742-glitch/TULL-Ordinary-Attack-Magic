#pragma once
#include "Core/PresentationSettings.h"
namespace RE {class NiAVObject;}
namespace TOAM::VFX::CircleTint {
struct State { Presentation::Color color{Presentation::Color::Default};RE::NiAVObject* attemptedRoot{}; };
void Prepare() noexcept;
// One attempt per prepared instance/root. Rejected instances retain original appearance.
void Apply(State&,RE::NiAVObject*) noexcept;
}
