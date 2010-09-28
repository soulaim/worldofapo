
#include "world.h"

#include <iostream>

using namespace std;


World::World()
{
 init();
}

void World::init()
{
  lvl.generate(50);
  apomath.init(300);
}

void World::updateModels()
{ 
  for(int i=0; i<units.size(); i++)
  {
    FixedPoint reference_x = units[i].position.x + apomath.getCos(units[i].angle);
    FixedPoint reference_y = units[i].position.y + apomath.getSin(units[i].angle);
    FixedPoint reference_h = lvl.getHeight(reference_x, reference_y);

    FixedPoint h_diff = reference_h - units[i].position.h;
    
    // finds velocity multiplier based on gradient of the direction we are running to
    int h_val = 1000-( (h_diff.number<0?-h_diff.number:h_diff.number) );
    if(h_val < -500)
      h_val = 0;
    else if(h_val < 200)
      h_val = 200;
    
    units[i].tick(apomath, h_val);
    units[i].position.h = lvl.getHeight(units[i].position.x, units[i].position.y);
    
    // deduce which animation to display
    if(units[i].keyState & 4)
    {
      models[i].setAction("walk");
    }
    else
    {
      models[i].setAction("idle");
    }
    
    // update state of model
    models[i].tick();
    models[i].parts[models[i].root].rotation_x = units[i].getAngle(apomath);
    models[i].parts[models[i].root].offset_x   = units[i].position.x.getFloat();
    models[i].parts[models[i].root].offset_z   = units[i].position.y.getFloat();
    models[i].parts[models[i].root].offset_y   = units[i].position.h.getFloat();
  }
}

void World::tick()
{
  updateModels();
}

// trololol..
void World::addUnit()
{
  units.push_back(Unit());
  units.back().position.x.number = 50000;
  units.back().position.y.number = 50000;
  
  models.push_back(Model());
  models.back().load("data/model.bones");
}

