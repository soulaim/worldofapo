/*
 * File:   beamweaponusage.h
 * Author: urtela
 *
 * Created on October 5, 2011, 10:20 PM
 */

#ifndef BEAMWEAPONUSAGE_H
#define	BEAMWEAPONUSAGE_H

#include <string>
#include <vector>

class WorldItem;
class World;
class Unit;

class BeamWeaponUsage {

public:
    void getDetails(WorldItem*, std::vector<std::string>&);
    void onActivatePrimary(WorldItem*, World&, Unit&);
    void onActivateReload(WorldItem*, World&, Unit&);
    void tick(WorldItem*, Unit&);
};


#endif	/* BEAMWEAPONUSAGE_H */

