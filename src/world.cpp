
#include "world.h"
#include "modelfactory.h"
#include "visualworld.h"

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

World::CheckSumType World::checksum() const
{
	CheckSumType hash = 5381;
	
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


// TODO: Force expansion should not be instant.
void World::instantForceOutwards(const FixedPoint& power, const Location& pos, const string& name, int owner)
{
	// find out who is inflicted an how.
	for(auto iter = units.begin(); iter != units.end(); iter++)
	{
		const Location& pos2 = iter->second.getPosition();
		Location velocity_vector = (pos2 - pos);
		
		FixedPoint distance = velocity_vector.length();
		velocity_vector.normalize();
		
		if(distance == FixedPoint(0))
			distance = FixedPoint(1, 10);
		
		velocity_vector *= power;
		velocity_vector /= distance;
		
		FixedPoint force = velocity_vector.length();
		FixedPoint damage = force * force * FixedPoint(100);
		
		int int_damage = damage.getInteger();
		if(int_damage > 0)
		{
			Unit& u = iter->second;
			u.takeDamage(int_damage);
			u.last_damage_dealt_by = owner;
			u("DAMAGED_BY") = name;
		}
		
		iter->second.velocity += velocity_vector;
	}
	
	// create some effect or something
	Location zero; zero.y = FixedPoint(1, 2);
	
	
	visualworld->explosion(nextUnitID(), pos, zero);
	
	// original values for the explosion effect.
	// visualworld->genParticleEmitter(pos, zero, 50, 5000, 5500, "WHITE", "ORANGE", "ORANGE", "DARK_RED", 1500, 10, 80);
	
	// 	genParticleEmitter(const Location& pos, const Location& vel, int life, int max_rand, int scale, int r, int g, int b, int scatteringCone = 500, int particlesPerFrame = 5, int particleLife = 50);
}

void World::atDeath(MovableObject& object, HasProperties& properties)
{
	if(properties.strVals.find("AT_DEATH") == properties.strVals.end())
		return;
	
	if(properties.strVals["AT_DEATH"] == "")
		properties.strVals["AT_DEATH"] = "NOTHING";
	
	// this way can only store one event to be executed at death :( should maybe reconcider that.
	if(properties("AT_DEATH") == "EXPLODE")
	{
		FixedPoint explode_power(properties["EXPLODE_POWER"]);
		const Location& pos = object.position;
		
		instantForceOutwards(explode_power, pos, properties("NAME"), properties["OWNER"]);
	}
	else if(properties("AT_DEATH") == "PUFF")
	{
		const Location& pos = object.position - object.velocity * 2;
		Location velocity(0, FixedPoint(1, 5), 0);
		visualworld->genParticleEmitter(pos, velocity, 4, 300, 1000, "GREY", "BLACK", "GREY", "BLACK", 200, 1, 50);
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
		event.a_position = units.find(actor_id)->second.position;
		event.a_position.y += FixedPoint(2);
		event.a_velocity = units.find(actor_id)->second.velocity;
	}
	
	if(actor_id != target_id)
	{
		event.actor_id  = actor_id;
		
		int i = currentWorldFrame % killWords.size();
		msg << killer << killWords[i] << unit.name << afterWords[i] << " ^g(" << unit("DAMAGED_BY") << ")";
		visualworld->add_message(msg.str());
	}
	else
	{
		event.actor_id  = -1; // For a suicide, no points are to be awarded.
		
		msg << killer << " has committed suicide by ^g" << unit("DAMAGED_BY");
		visualworld->add_message(msg.str());
	}

	// visualworld->addLight(nextUnitID(), event.t_position);
	
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
	visualworld->add_event(event);
}




void getTurnValues(Unit& me, Unit& target, int& best_angle, int& best_upangle)
{
	// turn towards the human unit until facing him. then RUSH FORWARD!
	Location direction = target.position - me.position;
	
	if(direction.length() == 0)
	{
		// wtf, same position as my target? :G
		return;
	}
	
	direction.normalize();
	
	Location myDirection;
	
	int hypo_angle = me.angle;
	int hypo_upangle = me.upangle;
	bool improved = true;
	FixedPoint best_error = FixedPoint(1000000);
	
	ApoMath apomath;
	
	while(improved)
	{
		improved = false;
		hypo_angle += 5;
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
			hypo_angle -= 10;
			myDirection.z = apomath.getSin(hypo_angle);
			myDirection.x = apomath.getCos(hypo_angle);
			
			error = (myDirection.x * 100 - direction.x * 100).squared() + (myDirection.z * 100 - direction.z * 100).squared();
			if(error < best_error)
			{
				best_error = error;
				improved = true;
			}
			else
				hypo_angle += 5;
		}
	}
	
	best_error = FixedPoint(1000000);
	improved = true;
	while(improved)
	{
		improved = false;
		hypo_upangle += 5;
		myDirection.y = apomath.getSin(hypo_upangle);
		FixedPoint error = (direction.y * 100 - myDirection.y * 100).squared();
		if(error < best_error)
		{
			best_error = error;
			improved = true;
		}
		else
		{
			hypo_upangle -= 10;
			myDirection.y = apomath.getSin(hypo_upangle);
			error = (direction.y * 100 - myDirection.y * 100).squared();
			if(error < best_error)
			{
				best_error = error;
				improved = true;
			}
			else
				hypo_upangle += 5;
		}
	}
	
	best_angle = hypo_angle;
	best_upangle = hypo_upangle;
}


void World::generateInput_RabidAlien(Unit& unit)
{
	FixedPoint bestSquaredDistance = FixedPoint(200 * 200);
	int unitID = -1;
	int myLeaderID = -1;
	int my_team = unit["TEAM"];
	
	if(unit["L"] == -1)
		unit["L"] = unit.id;
	
	if(unit.intVals[unit.weapons[unit.weapon].strVals["AMMUNITION_TYPE"]] == 0)
	{
		if(unit.weapon > 1)
			unit.weapon--;
		else
		{
			unit.strVals["DAMAGED_BY"] = "voluntary suicide";
			unit.hitpoints = -1;
			unit.last_damage_dealt_by = unit.id;
		}
	}
	
	// find the nearest human controlled unit + some "leader" for my team
	if( (unit.birthTime + currentWorldFrame) % 70 == 0)
	{
		for(map<int, Unit>::iterator it = units.begin(), et = units.end(); it != et; ++it)
		{
			if(it->second["TEAM"] != my_team)
			{
				FixedPoint tmp_dist = (it->second.position - unit.position).lengthSquared();
				if( tmp_dist < bestSquaredDistance )
				{
					bestSquaredDistance = tmp_dist;
					unitID = it->first;
				}
			}
			else
			{
				if(myLeaderID == -1)
				{
					myLeaderID = it->first;
				}
			}
		}
		
		unit["T"] = unitID;
		unit["L"] = myLeaderID;
		
		if(unitID == -1)
		{
			// try look
		}
		
	}
	else
	{
		unitID = unit["T"];
		myLeaderID = unit["L"];
		
		if(units.find(unitID) == units.end())
		{
			unit["T"] = -1;
			unitID = -1;
		}
		
		// there is always some leader
		if(units.find(myLeaderID) == units.end())
		{
			unit["L"] = -1;
			myLeaderID = unit.id;
		}
	}
	
	// if no opponent is near, gather to my leader!
	if(unitID == -1)
	{
		
		if(myLeaderID == unit.id)
		{
			// nothing to do :G
			unit.updateInput(0, 0, 0, 0);
		}
		else
		{
			if( (units.find(myLeaderID)->second.position - unit.position).lengthSquared() < FixedPoint(1000) )
			{
				unit.updateInput(0, 0, 0, 0);
				return;
			}
			
			int mousex, mousey;
			getTurnValues(unit, units[myLeaderID], mousex, mousey);
			
			unit.angle = mousex;
			unit.upangle = mousey;
			unit.updateInput(Unit::MOVE_FRONT, 0, 0, 0);
		}
		
		return;
	}
	
	
	bestSquaredDistance = (units.find(unitID)->second.position - unit.position).lengthSquared();
	
	int best_angle, best_upangle;
	getTurnValues(unit, units[unitID], best_angle, best_upangle);
	
	int keyState = 0;
	int mousex = 0;
	int mousey = 0;
	int mousebutton = 0;
	unit.angle = best_angle;
	unit.upangle = best_upangle;
	
	if(bestSquaredDistance < FixedPoint(1000))
		keyState |= Unit::MOVE_RIGHT;
	else
		keyState |= Unit::MOVE_FRONT;
	
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


World::World(VisualWorld* vw)
{
	assert(vw);
	visualworld = vw;

	init();
}

void World::buildTerrain(int n)
{
	lvl.generate(n);
	intVals["GENERATOR"] = n;
}

string World::generatorMessage()
{
	stringstream ss;
	ss << "-2 WORLD_GEN_PARAM " << intVals["GENERATOR"] << " " << strVals["AREA_NAME"] << "#";
	return ss.str();
}

void World::init()
{
	cerr << "World::init()" << endl;
	
	load("world.conf");
	
	unitIDgenerator.setNextID(10000);

	// lvl.generate(50);
	
	visualworld->init();
	show_errors = 0;
}

void World::terminate()
{
	cerr << "World::terminate()" << endl;
	
	cerr << "  clearing units" << endl;
	units.clear();
	
	cerr << "  clearing projectiles" << endl;
	projectiles.clear();
	
	cerr << "  clearing items" << endl;
	items.clear();
	
	cerr << "  clearing deadUnits" << endl;
	deadUnits.clear(); // redundant?
	
	// currentWorldFrame = -1;
	// unitIDgenerator.setNextID(10000);
	
	visualworld->terminate();
}

// TODO: Make this more senseful so it works
// TODO: Put this somewhere where it makes sense
FixedPoint getSlideAcceleration(const FixedPoint& unit_y, const FixedPoint& reference_y)
{
	FixedPoint threshold = FixedPoint(1, 16);
	FixedPoint tmp_val = (unit_y - reference_y + threshold);
	if(tmp_val > FixedPoint(0))
		return FixedPoint (0);
	return tmp_val;
}

void World::tickItem(WorldItem& item, Model* model)
{
	// wut
	// model->rotate_y(item.getAngle(apomath));
	if(visualworld->isActive())
	{
		assert(model && "item model does not exist");
		model->updatePosition(item.position.x.getFloat(), item.position.y.getFloat(), item.position.z.getFloat());
	}
	
	// some physics & game world information
	if( (item.velocity.y + item.position.y - FixedPoint(1, 20)) <= lvl.getHeight(item.position.x, item.position.z) )
	{
		// colliding with terrain right now
		FixedPoint friction = FixedPoint(88, 100);
		
		item.position.y = lvl.getHeight(item.position.x, item.position.z);
		
		item.velocity.y += FixedPoint(50, 1000); // no clue if this makes any sense
		item.velocity.x *= friction;
		item.velocity.z *= friction;
	}
	
	auto& potColl = octree->nearObjects(item.position);
	
	for(auto iter = potColl.begin(); iter != potColl.end(); ++iter)
	{
		// handle only unit collisions
		if ((*iter)->type != OctreeObject::UNIT)
			continue;
		
		Unit* u = static_cast<Unit*>(*iter);
		
		// now did they collide or not?
		if( (item.position - u->position).lengthSquared() < FixedPoint(16) )
		{
			item.collides(*u);
		}
	}
	
	// gravity
	item.velocity.y -= FixedPoint(35,1000);
	
	// apply motion
	item.position += item.velocity;
}

void World::tickUnit(Unit& unit, Model* model)
{
	if(unit.intVals["D"] > 0)
	{
		unit.intVals["D"] *= 0.95;
	}
	
	if(unit.controllerTypeID == Unit::AI_RABID_ALIEN)
	{
		generateInput_RabidAlien(unit);
	}
	else if(unit.controllerTypeID == Unit::INANIMATE_OBJECT)
	{
		// hmm?
	}
	
	if(unit.getKeyAction(Unit::RELOAD))
	{
		unit.weapons[unit.weapon].prepareReload(unit);
	}
	
	unit.soundInfo = "";
	
	// for server it's ok that there are no models sometimes :G
	if(visualworld->isActive())
	{
		assert(model && "this should never happen");
		
		model->rotate_y(unit.getAngle(apomath));
		model->updatePosition(unit.position.x.getFloat(), unit.position.y.getFloat(), unit.position.z.getFloat());
	}
	
	// TODO: heavy landing is a special case of any kind of collisions. Other collisions are still not handled.
	
	// some physics & game world information
	if( (unit.velocity.y + unit.position.y - FixedPoint(1, 20)) <= lvl.getHeight(unit.position.x, unit.position.z) )
		unit.mobility |= Unit::MOBILITY_STANDING_ON_GROUND;
	
	if( unit.mobility & Unit::MOBILITY_STANDING_ON_GROUND || unit.mobility & Unit::MOBILITY_STANDING_ON_OBJECT )
	{
		// TODO: Heavy landings should have a gameplay effect!
		if(unit.velocity.y < FixedPoint(-7, 10))
			unit.soundInfo = "jump_land";
		if(unit.velocity.y < FixedPoint(-12, 10))
		{
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
				unit.takeDamage(damage_int * damage_int / 500);
			}
			
			// allow deny only after surviving the first hit with ground.
			if(unit.hitpoints > 0)
			{
				// no deny by jumping down a cliff!
				unit.last_damage_dealt_by = unit.id;
			}
		}
		
		FixedPoint friction = FixedPoint(88, 100);
		
		if(unit.mobility & Unit::MOBILITY_STANDING_ON_GROUND)
		{
			unit.position.y = lvl.getHeight(unit.position.x, unit.position.z);
			unit.velocity.y = FixedPoint::ZERO;
		}
		
		unit.velocity.x *= friction;
		unit.velocity.z *= friction;
	}
	
	// gravity and air resistance.
	unit.velocity.y -= FixedPoint(35,1000);
	FixedPoint friction = FixedPoint(995, 1000);
	unit.velocity.x *= friction;
	unit.velocity.z *= friction;
	
	
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
	
	
	FixedPoint mobility = unit.getMobility();
	if(unit.getKeyAction(Unit::MOVE_FRONT))
	{
		FixedPoint scale = FixedPoint(10, 100) * mobility;
		unit.velocity.x += apomath.getCos(unit.angle) * scale;
		unit.velocity.z += apomath.getSin(unit.angle) * scale;
	}
	
	if(unit.getKeyAction(Unit::MOVE_BACK))
	{
		FixedPoint scale = FixedPoint(6, 100) * mobility;
		unit.velocity.x -= apomath.getCos(unit.angle) * scale;
		unit.velocity.z -= apomath.getSin(unit.angle) * scale;
	}

	if(unit.getKeyAction(Unit::MOVE_LEFT))
	{
		FixedPoint scale = FixedPoint(8, 100) * mobility;
		int dummy_angle = unit.angle - apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
	}
	if(unit.getKeyAction(Unit::MOVE_RIGHT))
	{
		FixedPoint scale = FixedPoint(8, 100) * mobility;
		int dummy_angle = unit.angle + apomath.DEGREES_90;
		
		unit.velocity.x -= apomath.getCos(dummy_angle) * scale;
		unit.velocity.z -= apomath.getSin(dummy_angle) * scale;
	}
	
	if(unit.getKeyAction(Unit::MOVE_RIGHT | Unit::MOVE_LEFT | Unit::MOVE_FRONT | Unit::MOVE_BACK) && (unit.soundInfo == ""))
		unit.soundInfo = "walk";

	if(unit.leap_cooldown == 0)
	{
		FixedPoint scale(950,1000);
		if(unit.getKeyAction(Unit::LEAP_LEFT) && (mobility > 0))
		{
			int dummy_angle = unit.angle - apomath.DEGREES_90;
			
			unit.velocity.x -= apomath.getCos(dummy_angle) * scale * mobility;
			unit.velocity.z -= apomath.getSin(dummy_angle) * scale * mobility;
			unit.velocity.y += FixedPoint(45, 100);
			unit.leap_cooldown = 40;
			
			unit.soundInfo = "jump";
			// unit.soundInfo = "leap";
		}
		if(unit.getKeyAction(Unit::LEAP_RIGHT) && (mobility > 0))
		{
			int dummy_angle = unit.angle + apomath.DEGREES_90;
			
			unit.velocity.x -= apomath.getCos(dummy_angle) * scale * mobility;
			unit.velocity.z -= apomath.getSin(dummy_angle) * scale * mobility;
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
	
	unit.weapons[unit.weapon].tick(unit);
	
	
	if(unit.getMouseAction(Unit::MOUSE_LEFT))
	{
		unit.weapons[unit.weapon].onActivate(*this, unit);
	}
	
	if(unit.getMouseAction(Unit::MOUSE_RIGHT))
	{
		unit.weapons[unit.weapon].onSecondaryActivate(*this, unit);
	}
	
	
	FixedPoint reference_x = unit.position.x + unit.velocity.x;
	FixedPoint reference_z = unit.position.z + unit.velocity.z;
	FixedPoint reference_y = lvl.getHeight(reference_x, reference_z);
	FixedPoint y_diff = reference_y - unit.position.y;
	
	FixedPoint yy_val = heightDifference2Velocity(y_diff);
	
	if(mobility == 0)
	{
		yy_val = FixedPoint(1);
	}

	if(unit.getKeyAction(Unit::JUMP) && mobility > 0)
	{
		unit.soundInfo = "jump";
		unit.velocity.y = FixedPoint(900, 1000);
	}
	
	
	unit.velocity.z *= yy_val;
	unit.velocity.x *= yy_val;
	
	unit.position += unit.velocity;
	
	/*
	if(unit.mobility == Unit::MOBILITY_STANDING_ON_GROUND)
	{
		
		Location r1 = unit.position + Location(FixedPoint(+1, 20), 0, 0);
		Location r2 = unit.position + Location(FixedPoint(-1, 20), 0, 0);
		Location r3 = unit.position + Location(0, 0, FixedPoint(+1, 20));
		Location r4 = unit.position + Location(0, 0, FixedPoint(-1, 20));
		
		FixedPoint ry1 = lvl.getHeight(r1.x, r1.z);
		Location c1(+1, 0, 0); c1 *= getSlideAcceleration(unit.position.y, ry1);
		
		FixedPoint ry2 = lvl.getHeight(r2.x, r2.z);
		Location c2(-1, 0, 0); c2 *= getSlideAcceleration(unit.position.y, ry2);
		
		FixedPoint ry3 = lvl.getHeight(r3.x, r3.z);
		Location c3(0, 0, +1); c3 *= getSlideAcceleration(unit.position.y, ry3);
		
		FixedPoint ry4 = lvl.getHeight(r4.x, r4.z);
		Location c4(0, 0, -1); c4 *= getSlideAcceleration(unit.position.y, ry4);
		
		unit.velocity += c1;
		unit.velocity += c2;
		unit.velocity += c3;
		unit.velocity += c4;
	}
	*/
	
	if(unit.position.x < 0)
	{
		unit.position.x = 0;
		if(unit.velocity.x < 0)
			unit.velocity.x = 0;
	}
	if(unit.position.x > lvl.max_x())
	{
		unit.position.x = lvl.max_x();
		if(unit.velocity.x > 0)
			unit.velocity.x = 0;
	}
	if(unit.position.z < 0)
	{
		unit.position.z = 0;
		if(unit.velocity.z < 0)
			unit.velocity.z = 0;
	}
	if(unit.position.z > lvl.max_z())
	{
		unit.position.z = lvl.max_z();
		if(unit.velocity.z > 0)
			unit.velocity.z = 0;
	}
	
	unit.mobility = Unit::MOBILITY_CLEAR_VALUE;
	unit.position += unit.posCorrection;
	unit.posCorrection.x = 0;
	unit.posCorrection.y = 0;
	unit.posCorrection.z = 0;
}

void World::tickProjectile(Projectile& projectile, Model* model)
{
	int ticks = projectile["TPF"]; assert(ticks > 0);
	
	int num_particles      = projectile["PARTICLES_PER_FRAME"];
	int death_at_collision = projectile["DEATH_IF_HITS_UNIT"];
	int projectile_owner   = projectile["OWNER"];
	string& projectile_name = projectile("NAME");
	
	static ParticleSource ps;
	
	if(num_particles > 0)
	{
		ps.getIntProperty("MAX_LIFE") = 10;
		ps.getIntProperty("CUR_LIFE") = 10;
		ps.getIntProperty("PSP_1000") = projectile["PARTICLE_RAND_1000"];
		ps.getIntProperty("PPF") = num_particles;
		ps.getIntProperty("PLIFE") = projectile["PARTICLE_LIFE"];
		ps.getIntProperty("SCALE") = projectile["PARTICLE_SCALE"];
		
		ps.setColors(projectile("START_COLOR_START"), projectile("START_COLOR_END"), projectile("END_COLOR_START"), projectile("END_COLOR_END"));
	}
	
	int team = -1;
	auto owner_unit_iter = units.find(projectile_owner);
	if(owner_unit_iter != units.end())
	{
		team = owner_unit_iter->second["TEAM"];
	}
	
	for(int i=0; i<ticks; i++)
	{
		int lifetime = projectile["LIFETIME"];
		
		if(lifetime <= 0 || projectile.destroyAfterFrame)
		{
			projectile.destroyAfterFrame = true;
			break;
		}

		// generate particles
		if(num_particles > 0)
		{
			ps.velocity = projectile.velocity * FixedPoint(projectile["PARTICLE_VELOCITY"], 1000);
			ps.position = projectile.position;
			ps.tick(visualworld->particles);
		}
		
		projectile.tick();
		
		if(projectile.collidesTerrain(lvl))
		{
			// behaviour here should be defined by the projectile..
			
			projectile.destroyAfterFrame = true;
			// intentional continue of execution
		}
		
		auto& potColl = octree->nearObjects(projectile.position);
		for(auto it = potColl.begin(); it != potColl.end(); ++it)
		{
			if ((*it)->type != OctreeObject::UNIT)
				continue;
			
			Unit* u = static_cast<Unit*>(*it);
			
			// Is this actually a good thing?
			if(u->hitpoints < 1)
				continue; // don't hit dead units
			
			// if the target unit is already dead, just continue.
			if(u->hitpoints <= 0)
				continue;
			
			
			if(projectile["DISTANCE_TEST"])
			{
				if(!friendly_fire && ((*u)["TEAM"] == team))
				{
				}
				else
				{
					// distance test
					Location distance_vector = projectile.position + Location(0, 2, 0) - u->position; // TODO: Point to point distance is maybe not ideal.
					FixedPoint real_distance = distance_vector.length();
					FixedPoint max_distance = FixedPoint(projectile["DISTANCE_MAX"], 1000);
					
					if(real_distance < max_distance)
					{
						u->takeDamage( ((max_distance - real_distance) / max_distance * projectile["DISTANCE_DAMAGE"]).getInteger() );
						u->last_damage_dealt_by = projectile_owner;
						(*u)("DAMAGED_BY") = projectile_name;
					}
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
				
				visualworld->add_event(event);
				u->velocity += projectile.velocity * FixedPoint(projectile["MASS"], 1000);
				
				if(!friendly_fire && ((*u)["TEAM"] == team))
				{
				}
				else
				{
					u->takeDamage(projectile["DAMAGE"]); // does damage according to weapon definition :)
					u->last_damage_dealt_by = projectile_owner;
					(*u)("DAMAGED_BY") = projectile_name;
				}
				
				projectile.destroyAfterFrame |= death_at_collision;
			}
		}
	}
	
	if(visualworld->isActive())
	{
		assert(model && "projectile model does not exist");
		model->updatePosition(projectile.position.x.getFloat(), projectile.position.y.getFloat(), projectile.position.z.getFloat());
	}
	
	// as a post frame update, update values of the projectile
	if(projectile["AIR_RESISTANCE"])
	{
		projectile.velocity *= FixedPoint(projectile["AIR_RESISTANCE"], 1000);
	}
	projectile.velocity.y += FixedPoint(projectile["GRAVITY"], 1000);
}


void World::worldTick(int tickCount)
{
	// TODO: should have a method to update the state of a MovableObject !! (instead of a separate tick for every type..)
	
	friendly_fire = (intVals["FRIENDLY_FIRE"] == 1);
	
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Build octree + do collisions    \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/
	
	octree.reset(new Octree(Location(0, 0, 0), Location(lvl.max_x(), FixedPoint(400), lvl.max_z())));
	currentWorldFrame = tickCount;
	
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		octree->insertObject(&(iter->second));
	}
	
	octree->doCollisions();
	
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Tick units and projectiles      \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/
	
	for(auto iter = units.begin(); iter != units.end(); ++iter)
	{
		tickUnit(iter->second, visualworld->models[iter->first]);
	}
	
	for(map<int, Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		tickProjectile(iter->second, visualworld->models[iter->first]);
	}
	
	for(auto iter = items.begin(); iter != items.end(); ++iter)
	{
		tickItem(iter->second, visualworld->models[iter->first]);
	}
	
	
	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Find dead units                 \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/
	
	for(auto iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		Projectile& projectile = iter->second;
		if(projectile.destroyAfterFrame)
		{
			atDeath(projectile, projectile); // NICE!! :D
			deadUnits.push_back(iter->first);
		}
	}
	
	for(auto iter = units.begin(); iter != units.end(); ++iter)
	{
		Unit& unit = iter->second;
		if(unit.hitpoints < 1)
		{
			if(unit.intVals["GOD_MODE"])
			{
				unit.hitpoints = 1000;
				continue;
			}
			
			// every time a unit dies, spawn some replenishment stuff
			Location gear_pos = unit.position + Location(0, 1, 0);
			Location gear_vel = unit.velocity + Location(0, 1, 0);
			
			if(gear_vel.lengthSquared() > FixedPoint(1))
			{
				gear_vel.normalize();
			}
			
			addItem(gear_pos, gear_vel, unitIDgenerator.nextID());
			
			// then do the more important death processing
			doDeathFor(unit);
			atDeath(unit, unit);
		}
	}
	
	for(auto iter = items.begin(); iter != items.end(); ++iter)
	{
		WorldItem& item = iter->second;
		if(item.dead)
		{
			deadUnits.push_back(iter->first);
			
			/*
			atDeath(item, item); // hmm? collection is not the same as death though.
			*/
		}
	}
	
	for(size_t i = 0; i < deadUnits.size(); ++i)
	{
		removeUnit(deadUnits[i]);
	}
	deadUnits.clear();
	
	
	// Graphical things.
	visualworld->tickParticles();
	visualworld->tickLights(units);
	
	
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
				visualworld->add_message(msg.str());
			}
		}
	}
}




void World::addUnit(int id, bool playerCharacter, int team)
{
	if(units.find(id) != units.end())
		cerr << "FUCK FUCK FUCK FUCK FUCK" << endl;
	
	units[id] = Unit();
	units[id].init();
	units[id].position = lvl.getRandomLocation(currentWorldFrame);
	units[id].id = id;
	
	units[id].birthTime = currentWorldFrame;
	
	if(!playerCharacter)
	{
		visualworld->models[id] = ModelFactory::create(World::ZOMBIE_MODEL);
		
		units[id].setDefaultMonsterAttributes();
		
		units[id].name = "Alien monster";
		units[id].controllerTypeID = Unit::AI_RABID_ALIEN;
		units[id].hitpoints = 500;
		units[id]["TEAM"] = team;
		units[id]["T"] = -1;
		units[id].weapon = currentWorldFrame % (units[id].weapons.size() - 1);
	}
	else
	{
		visualworld->models[id] = ModelFactory::create(World::PLAYER_MODEL);
		
		units[id].setDefaultPlayerAttributes();
		
		units[id].name = "Unknown Player";
		units[id].controllerTypeID = Unit::HUMAN_INPUT;
		units[id].hitpoints = 1000;
		units[id]["TEAM"] = id;
	}
}

void World::addProjectile(Location& location, int id, size_t model_prototype)
{
	Vec3 position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();
	
	Model* model = ModelFactory::create(model_prototype);
	auto iter = visualworld->models.insert(make_pair(id, model)).first;
	iter->second->realUnitPos = position;
	iter->second->currentModelPos = position;
	
	projectiles[id].position = location;
	projectiles[id].prototype_model = model_prototype;
}

void World::addItem(Location& location, Location& velocity, int id)
{
	Vec3 position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();
	
	// ModelFactory::create(World::PLAYER_MODEL); // TODO: Models for items
	visualworld->models[id] = ModelFactory::create(World::BULLET_MODEL);
	visualworld->models[id]->realUnitPos = position;
	visualworld->models[id]->currentModelPos = position;
	
	visualworld->models[id]->setScale(25.0f); // big balls!
	
	items[id].position = location;
	items[id].velocity = velocity;
}

int World::nextUnitID()
{
	return unitIDgenerator.nextID();
}

int World::currentUnitID() const
{
	return unitIDgenerator.currentID();
}

void World::removeUnit(int id)
{
	// Note that same id might be removed twice on the same frame!
	units.erase(id);
	projectiles.erase(id);
	items.erase(id);
	visualworld->removeUnit(id);
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

void World::setNextUnitID(int id)
{
	unitIDgenerator.setNextID(id);
}

void World::add_message(const std::string& message) const
{
	visualworld->add_message(message);
}

void World::add_event(const WorldEvent& event) const
{
	visualworld->add_event(event);
}

