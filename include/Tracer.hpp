#pragma once

#include "Accelerator.hpp"
#include "Color.hpp"
#include "Ray.hpp"
#include "Scene.hpp"
#include "Shader.hpp"

class Tracer {
public:
    Tracer(const Scene& s, const Shader& sh) : scene (s), shader (sh) {}
    virtual const Color lightAlongRay(const Ray& r) const = 0;

protected:
    const Scene& scene;
    const Shader& shader;
};

class BruteForceTracer : public Tracer {
public:
    BruteForceTracer(const Scene& s, const Shader& sh) : Tracer (s, sh) {}
    const Color lightAlongRay(const Ray& r) const;
};

class AcceleratedTracer : public Tracer {
public:
    AcceleratedTracer(const Scene& s, const Shader& sh, const Accelerator& a) : Tracer (s, sh), accel (a) {

    }

    const Color lightAlongRay(const Ray& r) const;

private:
    const Accelerator& accel;
};