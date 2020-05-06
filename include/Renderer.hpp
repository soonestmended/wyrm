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

class DebugRenderer : public Renderer {
public:
    DebugRenderer(const Camera& c, const Scene& s, const Tracer& t) : 
        Renderer(c, s, t) {}
    void render();
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

class MultisampleRenderer : public Renderer {
public:
    MultisampleRenderer(const Camera& c, const Scene& s, const Tracer& t, Image &_target, int _spp) : 
        Renderer(c, s, t),
        target (_target),
        spp (_spp) {}
    void render();

private:
    Image &target;
    int spp; // samples per pixel
};

class MultiThreadRenderer : public MultisampleRenderer {
public:
    MultiThreadRenderer(const Camera& c, const Scene& s, const Tracer& t, Image &_target, int _spp, int _threads) :
    MultisampleRenderer(c, s, t, _target, _spp), numThreads(_threads) {}
    void render();
    int numThreads;
};

