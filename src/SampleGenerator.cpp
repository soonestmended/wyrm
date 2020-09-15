#include "common.hpp"
#include "SampleGenerator.hpp"
#include "Utils.hpp"
using namespace std;

thread_local std::mt19937 mt_rand;
thread_local std::uniform_real_distribution<double> distribution {0.0, 1.0};

Vec2 SampleGenerator::randVec2() {
    //    return Vec2((Real) mt_rand() / (Real) RAND_MAX, (Real) mt_rand() / (Real) RAND_MAX);
    return Vec2(distribution(mt_rand), distribution(mt_rand));
}

void StratifiedSampleGenerator::generate() {
  int w = sqrt(N); // N guaranteed to be perfect square 
  samples.reserve(N);
  Vec2 ds = rangeSize / (Real) w;
  Vec2 s = rangeMin;
  for (; s.y < rangeMax.y; s.y+=ds.y) {
    for (; s.x < rangeMax.x; s.x+=ds.x) {
      samples.push_back(s + randVec2() * ds);
    }
    s.x = rangeMin.x;
  }
}
