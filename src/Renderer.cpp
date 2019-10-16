#include <iostream>

#include "Renderer.hpp"

using namespace std;

void QuickRenderer::render() {
    int w = target_.width();
    int h = target_.height();
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            //Color c ((float)i/(float)w, (float)j/(float)h, (float)(h-j)/(float)h);
            Color c (0, (float)j/(float)h, 0);
            target_(i, j) = c;
        }
    }
    cout << "Called render" << endl;
}