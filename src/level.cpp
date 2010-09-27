

#include "level.h"
#include <cstdlib>

using namespace std;

vector<vector<FixedPoint> > pointheight_info;
vector<vector<int> > walls_info;
  
void Level::generate(int seed)
{
  srand(seed);
  pointheight_info.resize(101);
  for(int i=0; i<static_cast<int>(pointheight_info.size()); i++)
  {
    pointheight_info[i].resize( 101 );
    for(int k=0; k<static_cast<int>(pointheight_info[i].size()); k++)
    {
      pointheight_info[i][k].number = 1000 * i / 2;
    }
  }
  
  
  pointheight_info[10][11].number = 18000;
  pointheight_info[11][10].number = 18000;
  pointheight_info[11][11].number = 18000;
  pointheight_info[10][10].number = 18000;
  
  walls_info.resize(100);
  for(int i=0; i<static_cast<int>(walls_info.size()); i++)
  {
    walls_info[i].resize(100, 0);
    
    for(int k=0; k<static_cast<int>(walls_info[i].size()); k++)
      if(rand() % 10 < 2)
        walls_info[i][k] = 1;
  }
  
}

FixedPoint Level::getHeight(FixedPoint& x, FixedPoint& y)
{
  int x_index = x.getInteger() / 8;
  int y_index = y.getInteger() / 8;
  
  int x_desimal = x.getDesimal() + (x.getInteger() & 7) * 1000;
  int y_desimal = y.getDesimal() + (y.getInteger() & 7) * 1000;
  
  if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) -2)
    return FixedPoint(0);
  if(y_index < 0 || y_index > static_cast<int>(pointheight_info.size()) -2)
    return FixedPoint(0);
  
  FixedPoint th1 = pointheight_info[x_index][y_index];
  FixedPoint th2 = pointheight_info[x_index+1][y_index];
  
  FixedPoint bh1 = pointheight_info[x_index][y_index+1];
  FixedPoint bh2 = pointheight_info[x_index+1][y_index+1];
  
  int bot_val = th1.number + x_desimal * (th2.number - th1.number) / 8000;
  int top_val = bh1.number + x_desimal * (bh2.number - bh1.number) / 8000;
  
  FixedPoint height_value;
  height_value.number = bot_val + y_desimal * (top_val - bot_val) / 8000;
  return height_value;
}



