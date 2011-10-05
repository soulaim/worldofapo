/*
 * File:   toolitemusage.h
 * Author: urtela
 *
 * Created on October 5, 2011, 10:20 PM
 */

#ifndef TOOLITEMUSAGE_H
#define	TOOLITEMUSAGE_H

class WorldItem;
class World;
class Unit;

class ToolItemUsage {

public:
    void onActivatePrimary(WorldItem*, World&, Unit&);
};

#endif	/* TOOLITEMUSAGE_H */

