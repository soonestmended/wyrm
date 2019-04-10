#pragma once

#include "Camera.hpp"
#include "Scene.hpp"

class Film {
public:    
    Film(const Camera& c, const Scene& s) : camera_ (c), scene_ (s) {}
    virtual void render() = 0;

protected: 
    const Camera& camera_;
    const Scene& scene_;
};

class QuickRender : public Film {
public:
    QuickRender(const Camera& c, const Scene& s, int width = 400, int height = 400) : 
    Film(c, s), width_ (width), height_ (height) {}
    void render();

private:
    int width_, height_;
};