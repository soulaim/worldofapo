/*
 * File:   monster_creator.h
 * Author: urtela
 *
 * Created on October 13, 2011, 5:27 PM
 */

#ifndef MONSTER_CREATOR_H
#define	MONSTER_CREATOR_H

#include "world/objects/unit.h"

class RandomMachine;

class MonsterCreator {
    void setAttributes(Unit& unit, int depth, RandomMachine& random);
public:
    Unit createMonster(int depth, int tick, int mon_num = 1);
};


#endif	/* MONSTER_CREATOR_H */

