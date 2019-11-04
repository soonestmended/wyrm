#pragma once

#include "IntersectRec.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

/* Accelerator classes speed up the process of finding the closest intersection in a scene or of
** deciding whether a given ray intersects anything over the specified interval.
*/

class Accelerator {
public:
    Accelerator(const Scene& s) : scene (s) {}
    virtual const bool closestIntersection(const Ray& ray, const float tmin, const float tmax, IntersectRec& ans) const = 0;
    virtual const bool intersectionYN(const Ray& ray, const float tmin, const float tmax) const = 0;
    virtual const bool build() = 0;
    
protected:
    const Scene& scene;
};