#pragma once
#include "Core/PresentationPolicy.h"
namespace RE {class BGSSoundDescriptorForm;}
namespace TOAM::Presentation {
Values Get() noexcept;
bool Reload() noexcept;
bool Save(const Values&) noexcept;
Color Snapshot(Spell) noexcept;
struct AudioChoice {unsigned id;float volume;};
AudioChoice Resolve(Event,float original) noexcept;
RE::BGSSoundDescriptorForm* Descriptor(unsigned id) noexcept;
void DataLoaded() noexcept;
float CameraStrength(float original) noexcept;
}
