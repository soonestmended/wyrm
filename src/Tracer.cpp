#include "Accelerator.hpp"
#include "common.hpp"
#include "Tracer.hpp"

#define MAX_DEPTH 5

const Color PathTracer::lightAlongRay(const Ray& r) const {
    IntersectRec ir;
    Color pathThroughput = Color(1.0);
    Color ans = Color(0.0);
    glm::vec3 wi_local;
    Color f;
    float pdf;
    bool isSpecular = true, hit;
    Ray nextRay = r;
    for (int i = 0; i < MAX_DEPTH; ++i) {
        hit = accel->closestIntersection(nextRay, EPSILON, POS_INF, ir);
        
        if (isSpecular) {
            if (hit) {
                ans += pathThroughput * ir.material->getEmission() * ir.primitive->getSurfaceArea();
            }
            else {
                // ray escaped scene
                for (const auto& l : scene->getLights()) {
                    ans += pathThroughput * l->LInf(nextRay);
                }
            }
        }

        if (!hit) {
            return ans;
        }
        ir.primitive->finishIntersection(ir);

        glm::vec3 wo_local = ir.onb.world2local(-r.d);
        ans += pathThroughput * estimateDirectLighting(wo_local, ir);
        ir.material->sample_f(wo_local, wi_local, &f, &pdf, &isSpecular);

        nextRay = Ray{ir.isectPoint, ir.onb.local2world(wi_local)};
        pathThroughput *= f * glm::dot(wi_local, ir.shadingNormal) / pdf;

        // possibly stop (Russian roulette)
        if (i > 3) {
            float q = utils::avg(pathThroughput);
            if (utils::rand01() < q) {
                break;
            }
            pathThroughput /= 1.0f - q;
        }

    }
    return ans;
}


const Color Tracer::EDLOneLight(const glm::vec3& wo_world, IntersectRec& ir, const Light& l) const {

    // TODO need wo also

    /*
    * OK, this is going to be done with multiple importance sampling. That means we will:
    * 1) Pick a direction to sample based on the Light
    *   - if not a delta light, compute MIS weight based on pdf of that direction from BRDF
    *   - add contribution
    * 2) Pick a direction to sample based on the BRDF of the point we are shading.
    *   - calculate MIS weight based on pdf of that direction using light distribution
    *   - add contribution with weight
    */

    Color ans = Color::Black();
    Color R;
    bool isSpecular;

    ONB onb{ir.normal};

    float lightPdf, irPdf;
    float weight;
    glm::vec3 wi_world;
    VisibilityTester vt;
    Color f; // BRDF
    // 1) pick direction based on light
    Color l_contrib = l.sample(glm::vec2(rand(), rand()), ir, wi_world, &lightPdf, vt);
    glm::vec3 wi_local = onb.world2local(wi_world);
    glm::vec3 wo_local = onb.world2local(wo_world);
    
    // float cosTheta = glm::clamp(glm::dot(ir.normal, wi_world), 0.f, 1.f);
    float absCosTheta = ONB::absCosTheta(wi_local);
    if (lightPdf > 0.f && !l_contrib.isBlack()) {
        f = ir.material->brdf(wo_local, wi_local, ir);
        if (!f.isBlack() && vt.testVisibile(*this->accel)) {
            if (l.isDeltaLight()) {
                ans += f * l_contrib * (absCosTheta / lightPdf);
            }
            else {
                irPdf = ir.material->pdf(wi_local, ir);
                weight = utils::powerHeuristic(1, lightPdf, 1, irPdf);
                ans += f * l_contrib * (absCosTheta * weight / lightPdf);
            }
        }
    }

    // 2) pick direction based on material brdf
    if (!l.isDeltaLight()) {
        ir.material->sample_f(wo_local, wi_local, &f, &irPdf, &isSpecular);
        if (!f.isBlack() && irPdf > 0.f) {
            if (!isSpecular) {
                weight = 1.f;
                lightPdf = l.pdf(ir, wi_local);
                if (lightPdf == 0.f) return ans; // no probability of choosing this ray given light distribution
                weight = utils::powerHeuristic(1, irPdf, 1, lightPdf);
            }
            l_contrib = Color::Black();
            Ray rayToLight{ir.isectPoint, wi_world};
            IntersectRec tempIr;
            if (accel->closestIntersection(rayToLight, .0001, MAXFLOAT, tempIr)) {
                if (l.getPrim() == tempIr.primitive) {
                    l_contrib = l.getColor() * l.getPower();
                }
            }
            else {
                l_contrib = l.LInf(rayToLight); // account for infinite light
            }
            if (!l_contrib.isBlack()) {
                ans += f * l_contrib * absCosTheta * weight / irPdf; 
            }
        }
    }
    return ans;
    //return Color::White();
}

const Color Tracer::estimateDirectLighting(const glm::vec3& wo_world, IntersectRec& ir) const {
    Color ans{0.0};
    for (const auto& l : scene->getLights()) {
        ans += EDLOneLight(wo_world, ir, *l);
    }
    return ans;
}