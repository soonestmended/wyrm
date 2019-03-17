#pragma once

#include "Color.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

class Tracer {
public:
    Tracer(const Scene& s) : scene_ (s) {}
    virtual const Color lightAlongRay(const Ray& r) = 0;

protected:
    const Scene& scene_;
};

class BruteForceTracer : public Tracer {
    BruteForceTracer(const Scene& s) : Tracer (s) {}
    const Color lightAlongRay(const Ray& r);
};