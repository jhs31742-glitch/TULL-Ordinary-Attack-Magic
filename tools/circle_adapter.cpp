#include "NifFile.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace nifly;
int wmain(int argc,wchar_t** argv){try{
 if(argc!=3)throw std::runtime_error("input output required");
 NifFile file;if(file.Load(std::filesystem::path(argv[1])))throw std::runtime_error("load");
 const auto oldCount=file.GetShapes().size();auto* root=file.GetRootNode();if(!root)throw std::runtime_error("root");
 auto fade=std::make_unique<BSFadeNode>();static_cast<NiNode&>(*fade)=*root;
 fade->name.get()="TOAM_EngineCircle";auto& header=file.GetHeader();
 header.ReplaceBlock(header.GetBlockID(root),std::move(fade));root=file.GetRootNode();
 const auto children=root->childRefs;root->childRefs.Clear();
 MatTransform initial;initial.scale=.001f;
 auto* pose=file.AddNode("AS_CirclePose",initial,root);if(!pose)throw std::runtime_error("pose");
 pose->childRefs=children;pose->flags=14;
 // Arcana's explicit emission flag is the sole shader delta; colors/alpha/textures stay TOAM-owned.
 for(std::uint32_t i=0;i<header.GetNumBlocks();++i)
  if(auto* shader=header.GetBlock<BSEffectShaderProperty>(i))shader->shaderFlags1|=SLSF1_OWN_EMIT;
 // Preserve TOAM geometry, shader values, textures, animation and local part transforms.
 file.FinalizeData();NifSaveOptions options;options.optimize=false;
 if(file.Save(std::filesystem::path(argv[2]),options))throw std::runtime_error("save");
 NifFile check;if(check.Load(std::filesystem::path(argv[2]))||!check.IsValid()||check.GetShapes().size()!=oldCount)throw std::runtime_error("reload");
 std::cout<<"TOAM_CIRCLE_WRAPPER_PASS shapes="<<oldCount<<" shaderAndGeometryPolicy=preserve\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}