
#include "world.h"

using namespace std;

void World::init()
{
}

void World::updateModels()
{
  for(int i=0; i<units.size(); i++)
  {
    units[i].tick();
    
    models[i].tick();
    models[i].parts[models[i].root].rotation_x = units[i].getAngle();
    models[i].parts[models[i].root].offset_x   = units[i].position.x.getFloat();
    models[i].parts[models[i].root].offset_z   = units[i].position.y.getFloat();
  }
}

void World::tick()
{
  updateModels();
}

void World::addUnit()
{
  units.push_back(Unit());
  models.push_back(Model());
  models.back().load("model.bones");
}

