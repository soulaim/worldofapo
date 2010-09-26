

/*
WARNING: THIS FILE IS FULL OF SHIT
*/

#include <vector>
#include <string>

#include "unit.h"
#include "model.h"
#include "level.h"

class World
{
  void updateModels();
  
public:
  World();
  void init();
  std::vector<Unit> units;   // each unit corresponds to
  std::vector<Model> models; // one of these BUT
  
  Level lvl;
  
  void tick();
  void addUnit();
};

