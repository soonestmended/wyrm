#include "PBRTParser.hpp"
#include "Image.hpp"
#include "Material.hpp"

#include <filesystem>
#include <functional>
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
   {"Transform", PBRTParser::Transform},
   {"Translate", PBRTParser::Translate},
   {"WorldBegin", PBRTParser::WorldBegin},
   {"WorldEnd", PBRTParser::WorldEnd}
  };

Mat4 PBRTParser::currentTransform = Mat4(1.0);
shared_ptr <Material> PBRTParser::currentMaterial = make_shared <DiffuseMaterial> ("defaultMaterial", Color::Blue());
shared_ptr <Material> PBRTParser::currentAreaLightMaterial = nullptr;
bool PBRTParser::currentReverseOrientation = false;

stack <Mat4> PBRTParser::transformStack = {};
stack <shared_ptr<Material>> PBRTParser::materialStack = {};
stack <shared_ptr<Material>> PBRTParser::areaLightMaterialStack = {};
stack <bool> PBRTParser::reverseOrientationStack = {};
map <string, shared_ptr <Material>> PBRTParser::namedMaterialMap = {};

vector <shared_ptr <Light>> PBRTParser::lights = {};
vector <shared_ptr <Material>> PBRTParser::materials = {};
vector <shared_ptr <Primitive>> PBRTParser::primitives = {};

HDEF(AreaLightSource) {
  // for now ignores bool twoSided and integer samples
  auto v = getParamVec <Color> (s.params, "L");
  if (v.size() > 0)
    currentAreaLightMaterial = make_shared <DiffuseMaterial>("anon", Color::Black(), v[0]);
  else
    currentAreaLightMaterial = make_shared <DiffuseMaterial>("anon", Color::Black(), Color::White());
    
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
    cerr << "Error: LookAt: supplied vec has size " << v.size() << endl;
  cout << "Post LookAt currentTransform: " << endl << currentTransform << endl;
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
  cout << "right: " << Vec3(currentTransform[0]) << endl;
  cout << "up: " << Vec3(currentTransform[1]) << endl;
  cout << "look: " << Vec3(currentTransform[2]) << endl;
  cout << "eye: " << o.camera2world * Vec4(0, 0, 0, 1) << endl;

}

shared_ptr <Material> PBRTParser::makeMaterial(const Statement& s) {
  shared_ptr <Material> ans = nullptr;
  auto nameV = getParamVec <string> (s.params, "name");
  if (nameV.size() == 0) nameV.push_back("unnamed_" + s.type);
  if (s.type.compare("glass") == 0) { // GlassMaterial
    auto KrV = getParamVec <Real> (s.params, "Kr");
    if (KrV.size() == 0) KrV.push_back(1);
    auto KtV = getParamVec <Real> (s.params, "Kt");
    if (KtV.size() == 0) KtV.push_back(1);
    auto etaV = getParamVec <Real> (s.params, "eta");
    if (etaV.size() == 0) etaV.push_back(1.5);
    
    // uroughness
    // vroughness
    // remaproughness
    //    Color Kr = KrV[0];
    //    Color Kt = KtV[0];
    ans = make_shared <GlassMaterial> (nameV[0], KrV[0], KtV[0], etaV[0]);
  }

  else if (s.type.compare("matte") == 0) {
    auto sigmaV = getParamVec <Real> (s.params, "sigma");
    if (sigmaV.size() == 0) sigmaV.push_back(0);
    auto KdV = getParamVec <Color> (s.params, "Kd");
    if (KdV.size() == 0) KdV.push_back(Color(0.5));
    if (sigmaV[0] > 0) {
      cerr << "Error: Oren-Nayar model not yet implemented." << endl;
      exit(0);
    }
    ans = make_shared <DiffuseMaterial> (nameV[0], KdV[0]);
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

HDEF(MaterialStatement) {
  currentMaterial = makeMaterial(s);
}

HDEF(NamedMaterial) {
  auto it = namedMaterialMap.find(s.type);
  if (it == namedMaterialMap.end()) {
    cerr << "Error: namedMaterial " << s.type << " not found." << endl;
    exit(0);
  }
  cout << "Called namedMaterial: " << s.type << endl;
  currentMaterial = it->second;
  cout << "Set material to: " << ((DiffuseMaterial*)currentMaterial.get())->lbrdf->color << endl;
}

//void PBRTParser::Rotate(const Statement& s, shared_ptr <Camera>& c, shared_ptr <Options>& o) {
HDEF(Rotate) {
  auto v = getParamVec <Real> (s.params, "anon");
  if (v.size() == 4) 
    currentTransform = glm::rotate(currentTransform, v[0], Vec3(v[1], v[2], v[3]));
  else
    cerr << "Error: Rotate: supplied vec has size " << v.size() << endl;

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
    cerr << "Error: Scale: supplied vec has size " << v.size() << endl;
  cout << "Post scale transform: " << endl << currentTransform << endl;
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
      primitives.push_back(make_shared <Sphere> (rV[0], currentAreaLightMaterial));
    }
    else {
      primitives.push_back(make_shared <Sphere> (rV[0], currentMaterial));
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

    cout << "Normals size: " << nV.size() << endl;
    
    if (!xn) {
      for (auto& t : tris) {
        Vec3 e1 = vV[t.v[1]] - vV[t.v[0]];
        Vec3 e2 = vV[t.v[2]] - vV[t.v[1]];
        nV.push_back(glm::normalize(glm::cross(e1, e2)));
        cout << "e1: " << e1 << "\te2: " << e2 << "\tN: " << glm::normalize(glm::cross(e1, e2)) << endl;
        t.vn[0] = t.vn[1] = t.vn[2] = nV.size() - 1;
      }
    }
    cout << "Normals size: " << nV.size() << endl;

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
    for (auto p : meshPrims)
      cout << *(p.get());
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
  cout << "Hello from handleWorldEnd" << endl;
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
  
  while (iss.good()) {
    char c = iss.get();
    //    cout << "STN got " << (char) c << " ";
    if (c != ' ' && c != '\n') {
      iss.unget();
      return true;
    }
    //cout << "Skipped: " << (char) c << "\t";
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
  if (paramType.compare("string") == 0) {
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
}


void PBRTParser::parseStatements(vector <Statement>& statements, Options& options) {
  for (auto& s : statements) {
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
  cout << contents << endl;
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
    //    cout << "Next iss char is: " << (char) iss.peek() << endl;
    switch (state) {
    case EXPECT_IDENTIFIER:
      {
        // read string into statement.identifier
        iss >> statement.identifier;
        //cout << "Got identifier: " << statement.identifier << endl;
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
        //      cout << "From paramInfo: " << paramInfo << "\tgot paramName: " << paramName << "\tand paramType: " << paramType << endl;

        // get param data into a string
        paramData.reserve(4096);

        skipToNext(iss);
        if (iss.peek() == '['){
          paramData = getStringBetweenBrackets(iss);
        }    
        else {
          iss >> paramData;
        }
        // cout << "paramData: " << paramData << endl;
      
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
  cout << endl << endl;
  cout << "Extracted statements:" << endl;
  for (auto s : statements)
    cout << s << endl;
  cout << "Parsing... " << endl;
  parseStatements(statements, options);
  return make_unique <Scene> (move(lights), move(materials), move(primitives));
}
