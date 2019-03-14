#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Light.hpp"
#include "Material.hpp"
#include "Primitive.hpp"



class Scene {
private:
    std::vector <std::shared_ptr<Light>> lights_;
    std::vector <std::shared_ptr<Material>> materials_;
    std::vector <std::shared_ptr<Primitive>> primitives_;
    static std::vector <MTLMat> parseMtl(std::string fileName);

public:
    Scene(std::vector <std::shared_ptr <Light>> &&lights, std::vector <std::shared_ptr<Material>> &&materials, std::vector <std::shared_ptr <Primitive>> &&primitives) :
    lights_ (std::move(lights)), materials_ (std::move(materials)), primitives_ (std::move(primitives)) {}

    void print();

    static std::unique_ptr <Scene> parseObj(std::string fileName);
    
};