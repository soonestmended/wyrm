#include "common.hpp"
#include "SampleGenerator.hpp"
#include "Utils.hpp"
using namespace std;


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
