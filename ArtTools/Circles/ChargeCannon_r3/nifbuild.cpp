// nifbuild: builds Skyrim SE NIF files (BSTriShape + BSEffectShaderProperty + loop controllers) from a .nifspec text file.
// Build: g++ -std=c++17 -O2 nifbuild.cpp -I<nifly>/include -I<nifly>/external <nifly>/build/src/libnifly.a -o nifbuild
#include "NifFile.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <cmath>
using namespace nifly;
static const float INV = std::numeric_limits<float>::lowest();
struct Key { float t, v; };

int main(int argc, char** argv) {
	if (argc < 3) { std::cerr << "usage: nifbuild in.nifspec out.nif\n"; return 1; }
	std::ifstream in(argv[1]);
	if (!in) { std::cerr << "cannot open spec\n"; return 1; }
	NifFile nif;
	nif.Create(NiVersion(V20_2_0_7, 12, 100));
	NiHeader& hdr = nif.GetHeader();
	NiNode* root = nif.GetRootNode();
	std::map<std::string, NiNode*> nodes; nodes["-"] = root;
	std::map<std::string, NiShape*> shapes;
	std::map<std::string, uint32_t> lastCtrl; // chain tail per target name
	std::string tok;
	auto parentOf = [&](const std::string& p) { auto it = nodes.find(p); return it == nodes.end() ? root : it->second; };

	auto linkController = [&](const std::string& key, NiObjectNET* target, uint32_t ctrlId) {
		auto it = lastCtrl.find(key);
		if (it == lastCtrl.end()) target->controllerRef.index = ctrlId;
		else hdr.GetBlock<NiTimeController>(it->second)->nextControllerRef.index = ctrlId;
		lastCtrl[key] = ctrlId;
	};
	auto setTiming = [&](NiTimeController* c, uint32_t targetId, bool loop, float stop) {
		c->flags = loop ? 0x0008 : 0x000C; // active, loop / clamp
		c->frequency = 1.0f; c->phase = 0.0f; c->startTime = 0.0f; c->stopTime = stop;
		c->targetRef.index = targetId;
	};

	while (in >> tok) {
		if (tok == "ROOT") {
			std::string rn; in >> rn; root->name.get() = rn;
		} else if (tok == "NODE") {
			std::string name, parent; float x, y, z, s; in >> name >> parent >> x >> y >> z >> s;
			MatTransform xf; xf.translation = Vector3(x, y, z); xf.scale = s;
			nodes[name] = nif.AddNode(name, xf, parentOf(parent));
		} else if (tok == "MESH") {
			std::string name, parent, tex; float r, g, b, a, emis; int dbl, fall; float fsa, fso, fsop, fstop; int clamp, nv, nt;
			in >> name >> parent >> tex >> r >> g >> b >> a >> emis >> dbl >> fall >> fsa >> fso >> fsop >> fstop >> clamp >> nv >> nt;
			std::vector<Vector3> v(nv), n(nv); std::vector<Vector2> uv(nv); std::vector<Triangle> t(nt);
			for (int i = 0; i < nv; i++) { float vx, vy, vz, u, w, nx, ny, nz; in >> vx >> vy >> vz >> u >> w >> nx >> ny >> nz; v[i] = Vector3(vx, vy, vz); uv[i] = Vector2(u, w); n[i] = Vector3(nx, ny, nz); }
			for (int i = 0; i < nt; i++) { int p1, p2, p3; in >> p1 >> p2 >> p3; t[i] = Triangle(p1, p2, p3); }
			NiShape* sh = nif.CreateShapeFromData(name, &v, &t, &uv, &n);
			nif.DeleteShader(sh);
			auto sp = std::make_unique<BSEffectShaderProperty>();
			sp->shaderFlags1 = 0x80000000u | (fall ? 0x40u : 0u);
			sp->shaderFlags2 = dbl ? 0x10u : 0u;
			sp->sourceTexture = NiString(tex);
			sp->textureClampMode = clamp; // 0 clamp/clamp, 3 wrap/wrap
			sp->falloffStartAngle = fsa; sp->falloffStopAngle = fso; sp->falloffStartOpacity = fsop; sp->falloffStopOpacity = fstop;
			sp->baseColor = Color4(r, g, b, a); sp->baseColorScale = emis;
			sp->softFalloffDepth = 100.0f;
			uint32_t spId = hdr.AddBlock(std::move(sp));
			sh->ShaderPropertyRef()->index = spId;
			auto ap = std::make_unique<NiAlphaProperty>();
			ap->flags = 0x100D; // blend on, SRC_ALPHA / ONE (additive), alpha test off
			ap->threshold = 0;
			nif.AssignAlphaProperty(sh, std::move(ap));
			nif.SetParentNode(sh, parentOf(parent));
			shapes[name] = sh;
		} else if (tok == "SPIN") {
			std::string node; int axis; float period; int dir; in >> node >> axis >> period >> dir;
			NiNode* nd = nodes[node]; uint32_t ndId = hdr.GetBlockID(nd);
			auto data = std::make_unique<NiTransformData>(); data->rotationType = LINEAR_KEY;
			for (int i = 0; i <= 8; i++) {
				float th = dir * 2.0f * 3.14159265f * i / 8.0f, s = std::sin(th / 2), c = std::cos(th / 2);
				NiAnimationKey<Quaternion> k; k.type = LINEAR_KEY; k.time = period * i / 8.0f;
				k.value = Quaternion(); k.value.w = c; k.value.x = axis == 0 ? s : 0; k.value.y = axis == 1 ? s : 0; k.value.z = axis == 2 ? s : 0;
				data->quaternionKeys.push_back(k);
			}
			uint32_t dId = hdr.AddBlock(std::move(data));
			auto ip = std::make_unique<NiTransformInterpolator>();
			ip->translation = Vector3(INV, INV, INV); ip->rotation = Quaternion(); ip->rotation.w = INV; ip->rotation.x = INV; ip->rotation.y = INV; ip->rotation.z = INV; ip->scale = INV;
			ip->dataRef.index = dId;
			uint32_t iId = hdr.AddBlock(std::move(ip));
			auto c = std::make_unique<NiTransformController>(); c->interpolatorRef.index = iId; setTiming(c.get(), ndId, true, period);
			linkController("n:" + node, nd, hdr.AddBlock(std::move(c)));
		} else if (tok == "NODESCALE") {
			std::string node; int loop, n; in >> node >> loop >> n; std::vector<Key> ks(n); float stop = 0;
			for (auto& k : ks) { in >> k.t >> k.v; stop = std::max(stop, k.t); }
			NiNode* nd = nodes[node]; uint32_t ndId = hdr.GetBlockID(nd);
			auto data = std::make_unique<NiTransformData>(); data->rotationType = NO_INTERP; data->scales.SetInterpolationType(LINEAR_KEY);
			for (auto& k : ks) { NiAnimationKey<float> kk; kk.type = LINEAR_KEY; kk.time = k.t; kk.value = k.v; data->scales.AddKey(kk); }
			uint32_t dId = hdr.AddBlock(std::move(data));
			auto ip = std::make_unique<NiTransformInterpolator>();
			ip->translation = Vector3(INV, INV, INV); ip->rotation = Quaternion(); ip->rotation.w = INV; ip->rotation.x = INV; ip->rotation.y = INV; ip->rotation.z = INV; ip->scale = INV;
			ip->dataRef.index = dId; uint32_t iId = hdr.AddBlock(std::move(ip));
			auto c = std::make_unique<NiTransformController>(); c->interpolatorRef.index = iId; setTiming(c.get(), ndId, loop != 0, stop);
			linkController("n:" + node, nd, hdr.AddBlock(std::move(c)));
		} else if (tok == "SHFLOAT") {
			std::string mesh; int var, loop, n; in >> mesh >> var >> loop >> n; std::vector<Key> ks(n); float stop = 0;
			for (auto& k : ks) { in >> k.t >> k.v; stop = std::max(stop, k.t); }
			NiShape* sh = shapes[mesh]; NiShader* sd = nif.GetShader(sh); uint32_t sdId = hdr.GetBlockID(sd);
			auto data = std::make_unique<NiFloatData>(); data->data.SetInterpolationType(LINEAR_KEY);
			for (auto& k : ks) { NiAnimationKey<float> kk; kk.type = LINEAR_KEY; kk.time = k.t; kk.value = k.v; data->data.AddKey(kk); }
			uint32_t dId = hdr.AddBlock(std::move(data));
			auto ip = std::make_unique<NiFloatInterpolator>(); ip->floatValue = INV; ip->dataRef.index = dId;
			uint32_t iId = hdr.AddBlock(std::move(ip));
			auto c = std::make_unique<BSEffectShaderPropertyFloatController>(); c->typeOfControlledVariable = var; c->interpolatorRef.index = iId;
			setTiming(c.get(), sdId, loop != 0, stop);
			linkController("s:" + mesh, sd, hdr.AddBlock(std::move(c)));
		}
	}
	nif.FinalizeData();
	int rc = nif.Save(argv[2]);
	if (rc) { std::cerr << "save failed " << rc << "\n"; return 2; }
	std::cout << "wrote " << argv[2] << "\n";
	return 0;
}
