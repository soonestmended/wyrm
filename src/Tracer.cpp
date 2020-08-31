#include "Accelerator.hpp"
#include "common.hpp"
#include "SampleGenerator.hpp"
#include "Tracer.hpp"

using namespace std;

// TODO note all but EDL turned off right now

const Color PathTracer::lightAlongRay(const Ray& r, const bool debug) const {
    //cout << "lightAlongRay entry" << endl;
    IntersectRec ir;
    Color pathThroughput = Color(1.0);
    Color ans = Color(0.0);
    Vec3 wi_local;
    Color f;
    Real pdf;
    bool isSpecular = true, hit;
    Ray nextRay = r;
    bool testB = false;
    if (debug) cout << "############### starting pathThroughput: " << pathThroughput << endl;
    //Primitive* lastPrim = nullptr;
    for (int i = 0; i < maxDepth; ++i) {
        if (debug) {
          cout << endl;
          cout << "Tracing ray: " << nextRay << endl;
            cout << "i: " << i << endl;
            cout << "isSpecular: " << isSpecular << endl;
        }
        //cout << "pre accel->CI" << endl;
        hit = accel->closestIntersection(nextRay, EPSILON, POS_INF, ir);
        
        //        if (hit) {
          //  cout << "hit";
          //return ((DiffuseMaterial*) ir.material.get())->lbrdf->color;
        //}
        
        //cout << "post accel->CI" << endl;
        if (debug) {
          if (hit) {
            cout << "Hit: " << ir.primitive->toString() << endl;
            cout << "Material: " << ir.material->name << endl;
            cout << "isect P: " << (Color) ir.isectPoint << endl;
            cout << "isect t: " << ir.t << endl;
            cout << "world geometric normal: " << (Color) ir.normal << endl;
          }
          else {
            cout << "Missed.";
          }
        }
        //if (hit && ir.t < .001) cout << "Close intersection with " << ir.primitive->toString() << ". ir.t: " << ir.t << endl;
        //if (hit && ir.primitive == lastPrim && strcmp(ir.primitive->toString().c_str(), "Sphere")) cout << "Self intersection at " << ir.t << ", prim is "<< ir.primitive->toString() << endl;
        if (isSpecular) {
            //cout << "isSpecular true. Hit " << hit << endl;
            //cout << "ir.material: " << ir.material << "\tir.primitive: " << ir.primitive << endl;
            if (hit) {
                //cout << "pathThroughput: " << pathThroughput << endl;
                //cout << "ir.material->getEmission: " << ir.material->getEmission() << endl;
                //cout << "ir.primitive->toString: " << ir.primitive->toString() << endl;
                //cout << "ir.primitive->getSurfaceArea: " << ir.primitive->getSurfaceArea() << endl;
                       // ir.primitive->finishIntersection(ir);
              Color luminaire = glm::dot(ir.normal, -nextRay.d) > 0 ? pathThroughput * ir.material->getEmission() : Color::Black();


              if (debug) {
                cout << "Adding luminaire emission if applicable: " << endl;
                cout << "\tans pre: " << ans << endl;
              }
              ans += luminaire;
              if (debug) {
                cout << "\tLuminaire: " << luminaire << endl;
                cout << "\tans post: " << ans << endl;
              }
                
                //return ((ADMaterial*) ir.material.get())->base_color;
            }
            else {
                // ray escaped scene
              if (debug) cout << "Ray escaped scene." << endl;
                for (const auto& l : scene->getLights()) {
                    ans += pathThroughput * l->LInf (nextRay);
                }
            }
            //cout << "post isSpecular block" << endl;
        }

        if (!hit) {
            return ans;
        }
        //cout << "pre primitive->FI" << endl;
        ir.primitive->finishIntersection(ir);
        if (debug) {
          cout << "world shading normal: " << ir.shadingNormal << endl;
        }
        //lastPrim = ir.primitive;
        //cout << "ir.isectPoint: " << ir.isectPoint << endl;
        //cout << "post primitive->FI" << endl;

        Vec3 wo_local = ir.onb.world2local(-nextRay.d);
        //cout << "pre EDL" << endl;
        if (debug) cout << "**************** BEGIN EDL *****************" << endl;
        Color edl = estimateDirectLighting(-nextRay.d, ir, debug);
        if (debug)cout << "**************** END EDL *******************" << endl;
        if (debug) {
            cout << "EDL at hit point: " << edl << endl;
        }
        ans += pathThroughput * edl;
        //cout << "post EDL" << endl;
        if (debug) {
          cout << "ans + EDL: " << ans << endl;
        }
        //cout << "pre material->sample_f" << endl;
        ir.material->sample_f(wo_local, wi_local, ir, sg->next(), &f, &pdf, &isSpecular);
        //cout << "post material->sample_f" << endl;
        Vec3 dir = ir.onb.local2world(wi_local);
        //if (i == 0) cout << (Color) dir << endl;
        Ray oldRay = nextRay;
        nextRay = Ray{ir.isectPoint + (Real) EPSILON * dir, dir};
        //cout << "pre bounce throughput: " << pathThroughput << endl;
        Color multiplier = f * ir.onb.absCosTheta(wi_local) / pdf;
                //        Color multiplier = f * abs(glm::dot(dir, ir.shadingNormal)) / pdf;
        if   (debug) cout << "~~~PRE multiplier PThroughput: " << pathThroughput << endl;
        pathThroughput *= multiplier;
        if   (debug) {
          cout << "Multiplier: " << multiplier << endl;
          cout << "~~~POST multiplier PThroughput: " << pathThroughput << endl;
        }
          if (debug) {
            Vec3 wi_world = ir.onb.local2world(wi_local);
            Vec3 wo_world = ir.onb.local2world(wo_local);
            cout << "BRDF at hit point: " << f << "\t";
            cout << "Sampled ray in world (wi_world): " << wi_world << endl;
            cout << "Sampled ray in local (wi_local): " << wi_local << endl;
            
            cout << "pdf for sampled ray: " << pdf << "\tcosTheta: " << ir.onb.cosTheta(wi_local) << endl;
            cout << "pathThroughput multiplier: " << multiplier << endl;
            cout << "pathThroughput: " << pathThroughput << endl;
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
            Real q = utils::avg(pathThroughput);
            if (sg->next().x < q) {
                break;
            }
            pathThroughput /= 1.0f - q;
        }

    }
    //return utils::clamp(ans, 0, 1);
    // cout << "\t" << ans << endl;
    return ans;
}


const Color Tracer::EDLOneLight(const Vec3& wo_world, IntersectRec& ir, const Light& l, const bool debug) const {

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

    Real lightPdf, irPdf;
    Real weight;
    Vec3 wi_world;
    VisibilityTester vt;
    Color f; // BRDF

    
    // 1) pick direction based on light
    Color l_contrib = l.sample(sg->next(), ir, wi_world, &lightPdf, vt);
    //cout << "\tl_contrib: " << l_contrib << " pdf: " << lightPdf << endl;
    Vec3 wi_local = ir.onb.world2local(wi_world);
    Vec3 wo_local = ir.onb.world2local(wo_world);
    
    //Real absCosTheta = glm::clamp(glm::dot(ir.shadingNormal, wi_world), 0, 1);
    Real absCosTheta = ONB::absCosTheta(wi_local);
    //absCosTheta = 1.0f;
    if (debug) {
        cout << "\tl_contrib: " << l_contrib << endl;
        cout << "\tlightPdf: " << lightPdf << endl;
    }
    //cout << "light contrib: " << l_contrib << "\tlightPdf: " << lightPdf << endl;
    if (lightPdf > 0 && !l_contrib.isBlack()) {
        if (debug) cout << "pre f" << endl;
        f = ir.material->brdf(wo_local, wi_local, ir, &isSpecular) * abs(glm::dot(wi_world, ir.shadingNormal));
        if (debug) cout << "post f" << endl;
        //if (utils::avg(f) < .1) cout << "SMALL BRDF:" << f << " avg: " << utils::avg(f) << endl;
        //cout << "\tEDL surface BRDF: " << f << endl;
        if (debug) {
            cout << "\tlight part 1: " << endl;
            cout << "\tf: " << f << endl;
        }
        if (!f.isBlack() && vt.testVisible(*this->accel)) {
            if (debug) cout << "\tpassed vt" << endl;
            if (l.isDeltaLight()) {
                ans += f * l_contrib / lightPdf;
            }
            else {
                irPdf = ir.material->pdf(wo_local, wi_local, ir);
                weight = utils::powerHeuristic(1, lightPdf, 1, irPdf);
                //weight = 1.0f;
                //cout << "irPdf: " << irPdf << endl;
                Color foo = f * l_contrib * weight / lightPdf;
                ans += foo;
                if (debug) {
                  cout << "\tpart 1 contrib: " << foo << endl;
                    cout << "\tirPdf: " << irPdf << endl;
                    cout << "\tweight: " << weight << endl;
                    cout << "\tans: " << ans << endl;
                }
            }
        }
        else {
            if (debug) {
                cout << "\tfailed vt" << endl;
                cout << "\tans: " << ans << endl;
            }
        }
    }

    // 2) pick direction based on material brdf
    if (!l.isDeltaLight()) {
        //cout << "*********************wrong path*******************" << endl;
        if (debug) {
            cout << "\tlight part 2: " << endl;
        }
        ir.material->sample_f(wo_local, wi_local, ir, sg->next(), &f, &irPdf, &isSpecular);
        //f *= ir.onb.absCosTheta(wi_local);
        wi_world = ir.onb.local2world(wi_local);
        f *= abs(glm::dot(wi_world, ir.shadingNormal));
        if (!f.isBlack() && irPdf > 0) {
          weight = 1;
          if (!isSpecular) {
            lightPdf = l.pdf(ir, wi_local);
            if (lightPdf == 0) {
              if (debug) cout << "\tzero lightPdf. Returning. " << endl;
              return ans; // no probability of choosing this ray given light distribution
            }
            weight = utils::powerHeuristic(1, irPdf, 1, lightPdf);
          }
          l_contrib = Color::Black();
            Ray rayToLight{ir.isectPoint, wi_world};
            IntersectRec tempIr;
            if (accel->closestIntersection(rayToLight, EPSILON, POS_INF, tempIr)) {
                if (l.getPrim() == tempIr.primitive) {
                  if (debug) cout << "\tray chosen by surface hits light!" << endl;
                    l_contrib = glm::dot(tempIr.normal, -rayToLight.d) > 0. ? tempIr.material->getEmission() : Color::Black();
                }
            }
            else {
                if (debug) cout << "\tray goes to infinity" << endl;
                l_contrib = l.LInf(rayToLight); // account for infinite light
            }
            if (!l_contrib.isBlack()) {
                Color local_contrib = f * l_contrib * absCosTheta * weight / irPdf;
                ans += local_contrib;
                // check this code
                if (debug) {
                    cout << "\tdist: " << tempIr.t << endl;
                    cout << "\tf: " << f << endl;
                    cout << "\tl_contrib: " << l_contrib << endl;
                    cout << "\tpart 2 contrib: " << local_contrib << endl;
                    cout << "\tabsCosTheta: " << absCosTheta << endl;
                    cout << "\tweight: " << weight << endl;
                    cout << "\tlightPdf: " << lightPdf << endl;
                    cout << "\t-rayToLight: " << -rayToLight.d << endl;
                    cout << "\tCos light N dot -rtl.d: " << glm::dot(tempIr.normal, -rayToLight.d) << endl;
                    cout << "\tirPdf: " << irPdf << endl;
                    cout << "\tans: " << ans << endl;
                    cout << endl;
                }

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
    //cout << ans << endl;
    return ans;
    //return Color::White();
}

const Color Tracer::estimateDirectLighting(const Vec3& wo_world, IntersectRec& ir, const bool debug) const {
    Color ans{0.0};
    for (const auto& l : scene->getLights()) {
        ans += EDLOneLight(wo_world, ir, *l, debug);
    }
    return ans;
}
