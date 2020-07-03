#pragma once

#include <vector>

#include "Color.hpp"

class Image {
    public:
        Image() =delete;
        Image(const std::string& filename);
        Image(int w, int h) : width_ (w), height_ (h) {
            data = std::vector <Color> (w*h);
        }
        bool writePNG(const std::string& filename);

        int width() const {return width_;}
        int height() const {return height_;}
    
        Color& operator()(int x, int y) { return data[y*width_+x]; }
        const Color& operator()(int x, int y) const { return data[y*width_+x]; }
        void gaussianBlur(Real sigma);
        void boxBlur(int w, int n);

    private:
        int width_, height_;
        std::vector <Color> data;
        void convolve(std::vector <Real> &kernel);
        void hBoxBlur(int w);
        void vBoxBlur(int w);
};
