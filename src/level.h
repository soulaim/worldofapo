
#include <vector>
#include <string>

#include "fixed_point.h"

struct Level
{
  std::vector<std::vector<FixedPoint> > pointheight_info;
  std::vector<std::vector<int> > walls_info;
  
  void generate(int);
  FixedPoint getHeight(FixedPoint& x, FixedPoint& y);
};

