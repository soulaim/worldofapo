
#include "world/objects/inventory.h"
#include "world/objects/unit.h"
#include "world/world.h"

#include <sstream>

Inventory::Inventory(Unit* u): unit(u), max_items(10), small_items_begin(7) {
    this->active_item = 6;
    for(unsigned i=0; i<this->max_items; ++i) {
        this->wieldedItems[i] = 0;
    }
}

Inventory::~Inventory() {
}

void Inventory::useActiveItemPrimary(World& world) {
    WorldItem* item = this->wieldedItems[this->active_item];
    if(item == 0) {
        world.add_message("^yNo item is active, using item ^rfailed");
    } else {
        item->onActivate(world, *unit);
    }
}

void Inventory::useActiveItemSecondary(World& world) {
    WorldItem* item = this->wieldedItems[this->active_item];
    if(item == 0) {
        world.add_message("^yNo item is active, using item ^rfailed");
    } else {
        item->onSecondaryActivate(world, *unit);
    }
}

void Inventory::activateItem(int i) {
    i+=this->small_items_begin-1;
    this->active_item = i;
}

WorldItem* Inventory::getItemActive() {
    return getItemSlot(this->active_item);
}

WorldItem* Inventory::getItemSlot(int slot_id) {
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
            ss << i << " " << this->wieldedItems[i]->copyOrder(wieldedItems[i]->id) << " ";
    }
    ss << "11 ";
    return ss.str();
}

void Inventory::handleCopyOrder(std::stringstream& ss) {
    int index;
    while(true) {
        ss >> index;
        if(index == 11)
            break;

        this->wieldedItems[index] = new WorldItem();
        this->wieldedItems[index]->handleCopyOrder(ss);
    }
}
