
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

void World::doDeathFor(Unit& unit, int causeOfDeath)
{
	stringstream msg;
	string killer = "an unknown entity";
	
	deaths.push_back(unit.id);
	if(units.find(causeOfDeath) != units.end()) {
		killer = units[causeOfDeath].name;
		if (units[causeOfDeath].human()) {
			kills.push_back(causeOfDeath);
		}
	}
	
	vector<string> killWords;
	vector<string> afterWords;
	killWords.push_back(" slaughtered "); afterWords.push_back("!");
	killWords.push_back(" made "); afterWords.push_back(" his bitch!");
	killWords.push_back(" has balls of steel! "); afterWords.push_back(" is a casualty");
	killWords.push_back(" owned "); afterWords.push_back("'s ass!");
	killWords.push_back(" ravaged "); afterWords.push_back(" inside out!");
	killWords.push_back(" dominated "); afterWords.push_back("!");
	killWords.push_back(" demonstrated to "); afterWords.push_back(" the art of .. spanking!");
	killWords.push_back(" has knocked "); afterWords.push_back(" out cold!");
	killWords.push_back(" defiled "); afterWords.push_back("'s remains!");
	killWords.push_back(" shoved it up "); afterWords.push_back("'s ass!");
	killWords.push_back(" is laughing at "); afterWords.push_back("'s lack of skill!");
	
	int i = currentWorldFrame % killWords.size();
	msg << killer << killWords[i] << unit.name << afterWords[i];
	worldMessages.push_back(msg.str());
	
	WorldEvent event;
	event.position = unit.position;
	event.position.y.number += 2000;
	event.velocity.y.number = 200;
	
	if(unit.human())
	{
		event.type = DEATH_PLAYER;
		
		// reset player hitpoints
		unit.hitpoints = 1000;
		
		// respawn player to random location
		unit.position = lvl.getRandomLocation(currentWorldFrame);
		
		// stop any movement, let the player drop down to the field of battle.
		unit.velocity.x = 0;
		unit.velocity.z = 0;
		unit.velocity.y = 0;
	}
	else
	{
		event.type = DEATH_ENEMY;
		deadUnits.push_back(unit.id);
	}
	
	// store the event information for later use.
	events.push_back(event);
}

void World::resolveUnitCollision(Unit& a, Unit& b)
{
	
	Location direction = (a.position - b.position);
	direction.normalize();
	direction *= FixedPoint(200, true);
	
	a.velocity += direction;
	b.velocity -= direction;
}


void World::generateInput_RabidAlien(Unit& unit)
{
	FixedPoint bestDistance = FixedPoint(1000);
	int unitID = -1;
	
	// find the nearest human controlled unit
	for(map<int, Unit>::iterator it = units.begin(), et = units.end(); it != et; ++it)
	{
		if(it->second.controllerTypeID == Unit::HUMAN_INPUT) // MMM!! MAYBE I CAN GO KILL THIS PLAYER ?:DD
		{
			FixedPoint tmp_dist = (it->second.position - unit.position).length();
			if( tmp_dist < bestDistance )
			{
				bestDistance = tmp_dist;
				unitID = it->first;
			}
		}
	}
	
	// if no nearby human controlled unit was found, sleep
	if(unitID == -1)
	{
		// nothing interesting going on, disable unit.
		unit.updateInput(0, 0, 0, 0);
		return;
	}
	
	// if close enough, do damage by DEVOURING
	if(bestDistance < FixedPoint(3))
	{
		// DEVOUR!
		units[unitID].hitpoints -= 173; // devouring does LOTS OF DAMAGE!
		if(units[unitID].hitpoints < 1)
			doDeathFor(units[unitID], unit.id);
		
		// save this information for later use.
		WorldEvent event;
		event.type = DAMAGE_DEVOUR;
		event.position = units[unitID].position;
		event.position.y.number += 2000;
		event.velocity.y.number = 900;
		events.push_back(event);
	}
	
	// turn towards the human unit until facing him. then RUSH FORWARD!
	Location direction = units[unitID].position - unit.position;
	direction.normalize();
	
	Location myDirection;
	myDirection.z = apomath.getSin(unit.angle);
	myDirection.x = apomath.getCos(unit.angle);
	
	FixedPoint error = (myDirection.x - direction.x) * (myDirection.x - direction.x) + (myDirection.z - direction.z) * (myDirection.z - direction.z);
	
	int keyState = 0;
	int mousex = 0, mousey = 0;
	int mousebutton = 0;
	
	mousex = error.number / 4;
	keyState |= Unit::MOVE_FRONT;
	
	unit.upangle = apomath.DEGREES_90 - apomath.DEGREES_90 / 50;
	
	if( ((currentWorldFrame + unit.birthTime) % 140) < 20)
	{
		keyState |= Unit::JUMP;
	}
	
	if( ((currentWorldFrame + unit.birthTime) % 140) > 100 )
	{
		mousebutton = 1;
	}
	
	unit.updateInput(keyState, mousex, mousey, mousebutton);
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
	show_errors = 0;
}

void World::terminate()
{
	_unitID_next_unit = 10000;
	units.clear();
	models.clear();
}


void World::tickUnit(Unit& unit, Model& model)
{
	if(unit.controllerTypeID == Unit::AI_RABID_ALIEN)
	{
		generateInput_RabidAlien(unit);
	}
	
	// update the information according to which the unit model will be updated from now on
	model.parts[model.root].rotation_x = unit.getAngle(apomath);
	model.updatePosition(unit.position.x.getFloat(), unit.position.y.getFloat(), unit.position.z.getFloat());
	
	// some physics & game world information
	bool hitGround = false;
	if( (unit.velocity.y + unit.position.y) <= lvl.getHeight(unit.position.x, unit.position.z) )
	{
		FixedPoint friction = FixedPoint(88) / FixedPoint(100);
		
		unit.position.y = lvl.getHeight(unit.position.x, unit.position.z);
		unit.velocity.y.number = 0;
		unit.velocity.x *= friction;
		unit.velocity.z *= friction;
		hitGround = true;
	}
	else
	{
		unit.velocity.y.number -= 35;
	}
	
	// do something about unit to unit collisions
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		if(iter->second.id == unit.id)
			continue;
		if((iter->second.position - unit.position).length() < FixedPoint(2))
			resolveUnitCollision(unit, iter->second);
	}
	
	
	if(unit.getKeyAction(Unit::MOVE_FRONT) && hitGround)
	{
		FixedPoint scale = FixedPoint(10) / FixedPoint(100);
		unit.velocity.x += apomath.getCos(unit.angle) * scale;
		unit.velocity.z += apomath.getSin(unit.angle) * scale;
	}
	
	
	if(unit.getKeyAction(Unit::MOVE_BACK) && hitGround)
	{
		FixedPoint scale = FixedPoint(6) / FixedPoint(100);
		unit.velocity.x -= apomath.getCos(unit.angle) * scale;
		unit.velocity.z -= apomath.getSin(unit.angle) * scale;
	}

	if(unit.getKeyAction(Unit::MOVE_LEFT) && hitGround)
	{
		FixedPoint scale = FixedPoint(8) / FixedPoint(100);
		int dummy_angle = unit.angle - apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
	}
	if(unit.getKeyAction(Unit::MOVE_RIGHT) && hitGround)
	{
		FixedPoint scale = FixedPoint(8) / FixedPoint(100);
		int dummy_angle = unit.angle + apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
	}

	if(unit.leap_cooldown == 0)
	{
		FixedPoint scale;
		scale.number = 950;
		if(unit.getKeyAction(Unit::LEAP_LEFT) && hitGround)
		{
			int dummy_angle = unit.angle - apomath.DEGREES_90;
			
			unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
			unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
			unit.velocity.y += FixedPoint(450) / FixedPoint(1000);
			unit.leap_cooldown = 60;
		}
		if(unit.getKeyAction(Unit::LEAP_RIGHT) && hitGround)
		{
			int dummy_angle = unit.angle + apomath.DEGREES_90;
			
			unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
			unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
			unit.velocity.y += FixedPoint(450) / FixedPoint(1000);
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
			unit.weapon_cooldown = 5;

			// TODO: Following is somewhat duplicated from Camera :G

			// TODO: Fix to use some common ApoMath.
			static ApoMath dorka;
			if(!dorka.ready())
				dorka.init(3000);

			Location position;
			position.x = 30;
			position.z = 0;
			position.y = 0;

			int angle   = -unit.angle;
			int upangle = unit.upangle;

			FixedPoint cos = dorka.getCos(angle);
			FixedPoint sin = dorka.getSin(angle);
			FixedPoint upcos = dorka.getCos(upangle);
			FixedPoint upsin = dorka.getSin(upangle);
			
			Location relative_position;
			FixedPoint x = position.x;
			FixedPoint y = position.y;
			FixedPoint z = position.z;
			relative_position.x = cos * upcos * x - sin * z + cos * upsin * y;
			relative_position.z = sin * upcos * x + cos * z + sin * upsin * y;
			relative_position.y =      -upsin * x           +       upcos * y;

			Location weapon_position = unit.position;
			Location projectile_direction = unit.position + relative_position;
			weapon_position.y += 4;
			projectile_direction.y += 4;


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
			projectile.owner = unit.id;
			
			projectile.tick();
			projectile.lifetime = 50;
		}
	}
	else
	{
		--unit.weapon_cooldown;
	}
	
	
	FixedPoint reference_x = unit.position.x + unit.velocity.x;
	FixedPoint reference_z = unit.position.z + unit.velocity.z;
	FixedPoint reference_y = lvl.getHeight(reference_x, reference_z);
	FixedPoint y_diff = reference_y - unit.position.y;
	
	FixedPoint yy_val = heightDifference2Velocity(y_diff);

	if(!hitGround)
	{
		yy_val = FixedPoint(1);
	}

	unit.position.z += unit.velocity.z * yy_val;
	unit.position.x += unit.velocity.x * yy_val;
	
	if(unit.getKeyAction(Unit::JUMP) && hitGround)
	{
		unit.velocity.y = FixedPoint(900) / FixedPoint(1000);
	}

	unit.position.y += unit.velocity.y;
}

void World::tickProjectile(Projectile& projectile, Model& model, int id)
{
	// model.parts[model.root].rotation_x = projectile.getAngle(apomath);
	model.updatePosition(projectile.curr_position.x.getFloat(), projectile.curr_position.y.getFloat(), projectile.curr_position.z.getFloat());

	// cerr << "Proj lifetime: " << projectile.lifetime << ", " << projectile.position << ", vel: " << projectile.velocity << "\n";
	if(projectile.lifetime > 0)
	{
		projectile.tick();

		for(map<int, Unit>::iterator it = units.begin(), et = units.end(); it != et; ++it)
		{
			Unit& unit = it->second;
			if(projectile.collides(unit))
			{
				bool shooterIsMonster = false;
				if(units.find(projectile.owner) != units.end())
					shooterIsMonster = !units[projectile.owner].human();
				
				
				// save this information for later use.
				WorldEvent event;
				event.type = DAMAGE_BULLET;
				event.position = unit.position;
				event.position.y.number += 2000;
				event.velocity.y.number = 200;
				events.push_back(event);
				
				
				// if monster is shooting a monster, just destroy the bullet. dont let them kill each other :(
				if(!unit.human() && shooterIsMonster)
				{
					deadUnits.push_back(id);
					continue;
				}
				
				unit.hitpoints -= 170; // bullet does SEVENTEEN HUNDRED DAMAGE (we need some kind of weapon definitions)
				unit.velocity += projectile.velocity * FixedPoint(10, true);
				
				if(unit.hitpoints < 1)
				{
					doDeathFor(unit, projectile.owner);
				}
				
				deadUnits.push_back(id);
			}
			else if(projectile.collidesTerrain(lvl))
			{
				deadUnits.push_back(id);
			}
		}
	}
	else
	{
		deadUnits.push_back(id);
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

void World::worldTick(int tickCount)
{
	currentWorldFrame = tickCount;
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		tickUnit(iter->second, models[iter->first]);
	}
	
	for(map<int, Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		tickProjectile(iter->second, models[iter->first], iter->first);
	}
	
	for(int i = 0; i < deadUnits.size(); i++)
	{
		removeUnit(deadUnits[i]);
	}
	
	deadUnits.clear();
	
	
	if(show_errors && (currentWorldFrame % 200) == 0)
	{

		for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
		{
			stringstream msg;
			msg << currentWorldFrame << ": " << iter->second.name << " (" << iter->first << "): " << iter->second.position.x.getFloat() << ", " << iter->second.position.z.getFloat();
			worldMessages.push_back(msg.str());
		}
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
void World::addUnit(int id, bool playerCharacter)
{
	units[id] = Unit();
	units[id].position.x = FixedPoint(50);
	units[id].position.z = FixedPoint(50);
	units[id].id = id;
	
	units[id].birthTime = currentWorldFrame;
	
	models[id] = Model();
	models[id].load("data/model.bones");
	
	
	if(!playerCharacter)
	{
		units[id].name = "Alien monster";
		units[id].controllerTypeID = Unit::AI_RABID_ALIEN;
	}
	else
	{
		units[id].name = "Unknown Player";
		units[id].controllerTypeID = Unit::HUMAN_INPUT;
	}
	
}

void World::addProjectile(Location& location, int id)
{
	Vec3 position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();
	
	models[id].load("data/bullet.bones");
	models[id].realUnitPos = position;
	models[id].currentModelPos = position;

	projectiles[id].curr_position = location;
	projectiles[id].owner = id;
	
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

int World::getZombies()
{
	int count = 0;
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		if (!iter->second.human())
			count++;
	}
	return count;
}
