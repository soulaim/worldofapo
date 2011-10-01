
#include "world.h"
#include "graphics/modelfactory.h"
#include "graphics/visualworld.h"

#include <iostream>
#include <stdexcept>

using namespace std;

#define LAZY_UPDATE 0

FixedPoint World::heightDifference2Velocity(const FixedPoint& h_diff)
{
	// no restrictions for moving downhill
	if(h_diff < FixedPoint::ZERO)
		return 1;
	if(h_diff >= FixedPoint(2))
		return FixedPoint::ZERO;
	return (FixedPoint(2) - h_diff)/FixedPoint(2);
}

void World::checksum(vector<World::CheckSumType>& checksums) const
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
	checksums.push_back(hash);

	hash = 5381;
	for (auto it = units.begin(); it != units.end(); ++it)
	{
		hash = ((hash << 5) + hash) + it->second.hitpoints;
	}
	checksums.push_back(hash);

	hash = 5381;
	for(auto it = items.begin(); it != items.end(); it++)
	{
		int id = it->first;
		Location pos = it->second.getPosition();
		hash = ((hash << 5) + hash) + id;
		hash = ((hash << 5) + hash) + pos.x.getInteger();
		hash = ((hash << 5) + hash) + pos.y.getInteger();
		hash = ((hash << 5) + hash) + pos.z.getInteger();
	}
	checksums.push_back(hash);

	hash = 5381;
	for(auto it = projectiles.begin(); it != projectiles.end(); it++)
	{
		int id = it->first;
		Location pos = it->second.getPosition();
		hash = ((hash << 5) + hash) + id;
		hash = ((hash << 5) + hash) + pos.x.getInteger();
		hash = ((hash << 5) + hash) + pos.y.getInteger();
		hash = ((hash << 5) + hash) + pos.z.getInteger();
	}
	checksums.push_back(hash);
}


void World::clampToLevelArea(MovableObject& object)
{
	if(object.position.x < 0)
	{
		object.position.x = 0;
		if(object.velocity.x < 0)
			object.velocity.x = 0;
	}
	if(object.position.x > lvl.max_x())
	{
		object.position.x = lvl.max_x();
		if(object.velocity.x > 0)
			object.velocity.x = 0;
	}
	if(object.position.z < 0)
	{
		object.position.z = 0;
		if(object.velocity.z < 0)
			object.velocity.z = 0;
	}
	if(object.position.z > lvl.max_z())
	{
		object.position.z = lvl.max_z();
		if(object.velocity.z > 0)
			object.velocity.z = 0;
	}
    if(object.position.y > 10) {
        object.position.y = 10;
        object.velocity.y = FixedPoint(-1, 10);
    }
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
		if(distance < FixedPoint(1))
			distance = FixedPoint(1);

		velocity_vector.normalize();

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

		iter->second.velocity += velocity_vector / FixedPoint(iter->second["MASS"], 1000);
	}

	// create some effect or something
	Location zero; zero.y = FixedPoint(1, 2);
	visualworld->explosion(pos, zero);
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


void World::createBaseBuildings()
{
    return;
	// void addAIUnit(int id, const Location& pos, int team, VisualWorld::ModelType model_type, int controllerType, float scale, const std::string& name)

	int id = unitIDgenerator.nextID();
	// Location green_base_location = lvl.getRandomLocation(200);
	Location green_base_location = Location(168, 0, 525);
	addAIUnit(id, green_base_location, 0, VisualWorld::ModelType::STONEBEAST_MODEL, Unit::BASE_BUILDING, 4, "Stone\\sBeast\\sOf\\sLife", 250, 0, 100000, 0);
	units[id].staticObject = 1;

	id = unitIDgenerator.nextID();
	// Location red_base_location = lvl.getRandomLocation(103);
	Location green_tower_location = Location(200, 0, 525);
	addAIUnit(id, green_tower_location, 0, VisualWorld::ModelType::STONEBEAST_MODEL, Unit::TOWER_BUILDING, 3, "Defense\\sTower",   100, 0, 100000, 2);
	units[id].staticObject = 1;

	id = unitIDgenerator.nextID();
	// Location red_base_location = lvl.getRandomLocation(103);
	green_tower_location = Location(161, 0, 567);
	addAIUnit(id, green_tower_location, 0, VisualWorld::ModelType::STONEBEAST_MODEL, Unit::TOWER_BUILDING, 3, "Defense\\sTower",   100, 0, 100000, 2);
	units[id].staticObject = 1;

	id = unitIDgenerator.nextID();
	// Location red_base_location = lvl.getRandomLocation(103);
	green_tower_location = Location(264, 0, 600);
	addAIUnit(id, green_tower_location, 0, VisualWorld::ModelType::STONEBEAST_MODEL, Unit::TOWER_BUILDING, 2, "Outer\\sDefense\\sTower",   60, 0, 100000, 2);
	units[id].staticObject = 1;



	id = unitIDgenerator.nextID();
	// Location red_base_location = lvl.getRandomLocation(103);
	Location red_base_location = Location(702, 0, 527);
	addAIUnit(id, red_base_location, 1, VisualWorld::ModelType::STONEBEAST_MODEL, Unit::BASE_BUILDING, 4, "Stone\\sBeast\\sOf\\sLife",   250, 0, 100000, 0);
	units[id].staticObject = 1;

	id = unitIDgenerator.nextID();
	// Location red_base_location = lvl.getRandomLocation(103);
	Location red_tower_location = Location(672, 0, 527);
	addAIUnit(id, red_tower_location, 1, VisualWorld::ModelType::STONEBEAST_MODEL, Unit::TOWER_BUILDING, 3, "Defense\\sTower",   100, 0, 100000, 2);
	units[id].staticObject = 1;

	id = unitIDgenerator.nextID();
	// Location red_base_location = lvl.getRandomLocation(103);
	red_tower_location = Location(666, 0, 561);
	addAIUnit(id, red_tower_location, 1, VisualWorld::ModelType::STONEBEAST_MODEL, Unit::TOWER_BUILDING, 3, "Defense\\sTower",   100, 0, 100000, 2);
	units[id].staticObject = 1;

	id = unitIDgenerator.nextID();
	// Location red_base_location = lvl.getRandomLocation(103);
	red_tower_location = Location(433, 0, 631);
	addAIUnit(id, red_tower_location, 1, VisualWorld::ModelType::STONEBEAST_MODEL, Unit::TOWER_BUILDING, 2, "Outer\\sDefense\\sTower",   60, 0, 100000, 2);
	units[id].staticObject = 1;
}

void World::resetGame()
{
	cerr << "Reseting world game to a feasible start" << endl;
	for(auto it = units.begin(); it != units.end(); ++it)
	{
		if(!it->second.human())
		{
			it->second.hitpoints = -1;
			it->second("DAMAGED_BY") = "Game\\sreset";
			it->second["DELETED"] = 1;
		}
		else
		{
			// restore player hitpoints to maximum values
			it->second.hitpoints = it->second.getMaxHP();

			// TODO: reset ammo counts?

			// TODO: clear world from ammo-items? (if you do - make sure you don't delete the warp point to another world)

		}
	}

	createBaseBuildings();
	teams[0].reset();
	teams[1].reset();

	Location red; findBasePosition(red, 1);
	Location green; findBasePosition(green, 0);

	for(auto it = units.begin(); it != units.end(); ++it)
	{
		if(it->second.human())
		{
			if(it->second["TEAM"] == 0)
			{
				it->second.position = green;
			}
			else if(it->second["TEAM"] == 1)
			{
				it->second.position = red;
			}
		}
	}
}


std::string World::getTeamColour(Unit& u)
{
	if(localPlayerID == -1)
		return "";

	auto it = units.find(localPlayerID);
	if(it == units.end())
		return "";

	if(it->second["TEAM"] == u["TEAM"])
		return "^G";
	if((it->second["TEAM"] ^ u["TEAM"]) == 1)
		return "^R";
	return "^W";
}

int World::getLocalTeam()
{
	if(localPlayerID == -1)
		return -1;

	auto it = units.find(localPlayerID);
	if(it == units.end())
		return -1;

	return it->second["TEAM"];
}

void World::doDeathFor(Unit& unit)
{

	string target_colour = getTeamColour(unit);
	string killer_colour;

	// deleted units don't deserve a burial!
	if(unit["DELETED"] == 1)
	{
		deadUnits.push_back(unit.id);
		return;
	}

	stringstream msg;
	string killer = "an unknown entity";

	int causeOfDeath = unit.last_damage_dealt_by;

	int actor_id  = -1;
	int target_id = -1;

	target_id = unit.id;

	auto killer_it = units.find(causeOfDeath);
	if(killer_it != units.end())
	{
		killer_colour = getTeamColour(killer_it->second);
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


	Location t_position = unit.getEyePosition();
	Location t_velocity = unit.velocity; t_velocity.y += FixedPoint(200,1000);
	Location a_position;
	Location a_velocity;

	bool verbose = false;

	if(actor_id != -1)
	{
		auto it = units.find(actor_id);
		a_position = it->second.getEyePosition();
		a_velocity = it->second.velocity;
		verbose = it->second.human();
	}

	verbose = verbose || unit.human();

	if(actor_id != target_id)
	{
		int i = currentWorldFrame % killWords.size();

		if(verbose)
		{
			msg << killer_colour << killer << "^W" << killWords[i] << target_colour << unit.name << "^W" << afterWords[i] << " ^g(" << unit("DAMAGED_BY") << ")";
			visualworld->add_message(msg.str());
		}
	}
	else
	{
		if(verbose)
		{
			actor_id  = -1; // For a suicide, no points are to be awarded.
			msg << target_colour << killer << " ^Whas committed suicide by ^g" << unit("DAMAGED_BY");
			visualworld->add_message(msg.str());
		}
	}

	// visualworld->addLight(nextUnitID(), event.t_position);

	if(unit.human())
	{
		{
			DeathPlayerEvent event;
			event.t_position = t_position;
			event.t_velocity = t_velocity;
			event.a_position = a_position;
			event.a_velocity = a_velocity;
			event.actor_id = actor_id;
			event.target_id = target_id;
			queueMsg(event);
		}

		// reset player hitpoints
		unit.hitpoints = unit.getMaxHP();

		// respawn player back in base
		Location pos;
		findBasePosition(pos, unit["TEAM"]);
		unit.setPosition(pos);

		// stop any movement, let the player drop down to the field of battle.
		unit.zeroMovement();
	}
	else
	{
		if(unit.controllerTypeID == Unit::BASE_BUILDING)
		{
			int local_team = getLocalTeam();

			GameOver event;
			event.win = 0;

			switch(local_team)
			{
				case -1:
					if(unit["TEAM"] == 0)
					{
						visualworld->add_message("^GGreen team won the match!");
						visualworld->add_message("^gNew round begins!");
					}
					else
					{
						visualworld->add_message("^RRed team won the match!");
						visualworld->add_message("^gNew round begins!");
					}
					break;

				case 0:
				case 1:
					if(unit["TEAM"] == local_team)
					{
						event.win = 0;
						visualworld->add_message("YOU HAVE ^RLOST^W THE MATCH!");
						visualworld->add_message("^gNew round begins!");
					}
					else
					{
						event.win = 1;
						visualworld->add_message("YOU HAVE ^GWON^W THE MATCH!");
						visualworld->add_message("^gNew round begins!");
					}
					break;
			}

			sendMsg(event);
			resetGame();
		}
		else if(unit.controllerTypeID == Unit::TOWER_BUILDING)
		{
			int local_team = getLocalTeam();

			int team = unit["TEAM"];
			if(team == 1 || team == 0)
			{
				++teams[team ^ 1].level;

				if(local_team == team)
					visualworld->add_message("^REnemy creeps have grown stronger!");
				else
					visualworld->add_message("^GFriendly creeps have grown stronger!");
			}
		}


		{
			DeathNPCEvent event;
			event.t_position = t_position;
			event.t_velocity = t_velocity;
			event.a_position = a_position;
			event.a_velocity = a_velocity;
			event.actor_id = actor_id;
			event.target_id = target_id;
			queueMsg(event);
		}

		deadUnits.push_back(unit.id);
	}
}


void World::findBasePosition(Location& pos, int team)
{
	for(auto it = units.begin(); it != units.end(); ++it)
	{
		if((it->second.controllerTypeID == Unit::BASE_BUILDING) && (it->second["TEAM"] == team) && (it->second.hitpoints > 0))
		{
			pos = it->second.position;
		}
	}
}

World::World(VisualWorld* vw)
{
	localPlayerID = -1; // default value.

	assert(vw);
	visualworld = vw;
	init();
}

void World::buildTerrain(int n, float& percentage_done)
{
	// TODO, post-passes should not be constant in code
	lvl.generate(n, 6, percentage_done);
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
	unitIDgenerator.setNextID(10000);
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

void World::tickItem(WorldItem& item, Model* model)
{
	// wut
	if(visualworld->isActive())
	{
		assert(model && "item model does not exist");
		model->updatePosition(item.position.x.getFloat(), item.position.y.getFloat(), item.position.z.getFloat());
	}

	if(item.intVals["AREA_CHANGE"])
	{
		visualworld->genParticleEmitter(item.position + Location(0, 1, 0), item.velocity + Location(0, 1, 0),
										3, 1000, 1000, "GREEN", "GREEN", "GREEN", "GREEN", 1000, 5, 100);
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

	clampToLevelArea(item);
}

void World::tickUnit(Unit& unit, Model* model)
{
	unit.intVals["D"] *= 0.95;

	if(currentWorldFrame % 50 == 0)
		unit.regenerate();

	switch(unit.controllerTypeID)
	{
		case Unit::AI_RABID_ALIEN:
		{
			AI_RabidAlien(unit);
			break;
		}

		case Unit::BASE_BUILDING:
		{
			AI_BaseBuilding(unit);
			break;
		}

		case Unit::TEAM_CREEP:
		{
			AI_TeamCreep(unit);
			// AI_TeamCreep(unit);
			break;
		}

		case Unit::TOWER_BUILDING:
		{
			AI_TowerBuilding(unit);
			break;
		}

		case Unit::INANIMATE_OBJECT:
		{
			break;
		}

		default:
			break;
	}

	// for server it's ok that there are no models sometimes :G
	if(visualworld->isActive())
	{
		assert(model && "this should never happen");
		model->rotate_y(unit.getAngle());
		model->updatePosition(unit.position.x.getFloat(), unit.position.y.getFloat(), unit.position.z.getFloat());
	}



	// some physics & game world information
	if( (unit.velocity.y + unit.position.y - FixedPoint(1, 20)) <= lvl.getHeight(unit.position.x, unit.position.z) )
		unit.mobility |= Unit::MOBILITY_STANDING_ON_GROUND;


	if(unit.hasSupportUnderFeet())
	{
		if(unit.velocity.y < FixedPoint(-7, 10))
			unit.soundInfo = "jump_land";

		unit.landingDamage();
		unit.applyFriction();

		if(unit.hasGroundUnderFeet())
		{
			unit.position.y = lvl.getHeight(unit.position.x, unit.position.z);
			unit.velocity.y = FixedPoint::ZERO;
		}
	}


	unit.applyGravity();
	unit.updateMobility();
	unit.processInput();

	// weapon activations
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

	unit.tick(yy_val);
	clampToLevelArea(unit);
	unit.postTick();
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

			// no collision with dead units
			if(!u->exists())
				continue;

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
					Location distance_vector = projectile.position - u->getEyePosition(); // TODO: Point to point distance is maybe not ideal.
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

			// boolean test, hits or doesn't hit
			if(projectile["COLLISION_TEST"] && projectile.collides(*u))
			{
				{
					BulletHitEvent event;
					event.t_position = u->getEyePosition();
					event.t_velocity = u->velocity;
					event.a_position = projectile.position;
					event.a_velocity = projectile.velocity * projectile["TPF"];
					queueMsg(event);
				}

				u->velocity += projectile.velocity * FixedPoint(projectile["MASS"], 1000) / FixedPoint(u->intVals["MASS"], 1000);

				if(!friendly_fire && ((*u)["TEAM"] == team))
				{
				}
				else
				{
					u->takeDamage(projectile["DAMAGE"]); // does damage according to weapon definition :)
					u->last_damage_dealt_by = projectile_owner;
					(*u)("DAMAGED_BY") = projectile_name;
				}

				// blood is blood is blood
				auto string_property_it = projectile.strVals.find("AT_DEATH");
				if(string_property_it != projectile.strVals.end())
				{
					if(string_property_it->second == "PUFF")
					{
						projectile.strVals.erase("AT_DEATH");
					}
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



void World::addRandomMonster()
{
	int enemies = getUnitCount();

	if(enemies >= intVals["MON_CAP"])
		return;

	vector<string> options;
	if(intVals.find("_CAVE") != intVals.end())
	{
		options.push_back("_CAVE");
	}

	if(intVals.find("_GRASS") != intVals.end())
	{
		options.push_back("_GRASS");
	}

	if(intVals.find("_MOUNTAIN") != intVals.end())
	{
		options.push_back("_MOUNTAIN");
	}

	string monster_home = options[currentWorldFrame % options.size()];
	int id = unitIDgenerator.nextID();

	if(monster_home == "_CAVE")
	{
		addUnit(id, false, -1);
		units[id].name = "Stone\\sbeast";

		int stonebeast_size = (currentWorldFrame % 4) + 3;
		units[id].intVals["STR"]  = 4 + 3 * stonebeast_size;
		units[id].intVals["DEX"]  = 4 - stonebeast_size;
		units[id].intVals["MASS"] = 5000;
	}
	else if(monster_home == "_MOUNTAIN")
	{
		addUnit(id, false, -1);
		units[id].name = "Troll";

		int troll_size = (currentWorldFrame % 4) + 3;
		units[id].intVals["STR"] = 4 + troll_size;
		units[id].intVals["DEX"] = 4;
		units[id].intVals["MASS"] = 2000;
	}
	else if(monster_home == "_GRASS")
	{
		addUnit(id, false, -1);
		units[id].name = "Moogle";

		int moogle_age = (currentWorldFrame % 4) + 3;
		units[id].intVals["STR"] = 4;
		units[id].intVals["DEX"] = 4 + 2 * moogle_age;
		units[id].intVals["MASS"] = 700;
	}
	else
	{
		throw std::logic_error("Adding random monster failed.\nUnknown monster home terrain type: " + monster_home);
	}

	units[id].scale     = FixedPoint(units[id].getModifier("STR"), 10);
	units[id].hitpoints = units[id].getMaxHP();

	VisualWorld::ModelType type = visualworld->getModelType(units[id].name);
	float scale = units[id].scale.getFloat();
	visualworld->createModel(id, units[id].position, type, scale);

	return;
}


void World::worldTick(int tickCount)
{

	currentWorldFrame = tickCount;

	// TODO: should have a method to update the state of a MovableObject !! (instead of a separate tick for every type..)

	friendly_fire = (intVals["FRIENDLY_FIRE"] == 1);

	// if this area has monsters autospawning
	if(intVals["MON_SPAWN"])
	{
		int& freq = intVals["MON_FREQ"];
		if(tickCount % freq == 0)
		{
			// TODO: Possibility to spawn monster groups.

			// it's time to create a monster!
			addRandomMonster();

		}
	}


	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Build octree + do collisions    \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/

	octree.reset(new Octree(Location(0, 0, 0), Location(lvl.max_x(), FixedPoint(400), lvl.max_z())));

	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		octree->insertObject(&(iter->second));
	}

	octree->doCollisions();


	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Tick units and projectiles      \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/

#if LAZY_UPDATE > 0
	bool players_present = (units.begin()->first < 10000);

	// shut down AI units if there are no players present.
	if(players_present)
	{
		for(auto iter = units.begin(); iter != units.end(); ++iter)
		{
			Model* model = visualworld->getModel(iter->first);
			tickUnit(iter->second, model);
		}
	}
#else
	for(auto iter = units.begin(); iter != units.end(); ++iter)
	{
		Model* model = visualworld->getModel(iter->first);
		tickUnit(iter->second, model);
	}
#endif

	// ticking projectiles even without players is ok, since projectiles will die with time.
	for(map<int, Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		Model* model = visualworld->getModel(iter->first);
		tickProjectile(iter->second, model);
	}

#if LAZY_UPDATE > 0
	// don't waste time on moving items either, if no players are there.
	if(players_present)
	{
		for(auto iter = items.begin(); iter != items.end(); ++iter)
		{
			Model* model = visualworld->getModel(iter->first);
			tickItem(iter->second, model);
		}
	}
#else
	for(auto iter = items.begin(); iter != items.end(); ++iter)
	{
		Model* model = visualworld->getModel(iter->first);
		tickItem(iter->second, model);
	}
#endif

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

			// add some ammunition into the world.
			FixedPoint half = FixedPoint(1, 2);
			int id1 = unitIDgenerator.nextID(); addItem(gear_pos, gear_vel + Location(half, 0, half), id1);
			items[id1].intVals["AMMO_BOOST"] = 1;
			items[id1].intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
			visualworld->createModel(id1, gear_pos, VisualWorld::ModelType::ITEM_MODEL, 1.0f);

			int id2 = unitIDgenerator.nextID(); addItem(gear_pos, gear_vel + Location(0, half, -half), id2);
			items[id2].intVals["AMMO_BOOST"] = 1;
			items[id2].intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
			visualworld->createModel(id2, gear_pos, VisualWorld::ModelType::ITEM_MODEL, 1.0f);

			int id3 = unitIDgenerator.nextID(); addItem(gear_pos, gear_vel + Location(-half, -half, 0), id3);
			items[id3].intVals["AMMO_BOOST"] = 1;
			items[id3].intVals["MODEL_TYPE"] = VisualWorld::ModelType::WEAPON_MODEL;
			visualworld->createModel(id3, gear_pos, VisualWorld::ModelType::WEAPON_MODEL, 2.0f);

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


void World::addAIUnit(int id, const Location& pos, int team, VisualWorld::ModelType model_type, int controllerType, FixedPoint scale, const std::string& name, int strength, int dexterity, int mass, int default_weapon)
{
	if(units.find(id) != units.end())
		throw std::logic_error("Trying to create a unit, but the unitID is already reserved.");

	units[id] = Unit();
	units[id].init();
	units[id].scale = scale;

	units[id].position = pos;
	units[id].id = id;
	units[id].birthTime = currentWorldFrame;

	units[id].model_type = model_type;
	visualworld->createModel(id, units[id].position, model_type, scale.getFloat());

	// TODO: FIX THIS SHIT
	units[id].setDefaultMonsterAttributes();
	units[id].hitpoints = 500;

	units[id].name = name;
	units[id].controllerTypeID = controllerType;

	units[id]["TEAM"] = team;
	units[id]["T"] = -1;

	units[id].intVals["STR"]  = strength;
	units[id].intVals["DEX"]  = dexterity;
	units[id].intVals["MASS"] = mass;

	units[id].hitpoints = units[id].getMaxHP();

	units[id].weapon = default_weapon;
	// all other weapons are ok, but no rocket launcher for AI controlled units!
}

void World::addUnit(int id, bool playerCharacter, int team)
{
	if(units.find(id) != units.end())
		throw std::logic_error("Trying to create a unit, but the unitID is already reserved.");

	units[id] = Unit();
	units[id].init();

	int r_seed = team + 7;
	if(playerCharacter)
	{
		r_seed += 17;
	}

	units[id].position = lvl.getRandomLocation(currentWorldFrame + r_seed);
	units[id].id = id;

	units[id].birthTime = currentWorldFrame;

	if(!playerCharacter)
	{
		units[id].model_type = VisualWorld::ModelType::ZOMBIE_MODEL;
		visualworld->createModel(id, units[id].position, VisualWorld::ModelType::ZOMBIE_MODEL, 1.0f);

		units[id].setDefaultMonsterAttributes();

		units[id].name = "Alien\\smonster";
		units[id].controllerTypeID = Unit::AI_RABID_ALIEN;
		units[id].hitpoints = 500;
		units[id]["TEAM"] = team;
		units[id]["T"] = -1;

		units[id].weapon = currentWorldFrame % (units[id].weapons.size() - 1);
		// all other weapons are ok, but no rocket launcher for enemies!
	}
	else
	{
		findBasePosition(units[id].position, units[id]["TEAM"]);

		units[id].model_type = VisualWorld::ModelType::PLAYER_MODEL;
		visualworld->createModel(id, units[id].position, VisualWorld::ModelType::PLAYER_MODEL, 1.0f);

		units[id].setDefaultPlayerAttributes();

		units[id].name = "Unknown\\sPlayer";
		units[id].controllerTypeID = Unit::HUMAN_INPUT;
		units[id].hitpoints = 1000;
		units[id]["TEAM"] = id % 2;
	}
}

void World::addProjectile(Location& location, int id, size_t model_prototype)
{
	vec3<float> position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();


	visualworld->createModel(id, location, VisualWorld::ModelType(model_prototype), 0.4f);
	projectiles[id].position = location;
	projectiles[id].prototype_model = model_prototype;
}

void World::addItem(const Location& location, const Location& velocity, int id)
{
	// NOTE: We don't know what kind of an item it is here, so let's not decide on the model here either.
	// visualworld->createModel(id, location, VisualWorld::ITEM_MODEL, 1.0f);

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

int World::getUnitCount()
{
	/*
	int count = 0;
	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		if (!iter->second.human())
			count++;
	}
	*/
	return units.size();
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

void World::handle(const GotPlayerID& event)
{
	localPlayerID = event.myID;
}

void World::add_message(const std::string& message) const
{
	visualworld->add_message(message);
}
