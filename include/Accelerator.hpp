#pragma once

#include "common.hpp"
#include "IntersectRec.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

/* Accelerator classes speed up the process of finding the closest intersection in a scene or of
** deciding whether a given ray intersects anything over the specified interval.
*/

class Accelerator {
public:
    Accelerator(const Scene& s) : scene (s) {}
    virtual const bool closestIntersection(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ans) const {
        Real bestT = POS_INF;
        bool hit = false;
        IntersectRec tempIR;
        for (auto &prim : scene.getPrimitives()) {
            if (prim->intersect(ray, tmin, bestT, tempIR)) {
                if (tempIR.t < bestT) {
                    ans = tempIR;
                    bestT = ans.t;
                    hit = true;
                }
            }
        }
        return hit;
    }
    virtual const bool intersectionYN(const Ray& ray, const Real tmin, const Real tmax) const {
        for (auto &prim : scene.getPrimitives()) 
            if (prim->intersectYN(ray, tmin, tmax)) return true;
        return false;
    }
    virtual const bool build() {return true;}
    
protected:
    const Scene& scene;
};