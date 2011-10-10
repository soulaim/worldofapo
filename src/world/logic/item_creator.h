/*
 * File:   item_creator.h
 * Author: urtela
 *
 * Created on October 10, 2011, 6:20 PM
 */

#ifndef ITEM_CREATOR_H
#define	ITEM_CREATOR_H

#include "world/objects/world_item.h"

class ItemCreator {
    WorldItem createAmulet(int, int, int);
    WorldItem createArmsArmor(int, int, int);
    WorldItem createTorsoArmor(int, int, int);
    WorldItem createHeadArmor(int, int, int);
    WorldItem createLegArmor(int, int, int);
    WorldItem createBeltArmor(int, int, int);

    WorldItem createWeapon(int, int, int);
    WorldItem createBallisticWeapon(int, int, int);
    WorldItem createBeamWeapon(int, int, int);

    WorldItem createSmallItem(int, int, int);

public:
    WorldItem makeItem(int depth, int item_num, int worldTick);
};

#endif	/* ITEM_CREATOR_H */

