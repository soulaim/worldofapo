
#include "world/objects/inventory.h"
#include "world/objects/unit.h"
#include "world/world.h"

#include <sstream>
#include <string>
#include <iostream>

Inventory::Inventory(): max_items(10), small_items_begin(7) {
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
    i+=this->small_items_begin-1;

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

bool Inventory::pickUp(WorldItem*) {
    // TODO:
    // delete item model from visual world.
    // remove item from world.
    // add item to inventory.
    return false;
}

void Inventory::dropItemCurrent(World& world) {
    dropItemSlot(world, this->active_item);
}

void Inventory::dropItemSlot(World&, int i) {
    delete this->wieldedItems[i];
    this->wieldedItems[i] = 0;
}

void Inventory::dropAll(World& world) {
    for(unsigned i=0; i<this->max_items; ++i) {
        if(this->wieldedItems[i] != 0)
            dropItemSlot(world, i);
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

    std::cerr << "handle invetory copy: " << std::endl;

    int index;
    while(true) {
        ss >> index;
        if(index == 11)
            break;

        this->wieldedItems[index] = new WorldItem();

        std::cerr << "handle inventory item copy: " << std::endl;
        this->wieldedItems[index]->handleInventoryCopy(ss);
    }
}
