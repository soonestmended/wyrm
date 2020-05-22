#include "common.hpp"
#include "SampleGenerator.hpp"
#include "Utils.hpp"
using namespace std;

vector <Vec2> SampleGenerator::generate(const Vec2& min, const Vec2& max, int N) const {
  vector <Vec2> ans;
  ans.reserve(N);
  Vec2 size = max - min;
  
  for (int i = 0; i < N; i++)
    ans.push_back(utils::rand01vec2()*size + min);
  return ans;
}

vector <Vec2> StratifiedSampleGenerator::generate(const Vec2& min, const Vec2& max, int N) const {
  vector <Vec2> ans;
  Real sn = sqrt(N);
  Real fsn = floor(sn);
  int w = (int) fsn;
  if (sn != fsn)  {
    w++;
    N = w*w;
  }
  //cout << "N: " << N << endl;
  ans.reserve(N);
  Vec2 size = max - min;
  Vec2 ds = size / (Real) w;
  Vec2 s = min;
  for (; s.y < max.y; s.y+=ds.y) {
    for (; s.x < max.x; s.x+=ds.x) {
      ans.push_back(s + utils::rand01vec2() * ds);
    }
    s.x = min.x;
  }
  return ans;
}
