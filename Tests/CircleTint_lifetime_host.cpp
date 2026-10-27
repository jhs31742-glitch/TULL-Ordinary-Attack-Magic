// Executes the actual candidate CircleTint.cpp against an intrusive-reference
// scene test double. Native allocation/rendering are deliberately not invoked.
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <iostream>
namespace RE {
struct NiObject;struct NiAVObject;struct NiNode;struct BSGeometry;struct NiTimeController;
inline int live=0,cloneCalls=0,setupFail=-1;inline bool cloneShared=false;
struct NiRTTI {const char* name;const char* GetName() const{return name;}};
struct BSFixedString {std::string s;BSFixedString()=default;BSFixedString(const char* a):s(a){} const char* c_str()const{return s.c_str();}bool operator==(const BSFixedString&)const=default;};
template<class T> struct NiPointer {
 T* p{};NiPointer()=default;NiPointer(T* q):p(q){inc();}NiPointer(const NiPointer& q):p(q.p){inc();}NiPointer(NiPointer&& q)noexcept:p(q.p){q.p=nullptr;}
 ~NiPointer(){dec();}void inc();void dec();void reset(T* q=nullptr){if(q==p)return;NiPointer n(q);std::swap(p,n.p);}
 NiPointer& operator=(const NiPointer& q){reset(q.p);return *this;}NiPointer& operator=(NiPointer&& q)noexcept{if(this!=&q){dec();p=q.p;q.p=nullptr;}return *this;}
 NiPointer& operator=(T* q){reset(q);return *this;}T* get()const{return p;}T* operator->()const{return p;}explicit operator bool()const{return p!=nullptr;}
};
struct NiObject {
 unsigned refs{};BSFixedString name;NiPointer<NiTimeController> controllers{};
 NiObject(){++live;}virtual ~NiObject(){--live;}void IncRef(){++refs;}void DecRef(){assert(refs);if(!--refs)delete this;}unsigned GetRefCount(){return refs;}
 virtual const NiRTTI* GetRTTI()const{static NiRTTI t{"NiObject"};return &t;}virtual NiObject* Clone(){++cloneCalls;return this;}
};
struct NiTimeController:NiObject {NiObject* target{};unsigned flags{};float frequency=1,phase{},loKeyTime{},hiKeyTime{};NiPointer<NiTimeController> next{};NiTimeController* GetNext(){return next.get();}};
template<class T>void NiPointer<T>::inc(){if(p)p->IncRef();}template<class T>void NiPointer<T>::dec(){if(p)p->DecRef();}
struct NiUpdateData {};
struct NiTransform {std::array<float,13> value{};};struct NiBound{std::array<float,4> v{};};
namespace BSGraphics {struct VertexDesc{std::uint64_t v=123;};}
struct NiAVObject:NiObject {
 void Update(NiUpdateData&){}
 NiNode* parent{};NiTransform local{};float fadeAmount=1;bool collisionObject=false;
 virtual NiNode* AsNode(){return nullptr;}virtual BSGeometry* AsGeometry(){return nullptr;}virtual NiAVObject* GetObjectByName(const BSFixedString& q){return q==name?this:nullptr;}
};
struct NiNode:NiAVObject {
 std::vector<NiPointer<NiAVObject>> children;
 const NiRTTI* GetRTTI()const override{static NiRTTI t{"NiNode"};return &t;}NiNode* AsNode()override{return this;}
 auto& GetChildren(){return children;}
 void SetAt(std::uint32_t i,NiAVObject* c){if(children[i])children[i]->parent=nullptr;children[i].reset(c);if(c)c->parent=this;}
 void Add(NiAVObject* c){children.emplace_back(c);if(c)c->parent=this;}
 NiAVObject* GetObjectByName(const BSFixedString& q)override{if(q==name)return this;for(auto& c:children)if(c)if(auto* found=c->GetObjectByName(q))return found;return nullptr;}
 NiObject* Clone()override{++cloneCalls;if(cloneShared)return this;auto* c=new NiNode;c->name=name;c->local=local;c->fadeAmount=fadeAmount;for(auto& ch:children)c->Add(ch?static_cast<NiAVObject*>(ch->Clone()):nullptr);return c;}
};
struct NiTexture:NiObject{};
struct NiSourceTexture:NiTexture{const NiRTTI* GetRTTI()const override{static NiRTTI t{"NiSourceTexture"};return &t;}};
struct NiColorA {float red=.2F,green=.4F,blue=1,alpha=.35F;};
struct BSShaderMaterial {enum class Type{kEffect};virtual ~BSShaderMaterial()=default;};
struct BSEffectShaderMaterial:BSShaderMaterial {
 NiPointer<NiSourceTexture> sourceTexture{};BSFixedString sourceTexturePath{};NiColorA baseColor{};float baseColorScale=.8F;
 Type GetType(){return Type::kEffect;}
 bool DoIsCopy(BSEffectShaderMaterial* m){return sourceTexture.get()==m->sourceTexture.get()&&sourceTexturePath==m->sourceTexturePath&&baseColorScale==m->baseColorScale&&!std::memcmp(&baseColor,&m->baseColor,sizeof(baseColor));}
};
struct BSShaderProperty:NiObject {
 std::uint64_t flags=1234;float alpha=.9F;BSEffectShaderMaterial* material{};
 ~BSShaderProperty(){delete material;}BSShaderMaterial* GetBaseMaterial(){return material;}
 bool SetupGeometry(BSGeometry*){if(setupFail==0){setupFail=-1;return false;}if(setupFail>0)--setupFail;return true;}
};
struct BSEffectShaderProperty:BSShaderProperty {
 const NiRTTI* GetRTTI()const override{static NiRTTI t{"BSEffectShaderProperty"};return &t;}
 BSEffectShaderMaterial* GetMaterial(){return material;}
 void SetMaterial(BSEffectShaderMaterial* m,bool){auto* n=new BSEffectShaderMaterial(*m);delete material;material=n;}
 NiObject* Clone()override{++cloneCalls;auto* c=new BSEffectShaderProperty;c->flags=flags;c->alpha=alpha;c->material=new BSEffectShaderMaterial(*material);return c;}
};
struct BSGeometry:NiAVObject {
 struct Runtime {NiPointer<BSShaderProperty> shaderProperty{};bool skinInstance=false;BSGraphics::VertexDesc vertexDesc{};}data;
 struct Model{NiBound modelBound{};}model;
 BSGeometry* AsGeometry()override{return this;}auto& GetGeometryRuntimeData(){return data;}auto& GetModelData(){return model;}
 const NiRTTI* GetRTTI()const override{static NiRTTI t{"BSTriShape"};return &t;}
 NiObject* Clone()override{++cloneCalls;auto* c=new BSGeometry;c->name=name;c->local=local;c->fadeAmount=fadeAmount;c->data=data;c->model=model;return c;}
};
struct ModelReferenceEffect {
 bool finished=false;NiPointer<NiAVObject> artObject3D{};struct{NiPointer<NiAVObject> attachedArt{},current3DRoot{};}hitEffectArtData;
 NiAVObject* Get3D(){return artObject3D.get();}
};
struct BSShaderManager {static void GetTexture(const char*,bool,NiPointer<NiTexture>& p,bool){p.reset(new NiSourceTexture);}};
}
namespace REL{struct Module{static Module& get(){static Module m;return m;}int version(){return 597;}};}
namespace SKSE{inline constexpr int RUNTIME_SSE_1_5_97=597;}
namespace logger{template<class... T>void warn(const char*,T&&...){}template<class... T>void info(const char*,T&&...){};}
#include "VFX/CircleTint.cpp"
using namespace TOAM::VFX;using Color=TOAM::Presentation::Color;
struct Fixture {
 RE::NiPointer<RE::NiNode> parent;RE::ModelReferenceEffect effect;RE::NiNode* root{};RE::NiNode* original{};RE::NiPointer<RE::BSGeometry> cachedGeometry;CircleTint::State state;
 Fixture(){
  parent.reset(new RE::NiNode);root=new RE::NiNode;root->name="Root";parent->Add(root);effect.artObject3D.reset(root);effect.hitEffectArtData.attachedArt.reset(root);
  original=new RE::NiNode;original->name="AS_CirclePose";root->Add(original);
  for(int i=0;i<3;++i){auto* g=new RE::BSGeometry;g->name="shape";auto* p=new RE::BSEffectShaderProperty;p->material=new RE::BSEffectShaderMaterial;p->material->sourceTexture.reset(new RE::NiSourceTexture);p->material->sourceTexturePath="textures\\TOAM\\VFX\\Arcana_Circle_Face.dds";g->data.shaderProperty.reset(p);original->Add(g);if(i==0)cachedGeometry.reset(g);}
 }
 ~Fixture(){state={};}
 RE::BSEffectShaderMaterial* material(){auto* g=root->GetChildren()[0]->AsNode()->GetChildren()[0]->AsGeometry();return static_cast<RE::BSEffectShaderProperty*>(g->data.shaderProperty.get())->GetMaterial();}
};
int main(){
 unsigned checks=0;auto check=[&](bool v){++checks;if(!v){std::cerr<<"Failed "<<checks<<'\n';std::abort();}};
 CircleTint::Prepare();int baseline=RE::live;
 {
  Fixture a,b;check(a.root->GetRefCount()==3&&a.original->GetRefCount()==1&&a.cachedGeometry->GetRefCount()==2);
  a.state.color=Color::Red;b.state.color=Color::Blue;CircleTint::Apply(a.state,&a.effect);CircleTint::Apply(b.state,&b.effect);
  check(bool(a.state.lease)&&bool(b.state.lease));check(a.root->GetChildren()[0].get()!=a.original);check(a.material()->baseColor.red==1&&b.material()->baseColor.red==.3F);
  auto* original=static_cast<RE::BSEffectShaderProperty*>(a.cachedGeometry->data.shaderProperty.get())->GetMaterial();check(original->baseColor.red==.2F&&original->baseColor.alpha==.35F);
  int clones=RE::cloneCalls;for(int frame=0;frame<500;++frame)CircleTint::Apply(a.state,&a.effect);check(RE::cloneCalls==clones);
  a.state={};check(a.root->GetChildren()[0].get()==a.original&&a.original->parent==a.root);check(b.material()->baseColor.red==.3F);
  for(int cast=0;cast<100;++cast){a.state.color=cast%2?Color::Gold:Color::Purple;CircleTint::Apply(a.state,&a.effect);check(bool(a.state.lease));a.state={};check(a.root->GetChildren()[0].get()==a.original&&a.cachedGeometry->GetRefCount()==2);}
  clones=RE::cloneCalls;CircleTint::Apply(a.state,&a.effect);check(RE::cloneCalls==clones&&a.root->GetChildren()[0].get()==a.original);
 }
 check(RE::live==baseline);
 {
  Fixture a;a.state.color=Color::Red;RE::setupFail=1;CircleTint::Apply(a.state,&a.effect);check(!a.state.lease&&a.root->GetChildren()[0].get()==a.original);a.state={};
  RE::NiPointer<RE::NiAVObject> extra(a.root);a.state.color=Color::Gold;CircleTint::Apply(a.state,&a.effect);check(!a.state.lease);extra.reset();a.state={};
  RE::NiPointer<RE::NiAVObject> branchAlias(a.original);a.state.color=Color::Gold;CircleTint::Apply(a.state,&a.effect);check(!a.state.lease);branchAlias.reset();a.state={};
  RE::cloneShared=true;a.state.color=Color::Red;CircleTint::Apply(a.state,&a.effect);check(!a.state.lease&&a.original->parent==a.root);RE::cloneShared=false;
 }
 check(RE::live==baseline);
 std::cout<<"ACTUAL_CIRCLETINT_HOST_PASS checks="<<checks<<" recasts=100 simultaneous=2 frames_without_reclone=500 rollback=true unknown_owners_rejected=true native_rendering=NOT_RUN\n";
}