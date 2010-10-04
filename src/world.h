

/*
WARNING: THIS FILE IS FULL OF SHIT
*/

#include <map>
#include <vector>
#include <string>

#include "unit.h"
#include "model.h"
#include "level.h"
#include "apomath.h"

class World
{
	void tickUnit(Unit&, Model&);    // world frame update
	void updateModel(Model&, Unit&); // view frame update
	
	int heightDifference2Velocity(int h_diff);
	
public:
	World();
	void init();
	
	std::map<int, Unit> units;   // each unit corresponds to
	std::map<int, Model> models; // one of these BUT
	
	Level lvl;
	ApoMath apomath;
	
	void worldTick();
	void viewTick();
	
	void addUnit(int id);
	
	int _unitID_next_unit;
	int nextUnitID();
	
	void terminate(); // don't call this unless you mean it :D
};

