#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <vector>
#include <string>
#include <memory>

#include "world_item.h"
#include "unit.h"
#include "projectile.h"
#include "level.h"
#include "apomath.h"
#include "octree.h"
#include "movable_object.h"
#include "worldevent.h"
#include "idgenerator.h"
#include "vec3.h"
#include "visualworld.h"

class Model;

class World : public HasProperties
{
	
	void AI_TeamCreep(Unit& unit);
	void AI_RabidAlien(Unit& unit);
	void AI_BaseBuilding(Unit& unit);
	
	void clampToLevelArea(MovableObject&);
	void findBasePosition(Location& pos, int team);
	
	void tickUnit(Unit&, Model*);                 // world frame update
	void tickProjectile(Projectile&, Model*);     // world frame update
	void tickItem(WorldItem& item, Model* model); // world frame update
	
	static FixedPoint heightDifference2Velocity(const FixedPoint& h_diff);
	void doDeathFor(Unit& unit);
	
	void instantForceOutwards(const FixedPoint& power, const Location& pos, const std::string& name, int owner);
	void atDeath(MovableObject&, HasProperties&);
	void resetGame();
	
public:

	int currentWorldFrame;
	VisualWorld* visualworld;
	typedef unsigned CheckSumType;
	
	World(VisualWorld*);
	void init();

	std::shared_ptr<Octree> octree;
	
	std::map<int, Unit> units;
	std::map<int, Projectile> projectiles;
	std::map<int, WorldItem> items;
	
	Level lvl;
	
	int getZombies();
	std::vector<Location> humanPositions() const;
	
	void worldTick(int tickCount);
	
	void createBaseBuildings();
	void addRandomMonster();
	
	void addAIUnit(int id, const Location& pos, int team, VisualWorld::ModelType model_type, int controllerType, FixedPoint scale, const std::string& name, int strength, int dexterity, int mass);
	void addUnit(int id, bool player = true, int team = -1);
	
	void addProjectile(Location& location, int id, size_t model_prototype);
	void addItem(const Location& location, const Location& velocity, int id); // TODO: more control over what is spawned
	
	void removeUnit(int id);
	
	void setNextUnitID(int id);
	int currentUnitID() const;
	int nextUnitID();
	
	int show_errors;
	bool friendly_fire; // not to be sent over network.
	
	void terminate();
	
	void checksum(std::vector<World::CheckSumType>&) const;
	
	void add_message(const std::string& message) const;
	void add_event(const WorldEvent& event) const;
	
	void buildTerrain(int n, float&);
	std::string generatorMessage();
	
private:
	std::vector<int> deadUnits;
	ApoMath apomath;
	
	IDGenerator unitIDgenerator;
};

#endif

