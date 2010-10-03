
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
	cerr << "world init" << endl;
	_unitID_next_unit = 0;
	lvl.generate(50);
	apomath.init(300);
}

void World::terminate()
{
	_unitID_next_unit = 0;
	units.clear();
	models.clear();
}


void World::tickUnit(Unit& unit, Model& model)
{
	// update the information according to which the unit model will be updated from now on
	model.parts[model.root].rotation_x = unit.getAngle(apomath);
	model.updatePosition(unit.position.x.getFloat(), unit.position.h.getFloat(), unit.position.y.getFloat());
	
	// some physics & game world information
	
	bool hitGround = false;
	if(unit.position.h.number - 100 <= lvl.getHeight(unit.position.x, unit.position.y).number)
	{
		FixedPoint friction;
		friction.number = 800;
		
		unit.position.h = lvl.getHeight(unit.position.x, unit.position.y);
		unit.velocity.h.number = 0;
		unit.velocity.x *= friction;
		unit.velocity.y *= friction;
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
		FixedPoint scale;
		scale.number = 150;
		
		unit.velocity.x += apomath.getCos(unit.angle) * scale;
		unit.velocity.y += apomath.getSin(unit.angle) * scale;
	}
	
	
	if(unit.movingBack() && hitGround)
	{
		FixedPoint scale;
		scale.number = 150;
		
		unit.velocity.x -= apomath.getCos(unit.angle) * scale;
		unit.velocity.y -= apomath.getSin(unit.angle) * scale;
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


void World::updateModel(Model& model, Unit& unit)
{     
	// deduce which animation to display
	if(unit.keyState & 4)
	{
		model.setAction("walk");
	}
	else
	{
		model.setAction("idle");
	}
	
	// update state of model
	model.tick();
	
	/*
	model.parts[model.root].offset_x   = unit.position.x.getFloat();
	model.parts[model.root].offset_z   = unit.position.y.getFloat();
	model.parts[model.root].offset_y   = unit.position.h.getFloat();
	*/
}

void World::worldTick()
{
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); iter++)
		tickUnit(iter->second, models[iter->first]);
}

void World::viewTick()
{
	for(map<int, Model>::iterator iter = models.begin(); iter != models.end(); iter++)
		updateModel(iter->second, units[iter->first]);
}


// trololol..
void World::addUnit(int id)
{
	units[id] = Unit();
	units[id].position.x.number = 50000;
	units[id].position.y.number = 50000;
	
	models[id] = Model();
	models[id].load("data/model.bones");
}



int World::nextUnitID()
{
	int id = _unitID_next_unit;
	_unitID_next_unit++;
	return id;
}

