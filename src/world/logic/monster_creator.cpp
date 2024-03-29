
#include "world/logic/monster_creator.h"
#include "world/random.h"

void MonsterCreator::setAttributes(Unit& unit, int depth, RandomMachine& random)
{
    HasProperties& stats = unit.getStatsEditor();

    // these are taken into account.
    stats.intVals["CONSTITUTION"] = depth - 9;
    stats.intVals["MOVEMENT"] = depth / 2 - 3;
    stats.intVals["TELEPATHIC"] = 10;

    // :G
    stats.intVals["BALLISTIC"] = random.getInt() % (depth+1);
    stats.intVals["BEAM"] = 10;
    stats.intVals["ENGINEER"] = 10;
    stats.intVals["ZEN"] = 10;
    stats.intVals["SNEAK"] = 10;
    stats.intVals["PERCEPTION"] = 10;
    stats.intVals["DARKVISION"] = 10;

    unit.intVals["SANITY"] = 100;
	unit.intVals["HEALTH"] = unit.getMaxHP();
	unit.intVals["REGEN"] = 0;
}

Unit MonsterCreator::createMonster(int depth, int tick, int mon_num) {
    Unit monster;
    RandomMachine random;
    random.setSeed(tick * 4217 + mon_num * 81321);

    setAttributes(monster, depth, random);

    monster.birthTime = tick;
    monster.name = "Alien";
    monster.controllerTypeID = Unit::AI_ALIEN;
    monster.id = mon_num;


    monster.intVals["MASS"] = 1000;
    monster["TEAM"] = 1;
    monster["T"] = -1;
    monster["HEALTH"] = monster.getMaxHP();

    monster["REWARD"] = 3 + depth * 2;
    return monster;
}
