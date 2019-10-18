#pragma once

#include "Color.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

class Tracer {
public:
    Tracer(const Scene& s) : scene (s) {}
    virtual const Color lightAlongRay(const Ray& r) const = 0;

protected:
    const Scene& scene;
};

class BruteForceTracer : public Tracer {
public:
    BruteForceTracer(const Scene& s) : Tracer (s) {}
    const Color lightAlongRay(const Ray& r) const;
};