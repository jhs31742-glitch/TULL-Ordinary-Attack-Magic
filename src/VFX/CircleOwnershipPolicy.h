#pragma once
#include <cstdint>
namespace TOAM::VFX::CircleOwnership {
constexpr std::uint32_t Expected(bool parent, bool attachedArt, bool currentRoot) noexcept {
 return 1U + unsigned(parent) + unsigned(attachedArt) + unsigned(currentRoot);
}
constexpr bool IsPrivate(std::uint32_t actual, unsigned parentEdges, bool hasParent,
 bool attachedArt, bool currentRoot) noexcept {
 return parentEdges == unsigned(hasParent) && actual == Expected(hasParent,attachedArt,currentRoot);
}
}
