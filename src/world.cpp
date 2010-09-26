
#include "world.h"

using namespace std;


World::World()
{
 init();
}

void World::init()
{
  lvl.generate(50);
}

void World::updateModels()
{ 
  for(int i=0; i<units.size(); i++)
  {
    units[i].tick();
    units[i].position.h = lvl.getHeight(units[i].position.x, units[i].position.y);
    
    models[i].tick();
    models[i].parts[models[i].root].rotation_x = units[i].getAngle();
    models[i].parts[models[i].root].offset_x   = units[i].position.x.getFloat();
    models[i].parts[models[i].root].offset_z   = units[i].position.y.getFloat();
    models[i].parts[models[i].root].offset_y   = units[i].position.h.getFloat();
  }
}

void World::tick()
{
  updateModels();
}

void World::addUnit()
{ 
  units.push_back(Unit());
  units.back().position.x.number = 50000;
  units.back().position.y.number = 50000;
  
  models.push_back(Model());
  models.back().load("data/model.bones");
}

