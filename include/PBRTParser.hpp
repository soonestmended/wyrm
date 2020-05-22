#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <variant>

#include <boost/algorithm/string.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "common.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Mesh.hpp"
#include "SampleGenerator.hpp"
#include "Scene.hpp"
#include "Tracer.hpp"

#define HDEC(x) static void x(const Statement& s, Options& o)

class Material;
 
typedef std::variant<
  std::vector <std::string>,
  std::vector <bool>,
  std::vector <int>,
  std::vector <Real>,
  std::vector <Vec2>,
  std::vector <Vec3>,
  std::vector <Color>> ParamVec;

struct Options  {
  int imageWidth, imageHeight;
  std::string imageFilename;
  int spp; // samples per pixel
  int nt; // number of threads
  Real maxLuminance;
  int tracerMaxDepth;
  Real rrThreshold;
  std::string tracerType;

  // Camera
  Mat4 world2camera, camera2world;
  Real frameAspectRatio = -1., screenWindow = 1., fov;

  std::string filterType;
  Real filterXWidth, filterYWidth;
  Real filterAlpha = 2.;
  Real filterB = 1. / 3., filterC = 1. / 3., filterTau = 3;

  std::string samplerType;
  bool samplerJitter = true;
  int samplerXSamples = 2, samplerYSamples = 2;

};

enum State {EXPECT_IDENTIFIER, EXPECT_TYPE, EXPECT_PARAM};

struct Statement {
  std::string identifier;
  std::string type;
  std::map <std::string, ParamVec> params;  
};


class PBRTParser {
public:
  
  static std::unique_ptr <Scene> parse(std::string fileName, Options& options);
  //  static void finalizeOptions(Options& o, const Scene& s);
  static void parseStatements(std::vector <Statement>& statements, Options& options);

  template <class T>
  static std::vector <T> getParamVec(const std::map <std::string, ParamVec>& m, const std::string& name) {
    try {
      return std::get<std::vector<T>>(m.at(name));
    } catch  (const std::out_of_range& oor) {
      // std::cerr << "Note: parameter " << name << " not supplied. " << oor.what() << std::endl;
      return std::vector <T> {}; // return empty vector if this parameter wasn't supplied.
    }
  }
  static std::shared_ptr <Material> makeMaterial(const Statement& s);

  
  HDEC(AreaLightSource);
  HDEC(AttributeBegin);
  HDEC(AttributeEnd);
  HDEC(ConcatTransform);
  HDEC(Film);
  HDEC(Identity);
  HDEC(Integrator);
  HDEC(LookAt);
  HDEC(MakeCamera);
  HDEC(MakeNamedMaterial);
  HDEC(MaterialStatement);
  HDEC(NamedMaterial);
  HDEC(PixelFilter);
  HDEC(Rotate);
  HDEC(Sampler);
  HDEC(Scale);
  HDEC(Shape);
  HDEC(Transform);
  HDEC(Translate);
  HDEC(WorldBegin);
  HDEC(WorldEnd);

  static std::map <std::string, std::function <void(const Statement&, Options&)>> handlers;

  // graphics state
  static std::stack <Mat4> transformStack;
  static Mat4 currentTransform;
  static std::stack <std::shared_ptr <Material>> materialStack;
  static std::shared_ptr <Material> currentMaterial;
  static std::stack <std::shared_ptr <Material>> areaLightMaterialStack;
  static std::shared_ptr <Material> currentAreaLightMaterial;
  static std::stack <bool> reverseOrientationStack;
  static bool currentReverseOrientation;

  static std::map <std::string, std::shared_ptr <Material>> namedMaterialMap;

  // vectors from which to make Scene
  static std::vector <std::shared_ptr <Light>> lights;
  static std::vector <std::shared_ptr <Material>> materials;
  static std::vector <std::shared_ptr <Primitive>> primitives;
};

