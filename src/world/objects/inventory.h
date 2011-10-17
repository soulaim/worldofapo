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

    friend class InventoryRenderer;

private:
    unsigned max_items;
    unsigned small_items_begin;

    WorldItem* wieldedItems[11];
    unsigned active_item;
public:
    Inventory();
    ~Inventory();

    Inventory(const Inventory& inventory);
    Inventory& operator = (const Inventory& inventory);

    enum SlotName {
        HEAD_SLOT = 0,
        TORSO_SLOT = 1,
        ARMS_SLOT = 2,
        LEGS_SLOT = 3,
        AMULET_SLOT = 4,
        BELT_SLOT = 5,
        WEAPON_SLOT1 = 6,
        WEAPON_SLOT2 = 7
    };

    // additionally an inventory contains room for 3 small items, which can be
    // used, but do not offer passive protection or abilities.

    WorldItem* getItemActive() const; // returns active item.
    WorldItem* getItemSlot(int slot_id) const; // returns item in a given slot.

    void dropItemCurrent(World&, Unit&); // drop active item (how to drop armor?)
    void dropItemSlot(World&, Unit&, int); // drop the item from a given slot.
    void dropAll(World&, Unit&); // when a unit dies, it drops all items.
    int getArmorClass();

    void pickUpHelper(World&, Unit&, WorldItem*, int);
    bool pickUp(World&, Unit&, WorldItem*); // returns true if picked up. false otherwise.

    void setActiveItem(World&, Unit&, unsigned);
    void reloadAction(World&, Unit&);
    void useActiveItemPrimary(World&, Unit&);
    void useActiveItemSecondary(World&, Unit&);

    std::string copyOrder() const;
	void handleCopyOrder(std::stringstream& ss);
};

#endif	/* INVENTORY_H */

