
#include "world/objects/inventory.h"
#include "world/objects/unit.h"
#include "world/world.h"

#include <sstream>
#include <string>
#include <iostream>

Inventory::Inventory(): max_items(11), small_items_begin(8) {
    this->active_item = 6;
    for(unsigned i=0; i<this->max_items; ++i) {
        this->wieldedItems[i] = 0;
    }

    this->wieldedItems[6] = new WorldItem();
    this->wieldedItems[6]->load("data/items/ballistic_1.dat");
}

Inventory::~Inventory() {
}

void Inventory::reloadAction(World& world, Unit& unit) {
    WorldItem* item = this->wieldedItems[this->active_item];
    if(item == 0) {
        world.add_message("^yNo item is active, reload item ^rfailed");
    } else {
        item->onReload(world, unit);
    }
}

void Inventory::useActiveItemPrimary(World& world, Unit& unit) {
    WorldItem* item = this->wieldedItems[this->active_item];
    if(item == 0) {
        world.add_message("^yNo item is active, using item ^rfailed");
    } else {
        item->onActivate(world, unit);
    }
}

void Inventory::useActiveItemSecondary(World& world, Unit& unit) {
    WorldItem* item = this->wieldedItems[this->active_item];
    if(item == 0) {
        world.add_message("^yNo item is active, using item ^rfailed");
    } else {
        item->onSecondaryActivate(world, unit);
    }
}

void Inventory::setActiveItem(World& world, Unit&, int i) {
    i+=this->small_items_begin-2;

    if(i >= this->max_items) {
        world.add_message("Bad active item selection..");
        return;
    }

    std::stringstream ss; ss << "Selected item " << i;
    world.add_message(ss.str());

    this->active_item = i;
}

WorldItem* Inventory::getItemActive() const {
    return getItemSlot(this->active_item);
}

WorldItem* Inventory::getItemSlot(int slot_id) const {
    return this->wieldedItems[slot_id];
}

int Inventory::getArmorClass() {
    // TODO
    return 0;
}

int getSlot(WorldItem* item) {
    int itemType = item->intVals["TYPE"];
    if(itemType < 3)
        return 6;
    if(itemType >= 5 && itemType <= 10)
        return itemType - 5;
    return 8;
}

void removeItemFromWorld(World& world, WorldItem* item) {
     std::map<int, WorldItem>::iterator it = world.items.find(item->id);
     if(it != world.items.end()) {
         world.visualworld->removeUnit(item->id);
         world.items.erase(it);
     }
}

bool Inventory::pickUp(World& world, Unit& unit, WorldItem* item) {

    // REMEMBER TO TAKE A COPY OF THE ITEM.
    // delete item model from visual world!

    // Step one: Check if pickUp is possible (see which slot the item is going to occupy).
    if(item == 0)
        return false;

    unsigned slot = getSlot(item);

    // armor slot, easy
    if(slot < 6) {
        // if wearing something already, THROW IT ON THE GROUND!!11!
        if(this->wieldedItems[slot] != 0)
            this->dropItemSlot(world, unit, slot);

        WorldItem* itemCopy = new WorldItem();
        *itemCopy = *item; // copy must work!

        this->wieldedItems[slot] = itemCopy; // catch the reserved memory.

        removeItemFromWorld(world, item);
        return true;
    }

    // weapon item, a bit more difficult
    if(slot >= 6 && slot < this->small_items_begin) {
        // if there is a weapon slot that is empty, then place the item there.
        if(this->wieldedItems[6] == 0) {
            WorldItem* itemCopy = new WorldItem();
            *itemCopy = *item;
            this->wieldedItems[6] = itemCopy;

            removeItemFromWorld(world, item);
            return true;
        }

        if(this->wieldedItems[7] == 0) {
            WorldItem* itemCopy = new WorldItem();
            *itemCopy = *item;
            this->wieldedItems[7] = itemCopy;

            removeItemFromWorld(world, item);
            return true;
        }


        // if a weapon is selected, then replace that weapon
        if(this->active_item >= 6u && this->active_item < this->small_items_begin) {
            this->dropItemSlot(world, unit, (int)this->active_item);
            WorldItem* itemCopy = new WorldItem();
            *itemCopy = *item;
            this->wieldedItems[this->active_item] = itemCopy;

            removeItemFromWorld(world, item);
            return true;
        }

        // replace first weapon item
        this->dropItemSlot(world, unit, 6);
        WorldItem* itemCopy = new WorldItem();
        *itemCopy = *item;
        this->wieldedItems[6] = itemCopy;

        removeItemFromWorld(world, item);
        return true;
    }


    // item is either a small item or something the player can't pick up.
    world.add_message("TODO: Handle picking up small items.");


    // finally, if could not handle item, simply return false.
    return false;
}

void Inventory::dropItemCurrent(World& world, Unit& unit) {
    dropItemSlot(world, unit, this->active_item);
}

void Inventory::dropItemSlot(World& world, Unit&, int i) {
    std::stringstream ss;
    ss << "Deleted item at slot: " << i;

    world.add_message(ss.str());
    world.add_message("TODO: place the dropped item into world.");

    delete this->wieldedItems[i];
    this->wieldedItems[i] = 0;
}

void Inventory::dropAll(World& world, Unit& unit) {
    for(unsigned i=0; i<this->max_items; ++i) {
        if(this->wieldedItems[i] != 0)
            dropItemSlot(world, unit, i);
    }
}

std::string Inventory::copyOrder() const {
    std::stringstream ss;
    for(unsigned i=0; i<this->max_items; ++i) {
        if(this->wieldedItems[i] != 0)
            ss << " " << i << " " << this->wieldedItems[i]->inventoryCopy();
    }
    ss << " 11 ";
    return ss.str();
}

void Inventory::handleCopyOrder(std::stringstream& ss) {
    int index;
    while(true) {
        ss >> index;
        if(index == 11)
            break;

        this->wieldedItems[index] = new WorldItem();
        this->wieldedItems[index]->handleInventoryCopy(ss);
    }
}
