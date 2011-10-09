
#include "world/objects/itempicker.h"
#include "world/objects/world_item.h"

ItemPicker::ItemPicker() {
    reset();
}

void ItemPicker::pushItem(WorldItem& item, FixedPoint dist) {
    if(dist < distance) {
        distance = dist;
        this->nearest = &item;
    }
}

void ItemPicker::reset() {
    distance = 1000;
    this->nearest = 0;
}

WorldItem* ItemPicker::get() {
    return nearest;
}