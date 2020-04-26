#include "IntersectRec.hpp"
#include "Material.hpp"
#include "Utils.hpp"

#include <memory>
#include <typeinfo>

using namespace std;
using namespace utils;

const Color ADMaterial::brdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir) const {
    // TODO: Compute brdf for given directions and shading point.
}
const float ADMaterial::pdf(const glm::vec3& wi_local, const IntersectRec& ir) const {
    // TODO: Compute pdf for having chosen given direction 
}

const void ADMaterial::sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const {
    // Run through the Autodesk shader code. Take each path with probability (1 - [accumulated edge weights])
    // OR
    // calculate each lobe of the material and weight appropriately. This would spawn multiple additional rays. I 
    // suppose these should be separate functions.
    Color pathWeight = Color::White(); // accumulated color of path
    float r = rand01();
    float q = avg(this->opacity); 
    float pathProb = q; // accumulated probability of path
    if (r > q) { // follow path for transmission
        *pdf = 1.0f - q;
        *bsdf = Color::White();
        wi_local = -wo_local;
        *isSpecular = false;
        return;
    }

    pathWeight *= this->opacity;

    // Next is coat layer.
    // coat_layer = coat * coat_brdf(...) + lerp(white, coat_color * (1 - reflectance(coat_brdf)), coat) * emission_specular_mixture
    // choose to be reflected by coat with probability m->coat

    r = rand01();
    q = this->coat;

    if (r < q) {
        pathProb *= q;
        coat_brdf->sample_f(wo_local, wi_local, bsdf, pdf, isSpecular);
        *pdf *= pathProb;
        *bsdf *= this->coat * pathWeight;
        return;
    }

    pathProb *= 1.0f - q;
    pathWeight *= lerp(Color::White(), this->coat_color * (glm::vec3(1) - coat_brdf_reflectance), this->coat);


// emission_specular_mixture = emission * emission_color * emission() + specular_mixture
    r = rand01();
    q = this->emission;
    if (r < q) {
        pathProb *= q;
        *pdf *= pathProb;
        *bsdf = this->emission * this->emission_color * pathWeight;
        // somehow set flags here to indicate emission
        return;
    }

    pathProb *= 1.0f - q;

// specular_mixture = metalness * metal_brdf(...) + (1 - metalness) * specular_reflection_layer
    r = rand01();
    q = this->metalness;
    if (r < q) {
        pathProb *= q;
        metal_brdf->sample_f(wo_local, wi_local, bsdf, pdf, isSpecular);
        *pdf *= pathProb;
        *bsdf *= this->metalness * pathWeight;
        return;
    }



    pathProb *= 1.0f - q;
    pathWeight *= 1.0f - this->metalness;

// specular_reflection_layer = specular * specular_color * specular_brdf(...) + 
// (1 - specular_color * specular * reflectance(specular_brdf)) * transmission_sheen_mix
    r = rand01();
    q = this->specular;
    if (r < q) {
        pathProb *= q;
        specular_brdf->sample_f(wo_local, wi_local, bsdf, pdf, isSpecular);
        *pdf *= pathProb;
        *bsdf *= pathWeight * this->specular * this->specular_color;
        return;       
    }

    pathProb *= 1.0f - q;
    pathWeight *= glm::vec3(1) - this->specular_color * this->specular * specular_brdf_reflectance;

// transmission_sheen_mix = transmission * transm_color * specular_btdf(...) + (1 - transmission) * sheen_layer
    r = rand01();
    q = this->transmission;
    if (r < q) {
        pathProb *= q;
        specular_btdf->sample_f(wo_local, wi_local, bsdf, pdf, isSpecular);
        *pdf *= pathProb;
        *bsdf *= this->transmission * this->transmission_color; // depth presumed to be 0
        return;
    }

    pathProb *= 1.0f - q;
    pathWeight *= 1.0f - this->transmission;

// skipping sheen for now
// base_mix = (1 - subsurface) * base * base_color * diffuse_brdf(...) + subsurface * subsurface_mix
    diffuse_brdf->sample_f(wo_local, wi_local, bsdf, pdf, isSpecular);
    *pdf *= pathProb;
    *bsdf *= this->base * this->base_color;
    return;
}
