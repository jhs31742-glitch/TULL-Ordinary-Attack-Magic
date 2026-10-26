#pragma once
#include <charconv>
#include <cmath>
#include <optional>
#include <string_view>
namespace TOAM::Core::DamagePolicy {
inline constexpr float kDefault=1.0F,kMinimum=0.01F,kMaximum=100.0F;
[[nodiscard]] inline bool Valid(float value) noexcept {
    return std::isfinite(value)&&value>=kMinimum&&value<=kMaximum;
}
[[nodiscard]] inline std::optional<float> Parse(std::string_view text) noexcept {
    while(!text.empty()&&(text.front()==' '||text.front()=='\t'))text.remove_prefix(1);
    while(!text.empty()&&(text.back()==' '||text.back()=='\t'))text.remove_suffix(1);
    if(text.empty())return std::nullopt;
    float value{};const auto result=std::from_chars(text.data(),text.data()+text.size(),value);
    if(result.ec!=std::errc{}||result.ptr!=text.data()+text.size()||!Valid(value))return std::nullopt;
    return value;
}
}
