#include "Accelerator.hpp"
#include "common.hpp"
#include "Tracer.hpp"

#define MAX_DEPTH 5

using namespace std;

const Color PathTracer::lightAlongRay(const Ray& r) const {
    //cout << "lightAlongRay entry" << endl;
    IntersectRec ir;
    Color pathThroughput = Color(1.0);
    Color ans = Color(0.0);
    glm::vec3 wi_local;
    Color f;
    float pdf;
    bool isSpecular = true, hit;
    Ray nextRay = r;
    bool testB = false;
    for (int i = 0; i < MAX_DEPTH; ++i) {
#ifdef DEBUG
        cout << "i " << i << " ";
#endif
        //cout << "pre accel->CI" << endl;
        hit = accel->closestIntersection(nextRay, EPSILON, POS_INF, ir);
        //cout << "post accel->CI" << endl;
        
        if (isSpecular) {
            //cout << "isSpecular true. Hit " << hit << endl;
            //cout << "ir.material: " << ir.material << "\tir.primitive: " << ir.primitive << endl;
            if (hit) {
                //cout << "pathThroughput: " << pathThroughput << endl;
                //cout << "ir.material->getEmission: " << ir.material->getEmission() << endl;
                //cout << "ir.primitive->toString: " << ir.primitive->toString() << endl;
                //cout << "ir.primitive->getSurfaceArea: " << ir.primitive->getSurfaceArea() << endl;
                       // ir.primitive->finishIntersection(ir);

                ans += pathThroughput * ir.material->getEmission() * ir.primitive->getSurfaceArea();
                //return ((ADMaterial*) ir.material.get())->base_color;
            }
            else {
                // ray escaped scene
                for (const auto& l : scene->getLights()) {
                    ans += pathThroughput * l->LInf(nextRay);
                }
            }
            //cout << "post isSpecular block" << endl;
        }

        if (!hit) {
            return ans;
        }
        //cout << "pre primitive->FI" << endl;
        ir.primitive->finishIntersection(ir);
        //cout << "ir.isectPoint: " << ir.isectPoint << endl;
        //cout << "post primitive->FI" << endl;

        glm::vec3 wo_local = ir.onb.world2local(-nextRay.d);
        //cout << "pre EDL" << endl;
        ans += pathThroughput * estimateDirectLighting(-nextRay.d, ir);
        //return ans;
        //cout << "post EDL" << endl;

        //cout << "pre material->sample_f" << endl;
        ir.material->sample_f(wo_local, wi_local, &f, &pdf, &isSpecular);
        //cout << "post material->sample_f" << endl;
        glm::vec3 dir = ir.onb.local2world(wi_local);
        //if (i == 0) cout << (Color) dir << endl;
        Ray oldRay = nextRay;
        nextRay = Ray{ir.isectPoint, dir};
#ifdef DEBUG
        cout << nextRay << endl;
#endif
        //cout << "pre bounce throughput: " << pathThroughput << endl;
        pathThroughput *= f * ir.onb.absCosTheta(wi_local) / pdf;

        if (utils::avg(pathThroughput) < 0) {
            glm::vec3 wi_world = ir.onb.local2world(wi_local);
            glm::vec3 wo_world = ir.onb.local2world(wo_local);
            cout << endl;
            cout << "i: " << i << endl;
            cout << "ERROR: negative throughput." << endl;
            cout << "pdf: " << pdf << "\tcosTheta: " << ir.onb.cosTheta(wi_local) << "\tf: " << f << endl;
            cout << "ray: " << oldRay << endl;
            cout << "isect P: " << (Color) ir.isectPoint << endl;
            cout << "isect t: " << ir.t << endl;
            cout << "world shading normal: " << (Color) ir.shadingNormal << endl;
            cout << "world wo: " << (Color) wo_world << endl;
            cout << "world wi: " << (Color)  wi_world << endl;
            cout << "wo_world dot N: " << glm::dot(wo_world, ir.shadingNormal) << endl;
            cout << "wi_world dot N: " << glm::dot(wi_world, ir.shadingNormal) << endl;
            cout << "onb.cosTheta(wo_local): " << ir.onb.cosTheta(wo_local) << endl;
            cout << "onb.cosTheta(wi_local): " << ir.onb.cosTheta(wi_local) << endl;
            cout << "Same hemisphere? " << ir.onb.sameHemisphere(wo_local, wi_local) << endl;
        }
        //cout << "pdf: " << pdf << endl;
        //cout << "post bounce throughput: " << pathThroughput << endl;

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

    //ONB onb{ir.normal};

    float lightPdf, irPdf;
    float weight;
    glm::vec3 wi_world;
    VisibilityTester vt;
    Color f; // BRDF
    // 1) pick direction based on light
    Color l_contrib = l.sample(glm::vec2(rand(), rand()), ir, wi_world, &lightPdf, vt);
    //cout << "\tl_contrib: " << l_contrib << " pdf: " << lightPdf << endl;
    glm::vec3 wi_local = ir.onb.world2local(wi_world);
    glm::vec3 wo_local = ir.onb.world2local(wo_world);
    
    //float absCosTheta = glm::clamp(glm::dot(ir.shadingNormal, wi_world), 0.f, 1.f);
    float absCosTheta = ONB::absCosTheta(wi_local);
    //absCosTheta = 1.0f;
    if (lightPdf > 0.f && !l_contrib.isBlack()) {
        f = ir.material->brdf(wo_local, wi_local, ir);
        //if (utils::avg(f) < .1) cout << "SMALL BRDF:" << f << " avg: " << utils::avg(f) << endl;
        //cout << "\tEDL surface BRDF: " << f << endl;

        if (!f.isBlack() && vt.testVisibile(*this->accel)) {
            //cout << "passed vt" << endl;
            if (l.isDeltaLight()) {
                ans += f * l_contrib * (absCosTheta / lightPdf);
            }
            else {
                irPdf = ir.material->pdf(wo_local, wi_local, ir);
                weight = utils::powerHeuristic(1, lightPdf, 1, irPdf);
                //weight = 1.0f;
                //cout << "irPdf: " << irPdf << endl;
                ans += f * l_contrib * (absCosTheta * weight / lightPdf);
            }
        }
    }

    // 2) pick direction based on material brdf
    if (!l.isDeltaLight()) {
        //cout << "*********************wrong path*******************" << endl;
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
    /*if (utils::avg(ans) < .05) {
        cout << "ERROR: small EDL: " << ans << endl;
        cout << "BRDF: " << f << endl;
        cout << "l_contrib: " << l_contrib << endl;
        cout << "absCosTheta: " << absCosTheta << endl;
        cout << "lightPdf: " << lightPdf << endl;
    }*/
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