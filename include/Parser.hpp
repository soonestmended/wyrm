#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "common.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"

class Parser {
public:
    /* Construct vec3 from tokens. If there aren't enough tokens, defaultValue is substituted. */
    static const Vec3 readVec3(const std::vector <std::string> &tokens, Real defaultValue, int numTokens);

    /* Construct vec4 from tokens. If there aren't enough tokens, defaultValue is substituted. */
    static const Vec4 readVec4(const std::vector <std::string> &tokens, const Real defaultValue, int numTokens);

    static std::vector <MTLMat> parseMtl(std::string fileName);

    static std::shared_ptr <Mesh> parseObj(std::string fileName);
    static std::unique_ptr <Scene> parseX3D(std::string fileName, std::shared_ptr <Camera>& cam);

};

