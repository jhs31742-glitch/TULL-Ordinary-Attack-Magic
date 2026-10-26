#pragma once
#include "VFX/TransientVFXManager.h"
namespace TOAM::VFX::EngineCircle {
enum class Style { Original, SunFull, SunOuter, Dome, Halo };
VFXHandle Spawn(RE::NiNode* parent,const RE::NiTransform& local,bool visible,Style style) noexcept;
bool Set(VFXHandle handle,const RE::NiTransform& local,bool visible) noexcept;
bool Visible(VFXHandle handle,bool visible) noexcept;
bool Growth(VFXHandle handle,float seconds) noexcept;
bool Retire(VFXHandle handle,float seconds) noexcept;
bool Release(VFXHandle handle) noexcept;
bool Alive(VFXHandle handle) noexcept;
bool Ready(VFXHandle handle) noexcept;
RE::NiAVObject* Node(VFXHandle handle) noexcept;
void Update(float dt) noexcept;
void Reset() noexcept;
}