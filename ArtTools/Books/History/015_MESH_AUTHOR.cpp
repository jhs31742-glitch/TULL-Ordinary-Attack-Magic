#include "NifFile.hpp"
#include <iostream>
#include <cassert>
#include <cstring>
using namespace nifly;
template<class T>void same(const std::vector<T>*a,const std::vector<T>*b){assert(bool(a)==bool(b));if(!a)return;assert(a->size()==b->size());assert(a->empty()||memcmp(a->data(),b->data(),a->size()*sizeof(T))==0);}
int wmain(int argc,wchar_t**argv){assert(argc==5);NifFile n;assert(!n.Load(std::filesystem::path(argv[1])));std::string mode=std::filesystem::path(argv[3]).string();bool verify=std::wstring(argv[4])==L"verify";NifFile b;if(verify)assert(!b.Load(std::filesystem::path(argv[2])));int changes=0;
for(auto*sh:n.GetShapes()){
 if(mode=="world"){
  bool inner=sh->name.get()=="CC_Face_Mesh"||sh->name.get()=="CC_NodeGlow_Mesh";
  if(verify){auto*t=b.FindBlockByName<NiShape>(sh->name.get());assert(t);same(n.GetVertsForShape(sh),b.GetVertsForShape(t));same(n.GetUvsForShape(sh),b.GetUvsForShape(t));same(n.GetNormalsForShape(sh),b.GetNormalsForShape(t));same(n.GetColorsForShape(sh),b.GetColorsForShape(t));same(n.GetTangentsForShape(sh),b.GetTangentsForShape(t));assert(std::abs(t->transform.scale-sh->transform.scale*(inner?1.65f:1.f))<.00001f);}
  else if(inner){sh->transform.scale*=1.65f;++changes;}
 }else{
  std::string tex;n.GetTextureSlot(sh,tex);if(tex.find("Arcana_Tome_d.dds")==std::string::npos)continue;
  auto expected=*n.GetUvsForShape(sh);auto*verts=n.GetVertsForShape(sh);auto*norm=n.GetNormalsForShape(sh);
  for(size_t i=0;i<expected.size();i++){auto&v=expected[i];assert(std::abs(v.u)<3.01f&&std::abs(v.v)<3.01f);if(verts->at(i).z<-.9f&&norm->at(i).z<-.5f&&v.u<1.f&&v.v<1.f)v.v+=1.f;v.u*=.5f;v.v*=.5f;}
  if(verify){auto*t=b.FindBlockByName<NiShape>(sh->name.get());assert(t);same(n.GetVertsForShape(sh),b.GetVertsForShape(t));same(n.GetNormalsForShape(sh),b.GetNormalsForShape(t));same(n.GetColorsForShape(sh),b.GetColorsForShape(t));same(n.GetTangentsForShape(sh),b.GetTangentsForShape(t));auto*v=b.GetUvsForShape(t);assert(expected.size()==v->size());for(size_t i=0;i<expected.size();i++){assert(std::abs(expected[i].u-v->at(i).u)<.001f&&std::abs(expected[i].v-v->at(i).v)<.001f);}std::vector<Triangle> a1,b1;sh->GetTriangles(a1);t->GetTriangles(b1);assert(a1==b1);}
  else{n.SetUvsForShape(sh,expected);++changes;}
 }
}
if(!verify){assert(changes>0);n.FinalizeData();NifSaveOptions options;options.optimize=false;options.sortBlocks=false;assert(!n.Save(std::filesystem::path(argv[2]),options));}else{assert(b.IsValid()&&n.GetShapes().size()==b.GetShapes().size());}
std::cout<<mode<<" "<<(verify?"GEOMETRY_UV_TRIANGLE_PRESERVATION_PASS":"AUTHORED")<<" changed="<<changes<<"\n";
}
