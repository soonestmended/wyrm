#pragma once

#include "Accelerator.hpp"
#include "Color.hpp"
#include "Scene.hpp"

class Shader {
public:
    Shader(const Scene& s, const Accelerator& a) : scene (s), accel (a) {}
    virtual const Color shade(IntersectRec& ir) const = 0;
    virtual void build() = 0;

protected:
    const Scene& scene;
    const Accelerator& accel;
};

class QuickShader : public Shader {
public:
    QuickShader(const Scene& s, const Accelerator& a) : Shader(s, a) {}
    const Color shade(IntersectRec& ir) const;
    void build() {}
};

class DirectLightingShader : public Shader {
public:
    DirectLightingShader(const Scene& s, const Accelerator& a) : Shader(s, a) {}
    const Color shade(IntersectRec& ir) const;
    void build();
};