#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <vector>
#include <string>
#include <memory>

#include "unit.h"
#include "projectile.h"
#include "level.h"
#include "apomath.h"
#include "octree.h"
#include "movable_object.h"
#include "worldevent.h"

class VisualWorld;
class Model;

class World : public HasProperties
{
	void tickUnit(Unit&, Model*);       // world frame update
	void tickProjectile(Projectile&, Model*); // world frame update
	
	static FixedPoint heightDifference2Velocity(const FixedPoint& h_diff);
	void generateInput_RabidAlien(Unit& unit);
	void doDeathFor(Unit& unit);
	
	void instantForceOutwards(const FixedPoint& power, const Location& pos, const std::string& name, int owner);
	void atDeath(MovableObject&, HasProperties&);
	
public:
	int currentWorldFrame;
	VisualWorld* visualworld;

	enum ModelType
	{
		INVISIBLE_MODEL,
		PLAYER_MODEL,
		BULLET_MODEL
	};
	
	World(VisualWorld*);
	void init();

	std::shared_ptr<Octree> octree;
	
	std::map<int, Unit> units;
	std::map<int, Projectile> projectiles;
	
	Level lvl;
	
	
	int getZombies();
	std::vector<Location> humanPositions() const;
	
	void worldTick(int tickCount);
	
	void addUnit(int id, bool player = true, int team = -1);
	void addProjectile(Location& location, int id, size_t model_prototype);
	void removeUnit(int id);
	
	void setNextUnitID(int id);
	int currentUnitID() const;
	int nextUnitID();
	int nextPlayerID();
	
	int show_errors;
	bool friendly_fire; // not to be sent over network.
	
	void terminate();
	
	unsigned long checksum() const;

	void add_message(const std::string& message) const;
	void add_event(const WorldEvent& event) const;
	
private:
	std::vector<int> deadUnits;
	ApoMath apomath;
	
	int _unitID_next_unit;
	int _playerID_next_player;
	
};

#endif

