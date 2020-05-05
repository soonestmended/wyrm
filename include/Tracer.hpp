#pragma once

#include "Accelerator.hpp"
#include "Color.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

class Tracer {
public:
    Tracer(const Scene* s, const Accelerator* a) : scene (s), accel (a) {}
    virtual const Color lightAlongRay(const Ray& r, const bool debug = false) const = 0;
    const Color estimateDirectLighting(const Vec3& wo_world, IntersectRec& ir, bool debug = false) const;
    const Color EDLOneLight(const Vec3& wo_world, IntersectRec& ir, const Light& l, bool debug = false) const;

protected:
    const Scene* scene;
    const Accelerator* accel;
};

class PathTracer : public Tracer {
public:
    PathTracer(const Scene* s, const Accelerator* a) : Tracer (s, a) {}
    const Color lightAlongRay(const Ray& r, const bool debug = false) const;
};