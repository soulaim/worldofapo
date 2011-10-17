
#include "world/logic/monster_creator.h"
#include "world/random.h"

void MonsterCreator::setAttributes(Unit& unit, int depth, RandomMachine&)
{
    unit.intVals["SANITY"] = 100;
	unit.intVals["HEALTH"] = 200;

    // these two are taken into account.
    unit.stats.intVals["CONSTITUTION"] = depth * 3;
    unit.stats.intVals["MOVEMENT"] = 5;

    // :G
    unit.stats.intVals["TELEPATHIC"] = 10;
    unit.stats.intVals["BALLISTIC"] = 10;
    unit.stats.intVals["BEAM"] = 10;
    unit.stats.intVals["ENGINEER"] = 10;
    unit.stats.intVals["ZEN"] = 10;
    unit.stats.intVals["SNEAK"] = 10;
    unit.stats.intVals["PERCEPTION"] = 10;
    unit.stats.intVals["DARKVISION"] = 10;

	unit.intVals["REGEN"] = 5;
}

Unit MonsterCreator::createMonster(int depth, int tick, int mon_num) {
    Unit monster;
    RandomMachine random;
    random.setSeed(tick * 4217 + mon_num * 81321);

    monster.birthTime = tick;
    monster.name = "Alien";
    monster.controllerTypeID = Unit::AI_ALIEN;
    monster.id = mon_num;


    monster.intVals["MASS"] = 1000;
    monster["TEAM"] = 1;
    monster["T"] = -1;
    monster.hitpoints = monster.getMaxHP();

    setAttributes(monster, depth, random);
    return monster;
}
