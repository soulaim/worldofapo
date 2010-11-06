#include "world.h"
#include "modelfactory.h"

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

unsigned long World::checksum() const {
	unsigned long hash = 5381;

	for (auto it = units.begin(); it != units.end(); ++it) {
		int id = it->first;
		Location pos = it->second.getPosition();
		hash = ((hash << 5) + hash) + id;
		hash = ((hash << 5) + hash) + pos.x.getInteger();
		hash = ((hash << 5) + hash) + pos.y.getInteger();
		hash = ((hash << 5) + hash) + pos.z.getInteger();
	}

	return hash;
}

// TODO: Force expansion should not be instant.
void World::instantForceOutwards(const FixedPoint& power, const Location& pos)
{
	for(auto iter = units.begin(); iter != units.end(); iter++)
	{
		const Location& pos2 = iter->second.getPosition();
		Location velocity_vector = (pos2 - pos);
		
		FixedPoint distance = velocity_vector.length();
		velocity_vector.normalize();
		
		if(distance == FixedPoint(0))
			distance = FixedPoint(1, 10);
		
		velocity_vector *= power;
		velocity_vector /= distance * distance;
		
		iter->second.velocity += velocity_vector;
	}
}

void World::atDeath(MovableObject& object, HasProperties& properties)
{
	// TODO: This function is a stub.
	object.getGravity();
	
	// this way can only store one event to be executed at death :( should maybe reconcider that.
	if(properties("AT_DEATH") == "EXPLODE")
	{
		FixedPoint explode_power(properties["EXPLODE_POWER"]);
		const Location& pos = object.position;
		
		instantForceOutwards(explode_power, pos);
	}
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
	
	WorldEvent event;
	event.target_id = target_id;
	if(actor_id != target_id)
	{
		event.actor_id  = actor_id;
		
		int i = currentWorldFrame % killWords.size();
		msg << killer << killWords[i] << unit.name << afterWords[i];
		worldMessages.push_back(msg.str());
	}
	else
	{
		event.actor_id  = -1; // For a suicide, no points are to be awarded.
		
		msg << killer << " has committed suicide!" << endl;
		worldMessages.push_back(msg.str());
	}
	
	event.position = unit.getPosition();
	event.position.y += FixedPoint(2);
	event.velocity.y = FixedPoint(200,1000);
	
	addLight(event.position);
	
	if(unit.human())
	{
		event.type = DEATH_PLAYER;
		
		// reset player hitpoints
		unit.hitpoints = 1000;
		
		// respawn player to random location
		unit.setPosition(lvl.getRandomLocation(currentWorldFrame));
		
		// stop any movement, let the player drop down to the field of battle.
		unit.zeroMovement();
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
	
	if(direction.length() == 0)
	{
		// wtf, same position as my target? :G
		
		// so probably im devouring him alot ahahaha :DD
		
		return;
	}
	
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
	
	// find the highest point in lvl and add a strong light there.
	LightObject tmp_light;
	tmp_light.unitBind = -1;
	tmp_light.lifeType = LightSource::IMMORTAL;
	tmp_light.behaviour = LightSource::RISE_AND_DIE;
	tmp_light.setDiffuse(8.0, 2.0, 2.0);
	tmp_light.setLife(150);
	tmp_light.activateLight();
	tmp_light.position = Location(FixedPoint(500), FixedPoint(80), FixedPoint(500));
	lights[nextUnitID()] = tmp_light;

	// Make sure there is atleast MAX_NUM_ACTIVE_LIGHTS.
	tmp_light.position = Location(FixedPoint(100), FixedPoint(80), FixedPoint(500));
	lights[nextUnitID()] = tmp_light;
	tmp_light.position = Location(FixedPoint(500), FixedPoint(80), FixedPoint(100));
	lights[nextUnitID()] = tmp_light;
	tmp_light.position = Location(FixedPoint(100), FixedPoint(80), FixedPoint(100));
	lights[nextUnitID()] = tmp_light;
	
	show_errors = 0;
}

void World::terminate()
{
	_unitID_next_unit = 10000;
	_playerID_next_player = 0;
	
	medikits.clear();
	units.clear();
	for(auto it = models.begin(); it != models.end(); ++it)
	{
		ModelFactory::destroy(it->second);
	}
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
	
	// TODO: heavy landing is a special case of any kind of collisions. Other collisions are still not handled.
	
	// some physics & game world information
	bool hitGround = false;
	if( (unit.velocity.y + unit.position.y) <= lvl.getHeight(unit.position.x, unit.position.z) )
	{
		// TODO: Heavy landings should have a gameplay effect!
		if(unit.velocity.y < FixedPoint(-7, 10))
			unit.soundInfo = "jump_land";
		if(unit.velocity.y < FixedPoint(-12, 10))
		{
			unit.last_damage_dealt_by = unit.id;
			
			FixedPoint damage_fp = unit.velocity.y + FixedPoint(12, 10);
			int damage_int = damage_fp.getDesimal() + damage_fp.getInteger() * FixedPoint::FIXED_POINT_ONE;
			
			if(damage_int < -500)
			{
				// is hitting the ground REALLY HARD. Nothing could possibly survive. Just insta-kill.
				unit.hitpoints = -1;
			}
			else
			{
				unit.velocity.x *= FixedPoint(10, 100);
				unit.velocity.z *= FixedPoint(10, 100);
				unit.hitpoints -= damage_int * damage_int / 500;
			}
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
	
	if(unit.getKeyAction(Unit::WEAPON3))
	{
		unit.switchWeapon(3);
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

	unit.weapon->tick();
	if (unit.getMouseAction(Unit::ATTACK_BASIC))
	{
		unit.weapon->onUse();
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

void World::tickProjectile(Projectile& projectile, Model* model)
{
	int ticks = projectile["TPF"];
	if(ticks < 1)
		ticks = 1;
	for(int i=0; i<ticks; i++)
	{
		model->updatePosition(projectile.position.x.getFloat(), projectile.position.y.getFloat(), projectile.position.z.getFloat());
		
		if(projectile["LIFETIME"] > 0 && !projectile.destroyAfterFrame)
		{
			projectile.tick();
			
			if(projectile.collidesTerrain(lvl))
			{
				// if collides with terrain, projectile will die at the end of this turn.
				// But should still check whether we hit something on the way to the point of collision!
				projectile.destroyAfterFrame = true;
				
				// intentional continue of execution
			}
			
			auto potColl = o->nearObjects(projectile.position);
			for(auto it = potColl.begin(); it != potColl.end(); ++it)
			{
				if ((*it)->type != OctreeObject::UNIT)
					continue;
				
				Unit* u = (Unit*) *it;
				
				// if the target unit is already dead, just continue. ALERT: This might cause desync, if potColl is unordered
				if(u->hitpoints <= 0)
					continue;
				
				if(projectile.collides(*u))
				{
					bool shooterIsMonster = false;
					if(units.find(projectile["OWNER"]) != units.end())
						shooterIsMonster = !units[projectile["OWNER"]].human();
					
					// if monster is shooting a monster, just destroy the bullet. dont let them kill each other :(
					if(!u->human() && shooterIsMonster)
					{
						projectile.destroyAfterFrame = true;
						continue;
					}
					
					// save this information for later use.
					WorldEvent event;
					event.type = DAMAGE_BULLET;
					event.position = u->position;
					event.position.y += FixedPoint(2);
					event.velocity.y = FixedPoint(200,1000);
					events.push_back(event);
					
					u->hitpoints -= projectile["DAMAGE"]; // does damage according to weapon definition :)
					u->velocity += projectile.velocity * FixedPoint(projectile["MASS"], 1000);
					u->last_damage_dealt_by = projectile["OWNER"];
					
					projectile.destroyAfterFrame = true;
					continue;
				}
			}
		}
		else
		{
			projectile.destroyAfterFrame = true;
		}
	}
	
	// as a post frame update, update values of the projectile
	if(projectile["AIR_RESISTANCE"])
		projectile.velocity *= FixedPoint(projectile["AIR_RESISTANCE"], 1000);
	projectile.velocity.y += FixedPoint(projectile["GRAVITY"], 1000);
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
	
	if(unit.getKeyAction(Unit::MOVE_FRONT))
	{
		if(unit.getKeyAction(Unit::MOVE_LEFT))
			model->setAction("run_forwardleft");
		else if(unit.getKeyAction(Unit::MOVE_RIGHT))
			model->setAction("run_forwardright");
		else
			model->setAction("run_forward");
	}
	else if(unit.getKeyAction(Unit::MOVE_BACK))
	{
		model->setAction("run_backward");
	}
	else if(unit.getKeyAction(Unit::MOVE_LEFT))
	{
		model->setAction("run_left");
	}
	else if(unit.getKeyAction(Unit::MOVE_RIGHT))
	{
		model->setAction("run_right");
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
		tickProjectile(iter->second, models[iter->first]);
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
		
		if(light.unitBind != -1)
		{
			if(units.find(light.unitBind) == units.end())
			{
				// master of the light is dead. must kill the light.
				light.deactivateLight();
				deadLights.push_back(iter->first);
			}
			else
			{
				// light.drawPos  = models.find(light.unitBind)->second.currentModelPos;
				light.position = units.find(light.unitBind)->second.getPosition();
				light.position.y += FixedPoint(7, 2);
			}
		}
	}
	
	for(size_t i=0; i<deadLights.size(); i++)
	{
		lights.erase(deadLights[i]);
	}
	deadLights.clear();
	
	
	
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
	
	for(auto iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		Projectile& projectile = iter->second;
		if(projectile.destroyAfterFrame)
		{
			atDeath(projectile, projectile); // NICE!! :D
			deadUnits.push_back(iter->first);
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

void World::addLight(Location& location)
{
//	cerr << "Adding light at " << location << endl;
	LightObject& light = lights[nextUnitID()];
	light.setDiffuse(8.f, 8.f, 8.f);
	light.setSpecular(0.f, 0.f, 0.f);
	light.setLife(200); // Some frames of LIGHT!
	light.setPower(5); // this doesnt actually do anything yet, but lets set it anyway.
	light.activateLight(); // ACTIVATE :D
	light.position = location;
	light.position.y += FixedPoint(3, 2);
}

void World::addUnit(int id, bool playerCharacter)
{
	units[id] = Unit();
	units[id].init(*this);
	units[id].position = lvl.getRandomLocation(currentWorldFrame);
	units[id].id = id;
	
	units[id].birthTime = currentWorldFrame;
	
	models[id] = ModelFactory::create(ModelFactory::PLAYER_MODEL);
	models[id]->texture_name = "marine";
	
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
	Vec3 position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();
	
	models[id] = ModelFactory::create(ModelFactory::BULLET_MODEL);
	models[id]->realUnitPos = position;
	models[id]->currentModelPos = position;
	
	projectiles[id].position = location;
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
	// Note that same id might be removed twice on the same frame!

	units.erase(id);
	projectiles.erase(id);
	medikits.erase(id);

	auto it = models.find(id);
	if(it != models.end())
	{
		ModelFactory::destroy(it->second);
		models.erase(it);
	}
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

