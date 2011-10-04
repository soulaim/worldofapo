
#include "world/objects/inventory.h"
#include "world/objects/unit.h"

#include <sstream>

Inventory::Inventory(Unit* u): unit(u), max_items(10), small_items_begin(7) {
    for(unsigned i=0; i<this->max_items; ++i) {
        this->wieldedItems[i] = 0;
    }
}

Inventory::~Inventory() {

}


WorldItem* Inventory::getItemActive() {
    return getItemSlot(this->active_item);
}

WorldItem* Inventory::getItemSlot(int slot_id) {
    return this->wieldedItems[slot_id];
}

bool Inventory::pickUp(WorldItem*) {
    // TODO
    return false;
}

void Inventory::dropItemCurrent(World& world) {
    dropItemSlot(world, this->active_item);
}

void Inventory::dropItemSlot(World&, int) {
    // TODO
}

void Inventory::dropAll(World& world) {
    for(unsigned i=0; i<this->max_items; ++i) {
        if(this->wieldedItems[i] != 0)
            dropItemSlot(world, i);
    }
}

int Inventory::getArmorClass() {
    // TODO

    return 0;
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
