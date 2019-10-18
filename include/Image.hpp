#pragma once

#include <vector>

#include "Color.hpp"

using namespace std;

class Image {
public:
    Image() =delete;
    Image(int w, int h) : width_ (w), height_ (h) {
        data = vector <Color> (w*h);
    }
    bool writePNG(const std::string& filename);

    int width() {return width_;}
    int height() {return height_;}
    
    Color& operator()(int x, int y) { return data[y*width_+x]; }
    const Color& operator()(int x, int y) const { return data[y*width_+x]; }


private:
    int width_, height_;
    vector <Color> data;
};