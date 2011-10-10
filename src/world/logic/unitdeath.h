/*
 * File:   unitdeath.h
 * Author: urtela
 *
 * Created on October 10, 2011, 4:21 PM
 */

#ifndef UNITDEATH_H
#define	UNITDEATH_H

class World;
class Unit;

class UnitDeathHandler {
public:
    void doDeathFor(World&, Unit&);
};

#endif	/* UNITDEATH_H */

