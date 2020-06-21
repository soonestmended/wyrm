#pragma once

#include "common.hpp"
#include "Image.hpp"
#include <vector>

class Distribution1D {
public:
  Distribution1D(const std::vector <Real>& input) {
    // scale input
    Real sum = 0;
    for (Real r : input)
      sum += r;
    scale = 1 / sum;
    cumulative.reserve(input.size() + 1);
    cumulative.push_back(0);
    for (Real r : input)
      cumulative.push_back(r * scale);
    for (int i = 1; i < cumulative.size() - 1; ++i) {
      cumulative[i] += cumulative[i-1];
    }
    cumulative[cumulative.size() - 1] = 1;
  }

  int sampleDiscrete(Real u, Real* pdf) {
    // choose integer from 0 to cumulative.size() - 1 based on recorded probabilities
    int ans = binSearch(u);
    if (ans > 0)
      *pdf = cumulative[ans] - cumulative[ans-1];
    else
      *pdf = cumulative[ans];
    return ans;
  }

  Real sampleContinuous(Real u, Real *pdf) {
    int left = binSearch(u);
    if (left == cumulative.size() - 1) return 1 / scale;
    Real w = cumulative[left + 1] - cumulative[left];
    *pdf = scale;

    return left + (u - cumulative[left]) / w;
  }

  int binSearch(Real u) {
    return rBS(u, 0, cumulative.size() - 1);
  }

  int rBS(Real u, int left, int right) {
    int mid = left + (right - left) / 2;
    if (mid == left) return mid;
    if (cumulative[mid] >= u) {
      return rBS(u, left, mid);
    }
    return rBS(u, mid, right);
  }
  
  std::vector <Real> cumulative;
  Real scale;
};

class Distribution2D {
  public:
    Distribution2D(const Image& _image) {
      // When D2D is intiialized with an image, it computes a luminance value for each pixel
      // and uses that for importance sampling.
      std::vector <Real> columnAverages;
      columnD1ds.reserve(_image.width());
      columnAverages.reserve(_image.width());
      for (int col = 0; col < _image.width(); col++) {
        std::vector <Real> curCol;
        curCol.reserve(_image.height());
        Real avg = 0;
        for (int row = 0; row < _image.height(); row++) {
          Real lum = _image(col, row).luminance();
          curCol.push_back(lum);
          avg += lum;
        }
        columnAverages.push_back(avg / _image.height());
        columnD1ds.emplace_back(curCol);
      }
      columnAveragesD1d = Distribution1D(columnAverages);
    }

    Real sampleContinuous(const Vec2& uv, Real *pdf) const {
      return 0;
    }
    Real sampleDiscrete(const Vec2& uv, Real *pdf) const {
      return 0;
    }

    std::vector <Distribution1D> columnD1ds;
    Distribution1D columnAveragesD1d;
};
