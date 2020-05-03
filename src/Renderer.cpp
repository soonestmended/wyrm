#include <iostream>

#include "Renderer.hpp"

using namespace std;

void QuickRenderer::render() {
    int w = target.width();
    int h = target.height();
    
    // one ray for each pixel in the result image
    // one sample along each ray

    for (int j = 0; j < h; ++j) {
        printf("row: %4d", j);
        fflush(stdout);
        for (int i = 0; i < w; ++i) {
            glm::vec2 p((float)i/(float)w-.5, (float)j/(float)h-.5);
            Ray r = camera.getRay(p);
            target(i, j) = utils::clamp(tracer.lightAlongRay(r), 0.f, 1.f);
        }
        printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
    cout << "Called render" << endl;
}

void MultisampleRenderer::render() {
    int w = target.width();
    int h = target.height();
    
    // one ray for each pixel in the result image
    // one sample along each ray
    Ray r;
    glm::vec2 pixelSize{1./(float) w, 1./(float)h};
    for (int j = 0; j < h; ++j) {
        printf("row: %4d", j);
        fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Color pixelColor = Color::Black();
            for (int k = 0; k < spp; ++k) {
                glm::vec2 p((float)i/(float)w, (float)j/(float)h); // center of pixel
                p += (utils::rand01vec2() - glm::vec2(.5)) * pixelSize;
                r = camera.getRay(p - glm::vec2(.5));
                pixelColor += tracer.lightAlongRay(r);
            }
            target(i, j) = utils::clamp(pixelColor / (float) spp, 0.f, 1.f);
        }
        printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
    cout << "Called render" << endl;
}

void DebugRenderer::render() {
    Ray testRay {glm::vec3(0, sqrt(2.f)/2.f, -10.f), glm::vec3(0, 0, 1) };
    for (int i = 0; i < 5; i++) {
        Color foo = tracer.lightAlongRay(testRay);
    }
}