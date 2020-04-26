#pragma once

#include "Accelerator.hpp"
#include "Color.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

class Tracer {
public:
    Tracer(const Scene* s, const Accelerator* a) : scene (s), accel (a) {}
    virtual const Color lightAlongRay(const Ray& r) const = 0;
    const Color estimateDirectLighting(const glm::vec3& wo_world, IntersectRec& ir) const;
    const Color EDLOneLight(const glm::vec3& wo_world, IntersectRec& ir, const Light& l) const;

protected:
    const Scene* scene;
    const Accelerator* accel;
};

class PathTracer : public Tracer {
public:
    PathTracer(const Scene* s, const Accelerator* a) : Tracer (s, a) {}
    const Color lightAlongRay(const Ray& r) const;
};