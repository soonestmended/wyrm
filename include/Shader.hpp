#pragma once

#include "Color.hpp"
#include "Scene.hpp"

class Shader {
public:
    Shader(const Scene& s) : scene (s) {}
    virtual const Color shade(IntersectRec& ir) const = 0;
    virtual void build() = 0;

private:
    const Scene& scene;
};

class QuickShader : public Shader {
public:
    QuickShader(const Scene& s) : Shader(s) {}
    const Color shade(IntersectRec& ir) const;
    void build() {}
};