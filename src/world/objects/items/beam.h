/*
 * File:   beamweaponusage.h
 * Author: urtela
 *
 * Created on October 5, 2011, 10:20 PM
 */

#ifndef BEAMWEAPONUSAGE_H
#define	BEAMWEAPONUSAGE_H

class WorldItem;
class World;
class Unit;

class BeamWeaponUsage {

public:
    void onActivatePrimary(WorldItem*, World&, Unit&);
    void onActivateReload(WorldItem*, World&, Unit&);
    void tick(WorldItem*, Unit&);
};


#endif	/* BEAMWEAPONUSAGE_H */

