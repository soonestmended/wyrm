#pragma once

#include "Accelerator.hpp"
#include "Color.hpp"
#include "Scene.hpp"

class Shader {
public:
    Shader(const Scene& s) : scene (s) {}
    virtual const Color shade(IntersectRec& ir) const = 0;
    virtual void build() = 0;

protected:
    const Scene& scene;
};

class QuickShader : public Shader {
public:
    QuickShader(const Scene& s) : Shader(s) {}
    const Color shade(IntersectRec& ir) const;
    void build() {}
};

class DirectLightingShader : public Shader {
public:
    DirectLightingShader(const Scene& s, const Accelerator& a) : Shader(s), accel (a) {}
    const Color shade(IntersectRec& ir) const;
    void build();
    const Accelerator& accel;
};