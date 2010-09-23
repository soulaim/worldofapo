

/*
WARNING: THIS FILE IS FULL OF SHIT
*/

#include <vector>
#include <string>

#include "unit.h"
#include "model.h"

class World
{
  
public:
  void init();
  std::vector<Unit> units;
  std::vector<Model> models;
  void addUnit();
};

