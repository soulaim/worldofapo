
#include "world.h"

using namespace std;

void World::addUnit()
{
  units.push_back(Unit());
  models.push_back(Model());
  models.back().load("model.bones");
}

