#include "Material.hpp"
#include "Shader.hpp"

#include <memory>
#include <typeinfo>

using namespace std;

const Color QuickShader::shade(IntersectRec& ir) const {
    if (typeid(*(ir.material)) == typeid(SimpleMaterial)) {
        SimpleMaterial* sm = (SimpleMaterial*) ir.material.get();
        return sm->color;
    }
    else {
        return Color::Red();
    }
}

const Color DirectLightingShader::shade(IntersectRec& ir) const {
    Color ans = Color::Black();
    Color R;
    if (typeid(*(ir.material)) == typeid(SimpleMaterial)) {
        SimpleMaterial* sm = (SimpleMaterial*) ir.material.get();
        R = sm->color;
    }
    
    else {
        R = Color(.5);
    }

    for (shared_ptr <Light> l : scene.getLights()) {
        float pdf;
        glm::vec3 wi_world;
        VisibilityTester vt;
        Color l_contrib = l->sample(glm::vec2(rand(), rand()), ir, wi_world, pdf, vt);
        float cosTheta = glm::clamp(glm::dot(ir.normal, wi_world), 0.f, 1.f);

        ans += R * l_contrib * cosTheta;

        //ans += R * glm::dot(ir.normal, wi_world);
        //ans += ir.normal;
        //cout << glm::to_string(ir.normal) << endl;
    }
    return ans;
}

void DirectLightingShader::build() {
    // assume Scene keeps track of all the lights.

}