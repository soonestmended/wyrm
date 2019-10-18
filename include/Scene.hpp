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
    std::vector <std::shared_ptr<Light>> lights;
    std::vector <std::shared_ptr<Material>> materials;
    std::vector <std::shared_ptr<Primitive>> primitives;
    static std::vector <MTLMat> parseMtl(std::string fileName);

public:
    Scene(std::vector <std::shared_ptr <Light>> &&lights_, std::vector <std::shared_ptr<Material>> &&materials_, std::vector <std::shared_ptr <Primitive>> &&primitives_) :
    lights (std::move(lights_)), materials (std::move(materials_)), primitives (std::move(primitives_)) {}

    void printInfo();

    const std::vector <std::shared_ptr<Primitive>> &getPrimitives() const {return primitives;}

    static std::unique_ptr <Scene> parseObj(std::string fileName);
};
