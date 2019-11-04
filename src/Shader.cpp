#include "Material.hpp"
#include "Shader.hpp"

#include <typeinfo>

const Color QuickShader::shade(IntersectRec& ir) const {
    if (typeid(*(ir.material)) == typeid(SimpleMaterial)) {
        SimpleMaterial* sm = (SimpleMaterial*) ir.material.get();
        return sm->color;
    }
    else {
        return Color::Red();
    }
}
