#pragma once
#include <cstdint>
namespace TOAM::Compatibility {
// Exact native-audited runtime allowlist. Build support is not runtime QA.
constexpr bool Supports(std::uint16_t major, std::uint16_t minor,
                        std::uint16_t patch, std::uint16_t build) noexcept {
    return major == 1 && build == 0 &&
        ((minor == 5 && patch == 97) || (minor == 6 && patch == 1170));
}
template <class Version>
constexpr bool Supports(const Version& version) noexcept {
    return Supports(version.major(), version.minor(), version.patch(), version.build());
}
}
