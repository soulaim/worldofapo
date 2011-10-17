/*
 * File:   unit_ai.h
 * Author: urtela
 *
 * Created on October 10, 2011, 4:20 PM
 */

#ifndef UNIT_AI_H
#define	UNIT_AI_H

#include "misc/vec3.h"

class World;
class Unit;

class UnitAI {

    void turnTowardsTarget(Unit&);
    void turnTowardsTarget(Unit&, Location&, int&, int&);
    void turnTowardsTarget(Unit&, Unit&, int&, int&);
    bool roamingNextStepOk(World&, Unit&);
    Location findLongestLine(World&, Unit&);

public:
    void tick(World&, Unit&);
};

#endif	/* UNIT_AI_H */

