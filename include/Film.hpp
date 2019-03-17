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