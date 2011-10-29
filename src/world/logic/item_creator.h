/*
 * File:   item_creator.h
 * Author: urtela
 *
 * Created on October 10, 2011, 6:20 PM
 */

#ifndef ITEM_CREATOR_H
#define	ITEM_CREATOR_H

#include "world/objects/world_item.h"

class RandomMachine;

class ItemCreator {
public:
    WorldItem createAmulet(int, RandomMachine&);
    WorldItem createArmsArmor(int, RandomMachine&);
    WorldItem createTorsoArmor(int, RandomMachine&);
    WorldItem createHeadArmor(int, RandomMachine&);
    WorldItem createLegArmor(int, RandomMachine&);
    WorldItem createBeltArmor(int, RandomMachine&);

    WorldItem createWeapon(int, RandomMachine&);
    WorldItem createBallisticWeapon(int, RandomMachine&);
    WorldItem createBeamWeapon(int, RandomMachine&);

    WorldItem createMediKit(int quality, RandomMachine& random);
    WorldItem createAntiDepressant(int quality, RandomMachine& random);
    WorldItem createTorch(int quality, RandomMachine& random);
    WorldItem createSmallItem(int, RandomMachine&);
    WorldItem makeItem(int depth, int item_num, int worldTick);
};

#endif	/* ITEM_CREATOR_H */

