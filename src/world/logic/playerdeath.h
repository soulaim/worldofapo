/*
 * File:   playerdeath.h
 * Author: urtela
 *
 * Created on October 10, 2011, 4:21 PM
 */

#ifndef PLAYERDEATH_H
#define	PLAYERDEATH_H

class World;
class Unit;

class PlayerDeathHandler {
public:
    void handle(World&, Unit&);
};

#endif	/* PLAYERDEATH_H */

