
#include "world.h"

#include <iostream>

using namespace std;

FixedPoint World::heightDifference2Velocity(const FixedPoint& h_diff) const
{
	// no restrictions for moving downhill
	if(h_diff > FixedPoint(-1)/FixedPoint(10))
		return 1;
	
	if(h_diff < FixedPoint(-1))
		return 0;
	
	return FixedPoint(1) + h_diff;
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
	apomath.init(3000);
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
	if( (unit.velocity.h + unit.position.h) <= lvl.getHeight(unit.position.x, unit.position.y) )
	{
		FixedPoint friction = FixedPoint(88) / FixedPoint(100);
		
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
		FixedPoint scale = FixedPoint(10) / FixedPoint(100);
		unit.velocity.x += apomath.getCos(unit.angle) * scale;
		unit.velocity.y += apomath.getSin(unit.angle) * scale;
	}
	
	
	if(unit.getKeyAction(Unit::MOVE_BACK) && hitGround)
	{
		FixedPoint scale = FixedPoint(6) / FixedPoint(100);
		unit.velocity.x -= apomath.getCos(unit.angle) * scale;
		unit.velocity.y -= apomath.getSin(unit.angle) * scale;
	}

	if(unit.getKeyAction(Unit::MOVE_LEFT) && hitGround)
	{
		FixedPoint scale = FixedPoint(8) / FixedPoint(100);
		int dummy_angle = unit.angle - apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.y -= apomath.getSin(dummy_angle) * scale;
	}
	if(unit.getKeyAction(Unit::MOVE_RIGHT) && hitGround)
	{
		FixedPoint scale = FixedPoint(8) / FixedPoint(100);
		int dummy_angle = unit.angle + apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.y -= apomath.getSin(dummy_angle) * scale;
	}

	if(unit.leap_cooldown == 0)
	{
		FixedPoint scale;
		scale.number = 950;
		if(unit.getKeyAction(Unit::LEAP_LEFT) && hitGround)
		{
			int dummy_angle = unit.angle - apomath.DEGREES_90;
			
			unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
			unit.velocity.y -= apomath.getSin(dummy_angle) * scale;
			unit.velocity.h += FixedPoint(450) / FixedPoint(1000);
			unit.leap_cooldown = 60;
		}
		if(unit.getKeyAction(Unit::LEAP_RIGHT) && hitGround)
		{
			int dummy_angle = unit.angle + apomath.DEGREES_90;
			
			unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
			unit.velocity.y -= apomath.getSin(dummy_angle) * scale;
			unit.velocity.h += FixedPoint(450) / FixedPoint(1000);
			unit.leap_cooldown = 60;
		}
	}
	else
	{
		--unit.leap_cooldown;
	}


	// TODO: THIS SHOULD DEFINITELY NOT LOOK SO FUCKING UGLY
	if(unit.weapon_cooldown == 0)
	{
		if(unit.getMouseAction(Unit::ATTACK_BASIC))
		{
			unit.weapon_cooldown = 50;

			// TODO: Following is somewhat duplicated from Camera :G

			// TODO: Fix to use some common ApoMath.
			static ApoMath dorka;
			if(!dorka.ready())
				dorka.init(3000);

			Location position;
			position.x = 30;
			position.y = 0;
			position.h = 0;

			int angle = -unit.angle;
			int upangle = unit.upangle;

			FixedPoint cos = dorka.getCos(angle);
			FixedPoint sin = dorka.getSin(angle);
			FixedPoint upcos = dorka.getCos(upangle);
			FixedPoint upsin = dorka.getSin(upangle);
			
			Location relative_position;
			FixedPoint x = position.x;
			FixedPoint y = position.h;
			FixedPoint z = position.y;
			relative_position.x = cos * upcos * x - sin * z + cos * upsin * y;
			relative_position.y = sin * upcos * x + cos * z + sin * upsin * y;
			relative_position.h =      -upsin * x           +       upcos * y;

			Location weapon_position = unit.position;
			Location projectile_direction = unit.position + relative_position;
			weapon_position.h += 4;
			projectile_direction.h += 4;


//			cerr << "Shooting from (" << unit.position.x << "," << unit.position.y << "," << unit.position.h << ") to (" <<
//				projectile_direction.x << "," << projectile_direction.y << "," << projectile_direction.h << ")\n";
//			extern vector<pair<Location,Location> > LINES;
//			LINES.push_back(make_pair(weapon_position, projectile_direction));

			int id = nextUnitID();
			addProjectile(weapon_position, id);
			Projectile& projectile = projectiles[id];
			projectile.velocity = projectile_direction - weapon_position;
			projectile.velocity.normalize();
			projectile.velocity *= FixedPoint(10)/FixedPoint(1);
			
			projectile.tick();
			projectile.lifetime = 50;
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
	
	FixedPoint hh_val = heightDifference2Velocity(h_diff);

	if(!hitGround)
	{
		hh_val = FixedPoint(1);
	}

	unit.position.y += unit.velocity.y * hh_val;
	unit.position.x += unit.velocity.x * hh_val;
	
	if(unit.getKeyAction(Unit::JUMP) && hitGround)
	{
		unit.velocity.h = FixedPoint(900) / FixedPoint(1000);
	}

	unit.position.h += unit.velocity.h;
}

void World::tickProjectile(Projectile& projectile, Model& model, int id)
{
	// model.parts[model.root].rotation_x = projectile.getAngle(apomath);
	model.updatePosition(projectile.curr_position.x.getFloat(), projectile.curr_position.h.getFloat(), projectile.curr_position.y.getFloat());

	// cerr << "Proj lifetime: " << projectile.lifetime << ", " << projectile.position << ", vel: " << projectile.velocity << "\n";
	if(projectile.lifetime > 0)
	{
		projectile.tick();

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
			else if(projectile.collidesTerrain(lvl))
			{
				//lvl.pointheight_info[projectile.position.x.getInteger()/8][projectile.position.y.getInteger()/8].number += 10000;
//				cerr << "TERRAIN HIT\n";
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
	if(unit.getKeyAction(Unit::MOVE_FRONT | Unit::MOVE_BACK | Unit::MOVE_LEFT | Unit::MOVE_RIGHT))
	{
		model.setAction("walk");
	}
	else
	{
		model.setAction("idle");
	}
	
	// update state of model
	model.tick();
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
	units[id].position.x = FixedPoint(50);
	units[id].position.y = FixedPoint(50);
	
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

	projectiles[id].curr_position = location;

//	cerr << "New projectile with id " << id << "\n";
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

