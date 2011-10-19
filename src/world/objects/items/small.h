/*
 * File:   small.h
 * Author: urtela
 *
 * Created on October 10, 2011, 9:59 PM
 */

#ifndef SMALL_H
#define	SMALL_H

#include <string>
#include <vector>

class WorldItem;
class World;
class Unit;

class SmallItemUsage {
    void applyEffect(WorldItem* item, World& world, Unit& unit, const std::string effect);
public:
    void onActivatePrimary(WorldItem* item, World& world, Unit& unit);
    void getDetails(WorldItem*, std::vector<std::string>&);
    void tick(WorldItem*, Unit&);
};

#endif	/* SMALL_H */

