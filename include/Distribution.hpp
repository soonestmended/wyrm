#pragma once

#include "common.hpp"
#include "Image.hpp"
#include "Utils.hpp"
#include <vector>

class Distribution1D {
  public:
    Distribution1D() {}
/*
    Distribution1D(const std::vector <Real>& input) {
      // scale input
      Real sum = 0;
      for (Real r : input)
        sum += r;
      //Real dx = 1 / (Real) input.size();
      cumulative.reserve(input.size() + 1);
      cumulative.push_back(0);
      if (sum > 0) {
        scale = 1 / sum;
        for (Real r : input)
          cumulative.push_back(r * scale);
      }
      else {
        scale = 1;
        for (int i = 0; i < input.size(); i++)
          cumulative.push_back(1. / (Real) input.size());
      }
      for (int i = 1; i < cumulative.size(); ++i) {
        cumulative[i] += cumulative[i-1];
      }
//      cumulative[cumulative.size() - 1] = 1;
    }
*/
    Distribution1D(const std::vector <Real>& input) {
      cumulative.push_back(0);
      for (int i = 1; i < input.size() + 1; ++i)
        cumulative.push_back(cumulative[i-1] + input[i-1] / (Real) input.size());
      sum = cumulative[input.size()];
      if (sum == 0)
        for (int i = 1; i < input.size() + 1; i++) cumulative[i] = Real(i) / Real(input.size());
      else
        for (int i = 1; i < input.size() + 1; i++) cumulative[i] /= sum;
    }
    int sampleDiscrete(Real u, Real* pdf) const {
      // choose integer from 0 to cumulative.size() - 1 based on recorded probabilities
      int left = binSearch(u);
      if (left < cumulative.size() - 1)
        *pdf = (cumulative[left+1] - cumulative[left]) / (cumulative.size() - 1);
      else
        *pdf = cumulative[left];
      return left;
    }

    Real sampleContinuous(Real u, Real *pdf) const {
      int left = binSearch(u);
      //std::cout << "u: " << u << " binSearch result: " << left << std::endl;

      Real w = cumulative[left + 1] - cumulative[left];

/*      if (w < EPSILON) {
        std::cout << "w: " << w << std::endl;
        std::cout << "left: " << left << std::endl;
        for (int i = left - 2; i < left + 3; i++) {
          if (i > 0 && i < cumulative.size() - 1)
            std::cout << "cumulative[" << i << "]: " << cumulative[i] << "   ";
        }
        std::cout << std::endl;
        }
*/
      // ? divide by scale
      *pdf = w * (cumulative.size() - 1);

      return (left + (u - cumulative[left]) / w) / (cumulative.size() - 1); // division is to scale to [0, 1]
    }

    Real continuousPdf(Real u) const {
      int left = binSearch(u);
      if (left == cumulative.size() - 1) left--;
      return (cumulative[left + 1] - cumulative[left]) * (Real) (cumulative.size() - 1);
    }

    int binSearch(Real u) const {
      return rBS(u, 0, cumulative.size() - 1);
    }

    int rBS(Real u, int left, int right) const {
      int mid = left + (right - left) / 2;
      if (mid == left) return mid;
      if (cumulative[mid] >= u) {
        return rBS(u, left, mid);
      }
      return rBS(u, mid, right);
    }
  
    std::vector <Real> cumulative;
    Real sum;
};

class Distribution2D {
  public:
    Distribution2D() {}
  
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
        columnAverages.push_back(avg);// / _image.height());
        columnD1ds.push_back(curCol);
        //std::cout << "cD1ds scale: " << columnD1ds[columnD1ds.size()-1].scale << std::endl;
      }
      columnAveragesD1d = Distribution1D(columnAverages);
      /*
      for (auto& d1d : columnD1ds) {
        std::cout << "Column of size ";
        std::cout << d1d.cumulative.size() - 1 << ": ";
        for (auto r : d1d.cumulative)
          std::cout << r << " ";
        std::cout << std::endl;
      }
      std::cout << std::endl << "Averages: ";
      for (Real r : columnAverages)
        std::cout << r << " ";
      std::cout << std::endl;
      std::cout << "Averages D1D: ";
      for (auto r : columnAveragesD1d.cumulative)
        std::cout << r << " ";
      std::cout << std::endl;
*/
    }

    Vec2 sampleContinuous(const Vec2& uv, Real *pdf) const {
      Real xPdf, yPdf;
      Vec2 ans;
      //std::cout << "\tSample x: " << std::endl;
      ans[0] = columnAveragesD1d.sampleContinuous(uv[0], &xPdf);
      //std::cout << "xPdf: " << xPdf << std::endl;
      //std::cout << "\tSample y: " << std::endl;
      int col = utils::clamp((int)(ans[0]*(columnD1ds.size())), 0, columnD1ds.size() - 1);
      ans[1] = columnD1ds[col].sampleContinuous(uv[1], &yPdf);
      //std::cout << "sample column: " << col << std::endl;
      //std::cout <<"yPdf: " << yPdf << std::endl;
      int row = utils::clamp((int)(ans[1]*(columnD1ds[0].cumulative.size() - 1)), 0, columnD1ds[0].cumulative.size() - 2);
      //std::cout << "sample row: " << row << std::endl;
      *pdf = xPdf * yPdf;
      //std::cout << "Distribution sample" << ans << "\tpdf: " << *pdf << std::endl;
     
      return ans;
    }

    Real continuousPdf(const Vec2& uv) const {

      Real xPdf, yPdf;
      Real xSample = columnAveragesD1d.sampleContinuous(uv[0], &xPdf);
      Real ySample = columnD1ds[(int)(xSample*columnD1ds.size())].sampleContinuous(uv[1], &yPdf);
      return xPdf * yPdf;

      //int iu = utils::clamp(int(uv[0] * this->columnD1ds.size()), 0, this->columnD1ds.size()-1);
      //int iv = utils::clamp(int(uv[1] * this->columnD1ds[0].cumulative.size()-1), 0, this->columnD1ds[0].cumulative.size()-2);
      //return (this->columnD1ds[iu].cumulative[iv+1] - this->columnD1ds[iu].cumulative[iv]) * this->columnAveragesD1d.scale;
    }

    glm::ivec2 sampleDiscrete(const Vec2& uv, Real *pdf) const {
      Real xPdf, yPdf;
      glm::ivec2 ans;
      ans[0] = columnAveragesD1d.sampleDiscrete(uv[0], &xPdf);
      ans[1] = columnD1ds[ans[0]].sampleDiscrete(uv[1], &yPdf);
      *pdf = xPdf * yPdf;
      return ans;
    }

    std::vector <Distribution1D> columnD1ds;
    Distribution1D columnAveragesD1d;
};
