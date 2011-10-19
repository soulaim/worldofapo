/*
 * File:   unitdeath.h
 * Author: urtela
 *
 * Created on October 10, 2011, 4:21 PM
 */

#ifndef UNITDEATH_H
#define	UNITDEATH_H

#include "world/logic/playerdeath.h"
#include <string>

class World;
class Unit;

class UnitDeathHandler {

    PlayerDeathHandler playerDeath;

    std::string target_colour;
	std::string killer_colour;
    std::string killer;

	int causeOfDeath;
	int actor_id;
	int target_id;

    void updateComponents(World&, Unit&);

public:
    void doDeathFor(World&, Unit&);
};

#endif	/* UNITDEATH_H */

