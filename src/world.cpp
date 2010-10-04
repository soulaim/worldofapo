
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
	
	_unitID_next_unit = 10000;
	_playerID_next_player = 0;
	
	lvl.generate(50);
	apomath.init(300);
}

void World::terminate()
{
	_unitID_next_unit = 10000;
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
	if( (unit.velocity.h.number <= 100) && (unit.position.h.number - 100 <= lvl.getHeight(unit.position.x, unit.position.y).number) )
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
	
	if(unit.getKeyAction(Unit::MOVE_FRONT) && hitGround)
	{
		FixedPoint scale;
		scale.number = 150;
		
		unit.velocity.x += apomath.getCos(unit.angle) * scale;
		unit.velocity.y += apomath.getSin(unit.angle) * scale;
	}
	
	
	if(unit.getKeyAction(Unit::MOVE_BACK) && hitGround)
	{
		FixedPoint scale;
		scale.number = 150;
		
		unit.velocity.x -= apomath.getCos(unit.angle) * scale;
		unit.velocity.y -= apomath.getSin(unit.angle) * scale;
	}

	if(unit.getKeyAction(Unit::MOVE_LEFT) && hitGround)
	{
		FixedPoint scale;
		scale.number = 150;
		int dummy_angle = unit.angle - apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.y -= apomath.getSin(dummy_angle) * scale;
	}
	if(unit.getKeyAction(Unit::MOVE_RIGHT) && hitGround)
	{
		FixedPoint scale;
		scale.number = 150;
		int dummy_angle = unit.angle + apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.y -= apomath.getSin(dummy_angle) * scale;
	}

	if(unit.weapon_cooldown == 0)
	{
		if(unit.getMouseAction(Unit::ATTACK_BASIC))
		{
			unit.weapon_cooldown = 5;

			// TODO: Following is somewhat duplicated from Camera :G

			// TODO: Fix to use some common ApoMath.
			static ApoMath dorka;
			if(!dorka.ready())
				dorka.init(300);
			
			Location position;
			position.x = 30;
			position.y = 0;
			position.h = 0;

			int angle = -unit.angle;

			FixedPoint cos = dorka.getCos(angle);
			FixedPoint sin = dorka.getSin(angle);
			
			Location relative_position;
			relative_position.x = cos * position.x - sin * position.y;
			relative_position.y = sin * position.x + cos * position.y;
			relative_position.h = position.y + unit.upangle/2; // TODO: this is dirty hack :)

			Location weapon_position = unit.position;
			weapon_position.h += 3;
			Location projectile_direction = unit.position + relative_position;


//			cerr << "Shooting from (" << unit.position.x << "," << unit.position.y << "," << unit.position.h << ") to (" <<
//				projectile_direction.x << "," << projectile_direction.y << "," << projectile_direction.h << ")\n";
//			extern vector<pair<Location,Location> > LINES;
//			LINES.push_back(make_pair(weapon_position, projectile_direction));

			int id = nextUnitID();
			addProjectile(weapon_position, id);
			Projectile& projectile = projectiles[id];
			projectile.velocity = projectile_direction - weapon_position;
			projectile.velocity.normalize();
			projectile.velocity *= FixedPoint(3)/FixedPoint(1);
			projectile.lifetime = 200;
		}
	}
	else
	{
		--unit.weapon_cooldown;
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

void World::tickProjectile(Projectile& projectile, Model& model, int id)
{
//	model.parts[model.root].rotation_x = projectile.getAngle(apomath);
	model.updatePosition(projectile.position.x.getFloat(), projectile.position.h.getFloat(), projectile.position.y.getFloat());

//	cerr << "Proj lifetime: " << projectile.lifetime << ", " << projectile.position << ", vel: " << projectile.velocity << "\n";
	if(projectile.lifetime > 0)
	{
		projectile.position.x += projectile.velocity.x;
		projectile.position.y += projectile.velocity.y;
		projectile.position.h += projectile.velocity.h;

		--projectile.lifetime;

		for(map<int, Unit>::iterator it = units.begin(), et = units.end(); it != et; ++it)
		{
			Unit& unit = it->second;
			if(projectile.collides(unit))
			{
				cerr << "HIT!\n";
				unit.weapon_cooldown = 100;
				unit.velocity.x = 0;
				unit.velocity.y = 0;
				unit.velocity.h = 3;

				removeUnit(id);
			}
		}
	}
	else
	{
		removeUnit(id);
	}
}

void World::updateModel(Model& model, Unit& unit)
{
	/*
	// deduce which animation to display
	if(unit.position.h.number - 100 > lvl.getHeight(unit.position.x, unit.position.y).number)
	{
		model.setAction("jump");
	}
	else */
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
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		tickUnit(iter->second, models[iter->first]);
	}
	for(map<int, Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		tickProjectile(iter->second, models[iter->first], iter->first);
	}
}

void World::viewTick()
{
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		updateModel(models[iter->first], iter->second);
	}
	for(map<int, Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		models[iter->first].setAction("idle");
		models[iter->first].tick();
	}
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

void World::addProjectile(Location& location, int id)
{
	Vec3 position;
	position.x = location.x.getFloat();
	position.y = location.h.getFloat();
	position.z = location.y.getFloat();
	models[id].load("data/bullet.bones");
	models[id].realUnitPos = position;
	models[id].currentModelPos = position;

	projectiles[id].position = location;

	cerr << "New projectile with id " << id << "\n";
}

int World::nextPlayerID()
{
	int id = _playerID_next_player;
	_playerID_next_player++;
	return id;
}

int World::nextUnitID()
{
	int id = _unitID_next_unit;
	_unitID_next_unit++;
	return id;
}

void World::removeUnit(int id)
{
	units.erase(id);
	projectiles.erase(id);
	models.erase(id);
}

