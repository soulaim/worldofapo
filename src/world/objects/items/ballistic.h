/*
 * File:   ballisticweaponusage.h
 * Author: urtela
 *
 * Created on October 5, 2011, 10:20 PM
 */

#ifndef BALLISTICWEAPONUSAGE_H
#define	BALLISTICWEAPONUSAGE_H

class WorldItem;
class World;
class Unit;


class BallisticWeaponUsage {

public:
    void onActivatePrimary(WorldItem*, World&, Unit&);
    void tick(WorldItem*, Unit&);
};

#endif	/* BALLISTICWEAPONUSAGE_H */

