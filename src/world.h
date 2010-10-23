#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <vector>
#include <string>
#include <memory>

#include "unit.h"
#include "projectile.h"
#include "model.h"
#include "level.h"
#include "apomath.h"
#include "octree.h"

#include "light_object.h"

struct WorldEvent
{
	int type;
	
	Location position;
	Location velocity;
	
	int actor_id;
	int target_id;
};

class World
{
	void tickUnit(Unit&, Model*);       // world frame update
	void tickProjectile(Projectile&, Model*, int id); // world frame update
	void updateModel(Model*, Unit&); // view frame update
	
	int currentWorldFrame;
	FixedPoint heightDifference2Velocity(const FixedPoint& h_diff) const;
	void generateInput_RabidAlien(Unit& unit);
	void doDeathFor(Unit& unit, int causeOfDeath);

public:
	
	// identifications for events where we want to do some SFX
	enum
	{
		DAMAGE_BULLET,
		DAMAGE_DEVOUR,
		DEATH_PLAYER,
		DEATH_ENEMY,
		CENTER_CAMERA
	};
	
	World();
	void init();

	std::shared_ptr<Octree> o;
	
	std::map<int, Unit> units;
	std::map<int, Model*> models;
	std::map<int, Projectile> projectiles;
	
	std::map<int, LightObject> lights;
	
	// TODO: Have some fun with explosions :D (BERBERI DO NOT TOUCH THIS!!)
	// std::map<int, Explosion> explosions;
	
	Level lvl;
	ApoMath apomath;
	
	std::vector<std::string> worldMessages;
	std::vector<int> deadUnits;
	std::vector<WorldEvent> events;

	std::vector<Location> humanPositions();
	
	void worldTick(int tickCount);
	void viewTick();
	
	void addUnit(int id, bool player = true);
	void addProjectile(Location& location, int);
	
	void removeUnit(int id);
	
	int _unitID_next_unit;
	int _playerID_next_player;
	
	int nextUnitID();
	int nextPlayerID();

	int show_errors;

	int getZombies();
	
	void terminate(); // don't call this unless you mean it :D

	void add_message(const std::string& message);
	void add_event(const WorldEvent& event);
};

#endif

