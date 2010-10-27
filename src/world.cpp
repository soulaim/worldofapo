#include "world.h"
#include "apomodel.h"
#include "skeletalmodel.h"

#include <iostream>

using namespace std;

FixedPoint World::heightDifference2Velocity(const FixedPoint& h_diff) const
{
	// no restrictions for moving downhill
	if(h_diff < FixedPoint::ZERO)
		return 1;
	if(h_diff >= FixedPoint(2))
		return FixedPoint::ZERO;
	return (FixedPoint(2) - h_diff)/FixedPoint(2);
}

void World::doDeathFor(Unit& unit)
{
	stringstream msg;
	string killer = "an unknown entity";
	
	int causeOfDeath = unit.last_damage_dealt_by;
	
	int actor_id  = -1;
	int target_id = -1;
	
	target_id = unit.id;
	
	if(units.find(causeOfDeath) != units.end())
	{
		killer = units[causeOfDeath].name;
		actor_id = units[causeOfDeath].id;
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
	
	event.target_id = target_id;
	event.actor_id  = actor_id;
	
	event.position = unit.position;
	event.position.y += FixedPoint(2);
	event.velocity.y = FixedPoint(200,1000);
	
	// ALERT code for creating lights should have it's own function.
	LightObject& light = lights[nextUnitID()];
	light.setDiffuse(8.f, 8.f, 8.f);
	light.setSpecular(0.f, 0.f, 0.f);
	light.setLife(100); // Some frames of LIGHT!
	light.setPower(5); // this doesnt actually do anything yet, but lets set it anyway.
	light.activateLight(); // ACTIVATE :D
	light.position = event.position;
	light.position.y += FixedPoint(3, 2);
	
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

void World::generateInput_RabidAlien(Unit& unit)
{
	FixedPoint bestSquaredDistance = FixedPoint(1000000);
	int unitID = -1;
	
	// find the nearest human controlled unit
	for(map<int, Unit>::iterator it = units.begin(), et = units.end(); it != et; ++it)
	{
		if(it->second.controllerTypeID == Unit::HUMAN_INPUT) // MMM!! MAYBE I CAN GO KILL THIS PLAYER ? :DD
		{
			FixedPoint tmp_dist = (it->second.position - unit.position).lengthSquared();
			if( tmp_dist < bestSquaredDistance )
			{
				bestSquaredDistance = tmp_dist;
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
	if(bestSquaredDistance < FixedPoint(9))
	{
		// DEVOUR!
		units[unitID].hitpoints -= 173; // devouring does LOTS OF DAMAGE!
		units[unitID].last_damage_dealt_by = unit.id;
		
		// save this information for later use.
		WorldEvent event;
		event.type = DAMAGE_DEVOUR;
		event.position = units[unitID].position;
		event.position.y += FixedPoint(2);
		event.velocity.y = FixedPoint(900,1000);
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
	int mousex = 0;
	int mousey = 0;
	int mousebutton = 0;
	
	error *= FixedPoint(250);
	mousex = error.getInteger();
	keyState |= Unit::MOVE_FRONT;
	
	unit.upangle = apomath.DEGREES_90 - apomath.DEGREES_90 / 50;
	
	if( ((currentWorldFrame + unit.birthTime) % 140) < 20)
	{
		keyState |= Unit::JUMP;
	}
	
	if( ((currentWorldFrame + unit.birthTime) % 140) > 100 )
	{
		mousex += ( ((unit.birthTime + currentWorldFrame) * 23) % 200) - 100;
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
	
	show_errors = 0;
}

void World::terminate()
{
	_unitID_next_unit = 10000;
	_playerID_next_player = 0;
	
	medikits.clear();
	units.clear();
	models.clear();
	projectiles.clear();
}


void World::tickUnit(Unit& unit, Model* model)
{
	if(unit.controllerTypeID == Unit::AI_RABID_ALIEN)
	{
		generateInput_RabidAlien(unit);
	}
	
	unit.soundInfo = "";
	
	// update the information according to which the unit model will be updated from now on
	model->rotate_y(unit.getAngle(apomath));
	model->updatePosition(unit.position.x.getFloat(), unit.position.y.getFloat(), unit.position.z.getFloat());
	
	// some physics & game world information
	bool hitGround = false;
	if( (unit.velocity.y + unit.position.y) <= lvl.getHeight(unit.position.x, unit.position.z) )
	{
		// TODO: Heavy landings should have a gameplay effect!
		if(unit.velocity.y < FixedPoint(-7, 10))
			unit.soundInfo = "jump_land";
		if(unit.velocity.y < FixedPoint(-12, 10))
		{
			unit.velocity.x *= FixedPoint(10, 100);
			unit.velocity.z *= FixedPoint(10, 100);
			unit.hitpoints -= 50;
			
			unit.last_damage_dealt_by = unit.id;
		}
		
		FixedPoint friction = FixedPoint(88, 100);
		
		unit.position.y = lvl.getHeight(unit.position.x, unit.position.z);
		unit.velocity.y = FixedPoint::ZERO;
		unit.velocity.x *= friction;
		unit.velocity.z *= friction;
		hitGround = true;
	}
	else
	{
		unit.velocity.y -= FixedPoint(35,1000);
		
		// air resistance :DD
		FixedPoint friction = FixedPoint(995, 1000);
		unit.velocity.x *= friction;
		unit.velocity.z *= friction;
	}

	if(unit.getKeyAction(Unit::WEAPON1))
	{
		unit.switchWeapon(1);
	}

	if(unit.getKeyAction(Unit::WEAPON2))
	{
		unit.switchWeapon(2);
	}
	
	if(unit.getKeyAction(Unit::MOVE_FRONT) && hitGround)
	{
		FixedPoint scale = FixedPoint(10, 100);
		unit.velocity.x += apomath.getCos(unit.angle) * scale;
		unit.velocity.z += apomath.getSin(unit.angle) * scale;
	}
	
	if(unit.getKeyAction(Unit::MOVE_BACK) && hitGround)
	{
		FixedPoint scale = FixedPoint(6, 100);
		unit.velocity.x -= apomath.getCos(unit.angle) * scale;
		unit.velocity.z -= apomath.getSin(unit.angle) * scale;
	}

	if(unit.getKeyAction(Unit::MOVE_LEFT) && hitGround)
	{
		FixedPoint scale = FixedPoint(8, 100);
		int dummy_angle = unit.angle - apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
	}
	if(unit.getKeyAction(Unit::MOVE_RIGHT) && hitGround)
	{
		FixedPoint scale = FixedPoint(8, 100);
		int dummy_angle = unit.angle + apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
	}
	
	if(unit.getKeyAction(Unit::MOVE_RIGHT | Unit::MOVE_LEFT | Unit::MOVE_FRONT | Unit::MOVE_BACK) && (unit.soundInfo == ""))
		unit.soundInfo = "walk";

	if(unit.leap_cooldown == 0)
	{
		FixedPoint scale(950,1000);
		if(unit.getKeyAction(Unit::LEAP_LEFT) && hitGround)
		{
			int dummy_angle = unit.angle - apomath.DEGREES_90;
			
			unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
			unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
			unit.velocity.y += FixedPoint(45, 100);
			unit.leap_cooldown = 40;
			
			unit.soundInfo = "jump";
			// unit.soundInfo = "leap";
		}
		if(unit.getKeyAction(Unit::LEAP_RIGHT) && hitGround)
		{
			int dummy_angle = unit.angle + apomath.DEGREES_90;
			
			unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
			unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
			unit.velocity.y += FixedPoint(45, 100);
			unit.leap_cooldown = 40;
			
			unit.soundInfo = "jump";
			// unit.soundInfo = "leap";
		}
	}
	else
	{
		--unit.leap_cooldown;
	}

	if (unit.getMouseAction(Unit::ATTACK_BASIC))
	{
		unit.weapon->fire();
	}
	else
	{
		unit.weapon->tick();
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

	if(unit.getKeyAction(Unit::JUMP) && hitGround)
	{
		unit.soundInfo = "jump";
		unit.velocity.y = FixedPoint(900, 1000);
	}
	
	/*
	// EISS VOIVVV :GG
	Location xz_movement = unit.velocity;
	xz_movement.y = 0;
	xz_movement.normalize();
	
	// terrain bump on xz-plane
	if( (unit.velocity.y + unit.position.y + FixedPoint(600, 1000) ) <= lvl.getHeight(unit.position.x + xz_movement.x, unit.position.z + xz_movement.z) )
	{
		unit.velocity.z = 0;
		unit.velocity.x = 0;
	}
	*/
	
	unit.position.z += unit.velocity.z * yy_val;
	unit.position.x += unit.velocity.x * yy_val;
	unit.position.y += unit.velocity.y;

	
	if(unit.position.x < 0)
	{
		unit.position.x = 0;
	}
	if(unit.position.x > lvl.max_x())
	{
		unit.position.x = lvl.max_x();
	}
	if(unit.position.z < 0)
	{
		unit.position.z = 0;
	}
	if(unit.position.z > lvl.max_z())
	{
		unit.position.z = lvl.max_z();
	}
}

void World::tickProjectile(Projectile& projectile, Model* model, int id)
{
	// model.parts[model.root].rotation_y = projectile.getAngle(apomath);
	model->updatePosition(projectile.curr_position.x.getFloat(), projectile.curr_position.y.getFloat(), projectile.curr_position.z.getFloat());

	// cerr << "Proj lifetime: " << projectile.lifetime << ", " << projectile.position << ", vel: " << projectile.velocity << "\n";
	if(projectile.lifetime > 0)
	{
		projectile.tick();
		if(projectile.collidesTerrain(lvl))
		{
			deadUnits.push_back(id);
		}
		auto potColl = o->nearObjects(projectile.curr_position);
		for(auto it = potColl.begin(); it != potColl.end(); ++it)
		{
			if ((*it)->type != OctreeObject::UNIT)
				continue;

			Unit* u = (Unit*) *it;
			if(projectile.collides(*u))
			{
				bool shooterIsMonster = false;
				if(units.find(projectile.owner) != units.end())
					shooterIsMonster = !units[projectile.owner].human();
				
				// if monster is shooting a monster, just destroy the bullet. dont let them kill each other :(
				if(!u->human() && shooterIsMonster)
				{
					deadUnits.push_back(projectile.id);
					continue;
				}
				
				// save this information for later use.
				WorldEvent event;
				event.type = DAMAGE_BULLET;
				event.position = u->position;
				event.position.y += FixedPoint(2);
				event.velocity.y = FixedPoint(200,1000);
				events.push_back(event);
				
				
				u->hitpoints -= 170; // bullet does SEVENTEEN HUNDRED DAMAGE (we need some kind of weapon definitions)
				u->velocity += projectile.velocity * FixedPoint(1, 100);
				u->last_damage_dealt_by = projectile.owner;
				
				deadUnits.push_back(projectile.id);
				break;
			}
		}
	}
	else
	{
		deadUnits.push_back(id);
	}
}

void World::updateModel(Model* model, Unit& unit)
{
	/*
	// deduce which animation to display
	if(unit.position.h - FixedPoint(100,1000) > lvl.getHeight(unit.position.x, unit.position.y))
	{
		model.setAction("jump");
	}
	else */
	
	if(model == 0)
	{
		cerr << "Model doesnt exist ffs" << endl;
		return;
	}
	
	if(unit.getKeyAction(Unit::MOVE_FRONT | Unit::MOVE_BACK | Unit::MOVE_LEFT | Unit::MOVE_RIGHT))
	{
		model->setAction("walk");
	}
	else
	{
		model->setAction("idle2");
	}
	
	// update state of model
	model->tick(currentWorldFrame);
}

void World::worldTick(int tickCount)
{
	
	// TODO: should have a method to update the state of a MovableObject !! (instead of a separate tick for every type..)
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	 *    \  Handling of light objects \
	 *    \_/""""""""""""""""""""""""""""/
	 */
	
	vector<int> deadLights;
	for(auto iter = lights.begin(); iter != lights.end(); iter++)
	{
		LightObject& light = iter->second;
		
		// TODO should have a world.tickMovableObject(&light); call here
		
		// update light qualities
		if(!light.tickLight())
		{
			// if light has died out, should maybe like prepare to erase it or something.
			light.deactivateLight();
			deadLights.push_back(iter->first);
			
			// cerr << "DEAD LIGHT! ERASING" << endl;
		}
	}
	
	for(size_t i=0; i<deadLights.size(); i++)
	{
		lights.erase(deadLights[i]);
	}
	deadLights.clear();
	
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Unit ticks + reconstruct octree \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/
	
	
	o.reset(new Octree(Location(0, 0, 0), Location(FixedPoint(lvl.max_x()), FixedPoint(400), FixedPoint(lvl.max_z()))));
	currentWorldFrame = tickCount;
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		tickUnit(iter->second, models[iter->first]);
		o->insertObject(&(iter->second));
	}
	
	
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Projectile ticks + collisions   \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/
	
	
	
	for(map<int, Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		tickProjectile(iter->second, models[iter->first], iter->first);
	}
	

	for(auto it = medikits.begin(); it != medikits.end(); ++it)
	{
		Medikit& kit = it->second;
		kit.tick(lvl.getHeight(kit.position.x, kit.position.z));
		if (kit.dead)
			deadUnits.push_back(it->first);
		else
			o->insertObject(&kit);
	}

	o->doCollisions();
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Find dead units                 \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/
	
	for(auto iter = units.begin(); iter != units.end(); ++iter)
	{
		if(iter->second.hitpoints < 1)
		{
			doDeathFor(iter->second);
		}
	}
	
	for(size_t i = 0; i < deadUnits.size(); ++i)
	{
		removeUnit(deadUnits[i]);
	}
	deadUnits.clear();
	
	
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Debug stuff                     \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/
	
	
	if(show_errors && (currentWorldFrame % 200) == 0)
	{
		for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
		{
			if(iter->second.human())
			{
				stringstream msg;
				msg << currentWorldFrame << ": " << iter->second.name << " (" << iter->first << "): " << iter->second.position.x.getFloat() << ", " << iter->second.position.z.getFloat();
				worldMessages.push_back(msg.str());
			}
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
		Model* model = models[iter->first];
		model->setAction("idle");
		model->tick(currentWorldFrame);
	}
}


// TODO: FIX THIS trololol..
void World::addUnit(int id, bool playerCharacter)
{
	units[id] = Unit();
	units[id].init(*this);
	units[id].position = lvl.getRandomLocation(currentWorldFrame);
	units[id].id = id;
	
	units[id].birthTime = currentWorldFrame;

	static SkeletalModel prototype;
	static bool loaded = false;
	if(!loaded)
	{
		loaded = true;
		prototype.load("models/model.skeleton");
		prototype.texture_name = "marine";
	}
	
	models[id] = new SkeletalModel(prototype);
	
	if(!playerCharacter)
	{
		units[id].name = "Alien monster";
		units[id].controllerTypeID = Unit::AI_RABID_ALIEN;
		units[id].hitpoints = 1000;
	}
	else
	{
		units[id].name = "Unknown Player";
		units[id].controllerTypeID = Unit::HUMAN_INPUT;
		units[id].hitpoints = 1000;
	}
	
}

void World::addProjectile(Location& location, int id)
{
	static ApoModel prototype;
	if(prototype.root == -1)
	{
		prototype.load("models/bullet.bones");
	}
	Vec3 position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();
	
	models[id] = new ApoModel(prototype); // TODO: still copies some extra constants.
	models[id]->realUnitPos = position;
	models[id]->currentModelPos = position;

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
	medikits.erase(id);
	delete models[id];
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

std::vector<Location> World::humanPositions()
{
	std::vector<Location> positions;
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		if (iter->second.human())
			positions.push_back(iter->second.position);
	}
	return positions;
}

void World::add_message(const std::string& message)
{
	worldMessages.push_back(message);
}

void World::add_event(const WorldEvent& event)
{
	events.push_back(event);
}

