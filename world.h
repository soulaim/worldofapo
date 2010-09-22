
#include <vector>
#include <string>

#include "unit.h"
#include "model.h"

class World
{
public:
  std::vector<Unit> units;
  std::vector<Model> models;
  void addUnit();
};

