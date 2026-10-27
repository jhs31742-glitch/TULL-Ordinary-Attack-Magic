#pragma once
#include <atomic>
#include <cstdint>

namespace TOAM::Core::ExclusiveCast {
enum class Owner : std::uint8_t { None, HermitLeft, HermitRight, World };
inline std::atomic<Owner> owner{Owner::None};

inline Owner Current() noexcept { return owner.load(std::memory_order_acquire); }
inline bool Allows(Owner requested) noexcept {
 const auto existing=Current();return existing==Owner::None||existing==requested;
}
inline bool Acquire(Owner requested) noexcept {
 if(requested==Owner::None)return false;
 Owner expected=Owner::None;
 return owner.compare_exchange_strong(expected,requested,std::memory_order_acq_rel)||expected==requested;
}
inline void Release(Owner requested) noexcept {
 if(requested==Owner::None)return;
 auto expected=requested;
 owner.compare_exchange_strong(expected,Owner::None,std::memory_order_acq_rel);
}
inline Owner HandOwner(int side) noexcept {
 return side==0?Owner::HermitLeft:Owner::HermitRight;
}
}
