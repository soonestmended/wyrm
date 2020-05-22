#pragma once

#include "common.hpp"

#include <vector>
#include <glm/vec2.hpp>

class SampleGenerator {
public:
  SampleGenerator() {}
  virtual std::vector <Vec2> generate(const Vec2& min, const Vec2& max, int N) const;
};

class StratifiedSampleGenerator : public SampleGenerator {
public:
  StratifiedSampleGenerator() : SampleGenerator() {}
  std::vector <Vec2> generate(const Vec2& min, const Vec2& max, int N) const;
};
