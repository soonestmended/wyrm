#pragma once

#include "common.hpp"
#include <ctime>
#include <random>
#include <thread>
#include <vector>
#include <glm/vec2.hpp>


extern thread_local std::mt19937 mt_rand;

class SampleGenerator {
public:
    SampleGenerator(const Vec2& _min, const Vec2& _max, int _N) : rangeMin(_min), rangeMax(_max), rangeSize(_max - _min), N(_N), sp(0) {
       // distribution = std::uniform_real_distribution<double>(0.0, 1.0);
    }

    static void seedRNG(unsigned int seed) {
      mt_rand.seed(seed);
    }

    virtual void generate() {
      samples.reserve(N);
      for (int i = 0; i < N; ++i) {
        samples.push_back(rangeMin + rangeSize * randVec2());
      }
    }

    virtual Vec2 next() {
      if (sp < samples.size()) {
        return samples[sp++];
      }
      else {
        Vec2 ans = rangeMin + rangeSize * randVec2();
        return ans;
      }
    }

    Vec2 randVec2();

    Vec2 rangeMin, rangeMax, rangeSize;
    int N, sp; // sample pointer
    std::vector <Vec2> samples;
    // std::mt19937 mt_rand;

};

class StratifiedSampleGenerator : public SampleGenerator {
public:
  StratifiedSampleGenerator(const Vec2& _min, const Vec2& _max, int _N) : SampleGenerator(_min, _max, calculateN(_N)) {}
  void generate();
  static int calculateN(int N) {
    Real sn = sqrt(N);
    Real fsn = floor(sn);
    int w = (int) fsn;
    if (sn != fsn)  {
      w++;
      N = w*w;
    }
    return N;
  }
};
