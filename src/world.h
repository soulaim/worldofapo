#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <vector>
#include <string>

#include "unit.h"
#include "projectile.h"
#include "model.h"
#include "level.h"
#include "apomath.h"

class World
{
	void tickUnit(Unit&, Model&);       // world frame update
	void tickProjectile(Projectile&, Model&, int id); // world frame update
	void updateModel(Model&, Unit&); // view frame update
	
	FixedPoint heightDifference2Velocity(const FixedPoint& h_diff) const;
	
public:
	World();
	void init();
	
	std::map<int, Unit> units;
	std::map<int, Model> models;
	std::map<int, Projectile> projectiles;
	
	Level lvl;
	ApoMath apomath;
	
	void worldTick();
	void viewTick();
	
	void addUnit(int id);
	void addProjectile(Location& location, int);

	void removeUnit(int id);
	
	int _unitID_next_unit;
	int _playerID_next_player;
	
	int nextUnitID();
	int nextPlayerID();
	
	void terminate(); // don't call this unless you mean it :D
};

#endif

