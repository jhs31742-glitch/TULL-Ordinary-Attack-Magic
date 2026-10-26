#include "VFX/CircleRenderer.h"
namespace TOAM::VFX {
CircleRenderer::~CircleRenderer(){ReleaseAll();}
bool CircleRenderer::IsCircleAsset(AssetID a) noexcept {
 return a==AssetID::kCircleStatic||a==AssetID::kCircleCoreStatic||a==AssetID::kCircleAnimated||
 a==AssetID::kCircleCoreAnimated||a==AssetID::kCircleSmall||a==AssetID::kCircleOuterBandStatic;
}
RE::NiTransform CircleRenderer::MakeTransform(const CirclePose& p) noexcept {
 RE::NiTransform t{};t.translate=p.translation;t.rotate.SetEulerAnglesXYZ(p.rotationRadians);t.scale=p.scale>0?p.scale:0.001F;return t;
}
bool CircleRenderer::Spawn(std::size_t slot,RE::NiNode* parent,AssetID asset,const CirclePose& p) noexcept {
 return SpawnTransform(slot,parent,asset,MakeTransform(p),p.visible);
}
bool CircleRenderer::SpawnTransform(std::size_t slot,RE::NiNode* parent,AssetID asset,const RE::NiTransform& t,bool visible) noexcept {
 if(slot>=kCapacity||!parent||!IsCircleAsset(asset))return false;
 static_cast<void>(Release(slot));
 auto style=_style;
 if(style==EngineCircle::Style::SunFull&&asset==AssetID::kCircleOuterBandStatic)style=EngineCircle::Style::SunOuter;
 auto handle=EngineCircle::Spawn(parent,t,visible,style,_color);
 if(!handle.IsValid())return false;
 _handles[slot]=handle;_parents[slot]=RE::NiPointer<RE::NiNode>(parent);return true;
}
bool CircleRenderer::SetPose(std::size_t slot,const CirclePose& p) noexcept {return SetTransform(slot,MakeTransform(p),p.visible);}
bool CircleRenderer::SetTransform(std::size_t slot,const RE::NiTransform& t,bool visible) noexcept {
 if(slot>=kCapacity||!_parents[slot])return false;
 return EngineCircle::Set(_handles[slot],_parents[slot]->world*t,visible);
}
bool CircleRenderer::SetVisible(std::size_t slot,bool visible) noexcept {
 return slot<kCapacity&&EngineCircle::Visible(_handles[slot],visible);
}
bool CircleRenderer::Release(std::size_t slot) noexcept {
 if(slot>=kCapacity)return false;const bool result=EngineCircle::Release(_handles[slot]);_handles[slot]={};_parents[slot].reset();return result;
}
void CircleRenderer::ReleaseAll() noexcept {for(std::size_t i=0;i<kCapacity;++i)static_cast<void>(Release(i));}
bool CircleRenderer::BeginGrowth(std::size_t slot,float seconds) noexcept {return slot<kCapacity&&EngineCircle::Growth(_handles[slot],seconds);}
bool CircleRenderer::Retire(std::size_t slot,float seconds) noexcept {
 if(slot>=kCapacity)return false;bool result=EngineCircle::Retire(_handles[slot],seconds);
 if(result){_handles[slot]={};_parents[slot].reset();}return result;
}
void CircleRenderer::RetireAll(float seconds) noexcept {for(std::size_t i=0;i<kCapacity;++i)static_cast<void>(Retire(i,seconds));}
bool CircleRenderer::IsActive(std::size_t slot) const noexcept {return slot<kCapacity&&EngineCircle::Alive(_handles[slot]);}
RE::NiAVObject* CircleRenderer::Node(std::size_t slot) noexcept {return slot<kCapacity?EngineCircle::Node(_handles[slot]):nullptr;}
std::size_t CircleRenderer::ActiveCount() const noexcept {std::size_t count=0;for(auto h:_handles)if(EngineCircle::Alive(h))++count;return count;}
}