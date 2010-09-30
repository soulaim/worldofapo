
#include "world.h"

#include <iostream>

using namespace std;

int World::heightDifference2Velocity(int h_diff)
{
  // no restrictions for moving downhill
  if(h_diff > -100)
    return 1000;
  
  if(h_diff < -1000)
    return 0;
  
  return 1000 + h_diff;
}


World::World()
{
 init();
}

void World::init()
{
  lvl.generate(50);
  apomath.init(300);
}


void World::tickUnit(Unit& unit)
{

  bool hitGround = false;
  if(unit.position.h.number - 100 <= lvl.getHeight(unit.position.x, unit.position.y).number)
  {
    unit.position.h = lvl.getHeight(unit.position.x, unit.position.y);
    unit.velocity.h.number = 0;
    unit.velocity.x.number = 0;
    unit.velocity.y.number = 0;
    hitGround = true;
  }
  else
  {
    unit.velocity.h.number -= 35;
  }
  
  
  if(unit.keyState & 1) // if should be moving, turns left
  {
	  unit.angle += 2;
  }
  if(unit.keyState & 2) // if should be moving, turns right
  {
	  unit.angle -= 2;
  }
  
  if(unit.movingFront() && hitGround)
  {
    unit.velocity.x = apomath.getCos(unit.angle);
    unit.velocity.y = apomath.getSin(unit.angle);
  }


  if(unit.movingBack() && hitGround)
  {
    unit.velocity.x.number = -apomath.getCos(unit.angle).number;
    unit.velocity.y.number = -apomath.getSin(unit.angle).number;
  }


  FixedPoint reference_x = unit.position.x + unit.velocity.x;
  FixedPoint reference_y = unit.position.y + unit.velocity.y;
  FixedPoint reference_h = lvl.getHeight(reference_x, reference_y);
  FixedPoint h_diff = reference_h - unit.position.h;

  int h_val = heightDifference2Velocity(h_diff.number);
  if(!hitGround)
  {
    h_val = 1000;
  }
  
  unit.position.y.number += unit.velocity.y.number * h_val / 1000;
  unit.position.x.number += unit.velocity.x.number * h_val / 1000;
  
  if( (unit.keyState & 16) && hitGround)
    unit.velocity.h.number = 900;
  
  unit.position.h += unit.velocity.h;
}


void World::updateModels()
{ 
  for(int i=0; i<units.size(); i++)
  {
    
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
  for(int i=0; i<units.size(); i++)
    tickUnit(units[i]);
  
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

