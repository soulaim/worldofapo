
#include "world/logic/item_creator.h"
#include "world/objects/inventory.h"
#include "world/objects/unit.h"
#include "world/world.h"

#include <sstream>
#include <string>
#include <iostream>

Inventory::Inventory(): max_items(11), small_items_begin(8) {
    this->last_pickup_time = 0;
    this->active_item = 6;
    for(unsigned i=0; i<this->max_items; ++i) {
        this->wieldedItems[i] = 0;
    }
}

int Inventory::getArmorClass() const {
    int ac = 0;
    for(unsigned i=0; i<6; ++i) {
        if(this->wieldedItems[i] == 0)
            continue;
        ac += this->wieldedItems[i]->intVals["AC"];
    }
    return ac;
}

Inventory::Inventory(const Inventory& inventory): max_items(11), small_items_begin(8) {
    for(unsigned i=0; i<this->max_items; ++i) {
        this->wieldedItems[i] = 0;
    }
    *this = inventory;
}

Inventory& Inventory::operator = (const Inventory& inventory) {
    for(unsigned i=0; i<this->max_items; ++i) {
        WorldItem* item = inventory.getItemSlot(i);

        if(item == 0)
            continue;

        this->wieldedItems[i] = new WorldItem();
        *(this->wieldedItems[i]) = *item;
    }
    this->active_item = inventory.active_item;
    this->last_pickup_time = inventory.last_pickup_time;
    return *this;
}

Inventory::~Inventory() {
    for(unsigned i=0; i<this->max_items; ++i) {
        if(this->wieldedItems[i] == 0)
            continue;

        delete this->wieldedItems[i];
        this->wieldedItems[i] = 0;
    }
}

void Inventory::destroyItem(WorldItem* item) {
    for(unsigned i=0; i<this->max_items; ++i) {
        WorldItem* myItem = getItemSlot(i);
        if(item == myItem) {
            delete myItem;
            this->wieldedItems[i] = 0;
        }
    }
}

void Inventory::reloadAction(World& world, Unit& unit) {
    WorldItem* item = this->wieldedItems[this->active_item];
    if(item == 0) {
    } else {
        item->onReload(world, unit);
    }
}

void Inventory::useActiveItemPrimary(World& world, Unit& unit) {
    if(unit["HEALTH"] > 0) {
        WorldItem* item = this->wieldedItems[this->active_item];
        if(item == 0) {
        } else {
            item->onActivate(world, unit);
        }
    }
}

void Inventory::useActiveItemSecondary(World& world, Unit& unit) {
    WorldItem* item = this->wieldedItems[this->active_item];
    if(item == 0) {
    } else {
        item->onSecondaryActivate(world, unit);
    }
}

void Inventory::setActiveItem(World& world, Unit&, unsigned i) {
    i+=this->small_items_begin-2;

    if(i >= this->max_items) {
        world.add_message("Bad active item selection..");
        return;
    }

    this->active_item = i;
}

WorldItem* Inventory::getItemActive() const {
    return getItemSlot(this->active_item);
}

WorldItem* Inventory::getItemSlot(int slot_id) const {
    return this->wieldedItems[slot_id];
}

int getSlot(WorldItem* item) {

    int itemType = item->intVals["TYPE"];

    if(itemType == 11)
        return 9; // operator items

    if(itemType < 3)
        return 6;
    if(itemType >= 5 && itemType <= 10) {
        int suggested_type = itemType - 5;
        if(suggested_type == 2)
            return 5;
        else if(suggested_type == 5)
            return 4;
        else if(suggested_type == 4)
            return 2;
        return suggested_type;
    }

    return 8; // small items


}

void removeItemFromWorld(World&, WorldItem* item) {
    item->dead = 1;
}

void Inventory::pickUpHelper(World& world, Unit& unit, WorldItem* item, int slot) {
    WorldItem* itemCopy = new WorldItem();
    *itemCopy = *item;
    this->wieldedItems[slot] = itemCopy;

    if(unit.id == world.getLocalPlayerID())
        world.add_message("^YPicked up ^G" + item->strVals["NAME"]);

    removeItemFromWorld(world, item);
    unit.itemPick.reset();
}

void Inventory::clear() {
    this->last_pickup_time = 0;
    this->active_item = 6;
    for(unsigned i=0; i<this->max_items; ++i) {
        if(this->wieldedItems[i] != 0)
            delete this->wieldedItems[i];
        this->wieldedItems[i] = 0;
    }
}

bool Inventory::pickUp(World& world, Unit& unit, WorldItem* item, bool forced) {

    assert(item != 0 && "Picking up an item failed: Nullpointer!");

    if(!forced) {
        if(last_pickup_time + 13 > world.currentWorldFrame) return false;
        last_pickup_time = world.currentWorldFrame;
    }

    unsigned slot = getSlot(item);


    // armor slot, easy
    if(slot < 6) {
        // if wearing something already, THROW IT ON THE GROUND!!11!
        if(this->wieldedItems[slot] != 0)
            this->dropItemSlot(world, unit, slot);
        this->pickUpHelper(world, unit, item, slot);
        return true;
    }

    // weapon item, a bit more difficult
    if(slot >= 6 && slot < this->small_items_begin) {
        // if there is a weapon slot that is empty, then place the item there.
        if(this->wieldedItems[6] == 0) {
            this->pickUpHelper(world, unit, item, 6);
            return true;
        }

        if(this->wieldedItems[7] == 0) {
            this->pickUpHelper(world, unit, item, 7);
            return true;
        }


        // if a weapon is selected, then replace that weapon
        if(this->active_item >= 6u && this->active_item < this->small_items_begin) {
            this->dropItemSlot(world, unit, (int)this->active_item);
            this->pickUpHelper(world, unit, item, active_item);
            return true;
        }

        // replace first weapon item
        this->dropItemSlot(world, unit, 6);
        this->pickUpHelper(world, unit, item, 6);
        return true;
    }


    // small item
    if(slot == 8) {
        for(unsigned i=this->small_items_begin; i<this->max_items; ++i) {
            if(this->wieldedItems[i] == 0) {
                this->pickUpHelper(world, unit, item, i);
                return true;
            }
        }

        if(this->active_item >= this->small_items_begin) {
            if(this->wieldedItems[active_item] != 0) {
                this->dropItemSlot(world, unit, active_item);
            }
            this->pickUpHelper(world, unit, item, active_item);
            return true;
        }

        // world.add_message("^YInventory full! Make room in the inventory first.");
        return false;
    }

    if(slot == 9) {
        world.add_message("^RWARNING: ^YOperator pick-up not implemented.");
    }
    else {
        world.add_message("^RWARNING: ^YUnrecognized item type. Interaction not allowed.");
    }

    return false;
}

void Inventory::dropItemCurrent(World& world, Unit& unit) {
    dropItemSlot(world, unit, this->active_item);
}

void Inventory::dropItemSlot(World& world, Unit& unit, int i) {
    if(this->wieldedItems[i] == 0)
        return;

    RandomMachine random;
    random.setSeed(unit.id);
    FixedPoint x = FixedPoint( (random.getInt() % 10) - 5, 10 );
    FixedPoint z = FixedPoint( (random.getInt() % 10) - 5, 10 );
    FixedPoint y = FixedPoint( (random.getInt() % 5)  + 1, 10 );

    int id = world.nextUnitID();
    world.addItem(*(this->wieldedItems[i]), VisualWorld::ModelType(this->wieldedItems[i]->intVals["MODEL_TYPE"]), id);
    world.items[id].velocity = Location(x, y, z);
    world.items[id].position = unit.getEyePosition();

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
