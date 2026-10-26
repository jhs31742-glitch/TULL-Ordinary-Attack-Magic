#include "NifFile.hpp"
#include <fstream>
#include <iostream>
#include <cmath>
#include <set>
#include <stdexcept>
using namespace nifly;
int wmain(int argc, wchar_t** argv) {
  try {
    if(argc!=4) throw std::runtime_error("usage: nifassemble baseline.nif mesh.spec output.nif");
    NifFile n; if(n.Load(std::filesystem::path(argv[1]))) throw std::runtime_error("baseline load");
    auto old=n.GetShapes(); for(auto* s:old)n.DeleteShape(s);
    auto* root=n.GetRootNode(); root->name.get()="FernStaff";
    std::ifstream in{std::filesystem::path(argv[2])}; if(!in)throw std::runtime_error("spec open");
    std::string token,name,mat; size_t nv,nt; float gloss,spec; int two;
    while(in>>token) {
      if(token!="MESH") throw std::runtime_error("unknown token");
      in>>name>>mat>>gloss>>spec>>two>>nv>>nt;
      if(nv>=15000||nt>=15000||nv<3||nt<1) throw std::runtime_error("part budget");
      std::vector<Vector3> v(nv),norm(nv);std::vector<Vector2> uv(nv);std::vector<Triangle> tr(nt);
      for(size_t i=0;i<nv;i++)in>>v[i].x>>v[i].y>>v[i].z>>uv[i].u>>uv[i].v>>norm[i].x>>norm[i].y>>norm[i].z;
      for(size_t i=0;i<nt;i++){int a,b,c;in>>a>>b>>c;if(a<0||b<0||c<0||a>=nv||b>=nv||c>=nv)throw std::runtime_error("index");tr[i]=Triangle(a,b,c);}
      if(!in)throw std::runtime_error("truncated spec");
      auto* sh=n.CreateShapeFromData(name,&v,&tr,&uv,&norm);sh->flags=14;
      auto* shader=dynamic_cast<BSLightingShaderProperty*>(n.GetShader(sh));
      if(!shader)throw std::runtime_error("lighting shader missing");
      shader->shaderFlags1=SLSF1_SPECULAR|SLSF1_RECEIVE_SHADOWS|SLSF1_CAST_SHADOWS|SLSF1_ZBUFFER_TEST;
      shader->shaderFlags2=SLSF2_ZBUFFER_WRITE|(two?SLSF2_DOUBLE_SIDED:0);
      shader->glossiness=gloss;shader->specularStrength=spec;shader->specularColor=Vector3(1,1,1);
      shader->emissiveColor=Vector3(0,0,0);shader->emissiveMultiple=0;shader->alpha=1;
      std::string diffuse="textures\\FernStaff\\"+mat+"_d.dds",normal="textures\\FernStaff\\"+mat+"_n.dds";
      n.SetTextureSlot(sh,diffuse,0);n.SetTextureSlot(sh,normal,1);
      sh->UpdateBounds();n.CalcTangentsForShape(sh);
      std::cout<<name<<" verts="<<nv<<" triangles="<<nt<<'\n';
    }
    n.FinalizeData(); if(n.Save(std::filesystem::path(argv[3])))throw std::runtime_error("save");
    NifFile verify; if(verify.Load(std::filesystem::path(argv[3]))||!verify.IsValid())throw std::runtime_error("reload");
    std::cout<<"RELOAD_PASS shapes="<<verify.GetShapes().size()<<'\n';return 0;
  }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
