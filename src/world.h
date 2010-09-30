

/*
WARNING: THIS FILE IS FULL OF SHIT
*/

#include <vector>
#include <string>

#include "unit.h"
#include "model.h"
#include "level.h"
#include "apomath.h"

class World
{
  void updateModels();
  int heightDifference2Velocity(int h_diff);
  
public:
  World();
  void init();
  std::vector<Unit> units;   // each unit corresponds to
  std::vector<Model> models; // one of these BUT
  
  Level lvl;
  ApoMath apomath;
  
  void tickUnit(Unit& unit);
  void tick();
  void addUnit();
};

