#include "NifFile.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>
using namespace nifly;
void demote(NifFile& n,NiShape* sh){
 auto& h=n.GetHeader();auto* old=h.GetBlock<NiSkinInstance>(sh->SkinInstanceRef());
 auto plain=std::make_unique<NiSkinInstance>();*plain=*old;h.ReplaceBlock(sh->SkinInstanceRef()->index,std::move(plain));
}
void skin(NifFile&n,NiShape*sh,std::vector<int> ids,const std::vector<Vector3>& pos,std::vector<std::vector<std::pair<int,float>>> weights){
 n.CreateSkinning(sh);n.SetShapeBoneIDList(sh,ids);auto&h=n.GetHeader();auto*si=h.GetBlock<NiSkinInstance>(sh->SkinInstanceRef());auto*sd=h.GetBlock(si->dataRef);
 n.ClearShapeVertWeights(sh->name.get());n.SetShapeTransformGlobalToSkin(sh,MatTransform());
 for(size_t i=0;i<ids.size();i++){MatTransform inv;inv.translation=pos[i]*-1.0f;n.SetShapeTransformSkinToBone(sh,(uint32_t)i,inv);}
 for(uint16_t v=0;v<weights.size();v++){
  std::vector<uint8_t> bs;std::vector<float> ws;
  for(auto [b,w]:weights[v])if(w>0){bs.push_back((uint8_t)b);ws.push_back(w);sd->bones[b].vertexWeights.emplace_back(v,w);}
  n.SetShapeVertWeights(sh->name.get(),v,bs,ws);
 }
 auto*verts=n.GetVertsForShape(sh);
 for(size_t b=0;b<ids.size();b++){
  auto&bd=sd->bones[b];bd.numVertices=(uint16_t)bd.vertexWeights.size();std::vector<Vector3> pp;
  for(auto&w:bd.vertexWeights)pp.push_back(verts->at(w.index)-pos[b]);
  if(pp.empty())pp.push_back(Vector3());bd.bounds=BoundingSphere(pp);
 }
 n.UpdateSkinPartitions(sh);demote(n,sh);
}
int wmain(int argc,wchar_t**argv){try{
 if(argc!=5)throw std::runtime_error("usage nifskin static.nif ribbon.skin smp.nif helper.nif");
 NifFile n;if(n.Load(std::filesystem::path(argv[1])))throw std::runtime_error("load");
 auto*root=n.GetRootNode();auto*staff=n.AddNode("FSFern_StaffRoot",MatTransform(),root);
 std::ifstream f{std::filesystem::path(argv[2])};std::string tok,name;int count;f>>tok>>count;
 if(tok!="BONES"||count!=9)throw std::runtime_error("bone spec");
 std::vector<Vector3> positions;std::vector<int> ids;NiNode*parent=staff;Vector3 prev;
 for(int i=0;i<count;i++){Vector3 p;f>>name>>p.x>>p.y>>p.z;MatTransform xf;xf.translation=p-prev;auto*node=n.AddNode(name,xf,parent);ids.push_back(n.GetHeader().GetBlockID(node));positions.push_back(p);parent=node;prev=p;}
 while(f>>tok){if(tok!="SKIN")throw std::runtime_error("skin token");int nv;f>>name>>nv;auto*sh=n.FindBlockByName<NiShape>(name);if(!sh||sh->GetNumVertices()!=nv)throw std::runtime_error("mesh mismatch");
  std::vector<std::vector<std::pair<int,float>>> weights(nv);
  for(int v=0;v<nv;v++){int a,b;float wa,wb;f>>a>>wa>>b>>wb;weights[v]={{a,wa},{b,wb}};}
  skin(n,sh,ids,positions,weights);std::cout<<"SKINNED "<<name<<" verts="<<nv<<'\n';
 }
 n.FinalizeData();if(n.Save(std::filesystem::path(argv[3])))throw std::runtime_error("save smp");
 NifFile helper;helper.Create(NiVersion(V20_2_0_7,12,100));auto*hr=helper.GetRootNode();hr->name.get()="FSFern_SMP_Helper";
 auto*bone=helper.AddNode("NPC Root [Root]",MatTransform(),hr);
 std::vector<Vector3> v={Vector3(0,0,0),Vector3(.001f,0,0),Vector3(0,.001f,0)},no(3,Vector3(0,0,1));std::vector<Triangle>tr={Triangle(0,1,2)};std::vector<Vector2>uv={Vector2(0,0),Vector2(1,0),Vector2(0,1)};
 auto*sh=helper.CreateShapeFromData("FSFern_InvisibleBridge",&v,&tr,&uv,&no);sh->flags=15;
 skin(helper,sh,{(int)helper.GetHeader().GetBlockID(bone)},{Vector3()},{{{0,1}},{{0,1}},{{0,1}}});
 auto extra=std::make_unique<NiStringExtraData>();extra->name.get()="HDT Skinned Mesh Physics Object";extra->stringData.get()="SKSE\\Plugins\\hdtSkinnedMeshConfigs\\FernStaff\\Ribbon.xml";
 hr->extraDataRefs.AddBlockRef(helper.GetHeader().AddBlock(std::move(extra)));
 helper.FinalizeData();if(helper.Save(std::filesystem::path(argv[4])))throw std::runtime_error("save helper");
 return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
