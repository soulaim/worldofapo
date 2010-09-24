

/*
WARNING: THIS FILE IS FULL OF SHIT
*/

#include <vector>
#include <string>

#include "unit.h"
#include "model.h"

class World
{
  void updateModels();
  
public:
  void init();
  std::vector<Unit> units;   // each unit corresponds to
  std::vector<Model> models; // one of these BUT
  
  // heroes should be stored in a separate vector!? how can.
  
  void tick();
  void addUnit();
};

