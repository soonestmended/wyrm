#pragma once

#include "Camera.hpp"
#include "Image.hpp"
#include "Scene.hpp"
#include "Tracer.hpp"

class Renderer {
public:    
    Renderer(const Camera& c, const Scene& s, const Tracer& t) : camera (c), scene (s), tracer (t) {}
    virtual void render() = 0;

protected: 
    const Camera& camera;
    const Scene& scene;
    const Tracer& tracer;
};

class QuickRenderer : public Renderer {
public:
    QuickRenderer(const Camera& c, const Scene& s, const Tracer& t, Image &target_) : 
        Renderer(c, s, t),
        target (target_) {}
    void render();

private:
    Image &target;
};