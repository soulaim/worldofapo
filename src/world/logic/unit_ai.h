/*
 * File:   unit_ai.h
 * Author: urtela
 *
 * Created on October 10, 2011, 4:20 PM
 */

#ifndef UNIT_AI_H
#define	UNIT_AI_H

class World;
class Unit;

class UnitAI {
public:
    void tick(World&, Unit&);
};

#endif	/* UNIT_AI_H */

