#pragma once

#include <vector>

using namespace std;

class Image {
public:
    Image() =delete;
    Image(int w, int h) : width_ (w), height_ (h) {
        data_ = vector <float> (w*h);
    }
    bool writePNG(const std::string& filename);

private:
    int width_, height_;
    vector <float> data_;
};