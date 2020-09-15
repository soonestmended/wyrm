#pragma once

#include "Accelerator.hpp"
#include "Color.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

class SampleGenerator;

class Tracer {
public:
  Tracer(const Scene* s, const Accelerator* a) : scene (s), accel (a) {}
  virtual const Color lightAlongRay(const Ray& r, SampleGenerator* sg, const bool debug = false) const = 0;
  const Color estimateDirectLighting(const Vec3& wo_world, IntersectRec& ir, SampleGenerator* sg, bool debug = false) const;
  const Color EDLOneLight(const Vec3& wo_world, IntersectRec& ir, const Light& l, SampleGenerator* sg, bool debug = false) const;

  
  const Scene* scene;
  const Accelerator* accel;
};

class PathTracer : public Tracer {
public:
  PathTracer(const Scene* s, const Accelerator* a, int _maxDepth) : Tracer (s, a), maxDepth (_maxDepth) {}
    const Color lightAlongRay(const Ray& r, SampleGenerator* sg, const bool debug = false) const;
  int maxDepth;
};
