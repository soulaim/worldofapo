#include "world.h"
#include "modelfactory.h"

#include <iostream>

using namespace std;

FixedPoint World::heightDifference2Velocity(const FixedPoint& h_diff)
{
	// no restrictions for moving downhill
	if(h_diff < FixedPoint::ZERO)
		return 1;
	if(h_diff >= FixedPoint(2))
		return FixedPoint::ZERO;
	return (FixedPoint(2) - h_diff)/FixedPoint(2);
}

unsigned long World::checksum() const
{
	unsigned long hash = 5381;
	
	for (auto it = units.begin(); it != units.end(); ++it)
	{
		int id = it->first;
		Location pos = it->second.getPosition();
		hash = ((hash << 5) + hash) + id;
		hash = ((hash << 5) + hash) + pos.x.getInteger();
		hash = ((hash << 5) + hash) + pos.y.getInteger();
		hash = ((hash << 5) + hash) + pos.z.getInteger();
	}
	
	return hash;
}

// TODO: this should be a bit smarter maybe? oh well.
void getColor(const string& a, int& r, int& g, int& b)
{
	if(a == "WHITE")
	{
		r = 255;
		g = 255;
		b = 255;
		return;
	}
	else if(a == "ORANGE")
	{
		r = 255;
		g = 165;
		b = 0;
		return;
	}
	else if(a == "DARK_RED")
	{
		r = 100;
		g = 0;
		b = 0;
		return;
	}
	else
	{
		r = 0;
		g = 255;
		b = 255;
		return;
	}
}


void VisualWorld::genParticleEmitter(const Location& pos, const Location& vel, int life, int max_rand, int scale, int r, int g, int b, int scatteringCone, int particlesPerFrame, int particleLife)
{
	ParticleSource pe;
	pe.intVals["PPF"] = particlesPerFrame;
	pe.intVals["CUR_LIFE"] = life;
	pe.intVals["MAX_LIFE"] = life;
	pe.intVals["PLIFE"]    = particleLife;
	
	pe.intVals["SRED"]     = r;
	pe.intVals["ERED"]     = r / 2;
	
	pe.intVals["SGREEN"]   = g;
	pe.intVals["EGREEN"]   = g / 2;
	
	pe.intVals["SBLUE"]    = b;
	pe.intVals["EBLUE"]    = b / 2;
	
	pe.getIntProperty("MAX_RAND") = max_rand;
	pe.getIntProperty("SCALE") = scale;
	
	pe.getIntProperty("PSP_1000") = scatteringCone;
	
	/*
	pe.getIntProperty("RAND_X_1000") = 500;
	pe.getIntProperty("RAND_Y_1000") = 0;
	*/
	
	pe.position = pos;
	pe.velocity = vel;
	
	psources.push_back(pe);
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
	
	auto killer_it = units.find(causeOfDeath);
	if(killer_it != units.end())
	{
		killer = killer_it->second.name;
		actor_id = killer_it->second.id;
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
	event.t_position = unit.getPosition();
	event.t_position.y += FixedPoint(2);
	
	event.t_velocity = unit.velocity;
	event.t_velocity.y += FixedPoint(200,1000);
	
	if(actor_id != -1)
	{
		event.a_position = units[actor_id].position;
		event.a_position.y += FixedPoint(2);
		event.a_velocity = units[actor_id].velocity;
	}
	
	if(actor_id != target_id)
	{
		event.actor_id  = actor_id;
		
		int i = currentWorldFrame % killWords.size();
		msg << killer << killWords[i] << unit.name << afterWords[i] << " ^g(" << unit("DAMAGED_BY") << ")";
		visualworld.worldMessages.push_back(msg.str());
	}
	else
	{
		event.actor_id  = -1; // For a suicide, no points are to be awarded.
		
		msg << killer << " has committed suicide by ^g" << unit("DAMAGED_BY");
		visualworld.worldMessages.push_back(msg.str());
	}

	visualworld.addLight(nextUnitID(), event.t_position);
	
	if(unit.human())
	{
		event.type = WorldEvent::DEATH_PLAYER;
		
		// reset player hitpoints
		unit.hitpoints = 1000;
		
		// respawn player to random location
		unit.setPosition(lvl.getRandomLocation(currentWorldFrame));
		
		// stop any movement, let the player drop down to the field of battle.
		unit.zeroMovement();
	}
	else
	{
		event.type = WorldEvent::DEATH_ENEMY;
		deadUnits.push_back(unit.id);
	}
	
	// store the event information for later use.
	visualworld.add_event(event);
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
		units[unitID]("DAMAGED_BY") = "devour";
		
		// save this information for later use.
		WorldEvent event;
		event.type = WorldEvent::DAMAGE_DEVOUR;
		event.t_position = units[unitID].position;
		event.t_position.y += FixedPoint(2);
		event.t_velocity.y = FixedPoint(900,1000);
		visualworld.add_event(event);
	}
	
	// turn towards the human unit until facing him. then RUSH FORWARD!
	Location direction = unit.position - units[unitID].position;
	
	if(direction.length() == 0)
	{
		// wtf, same position as my target? :G
		
		// so probably im devouring him alot ahahaha :DD
		
		return;
	}
	
	direction.normalize();
	
	Location myDirection;
	
	int hypo_angle = unit.angle;
	int hypo_upangle = unit.upangle;
	bool improved = true;
	FixedPoint best_error = FixedPoint(100000);
	
	while(improved)
	{
		improved = false;
		hypo_angle += 2;
		myDirection.z = apomath.getSin(hypo_angle);
		myDirection.x = apomath.getCos(hypo_angle);
		
		FixedPoint error = (myDirection.x * 100 - direction.x * 100).squared() + (myDirection.z * 100 - direction.z * 100).squared();
		if(error < best_error)
		{
			best_error = error;
			improved = true;
		}
		else
		{
			hypo_angle -= 4;
			myDirection.z = apomath.getSin(hypo_angle);
			myDirection.x = apomath.getCos(hypo_angle);
			
			error = (myDirection.x * 100 - direction.x * 100).squared() + (myDirection.z * 100 - direction.z * 100).squared();
			if(error < best_error)
			{
				best_error = error;
				improved = true;
			}
			else
				hypo_angle += 2;
		}
	}
	
	best_error = FixedPoint(100000);
	improved = true;
	while(improved)
	{
		improved = false;
		hypo_upangle += 2;
		myDirection.y = apomath.getSin(hypo_upangle);
		FixedPoint error = (myDirection.y * 100 - direction.y * 100).squared();
		if(error < best_error)
		{
			best_error = error;
			improved = true;
		}
		else
		{
			hypo_upangle -= 4;
			myDirection.y = apomath.getSin(hypo_upangle);
			error = (myDirection.y * 100 - direction.y * 100).squared();
			if(error < best_error)
			{
				best_error = error;
				improved = true;
			}
			else
				hypo_upangle += 2;
		}
	}
	
	int keyState = 0;
	int mousex = hypo_angle - unit.angle;
	int mousey = 0;
	unit.upangle = hypo_upangle;
//	int mousey = -hypo_upangle + unit.upangle;
	int mousebutton = 0;
	
	keyState |= Unit::MOVE_FRONT;
// 	unit.upangle = apomath.DEGREES_90 - apomath.DEGREES_90 / 50;
	
	if( ((currentWorldFrame + unit.birthTime) % 140) < 20)
	{
		keyState |= Unit::JUMP;
	}
	
	if( ((currentWorldFrame + unit.birthTime) % 140) > 50 )
	{
		// mousex += ( ((unit.birthTime + currentWorldFrame) * 23) % 200) - 100;
		mousebutton = 1;
	}
	
	unit.updateInput(keyState, mousex, mousey, mousebutton);
}


World::World()
{
	init();
}

void VisualWorld::init()
{
	cerr << "VisualWorld init" << endl;
	
	particles.reserve(40000);
}

void World::init()
{
	cerr << "World init" << endl;
	
	_unitID_next_unit = 10000;
	_playerID_next_player = 0;

	lvl.generate(50);

	visualworld.init();
	
	// find the highest point in lvl and add a strong light there.
	LightObject tmp_light;
	tmp_light.unitBind = -1;
	tmp_light.lifeType = LightSource::IMMORTAL;
	tmp_light.behaviour = LightSource::RISE_AND_DIE;
	tmp_light.setDiffuse(8.0, 2.0, 2.0);
	tmp_light.setLife(150);
	tmp_light.activateLight();
	tmp_light.position = Location(FixedPoint(500), FixedPoint(80), FixedPoint(500));
	visualworld.lights[nextUnitID()] = tmp_light;

	// Make sure there is atleast MAX_NUM_ACTIVE_LIGHTS.
	tmp_light.position = Location(FixedPoint(100), FixedPoint(80), FixedPoint(500));
	visualworld.lights[nextUnitID()] = tmp_light;
	tmp_light.position = Location(FixedPoint(500), FixedPoint(80), FixedPoint(100));
	visualworld.lights[nextUnitID()] = tmp_light;
	tmp_light.position = Location(FixedPoint(100), FixedPoint(80), FixedPoint(100));
	visualworld.lights[nextUnitID()] = tmp_light;
	
	show_errors = 0;
}

void World::terminate()
{
	_unitID_next_unit = 10000;
	_playerID_next_player = 0;
	
	units.clear();
	projectiles.clear();

	visualworld.terminate();
}

void VisualWorld::terminate()
{
	for(auto it = models.begin(); it != models.end(); ++it)
	{
		ModelFactory::destroy(it->second);
	}
	models.clear();
}



void World::tickUnit(Unit& unit, Model* model)
{
	if(unit.controllerTypeID == Unit::AI_RABID_ALIEN)
	{
		generateInput_RabidAlien(unit);
	}
	else if(unit.controllerTypeID == Unit::INANIMATE_OBJECT)
	{
		// hmm?
	}
	
	unit.soundInfo = "";
	
	// ALERT!! WHY THE FUCK IS MODEL ZERO HERE EVER??
	// update the information according to which the unit model will be updated from now on
	if(model != 0)
	{
		model->rotate_y(unit.getAngle(apomath));
		model->updatePosition(unit.position.x.getFloat(), unit.position.y.getFloat(), unit.position.z.getFloat());
	}
	
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
			unit("DAMAGED_BY") = "falling";
			
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
	
	if(unit.getKeyAction(Unit::WEAPON4))
	{
		unit.switchWeapon(4);
	}
	
	if(unit.getKeyAction(Unit::WEAPON5))
	{
		unit.switchWeapon(5);
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
	
	assert(ticks > 0);
	
	int num_particles = projectile["PARTICLES_PER_FRAME"];
	
	static ParticleSource ps;
	// static int sered, segreen, seblue;
	// static int eered, eegreen, eeblue;
	static int esred, esgreen, esblue;
	static int ssred, ssgreen, ssblue;
	
	if(num_particles > 0)
	{
		ps.getIntProperty("MAX_LIFE") = 10;
		ps.getIntProperty("CUR_LIFE") = 10;
		ps.getIntProperty("PSP_1000") = projectile["PARTICLE_RAND_1000"];
		ps.getIntProperty("PPF") = num_particles;
		ps.getIntProperty("PLIFE") = projectile["PARTICLE_LIFE"];
		
		// this should be in making a new ParticleSource
		getColor( projectile("START_COLOR_START"), ssred, ssgreen, ssblue );
		// getColor( projectile("START_COLOR_END")  , sered, segreen, seblue );
		
		getColor( projectile("END_COLOR_START")  , esred, esgreen, esblue );
		// getColor( projectile("END_COLOR_END")    , eered, eegreen, eeblue );
	}
	
	
	bool shooterIsMonster = false;
	auto owner_unit_iter = units.find(projectile["OWNER"]);
	if(owner_unit_iter != units.end())
	{
		shooterIsMonster = !owner_unit_iter->second.human();
	}
	
	for(int i=0; i<ticks; i++)
	{
		model->updatePosition(projectile.position.x.getFloat(), projectile.position.y.getFloat(), projectile.position.z.getFloat());
		int lifetime = projectile["LIFETIME"];
		
		if(lifetime <= 0 || projectile.destroyAfterFrame)
		{
			projectile.destroyAfterFrame = true;
			break;
		}

		if(num_particles > 0)
		{
			float percentage_life = float(lifetime) / projectile["MAX_LIFETIME"];
			
			// this should usually be in ParticleSource.tick() probably
			ps.getIntProperty("SRED")   = esred   + (ssred - esred)     * percentage_life;
			ps.getIntProperty("SGREEN") = esgreen + (ssgreen - esgreen) * percentage_life;
			ps.getIntProperty("SBLUE")  = esblue  + (ssblue - esblue)   * percentage_life;
			
			// these values are never used
			/*
			ps.getIntProperty("ERED")   = eered   + (sered - eered)     * percentage_life;
			ps.getIntProperty("EGREEN") = eegreen + (segreen - eegreen) * percentage_life;
			ps.getIntProperty("EBLUE")  = eeblue  + (seblue - eeblue)   * percentage_life;
			*/
			
			ps.velocity = projectile.velocity * FixedPoint(projectile["PARTICLE_VELOCITY"], 1000);
			ps.position = projectile.position;
			
			ps.tick(visualworld.particles);
		}
		
		projectile.tick();
		
		if(projectile.collidesTerrain(lvl))
		{
			// if collides with terrain, projectile will die at the end of this turn.
			// But should still check whether we hit something on the way to the point of collision!
			projectile.destroyAfterFrame = true;
			
			// intentional continue of execution
		}
		
		auto& potColl = o->nearObjects(projectile.position);
		for(auto it = potColl.begin(); it != potColl.end(); ++it)
		{
			if ((*it)->type != OctreeObject::UNIT)
				continue;
			
			Unit* u = static_cast<Unit*>(*it);
			
			// if the target unit is already dead, just continue. ALERT: This might cause desync, if potColl is unordered
			if(u->hitpoints <= 0)
				continue;
			
			// if monster is shooting a monster, just destroy the bullet. dont let them kill each other :(
			if(!u->human() && shooterIsMonster)
			{
				//projectile.destroyAfterFrame = projectile["DEATH_IF_HITS_UNIT"];
				continue;
			}
			
			if(projectile["DISTANCE_TEST"])
			{
				// distance test
				Location distance_vector = projectile.position - u->position; // TODO: Unit position is at ground level. sux.
				if(distance_vector.length() < FixedPoint(projectile["DISTANCE_MAX"], 1000))
				{
					u->hitpoints -= projectile["DISTANCE_DAMAGE"];
					u->last_damage_dealt_by = projectile["OWNER"];
					(*u)("DAMAGED_BY") = projectile("NAME");
				}
			}
			
			// boolean test, hits or doesnt hit
			if(projectile["COLLISION_TEST"] && projectile.collides(*u))
			{
				// save this information for later use.
				WorldEvent event;
				event.type = WorldEvent::DAMAGE_BULLET;
				event.t_position = u->position;
				event.t_position.y += FixedPoint(2);
				event.t_velocity = u->velocity;
				
				event.a_position = projectile.position;
				event.a_velocity = projectile.velocity * projectile["TPF"];
				
				visualworld.add_event(event);
				
				u->hitpoints -= projectile["DAMAGE"]; // does damage according to weapon definition :)
				u->velocity += projectile.velocity * FixedPoint(projectile["MASS"], 1000);
				u->last_damage_dealt_by = projectile["OWNER"];
				(*u)("DAMAGED_BY") = projectile("NAME");
				
				projectile.destroyAfterFrame = projectile["DEATH_IF_HITS_UNIT"];
			}
		}
	}
	
	// as a post frame update, update values of the projectile
	if(projectile["AIR_RESISTANCE"])
	{
		projectile.velocity *= FixedPoint(projectile["AIR_RESISTANCE"], 1000);
	}
	projectile.velocity.y += FixedPoint(projectile["GRAVITY"], 1000);
}

void VisualWorld::updateModel(Model* model, const Unit& unit, int currentWorldFrame)
{
	/*
	// deduce which animation to display
	if(unit.position.h - FixedPoint(100,1000) > lvl.getHeight(unit.position.x, unit.position.y))
	{
		model.setAction("jump");
	}
	else */
	
	assert(model != 0);
	
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

void VisualWorld::tickParticles()
{
	for(size_t i=0; i<psources.size(); ++i)
	{
		psources[i].tick(particles);
		if(!psources[i].alive())
		{
			psources[i] = psources.back();
			psources.pop_back();
			--i;
		}
	}
	
	for(size_t i=0; i<particles.size(); ++i)
	{
		if(!particles[i].alive())
		{
			particles[i] = particles.back();
			particles.pop_back();
			--i;
		}
		else
			particles[i].tick();
	}
}

void VisualWorld::tickLights(const std::map<int, Unit>& units)
{
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
		tickUnit(iter->second, visualworld.models[iter->first]);
		o->insertObject(&(iter->second));
	}
	
	
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Projectile ticks + collisions   \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/
	
	for(map<int, Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		tickProjectile(iter->second, visualworld.models[iter->first]);
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
	
	
	// Graphical things.
	visualworld.tickParticles();
	visualworld.tickLights(units);
	
	
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
				visualworld.worldMessages.push_back(msg.str());
			}
		}
	}
}

void VisualWorld::viewTick(const std::map<int, Unit>& units, const std::map<int, Projectile>& projectiles, int currentWorldFrame)
{
	for(size_t i=0; i<particles.size(); ++i)
	{
		particles[i].viewTick();
	}
	
	for(map<int, Unit>::const_iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		updateModel(models[iter->first], iter->second, currentWorldFrame);
	}
	
	for(map<int, Projectile>::const_iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		Model* model = models[iter->first];
		model->setAction("idle");
		model->tick(currentWorldFrame);
	}
}

void VisualWorld::addLight(int id, Location& location)
{
//	cerr << "Adding light at " << location << endl;
	LightObject& light = lights[id];
	light.setDiffuse(1.f, 1.f, 1.f);
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
	
	visualworld.models[id] = ModelFactory::create(World::PLAYER_MODEL);
	
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

void World::addProjectile(Location& location, int id, size_t model_prototype)
{
	Vec3 position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();
	
	Model* model = ModelFactory::create(model_prototype);
	auto iter = visualworld.models.insert(make_pair(id, model)).first;
	iter->second->realUnitPos = position;
	iter->second->currentModelPos = position;
	
	projectiles[id].position = location;
	projectiles[id].prototype_model = model_prototype;
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

int World::currentUnitID() const
{
	return _unitID_next_unit;
}

void VisualWorld::removeUnit(int id)
{
	auto it = models.find(id);
	if(it != models.end())
	{
		ModelFactory::destroy(it->second);
		models.erase(it);
	}
}

void World::removeUnit(int id)
{
	// Note that same id might be removed twice on the same frame!

	units.erase(id);
	projectiles.erase(id);

	visualworld.removeUnit(id);
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

std::vector<Location> World::humanPositions() const
{
	std::vector<Location> positions;
	for(map<int, Unit>::const_iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		if (iter->second.human())
			positions.push_back(iter->second.position);
	}
	return positions;
}

void VisualWorld::add_message(const std::string& message)
{
	worldMessages.push_back(message);
}

void World::add_message(const std::string& message)
{
	visualworld.add_message(message);
}

void VisualWorld::add_event(const WorldEvent& event)
{
	events.push_back(event);
}

void World::add_event(const WorldEvent& event)
{
	visualworld.add_event(event);
}

void World::setNextUnitID(int id)
{
	_unitID_next_unit = id;
}

