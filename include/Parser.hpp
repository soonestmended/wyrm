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

#include "Color.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"

class Parser {
private:
    /* Construct vec3 from tokens. If there aren't enough tokens, defaultValue is substituted. */
    static const glm::vec3 readVec3(const std::vector <std::string> &tokens, float defaultValue, int numTokens);

    /* Construct vec4 from tokens. If there aren't enough tokens, defaultValue is substituted. */
    static const glm::vec4 readVec4(const std::vector <std::string> &tokens, const float defaultValue, int numTokens);

    static std::vector <MTLMat> parseMtl(std::string fileName);

public:
    static std::shared_ptr <Mesh> parseObj(std::string fileName);
    static std::unique_ptr <Scene> parseX3D(std::string fileName);

};

