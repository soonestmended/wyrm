#pragma once

#include "Camera.hpp"
#include "Scene.hpp"
#include "Tracer.hpp"

class Renderer {
public:    
    Renderer(const Camera& c, const Scene& s, const Tracer& t) : camera_ (c), scene_ (s), tracer_ (t) {}
    virtual void render() = 0;

protected: 
    const Camera& camera_;
    const Scene& scene_;
    const Tracer& tracer_;
};

class QuickRenderer : public Renderer {
public:
    QuickRenderer(const Camera& c, const Scene& s, const Tracer& t, int &target) : 
        Renderer(c, s, t),
        target_ (target) {}
    void render();

private:
    int &target_;
};