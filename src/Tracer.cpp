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
