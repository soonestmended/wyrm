#include "PBRTParser.hpp"
#include "Image.hpp"
#include "Material.hpp"

#include <cctype>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iterator>
#include <map>
#include <stack>

#define HDEF(x) void PBRTParser::x(const Statement& s, Options& o)

namespace fs = std::filesystem;
using namespace std;

std::map <std::string, std::function <void(const Statement&, Options&)>> PBRTParser::handlers =
  {
   {"AreaLightSource", PBRTParser::AreaLightSource},
   {"AttributeBegin", PBRTParser::AttributeBegin},
   {"AttributeEnd", PBRTParser::AttributeEnd},
   {"ConcatTransform", PBRTParser::ConcatTransform},
   {"Film", PBRTParser::Film},
   {"Identity", PBRTParser::Identity},
   {"Integrator", PBRTParser::Integrator},
   {"LightSource", PBRTParser::LightSource}, 
   {"LookAt", PBRTParser::LookAt},
   {"Camera", PBRTParser::MakeCamera},
   {"MakeNamedMaterial", PBRTParser::MakeNamedMaterial},
   {"Material", PBRTParser::MaterialStatement},
   {"NamedMaterial", PBRTParser::NamedMaterial},
   {"PixelFilter", PBRTParser::PixelFilter}, 
   {"Rotate", PBRTParser::Rotate},
   {"Sampler", PBRTParser::Sampler}, 
   {"Scale", PBRTParser::Scale},
   {"Shape", PBRTParser::Shape},
   {"Texture", PBRTParser::MakeTexture},
   {"Transform", PBRTParser::Transform},
   {"Translate", PBRTParser::Translate},
   {"WorldBegin", PBRTParser::WorldBegin},
   {"WorldEnd", PBRTParser::WorldEnd}
  };

Mat4 PBRTParser::currentTransform = Mat4(1.0);
shared_ptr <Material> PBRTParser::currentMaterial = make_shared <DiffuseMaterial> ("defaultMaterial", make_shared <ConstantTexture <Color>> (Color::Blue()));
shared_ptr <Material> PBRTParser::currentAreaLightMaterial = nullptr;
bool PBRTParser::currentReverseOrientation = false;

stack <Mat4> PBRTParser::transformStack = {};
stack <shared_ptr<Material>> PBRTParser::materialStack = {};
stack <shared_ptr<Material>> PBRTParser::areaLightMaterialStack = {};
stack <bool> PBRTParser::reverseOrientationStack = {};
map <string, shared_ptr <Material>> PBRTParser::namedMaterialMap = {};
map <string, shared_ptr <Texture <Color>>> PBRTParser::namedTextureMap = {};

vector <shared_ptr <Light>> PBRTParser::lights = {};
vector <shared_ptr <Material>> PBRTParser::materials = {};
vector <shared_ptr <Primitive>> PBRTParser::primitives = {};

HDEF(AreaLightSource) {
  // for now ignores bool twoSided and integer samples
  auto v = getParamVec <Color> (s.params, "L");
  if (v.size() > 0)
    currentAreaLightMaterial = make_shared <DiffuseMaterial>("anon", make_shared <ConstantTexture <Color>> (Color::Black()), v[0]);
  else
    currentAreaLightMaterial = make_shared <DiffuseMaterial>("anon", make_shared <ConstantTexture <Color>> (Color::Black()), Color::White());
    
}

HDEF(AttributeBegin) {
  // push graphics state stack
  transformStack.push(currentTransform);
  materialStack.push(currentMaterial);
  areaLightMaterialStack.push(currentAreaLightMaterial);
  reverseOrientationStack.push(currentReverseOrientation);
}

HDEF(AttributeEnd) {
  currentTransform = transformStack.top();
  currentMaterial = materialStack.top();
  currentAreaLightMaterial = areaLightMaterialStack.top();
  currentReverseOrientation = reverseOrientationStack.top();

  transformStack.pop();
  materialStack.pop();
  areaLightMaterialStack.pop();
  reverseOrientationStack.pop();

}

HDEF(ConcatTransform) {
  auto v = getParamVec <Real> (s.params, "anon");
  if (v.size() == 16)
    currentTransform = currentTransform * Mat4(v[0], v[4], v[8], v[12], v[1], v[5], v[9], v[13], v[2], v[6], v[10], v[14], v[3], v[7], v[11], v[15]);
  else
    cerr << "Error: ConcatTransform: supplied vec has size " << v.size() << endl;
}

HDEF(Film) {
  auto w = getParamVec <int> (s.params, "xresolution");
  if (w.size() == 0) w.push_back(500);
  auto h = getParamVec <int> (s.params, "yresolution");
  if (h.size() == 0) h.push_back(500);
  auto fn = getParamVec <string> (s.params, "filename");
  if (fn.size() == 0) fn.push_back("image.png");
  auto ml = getParamVec <Real> (s.params, "maxsampleluminance");
  if (ml.size() == 0) ml.push_back(POS_INF);
  o.imageWidth = w[0];
  o.imageHeight = h[0];
  o.imageFilename = fn[0];
  o.maxLuminance = ml[0];
  // scale
  // diagonal
  // cropwindow
}

HDEF(Identity) {
  currentTransform = Mat4(1.0);
}

HDEF(Integrator) {
  if (s.type.compare("path") != 0) {
    cerr << "Error. Integrator " << s.type << " requested. Only path implemented so far." << endl;
    exit(0);
  }
  o.tracerType = s.type;
  // path only here
  auto md = getParamVec <int> (s.params, "maxdepth");
  if (md.size() == 0) md.push_back(5);
  auto rrt = getParamVec <Real> (s.params, "rrthreshold");
  if (rrt.size() == 0) rrt.push_back(1);
  // pixelbounds
  // lightsamplestrategy
  o.tracerMaxDepth = md[0];
  o.rrThreshold = rrt[0];
}

HDEF(LightSource) {
  auto vScale = getParamVec <Color> (s.params, "scale");
  if (s.type == "distant") {
    auto vL = getParamVec <Color> (s.params, "L");
    if (vL.size() == 0) {
      cout << "Error: no color specified for distant light source. Using white." << endl;
      vL.emplace_back(1, 1, 1);
    }
    Color L = vL[0];
    if (vScale.size() == 1) {
      L *= vScale[0];
    }
    Vec3 from, to;
    auto vFrom = getParamVec <Vec3> (s.params, "from");
    if (vFrom.size() == 1) {
      from = vFrom[0];
    }
    else {
      from = Vec3(0);
    }
    auto vTo = getParamVec <Vec3> (s.params, "to");
    if (vTo.size() == 1) {
      to = vTo[0];
    }
    else {
      to = Vec3(0, 0, 1);
    }
    Vec3 dir = Vec3(currentTransform * Vec4(to - from, 0));
    lights.push_back(make_shared <DirectionalLight> (dir, L));
    // cout << "Created distant light" << endl;
  }
  else if (s.type == "point") {
    Color I;
    Vec3 from;
    auto vI = getParamVec <Color> (s.params, "I");
    if (vI.size() == 0) {
      cout << "Error: no intensity specified for point light. Using <100, 100, 100>." << endl;
      vI.emplace_back(100, 100, 100);
    }
    I = vI[0];

    auto vFrom = getParamVec <Vec3> (s.params, "from");
    if (vFrom.size() == 1) {
      from = vFrom[0];
    }
    else {
      cout << "Error: no location specified for point light. Using <100, 100, 100>." << endl;
      from = Vec3{100};
    }
    lights.push_back(make_shared <PointLight> (from, I));
  }
  else if (s.type == "infinite") {
    auto vMapname = getParamVec <string> (s.params, "mapname");
    fs::path texPath = fs::path(vMapname[0]);
    texPath = fs::absolute(texPath);
    // cout << "texPath: " << texPath.string() << endl;
    // get color scale
    shared_ptr <ImageTexture> texPtr = make_shared <ImageTexture> (texPath.c_str());
    auto vScaleColor = getParamVec <Color> (s.params, "L");
    if (vScaleColor.size() == 0)
      vScaleColor.push_back(Color{1});

    lights.push_back(make_shared <InfiniteLight> (texPtr, currentTransform, vScaleColor[0]));
  }
  else {
    cout << "Error: " << s.type << " light not supported yet." << endl;
    return;
  }
}

//void PBRTParser::LookAt(const Statement& s, shared_ptr <Camera>& c, shared_ptr <Options>& o) {
HDEF(LookAt) {
  auto v = getParamVec <Real> (s.params, "anon");
  if (v.size() == 9) {
    Mat4 la;

    Vec3 look = glm::normalize(Vec3(v[3], v[4], v[5]) - Vec3(v[0], v[1], v[2])); // target - eye
    Vec3 left = glm::normalize(glm::cross(glm::normalize(Vec3(v[6], v[7], v[8])), look)); // left = up x look
    Vec3 up = glm::cross(look, left); // renormalize up vector

    la[0] = Vec4(left, 0);
    la[1] = Vec4(up, 0);
    la[2] = Vec4(look, 0);
    la[3] = Vec4(v[0], v[1], v[2], 1); // eye

    currentTransform = currentTransform * glm::inverse(la);
  }
  else
    cout << "Error: LookAt: supplied vec has size " << v.size() << endl;
  // cout << "Post LookAt currentTransform: " << endl << currentTransform << endl;
}

HDEF(MakeCamera) {
  // current transform matrix is world to camera
  o.world2camera = currentTransform;
  o.camera2world = glm::inverse(currentTransform);
  auto far = getParamVec <Real> (s.params, "frameaspectratio");
  if (far.size() == 0) far.push_back(0);
  auto sw = getParamVec <Real> (s.params, "screenwindow");
  if (sw.size() == 0) sw.push_back(1);
  auto fov = getParamVec <Real> (s.params, "fov");
  if (fov.size() == 0) far.push_back(90);
  o.frameAspectRatio = far[0];
  o.screenWindow = sw[0];
  o.fov = fov[0];
  // cout << "right: " << Vec3(currentTransform[0]) << endl;
  // cout << "up: " << Vec3(currentTransform[1]) << endl;
  // cout << "look: " << Vec3(currentTransform[2]) << endl;
  // cout << "eye: " << o.camera2world * Vec4(0, 0, 0, 1) << endl;

}

shared_ptr <Texture <Color>> PBRTParser::getNamedTexture(string name) {
  // TODO lookup name in named texture map, return texture shared pointer if it exist. Otherwise nullptr
  auto it = namedTextureMap.find(name);
  if (it == namedTextureMap.end()) {
    return nullptr;
  }
  return it->second;
}

shared_ptr <Texture <Color>> PBRTParser::getColorOrTexture(map<string, ParamVec> m, string name) {
  shared_ptr <Texture <Color>> tex;
  try {
    auto pvv = m.at(name); // ParamVecVariant
    if (holds_alternative<vector <Color>> (pvv)) {
      // pv is a single color
      auto pv = std::get <vector <Color>> (pvv); // pv for ParamVec (variant state removed)
      tex = make_shared <ConstantTexture <Color>> (pv[0]);
    }
    else if (holds_alternative<vector <string>> (pvv)) {
      // pv is a named texture
      auto pv = std::get <vector <string>> (pvv);
      tex = getNamedTexture(pv[0]);
      if (tex == nullptr) {
        cout << "Error: texture " << pv[0] << " not found. Using grey..." << endl;
        tex = make_shared <ConstantTexture<Color>> (Color::Grey());
      }
    }
  } catch (const std::out_of_range& oor) {
    // no Kr in paramvec
    cout << "Error: parameter " << name << " not found. Using grey..." << endl;
    tex = make_shared <ConstantTexture <Color>> (Color::Grey());
  }
  return tex;
}

shared_ptr <Material> PBRTParser::makeMaterial(const Statement& s) {
  shared_ptr <Material> ans = nullptr;
  auto nameV = getParamVec <string> (s.params, "name");
  if (nameV.size() == 0) nameV.push_back("unnamed_" + s.type);
  if (s.type.compare("glass") == 0) { // GlassMaterial
    shared_ptr <Texture <Color>> texR, texT;
    texR = getColorOrTexture(s.params, "Kr");
    texT = getColorOrTexture(s.params, "Kt");

    auto etaV = getParamVec <Real> (s.params, "eta");
    if (etaV.size() == 0) etaV.push_back(1.5);
    
    // uroughness
    // vroughness
    // remaproughness
    //    Color Kr = KrV[0];
    //    Color Kt = KtV[0];
    ans = make_shared <GlassMaterial> (nameV[0], texR, texT, etaV[0]);
  }

// TODO: Recognize if parameters are given as textures and handle appropriately

  else if (s.type.compare("matte") == 0) {
    auto sigmaV = getParamVec <Real> (s.params, "sigma");
    if (sigmaV.size() == 0) sigmaV.push_back(0);
    if (sigmaV[0] > 0) {
      cerr << "Error: Oren-Nayar model not yet implemented. Ignoring roughness." << endl;
    }

    shared_ptr <Texture <Color>> tex = getColorOrTexture(s.params, "Kd");
    ans = make_shared <DiffuseMaterial> (nameV[0], tex);
  }

  else if (s.type == "metal") {
    auto roughnessV = getParamVec <Real> (s.params, "roughness");
    if (roughnessV.size() == 0) roughnessV.push_back(.01);
    if (s.params.find("R0") != s.params.end()) {
      shared_ptr <Texture <Color>> R0 = getColorOrTexture(s.params, "R0");
      ans = make_shared <MetalMaterial> (nameV[0], R0, make_shared <ConstantTexture <Real>> (roughnessV[0]));
    }
    else {
      shared_ptr <Texture <Color>> texEta = getColorOrTexture(s.params, "eta");
      shared_ptr <Texture <Color>> texK = getColorOrTexture(s.params, "k");
      ans = make_shared <MetalMaterial> (nameV[0], texEta, texK, make_shared <ConstantTexture <Real>> (roughnessV[0]));
    }
  }

  else {
    cerr << "Error: material type " << s.type << " not implemented yet." << endl;
    exit(0);
  }
  return ans;
}

HDEF(MakeNamedMaterial) {
  string matName = s.type;
  auto matType = getParamVec <string> (s.params, "type");
  if (matType.size() == 0) {
    cerr << "Error: material type not specified in MakeNamedMaterial." << endl;
  }
  Statement matStatement = s;
  matStatement.type = matType[0];
  ParamVec pv = vector <string> (1, matName);
  matStatement.params.insert({"name", pv});
  shared_ptr <Material> mat = makeMaterial(matStatement);
  namedMaterialMap.insert({matName, mat});
}

HDEF(MakeTexture) {
	auto texInfo = getParamVec <string> (s.params, "texInfo");
	if (texInfo.size() != 3) {
		cout << "Error: invalid Texture statement. Parsed: " << endl;
		for (auto s : texInfo)
			cout << "[" << s << "]"<< endl;
		return;
	}
	string texName = texInfo[0];
	string texType = texInfo[1];
	string texClass = texInfo[2];
	if (texType != "spectrum" && texType != "color") {
		cout << "Error: texture type " << texType << " not yet supported." << endl;
		return;
	}

	if (texClass == "imagemap") {
		auto fnv = getParamVec <string> (s.params, "filename");
        // cout << "s.params filename: " << fnv[0] << endl;
        fs::path texPath = fs::path(fnv[0]);
        texPath = fs::absolute(texPath);
        // cout << "texPath: " << texPath.string() << endl;
		shared_ptr <ImageTexture> texPtr = make_shared <ImageTexture> (texPath.c_str());
		namedTextureMap.insert({texName, texPtr});
		return;
	}
}

HDEF(MaterialStatement) {
  currentMaterial = makeMaterial(s);
}

HDEF(NamedMaterial) {
  auto it = namedMaterialMap.find(s.type);
  if (it == namedMaterialMap.end()) {
    cerr << "Error: namedMaterial " << s.type << " not found." << endl;
    exit(0);
  }
  //cout << "Called namedMaterial: " << s.type << endl;
  currentMaterial = it->second;
  //cout << "Set material to: " << ((DiffuseMaterial*)currentMaterial.get())->lbrdf->color << endl;
}

//void PBRTParser::Rotate(const Statement& s, shared_ptr <Camera>& c, shared_ptr <Options>& o) {
HDEF(Rotate) {
  auto v = getParamVec <Real> (s.params, "anon");
  if (v.size() == 4) {
    //currentTransform = glm::rotate(currentTransform, v[0] * M_PI / 180., Vec3(v[1], v[2], v[3]));
    currentTransform = currentTransform * glm::rotate(Mat4{1}, v[0] * M_PI / 180., Vec3(v[1], v[2], v[3]));
  }
  else {
    cerr << "Error: Rotate: supplied vec has size " << v.size() << endl;
  }
}

HDEF(PixelFilter) {
  o.filterType = s.type;
  auto xwV = getParamVec <Real> (s.params, "xwidth");
  if (xwV.size() == 0) {
    if (o.filterType.compare("box") == 0) {
      xwV.push_back(0.5);
    }
    else if (o.filterType.compare("sinc") == 0) {
      xwV.push_back(4);
    }
    else {
      xwV.push_back(2);
    }
  }

  auto ywV = getParamVec <Real> (s.params, "ywidth");
  if (ywV.size() == 0) {
    if (o.filterType.compare("box") == 0) {
      ywV.push_back(0.5);
    }
    else if (o.filterType.compare("sinc") == 0) {
      ywV.push_back(4);
    }
    else {
      ywV.push_back(2);
    }
  }

  o.filterXWidth = xwV[0];
  o.filterYWidth = ywV[0];
}

HDEF(Sampler) {
  o.samplerType = s.type;
  auto psV = getParamVec <int> (s.params, "pixelsamples");
  if (psV.size() == 0) psV.push_back(16);

  auto jV = getParamVec <bool> (s.params, "jitter");
  if (jV.size() == 0) jV.push_back(true);

  auto xsV = getParamVec <int> (s.params, "xsamples");
  if (xsV.size() == 0) xsV.push_back(2);

  auto ysV = getParamVec <int> (s.params, "ysamples");
  if (ysV.size() == 0) ysV.push_back(2);

  o.spp = psV[0];
  o.samplerJitter = jV[0];
  o.samplerXSamples = xsV[0];
  o.samplerYSamples = ysV[0];
}

//void PBRTParser::Scale(const Statement& s, shared_ptr <Camera>& c, shared_ptr <Options>& o) {
HDEF(Scale) {
  auto v = getParamVec <Real> (s.params, "anon");
  if (v.size() == 3)
    currentTransform = glm::scale(currentTransform, Vec3(v[0], v[1], v[2]));
  else
    cout << "Error: Scale: supplied vec has size " << v.size() << endl;
  // cout << "Post scale transform: " << endl << currentTransform << endl;
}

HDEF(Shape) {
  
  if (s.type == "cone") {
  }
  else if (s.type == "cylinder") {
  }
  else if (s.type == "sphere") {
    auto rV = getParamVec <Real> (s.params, "radius");
    if (rV.size() == 0) rV.push_back(1);
    if (currentAreaLightMaterial != nullptr) {
      primitives.push_back(make_shared <TransformedPrimitive>(currentTransform, make_shared <Sphere> (rV[0], currentAreaLightMaterial)));
    }
    else {
      primitives.push_back(make_shared <TransformedPrimitive>(currentTransform, make_shared <Sphere> (rV[0], currentMaterial)));
    }      
  }
  else if (s.type == "trianglemesh") {
    auto vV = getParamVec <Vec3> (s.params, "P");
    if (vV.size() == 0) {
      cerr << "Error: no vertices specified in Mesh." << endl;
      exit(0);
    }

    auto iV = getParamVec <int> (s.params, "indices");
    if (iV.size() == 0) {
      cerr << "Error: no indices specified in Mesh." << endl;
      exit (0);
    }

    bool xn = true; // explicit normals
    auto nV = getParamVec <Vec3> (s.params, "N");
    if (nV.size() == 0) xn = false;
    auto uvV = getParamVec <Real> (s.params, "uv");
    
    vector <Vec4> vertices; // my Mesh class expects Vec4
    vertices.reserve(iV.size());

    vector <Vec3> texCoords;
    texCoords.reserve(uvV.size() / 2);
    
    for (auto v : vV)
      vertices.push_back(Vec4(v, 1.0));

    for (int i = 0; i < uvV.size(); i += 2)
      texCoords.push_back(Vec3(uvV[i], uvV[i+1], 0));

    vector <Tri> tris;
    tris.reserve(iV.size() / 3);
    
    for (int i = 0; i < iV.size(); i += 3) {
      tris.push_back(Tri 
        {
         {iV[i], iV[i+1], iV[i+2]}, // vertex indices
         {iV[i], iV[i+1], iV[i+2]}, // texCoord indices
         {iV[i], iV[i+1], iV[i+2]}, // normal indices
         0,
         0,
         xn
        });
        
    }

    // cout << "Normals size: " << nV.size() << endl;
    
    if (!xn) {
      for (auto& t : tris) {
        Vec3 e1 = vV[t.v[1]] - vV[t.v[0]];
        Vec3 e2 = vV[t.v[2]] - vV[t.v[1]];
        nV.push_back(glm::normalize(glm::cross(e1, e2)));
        // cout << "e1: " << e1 << "\te2: " << e2 << "\tN: " << glm::normalize(glm::cross(e1, e2)) << endl;
        t.vn[0] = t.vn[1] = t.vn[2] = nV.size() - 1;
      }
    }
    // cout << "Normals size: " << nV.size() << endl;

    vector <shared_ptr <Material>> foo;
    foo.push_back(currentMaterial);
    shared_ptr <Mesh> mptr = make_shared <Mesh> (move(vertices), move(texCoords), move(nV), move(foo), move(tris));
    shared_ptr <MeshInstance> miptr;
    if (currentAreaLightMaterial != nullptr) {
      miptr = make_shared <MeshInstance> (mptr, currentTransform, currentAreaLightMaterial);
    }
    else {
      miptr = make_shared <MeshInstance> (mptr, currentTransform, currentMaterial);
    }
    vector <shared_ptr <Primitive>> meshPrims = miptr->toPrimitives();
    // for (auto p : meshPrims)
      // cout << *(p.get());
    primitives.insert(primitives.end(), meshPrims.begin(), meshPrims.end());
    
  }
  else if (s.type == "plymesh") {
    auto fn = getParamVec <string> (s.params, "filename");
    vector <Vec4> vertices;
    vector <Vec3> normals;
    vector <Vec3> texCoords;
    vector <Tri> tris;
    if (!parsePLY(fn[0], vertices, normals, texCoords, tris)) {
      cout << "Fatal error parsing PLY file. " << endl;
      exit(0);
    }

    if (normals.size() == 0) {
      for (auto& t : tris) {
        Vec3 e1 = vertices[t.v[1]] - vertices[t.v[0]];
        Vec3 e2 = vertices[t.v[2]] - vertices[t.v[1]];
        normals.push_back(glm::normalize(glm::cross(e1, e2)));
        //        cout << "e1: " << e1 << "\te2: " << e2 << "\tN: " << glm::normalize(glm::cross(e1, e2)) << endl;
        t.vn[0] = t.vn[1] = t.vn[2] = normals.size() - 1;
      }
    }

    vector <shared_ptr <Material>> foo;
    foo.push_back(currentMaterial);
    shared_ptr <Mesh> mptr = make_shared <Mesh> (move(vertices), move(texCoords), move(normals), move(foo), move(tris));
    shared_ptr <MeshInstance> miptr;
    if (currentAreaLightMaterial != nullptr) {
      miptr = make_shared <MeshInstance> (mptr, currentTransform, currentAreaLightMaterial);
    }
    else {
      miptr = make_shared <MeshInstance> (mptr, currentTransform, currentMaterial);
    }
    vector <shared_ptr <Primitive>> meshPrims = miptr->toPrimitives();
    //    for (auto p : meshPrims)
    //  cout << *(p.get());
    primitives.insert(primitives.end(), meshPrims.begin(), meshPrims.end());
    
  }
  else {
    cerr << "Error: unsupported shape type: " << s.type << endl;
  }


}

//void PBRTParser::Transform(const Statement& s, shared_ptr <Camera>& c, shared_ptr <Options>& o) {
HDEF(Transform) {
  auto v = getParamVec <Real> (s.params, "anon");
  if (v.size()  == 16) {
    currentTransform = Mat4(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
    //    currentTransform = Mat4(v[0], v[4], v[8], v[12], v[1], v[5], v[9], v[13], v[2], v[6], v[10], v[14], v[3], v[7], v[11], v[15]);
  }
  else
    cerr << "Error: Transform: supplied vec has size " << v.size() << endl;

}

//void PBRTParser::Translate(const Statement& s, shared_ptr <Camera>& c, shared_ptr <Options>& o) {
HDEF(Translate) {
  auto v = getParamVec <Real> (s.params, "anon");
  if (v.size() == 3)
    currentTransform = glm::translate(currentTransform, Vec3(v[0], v[1], v[2]));
  else
    cerr << "Error: Translate: supplied vec has size " << v.size() << endl;

}

//void PBRTParser::WorldBegin(const Statement& s, shared_ptr <Camera>& c, shared_ptr <Options>& o) {
HDEF(WorldBegin) {
  transformStack = {};
  currentTransform = Mat4(1.0);
}

//void PBRTParser::WorldEnd(const Statement& s, shared_ptr <Camera>& c, shared_ptr <Options>& o) {
HDEF(WorldEnd) {
  // cout << "Hello from handleWorldEnd" << endl;
}


bool readFile(fs::path path, string& result)
{
    // Open the stream to 'lock' the file.
    std::ifstream f(path, std::ios::in | std::ios::binary);

    // Obtain the size of the file.
    const auto sz = fs::file_size(path);

    // Create a buffer.
    result = string(sz, '\0');

    // Read the whole file into the buffer.
    f.read(result.data(), sz);

    if (result.size() > 0) return true;
    return false;
}


std::ostream& operator<<(std::ostream& os, const Statement& s) {
  os << s.identifier << " " << s.type << endl;

  auto print = [](const auto& pv) {
                 for (auto val : pv) cout << val << " ";
               };
  for (auto p : s.params) {
    cout << "\t" << p.first << ": ";
    visit(print, p.second);
    cout << endl;
  }
  return os;
}


bool skipToNext(istringstream& iss) {
  //  cout << "Skipping: ";
  while (iss.good()) {
    char c = iss.get();
    cout << (int) c << " ";
    if (!isspace(c)) { // }c != ' ' && c != '\n' && c != '\t') {
      //cout << "Stopped skipping at: " << (int) c << endl;
      iss.unget();
      return true;
    }
    // cout << "Skipped: " << (char) c << "\t";
  }
  return false;
}

string getStringBetweenBrackets(istringstream& iss) {
  string ans = "";
  if (iss.peek() != '[') {
    //cout << "Error: no brackets." << endl;
    return ans;
  }
 
  iss.get();
  int sp = iss.tellg();
  skipToNext(iss);
  if (iss.peek() == '\"') {
    iss >> quoted(ans);
    iss.ignore(4096, ']');
    iss.get();
  }
  else {
    iss.seekg(sp);
    
    while (iss.peek() != ']') {
      ans.push_back(iss.get());
    }
    iss.get();
  }
  return ans;
}

vector <string> tokenize(string& input) {
  istringstream iss(input);
  vector<string> tokens{istream_iterator<string>{iss},
                        istream_iterator<string>{}};
  for (auto& t : tokens) 
    t.erase(remove(t.begin(), t.end(), '\"'), t.end());

  return tokens;
}

ParamVec tokenizeParamData(string& paramType, string& paramData) {
  //  ParamVec ans;
  //  cout << endl << "Tokenize:" << endl;
  //  cout << "\t" << "Input: " << paramData << endl;
  vector <string> tokens = tokenize(paramData);
  //  cout << "\tTokens: " << endl << "\t\t";
  //  for (auto t : tokens)
  //   cout << t << ", ";
  //  cout << endl;
  if (paramType == "string" || paramType =="texture") {
    vector <string> tmp;
    tmp.reserve(tokens.size());
    for (auto t : tokens)
      tmp.push_back(t);
    return tmp;
  }
  else if (paramType.compare("bool") == 0) {
    vector <bool> tmp;
    tmp.reserve(tokens.size());
    for (auto t : tokens) {
      tmp.push_back(t.compare("true") == 0 ? true : false);
    }
    return tmp;
  }
  else if (paramType.compare("integer") == 0) {
    vector <int> tmp;
    tmp.reserve(tokens.size());
    for (auto t : tokens) {
      tmp.push_back(stoi(t));
    }
    return tmp;
  }
  else if (paramType.compare("float") == 0) {
    vector <Real> tmp;
    tmp.reserve(tokens.size());
    for (auto t : tokens) {
      tmp.push_back((Real) stof(t));
    }
    /*
    cout << "\tParamvec: ";
    for (auto r : tmp)
      cout << r << " ";
    cout << endl;
    */
    return tmp;
  }
  else if (paramType.compare("point2") == 0 || paramType.compare("vector2") == 0) {
    vector <Vec2> tmp;
    if (tokens.size() % 2 != 0) {
      cout << "Error: bad number of values for Vec2 initialization: " << tokens.size() << endl;
      return tmp;
    }
    tmp.reserve(tokens.size());
    for (int i = 0; i < tokens.size(); i +=2) { 
      tmp.push_back(Vec2((Real)stof(tokens[i]), (Real)stof(tokens[i+1])));
    }
    /*
    cout << "\tParamvec: ";
    for (auto r : tmp)
      cout << r << " ";
    cout << endl;
    */
    return tmp;
  }

  else if (paramType.compare("point3") == 0 || paramType.compare("vector3") == 0 ||
           paramType.compare("point")  == 0 || paramType.compare("vector")  == 0 ||
           paramType.compare("normal") == 0 || paramType.compare("normal3") == 0) {
    vector <Vec3> tmp;
    if (tokens.size() % 3 != 0) {
      cout << "Error: bad number of values for Vec3 initialization: " << tokens.size() << endl;
      return tmp;
    }
    tmp.reserve(tokens.size());
    for (int i = 0; i < tokens.size(); i +=3) { 
      tmp.push_back(Vec3((Real)stof(tokens[i]), (Real)stof(tokens[i+1]), (Real)stof(tokens[i+2])));
    }
    /*
    cout << "\tParamvec: ";
    for (auto r : tmp)
      cout << r << " ";
    cout << endl;
    */
    return tmp;
  }
  else if (paramType.compare("spectrum") == 0 || paramType.compare("rgb") == 0 ||
           paramType.compare("color") == 0) {
    vector <Color> tmp;
    if (tokens.size() % 3 != 0) {
      cout << "Error: bad number of values for Color initialization: " << tokens.size() << endl;
      return tmp;
    }
    tmp.reserve(tokens.size());
    for (int i = 0; i < tokens.size(); i +=3) { 
      tmp.push_back(Color((Real)stof(tokens[i]), (Real)stof(tokens[i+1]), (Real)stof(tokens[i+2])));
    }
    /*
    cout << "\tParamvec: ";
    for (auto r : tmp)
      cout << r << " ";
    cout << endl;
    */
    return tmp;
  }
  else {
    cout << "Error: Bad param type: " << paramType << endl;
    return ParamVec{};
  }
}

void reverseBuffer(char* buffer, int size) {
  for (int i = 0; i < size/2; i++) {
    char tmp = buffer[i];
    buffer[i] = buffer[size-i-1];
    buffer[size-i-1] = tmp;
  }  
}

double bufferToDouble(char* buffer, bool littleEndian) {
  // always compiling on little endian architecture
  if (!littleEndian) {
    reverseBuffer(buffer, 8);
  }
  double ans;
  memcpy(&ans, buffer, sizeof(double));
  return ans;
}

float bufferToFloat(char* buffer, bool littleEndian) {
  if (!littleEndian) {
    reverseBuffer(buffer, 4);
  }
  float ans;
  memcpy(&ans, buffer, sizeof(float));
  return ans;
}

int bufferToInt(char* buffer, int size, bool littleEndian) {
  if (!littleEndian && size > 1) {
    reverseBuffer(buffer, size);
  }
  if (size == 1) {
    return int((unsigned char)buffer[0]);
  }
  else if (size == 2) {
    return int((unsigned char) buffer[1] << 8 |
               (unsigned char) buffer[0]);
  }
  else if (size == 4) {
    return int((unsigned char) buffer[3] << 24 |
               (unsigned char) buffer[2] << 16 |
               (unsigned char) buffer[1] << 8  |
               (unsigned char) buffer[0]);
  }
  else return -1;
}

bool PBRTParser::parsePLY(const std::string& fileName, std::vector <Vec4> &vertices, std::vector <Vec3> &normals, std::vector <Vec3> &texCoords, std::vector <Tri> &tris) {
  // read file
  string contents, line;
  if (!readFile(fs::path(fileName), contents)) {
    cout << "Error: unable to open ply file: " << fileName << endl;
    return false;
  }
  istringstream iss{contents};
  iss >> line;
  if (line != "ply") {
    cout << "Error: not a .ply file." << endl;
    return false;
  }
  else {
    cout << "Parsing PLY file..." << endl;
  }
  vector <string> tokens;
  int numVertices = 0, numFaces = 0;
  int nOffset = 0, tcOffset = 0;
  bool hasNormals = false, hasTexCoords = false, binaryMode = true, littleEndian = true;
  string vType = string("none"), nType = string("none"), tcType = string("none");
  string fSizeType, fIndexType;
  while (iss.good()) {
    getline(iss, line);
    tokens = tokenize(line);
    if (tokens.size() == 0) continue;
    
    if (tokens[0] == "comment") continue;
    else if (tokens[0] == "format") {
      if (tokens[1] == "ascii") {
        binaryMode = false;
      }
      else if (tokens[1] == "binary_little_endian") {
        binaryMode = true;
        littleEndian = true;
      }
      else if (tokens[1] == "binary_big_endian") {
        binaryMode = true;
        littleEndian = false;
      }
    }
    else if (tokens[0] == "element") {
      if (tokens[1] == "vertex") {
        numVertices = stoi(tokens[2]);
        vertices.reserve(numVertices);
      }
      else if (tokens[1] == "face") {
        numFaces = stoi(tokens[2]);
        tris.reserve(numFaces);
      }
    }
    else if (tokens[0] == "property") {
      string &pn = tokens[tokens.size()-1];
      if (pn == "x") {
        vType = tokens[1];
      }
      else if (pn == "nx") {
        nType = tokens[1];
        hasNormals = true;
      }
      else if (pn == "u") {
        tcType = tokens[1];
        hasTexCoords = true;
      }
      else if (pn == "vertex_index" || pn == "vertex_indices") {
        if (tokens[1] != "list") {
          cout << "Error reading PLY file." << endl;
          return false;
        }
        fSizeType = tokens[2];
        fIndexType = tokens[3];
      }
    }
    else if (tokens[0] == "end_header") {
      break;
    }
  }

  // cout << "Vertices: " << numVertices << endl;
  // cout << "Faces: " << numFaces << endl;
  // cout << "Has normals? " << hasNormals << endl;
  // cout << "Has texCoords? " << hasTexCoords << endl;
  // cout << "Little endian? " << littleEndian << endl;
  if (binaryMode) {
    // calculate size of each vertex
    int vertexSize;
    if (vType == "char" || vType == "uchar" || vType == "int8" || vType == "uint8") {
      vertexSize = 1 * 3;
    }
    else if (vType == "short" || vType == "ushort" || vType == "int16" || vType == "uint16") {
      vertexSize = 2 * 3;      
    }
    else if (vType == "int" || vType == "uint" || vType == "float" || vType == "int32" || vType == "uint32" || vType == "float32") {
      vertexSize = 4 * 3;
    }
    else {
      vertexSize = 8 * 3;
    }
    nOffset = vertexSize;
    if (hasNormals) {
      if (nType == "char" || nType == "uchar" || nType == "int8" || nType == "uint8") {
        vertexSize += 1 * 3;
      }
      else if (nType == "short" || nType == "ushort" || nType == "int16" || nType == "uint16") {
        vertexSize += 2 * 3;
      }
      else if (nType == "int" || nType == "uint" || nType == "float" || nType == "int32" || nType == "uint32" || nType == "float32") {
        vertexSize += 4 * 3;
      }
      else {
        vertexSize += 8 * 3;
      }
    }
    tcOffset = vertexSize;
    if (hasTexCoords) {
      if (tcType == "char" || tcType == "uchar" || tcType == "int8" || tcType == "uint8") {
        vertexSize += 1 * 2;
      }
      else if (tcType == "short" || tcType == "ushort" || tcType == "int16" || tcType == "uint16") {
        vertexSize += 2 * 2;
      }
      else if (tcType == "int" || tcType == "uint" || tcType == "float" || tcType == "int32" || tcType == "uint32" || tcType == "float32") {
        vertexSize += 4 * 2;
      }
      else {
        vertexSize += 8 * 2;
      }
    }
      
    int vertexDataSize = numVertices * vertexSize;
    char vertexDataBuffer[vertexDataSize];
    iss.read(vertexDataBuffer, vertexDataSize);
    if (!iss) {
      cout << "Error: only " << iss.gcount() << " bytes read." << endl;
    }
    else {
      // cout << "Success. " << iss.gcount() << " of "<< vertexDataSize << " bytes read." << endl;
    }

    // now process vertices, normals, texCoords
    
    for (int i = 0; i < vertexDataSize; i+=vertexSize) {
      if (vType == "double" || vType == "float64") {
        double x, y, z;
        x = bufferToDouble(vertexDataBuffer+i, littleEndian);
        y = bufferToDouble(vertexDataBuffer+i+8, littleEndian);
        z = bufferToDouble(vertexDataBuffer+i+16, littleEndian);
        vertices.emplace_back(x, y, z, 1);
      }
      else if (vType == "float" || vType == "float32") {
        float x, y, z;
        x = bufferToFloat(vertexDataBuffer+i, littleEndian);
        y = bufferToFloat(vertexDataBuffer+i+4, littleEndian);
        z = bufferToFloat(vertexDataBuffer+i+8, littleEndian);
        vertices.emplace_back(x, y, z, 1);
        // cout << "x: " << x << "  y: " << y << "  z:" << z << endl;
      }
      else {
        cout << "Error: only float or double vertices allowed at this time." << endl;
        return false;
      }
      if (hasNormals) {
        if (nType == "double" || nType == "float64") {
          double x, y, z;
          x = bufferToDouble(vertexDataBuffer+i+nOffset, littleEndian);
          y = bufferToDouble(vertexDataBuffer+i+nOffset+8, littleEndian);
          z = bufferToDouble(vertexDataBuffer+i+nOffset+16, littleEndian);
          normals.emplace_back(x, y, z);
        }
        else if (nType == "float" || nType == "float32") {
          float x, y, z;
          x = bufferToFloat(vertexDataBuffer+i+nOffset, littleEndian);
          y = bufferToFloat(vertexDataBuffer+i+nOffset+4, littleEndian);
          z = bufferToFloat(vertexDataBuffer+i+nOffset+8, littleEndian);
          normals.emplace_back(x, y, z);
        }
      }
      if (hasTexCoords) {
        if (tcType == "double" || tcType == "float64") {
          double u, v;
          u = bufferToDouble(vertexDataBuffer+i+tcOffset, littleEndian);
          v = bufferToDouble(vertexDataBuffer+i+tcOffset+8, littleEndian);
          texCoords.emplace_back(u, v, 0);
        }
        else if (tcType == "float" || tcType == "float32") {
          float u, v;
          u = bufferToFloat(vertexDataBuffer+i+tcOffset, littleEndian);
          v = bufferToFloat(vertexDataBuffer+i+tcOffset+4, littleEndian);
          texCoords.emplace_back(u, v, 0);
        }
      }
      
    }

    // cout << "Found " << vertices.size() << " vertices, " << normals.size() << "normals, and " << texCoords.size() << " texture coordinates." << endl;
   
    int fSizeTypeSize, fIndexTypeSize;
    // account for size of type of number indicating number of verts in face
    if (fSizeType == "char" || fSizeType == "uchar" || fSizeType == "int8" || fSizeType == "uint8") {
      fSizeTypeSize = 1;
    }
    else if (fSizeType == "short" || fSizeType == "ushort" || fSizeType == "int16" || fSizeType == "uint16") {
      fSizeTypeSize = 2;
    }
    else if (fSizeType == "int" || fSizeType == "uint" || fSizeType == "float" || fSizeType == "int32" || fSizeType == "uint32" || fSizeType == "float32") {
      fSizeTypeSize = 4;
    }
    else {
      fSizeTypeSize = 8;
    }

    // account for size of type of number 
    if (fIndexType == "char" || fIndexType == "uchar" || fIndexType == "int8" || fIndexType == "uint8") {
      fIndexTypeSize = 1;
    }
    else if (fIndexType == "short" || fIndexType == "ushort" || fIndexType == "int16" || fIndexType == "uint16") {
      fIndexTypeSize = 2;
    }
    else if (fIndexType == "int" || fIndexType == "uint" || fIndexType == "int32" || fIndexType == "uint32") {
      fIndexTypeSize = 4;
    }
    else {
      fIndexTypeSize = 8;
    }

    // faces can have variable size, so need to be read one at a time. might as well process
    // them at the same time!
    
    char faceBuffer[1024]; // plenty of space
    char fSizeBuffer[8];
    for(int i = 0; i < numFaces && iss.good(); i++) {
      int faceStart = iss.tellg();
      // faces are a <fSizeType> followed by that many <fIndexSizeType>s
      iss.read(fSizeBuffer, fSizeTypeSize);
      //      cout << "fSizeBuffer: " << (unsigned int)fSizeBuffer[0] << " fSizeTypeSize: " << fSizeTypeSize << endl;
      int numFaceVerts = bufferToInt(fSizeBuffer, fSizeTypeSize, littleEndian);
      if (numFaceVerts != 3) {
        cout << "Error: found " << numFaceVerts << " polygon vertices. Only triangles supported in PLY files at this time." << endl;
        return false;
      }
      //      iss.seekg(faceStart);
      iss.read(faceBuffer, numFaceVerts * fIndexTypeSize);
      int indices[numFaceVerts];
      int ii = 0;
      for (int i = 0; i < numFaceVerts*fIndexTypeSize; i+=fIndexTypeSize) {
        indices[ii++] = bufferToInt(faceBuffer+i, fIndexTypeSize, littleEndian);
      }
      tris.push_back(
                     Tri {
                          {indices[0], indices[1], indices[2]},
                          {indices[0], indices[1], indices[2]},
                          {indices[0], indices[1], indices[2]},
                          0,
                          0,
                          hasNormals
                     });
    }
    // cout << "Found " << tris.size() << " triangles." << endl;
  }
  else {
    // process ascii
    for (int i = 0; i < numVertices; i++) {
      double x, y, z, nx, ny, nz, u, v;
      iss >> x >> y >> z;
      vertices.emplace_back(x, y, z, 1);
      if (hasNormals) {
        iss >> nx >> ny >> nz;
        normals.emplace_back(nx, ny, nz);
      }
      if (hasTexCoords) {
        iss >> u >> v;
        texCoords.emplace_back(u, v, 0);
      }      
    }

    for (int i = 0; i < numFaces; i++) {
      int numFaceVerts;
      iss >> numFaceVerts;
      if (numFaceVerts != 3) {
        cout << "Error: only triangles supported at this time. " << endl;
        return false;
      }
      int indices[numFaceVerts];
      for (int j = 0; j < numFaceVerts; j++) {
        iss >> indices[j];
      }
      tris.push_back(
                     Tri {
                          {indices[0], indices[1], indices[2]},
                          {indices[0], indices[1], indices[2]},
                          {indices[0], indices[1], indices[2]},
                          0,
                          0,
                          hasNormals
                     });
    }
  }
  return true;
}



void PBRTParser::parseStatements(vector <Statement>& statements, Options& options) {
  for (auto& s : statements) {
    //cout << "Parsing statement: " << s << endl;
    auto h = handlers.find(s.identifier);
    if (h != handlers.end())
      h->second(s, options);      
  }
}


unique_ptr <Scene> PBRTParser::parse(string fileName, Options& options) {
  // Graphics state stacks
    
  string contents;
  
  if (!readFile(fs::path(fileName), contents)) {
    cout << "Error opening file." << endl;
    return nullptr;
  }
  for (int i = 0; i < contents.size(); i++) {
    if (contents[i] == '#') {
      while (contents[i] != '\n') {
        contents[i++] = ' ';
      }
    }
  }
  // cout << contents << endl;
  istringstream iss (contents);
  Statement statement;
  vector <Statement> statements;
  State state = EXPECT_IDENTIFIER;
  map <string, int> fixedParamsMap =
    {
     {"Identity", 0},
     {"Translate", 3},
     {"Scale", 3},
     {"Rotate", 4},
     {"LookAt", 9},
     {"Transform", 16},
     {"ConcatTransform", 16},
     {"WorldBegin", 0},
     {"WorldEnd", 0},
     {"AttributeBegin", 0},
     {"AttributeEnd", 0},
     {"ObjectEnd", 0}
    };
  
  while (iss.good()) {
    // identifier "type" parameter-list
    // Param type is std::pair <string, variant>
    // discard junk chars (spaces, comments)

    // Maybe we want to just replace all comments with spaces before starting this?
    skipToNext(iss);
    cout << "Next iss char is: " << (char) iss.peek() << endl;
    switch (state) {
    case EXPECT_IDENTIFIER:
      {
        // read string into statement.identifier
        iss >> statement.identifier;
        cout << "Got identifier: " << statement.identifier << endl;

        // Texture requires special handling, apparently
        if (statement.identifier == "Texture") {
			string textureName, textureType, textureClass;
			iss >> quoted(textureName) >> quoted(textureType) >> quoted(textureClass);
			vector <string> texInfo = {textureName, textureType, textureClass};
			statement.params.insert({"texInfo", texInfo});
			for (auto s : texInfo)
				cout << "[" << s << "]" << endl;
	//		cout << "[" << (char) iss.peek() << "]" << endl; 
			state = EXPECT_PARAM;
			break;
        }
        auto it = fixedParamsMap.find(statement.identifier);
        if (it != fixedParamsMap.end()) {
          /*
          Real r;
          vector <Real> vals;
          for (int i = 0; i < it->second; ++i) {
            iss >> r;
            vals.push_back(r);
            cout << "Read val: " << r << " ";
          }
          cout << endl;
          statement.params.push_back({"blank", vals});
          */
          string paramData;
          statement.type = "";
          //          statement.name = "blank";
          skipToNext(iss);
          if (iss.peek() == '[') {
            paramData = getStringBetweenBrackets(iss);
          }
          else {
            // get string without brackets
            for (int i = 0; i < it->second; i++) {
              string s;
              iss >> s;
              paramData.append(s);
              paramData.push_back(' ');
            }
          }
          //cout << "Got paramData: " << paramData << endl;

          // now tokenize paramData and convert to the right type
          string paramType = "float";
          statement.params.insert({"anon", tokenizeParamData(paramType, paramData)});
          statements.push_back(statement);
          statement = {};
          state = EXPECT_IDENTIFIER;
        }
        else {
          state = EXPECT_TYPE;
        }
        // if (in fixed floats map) {
        //   get number of floats to read
        //   read that many floats -- includes statements with 0 floats (eg WorldBegin)
        //   close statement, push onto vector
        //   remain in EXPECT_IDENTIFIER state
        // }
        // else {
        //   go to EXPECT_TYPE state
        // }
      }
      break;
    case EXPECT_TYPE:
      {
        iss >> quoted(statement.type);
        //  cout << "Got type: " << statement.type << endl;
        state = EXPECT_PARAM;
        // if find quoted string: read into statement.type. go to EXPECT_PARAM
        // else error
      }
      break;
    case EXPECT_PARAM:
      {
        // if not find quoted string: return to EXPECT_IDENTIFIER state
        // start param, get <type> and <name>
        // if '[': read <type> into vec until closing ']'
        // else if single <type>: read one into param vec
        // return to EXPECT_IDENTIFIER state
        if (iss.peek() != '\"') {
          state = EXPECT_IDENTIFIER;
          statements.push_back(statement);
          statement = {};
          break;
        }
        string paramInfo, paramName, paramType, paramData;
      
        iss >> quoted(paramInfo);
        paramType = paramInfo.substr(0, paramInfo.find(" "));
        paramName = paramInfo.substr(paramInfo.rfind(' ')+1, string::npos);
        cout << "From paramInfo: " << paramInfo << "\tgot paramName: " << paramName << "\tand paramType: " << paramType << endl;

        // get param data into a string
        paramData.reserve(4096);

        skipToNext(iss);
        if (iss.peek() == '['){
          paramData = getStringBetweenBrackets(iss);
        }    
        else {
          iss >> paramData;
        }
        cout << "paramData: " << paramData << endl;
      
        // now tokenize paramData and convert to the right type
    	statement.params.insert({paramName, tokenizeParamData(paramType, paramData)});

        
        // skip over spaces to see if we're getting another param or new identifier
        skipToNext(iss);
        if (iss.peek() == '\"') {
          state = EXPECT_PARAM;
        }
        else {
          statements.push_back(statement);
          statement = {};
          state = EXPECT_IDENTIFIER;
        }
      }
      break;
    default:
      break;
    }
  }
  // cout << endl << endl;
  // cout << "Extracted statements:" << endl;
  // for (auto s : statements)
    // cout << s << endl;
  // cout << "Parsing... " << endl;
  parseStatements(statements, options);
  return make_unique <Scene> (move(lights), move(materials), move(primitives));
}
