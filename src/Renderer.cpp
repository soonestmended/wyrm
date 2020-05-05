#include <iostream>

#include "common.hpp"
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
            Vec2 p((Real)i/(Real)w-.5, (Real)j/(Real)h-.5);
            Ray r = camera.getRay(p);
            target(i, j) = utils::clamp(tracer.lightAlongRay(r), 0, 1);
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
    Vec2 pixelSize{1./(Real) w, 1./(Real)h};
    for (int j = 0; j < h; ++j) {
        printf("row: %4d", j);
        fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Color pixelColor = Color::Black();
            for (int k = 0; k < spp; ++k) {
                Vec2 p((Real)i/(Real)w, (Real)j/(Real)h); // center of pixel
                p += (utils::rand01vec2() - Vec2(.5)) * pixelSize;
                r = camera.getRay(p - Vec2(.5));
                if (j == 259 && i == 109) {
                    pixelColor += tracer.lightAlongRay(r, false);
                }
                else {
                    pixelColor += tracer.lightAlongRay(r, false);
                }
            }
            target(i, j) = utils::clamp(pixelColor / (Real) spp, 0, 1);
            //if (utils::avg(pixelColor) > 10.0) cout << "col: " << i << "\tray: " << r << "\tcolor: " << pixelColor << endl;
        }
        printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
    cout << "Called render" << endl;
}

void DebugRenderer::render() {
    int w = 512;
    int h = 512;
    
    // one ray for each pixel in the result image
    // one sample along each ray
    Ray r;
    Vec2 pixelSize{1./(Real) w, 1./(Real)h};
    for (int j = 0; j < h; ++j) {
        //printf("row: %4d", j);
        //fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Color pixelColor = Color::Black();
            Vec2 p((Real)109/(Real)w, (Real)259/(Real)h); // center of pixel
            p += (utils::rand01vec2() - Vec2(.5)) * pixelSize;
            r = camera.getRay(p - Vec2(.5));
            pixelColor = tracer.lightAlongRay(r);
            // 0.228245, 0.004493, 0.973594
            if (j == 259 && i == 109)
                cout << "Ray: " << r << "\tColor: " << pixelColor << endl;
        }
        //printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
    cout << "Called render" << endl;
}