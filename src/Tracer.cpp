#include "Accelerator.hpp"
#include "common.hpp"
#include "Tracer.hpp"

const Color BruteForceTracer::lightAlongRay(const Ray& r) const {
    for (auto &prim : scene.getPrimitives()) {
        if (prim->intersectYN(r, EPSILON, POS_INF)) {
            return Color::Blue();
        }
    }
    return Color::Black();
}

const Color AcceleratedTracer::lightAlongRay(const Ray& r) const {
    IntersectRec ir;
    if (accel.closestIntersection(r, EPSILON, POS_INF, ir)) {
        //std::cout << "Y";
        return shader.shade(ir);
    }
    else {
        return Color::Black();
    }
}
