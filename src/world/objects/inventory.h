/*
 * File:   inventory.h
 * Author: urtela
 *
 * Created on October 5, 2011, 12:15 AM
 */

#ifndef INVENTORY_H
#define	INVENTORY_H

#include "world/objects/world_item.h"
#include <string>
#include <sstream>

class World;
class Unit;

class Inventory {

private:
    Unit* unit;

    unsigned max_items;
    unsigned small_items_begin;

    WorldItem* wieldedItems[10];
    int active_item;
public:
    Inventory(Unit* u);
    ~Inventory();

    enum SlotName {
        HEAD_SLOT = 0,
        TORSO_SLOT = 1,
        ARMS_SLOT = 2,
        LEGS_SLOT = 3,
        AMULET_SLOT = 4,
        BELT_SLOT = 5,
        WEAPON_SLOT = 6
    };
    // additionally an inventory contains room for 3 small items, which can be
    // used, but do not offer passive protection or abilities.

    WorldItem* getItemActive(); // returns active item.
    WorldItem* getItemSlot(int slot_id); // returns item in a given slot.

    bool pickUp(WorldItem* item); // returns true if picked up. false otherwise.
    void dropItemCurrent(World&); // drop active item (how to drop armor?)
    void dropItemSlot(World&, int); // drop the item from a given slot.
    void dropAll(World&); // when a unit dies, it drops all items.
    int getArmorClass();

    void activateItem(int);
    void useActiveItemPrimary(World&);
    void useActiveItemSecondary(World&);

    std::string copyOrder() const;
	void handleCopyOrder(std::stringstream& ss);
};

#endif	/* INVENTORY_H */

