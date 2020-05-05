#include <memory>
#include <typeinfo>

#include "IntersectRec.hpp"
#include "Material.hpp"
#include "Utils.hpp"


using namespace std;
using namespace utils;

const Color ADMaterial::brdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const {
    Color ans = Color::Black();
    bool sameHemisphere = ir.onb.sameHemisphere(wo_local, wi_local);
    //cout << "\t\topacity: " << opacity << endl;

    Color pathWeight = opacity;
    //cout << "\t\taccumPathWeight: " << pathWeight << endl;

    Color coatPathWeight = pathWeight * coat;
    //cout << "\t\tcoatPathWeight: " << coatPathWeight << endl;
    if (!coatPathWeight.isBlack() && sameHemisphere) {
        ans += coatPathWeight * coat_brdf->f(wo_local, wi_local, isSpecular);
    }
        //cout << "\t\taccumPathWeight: " << pathWeight << endl;

    pathWeight *= lerp(Color::White(), this->coat_color * (Color(1) - coat_brdf_reflectance), this->coat);
    //cout << "\t\taccumPathWeight: " << pathWeight << endl;

    Color metalPathWeight = pathWeight * metalness; 
    // metalness
    //cout << "\t\tmetalPathWeight: " << metalPathWeight << endl;
    //cout << "\t\taccumPathWeight: " << pathWeight << endl;
    if (!metalPathWeight.isBlack() && sameHemisphere)
        ans += metalPathWeight * metal_brdf->f(wo_local, wi_local, isSpecular);

    pathWeight *= 1. - metalness;
    
    Color specularPathWeight = pathWeight * specular;
    //cout << "\t\tspecularPathWeight: " << specularPathWeight << endl;    
    //cout << "\t\taccumPathWeight: " << pathWeight << endl;
    if (!specularPathWeight.isBlack() && sameHemisphere)
        ans += specularPathWeight * specular_brdf->f(wo_local, wi_local, isSpecular);
    
    pathWeight *= Color(1) - this->specular_color * this->specular * specular_brdf_reflectance;
    Color transPathWeight = pathWeight * transmission * transmission_color;
    //cout << "\t\ttransPathWeight: " << transPathWeight << endl;
    //cout << "\t\taccumPathWeight: " << pathWeight << endl;
    
    if (!transPathWeight.isBlack() && !sameHemisphere) 
        ans += transPathWeight * specular_btdf->f(wo_local, wi_local, isSpecular);

    pathWeight *= 1. - transmission;
    //cout << "\t\tdiffusePathWeight: " << pathWeight << endl;

    if (!pathWeight.isBlack() && sameHemisphere) 
        ans += pathWeight * diffuse_brdf->f(wo_local, wi_local, isSpecular);
    
    return ans;
}

const float ADMaterial::pdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir) const {
    // TODO: Compute pdf for having chosen given direction
    // weighted average of pdfs for individual lobes

    // chance of choosing straight through tranmission direction is zero
    float ans = 0.0f;
    float pathProb = avg(opacity);
    float coatProb = pathProb * coat;
    //cout << "coatProb: " << coatProb << endl;
    if (coatProb > 0)
        ans += coatProb * coat_brdf->pdf(wo_local, wi_local);

    pathProb *= 1. - coat;
    float metalProb = pathProb * metalness;
    //cout << "metalProb: " << metalProb << endl;

    if (metalProb > 0) 
        ans += metalProb * metal_brdf->pdf(wo_local, wi_local);

    pathProb *= 1. - metalness;
    float specularProb = pathProb * specular;
    //cout << "specProb: " << specularProb << endl;

    if (specularProb > 0) 
        ans += specularProb * specular_brdf->pdf(wo_local, wi_local);
    
    pathProb *= 1. - specular;
    float transmissionProb = pathProb * transmission;
    //cout << "transProb: " << transmissionProb << endl;

    if (transmissionProb > 0)
        ans += transmissionProb * specular_btdf->pdf(wo_local, wi_local);
    
    pathProb *= 1. - transmission;
    //cout << "pathProb: " << pathProb << endl;
    ans += pathProb * diffuse_brdf->pdf(wo_local, wi_local);
    return ans;

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
    pathWeight *= lerp(Color::White(), this->coat_color * (Color(1) - coat_brdf_reflectance), this->coat);

/*
// emission_specular_mixture = emission * emission_color * emission() + specular_mixture
    r = rand01();
    q = this->emission;
    if (r < q) {
        pathProb *= q;
        *pdf *= pathProb;
        *bsdf = this->emission * this->emission_color * pathWeight;
        // somehow set flags here to indicate emission
        *isSpecular = false;
        return;
    }

    pathProb *= 1.0f - q;
*/
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
        *bsdf *= pathWeight * this->specular;
        cout << "pdf: " << *pdf << endl;
        cout << "pathweight: " << pathWeight << endl;
        cout << "bsdf: " << *bsdf << endl;
        cout << "cosTheta: " << ONB::cosTheta(wo_local) << endl;
        return;       
    }

    pathProb *= 1.0f - q;
    pathWeight *= Color(1) - this->specular_color * this->specular * specular_brdf_reflectance;

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
    *bsdf *= this->base;
    return;
}
