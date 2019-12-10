#include <iostream>

#include "Renderer.hpp"

using namespace std;

void QuickRenderer::render() {
    int w = target.width();
    int h = target.height();
    
    // one ray for each pixel in the result image
    // one sample along each ray

    for (int j = 0; j < h; ++j) {
        //cout << "row: " << j << endl;
        for (int i = 0; i < w; ++i) {
            glm::vec2 p((float)i/(float)w-.5, (float)j/(float)h-.5);
            Ray r = camera.getRay(p);
            target(i, j) = tracer.lightAlongRay(r);
        }
    }
    
    cout << "Called render" << endl;
}