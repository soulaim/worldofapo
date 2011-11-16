#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <vector>
#include <string>
#include <memory>

#include "graphics/visualworld.h"
#include "world/objects/world_item.h"
#include "world/objects/unit.h"
#include "world/objects/projectile.h"
#include "world/level/level.h"
#include "world/worldevent.h"
#include "physics/octree.h"
#include "physics/movable_object.h"
#include "misc/idgenerator.h"
#include "misc/vec3.h"
#include "misc/apomath.h"
#include "misc/messaging_system.h"

#include "world/logic/item_creator.h"
#include "world/logic/projectile_ticker.h"
#include "world/logic/unit_ticker.h"
#include "world/logic/item_ticker.h"
#include "world/logic/unitdeath.h"

class Model;

class World : public HasProperties, public MessagingSystem<GotPlayerID>, public MessagingSystem<SoundEvent>
{

	struct Team
	{
		Team();
		void reset();

		int level;
	};

	Team teams[2];

    ItemCreator      itemCreator;
    ProjectileTicker projectileTicker;
    UnitTicker       unitTicker;
    ItemTicker       itemTicker;
    UnitDeathHandler unitDeathHandler;

	void tickItem(WorldItem& item, Model* model); // world frame update

	void instantForceOutwards(const FixedPoint& power, const Location& pos, const std::string& name, int owner);
	void atDeath(MovableObject&, HasProperties&);

	int getLocalTeam();
    void createLevelObjects(); //fazias

public:

    void handle(const SoundEvent& event);
    std::string getTeamColour(Unit&);

	int currentWorldFrame;
	VisualWorld* visualworld;
	typedef unsigned CheckSumType;

    void awardExperience(Unit& deadUnit);
    void unitHasDied(int id);
	World(VisualWorld*);
	void init();

	std::shared_ptr<Octree> octree;

	std::map<int, Unit> units;
	std::map<int, Projectile> projectiles;
	std::map<int, WorldItem> items;

	Level lvl;

	void resetGame();
	void handle(const GotPlayerID& event);

	int getUnitCount(); // TODO: Maybe it's about time to change this..
	void worldTick(int tickCount);

    void addBoxUnit(int id, const Location& location);
	void addAIUnit(int id, const Location& pos, int depth = 1);
	void addUnit(int id, bool player = true, int team = -1);

	void addProjectile(Location& location, int id, size_t model_prototype);
    void addItem(WorldItem& item, VisualWorld::ModelType modelType, int id);

	void removeUnit(int id);

	void setNextUnitID(int id);
	int currentUnitID() const;
	int nextUnitID();

	int show_errors;
	bool friendly_fire; // not to be sent over network.

	void terminate();

	void checksum(std::vector<World::CheckSumType>&) const;
	void add_message(const std::string& message) const;
	void buildTerrain(int n, float&);
	std::string generatorMessage();

    ApoMath apomath;

    int getLocalPlayerID();

private:
	int localPlayerID;
	std::vector<int> deadUnits;
	IDGenerator unitIDgenerator;
};

#endif

