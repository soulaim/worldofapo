
#include "world/objects/items/small.h"
#include "world/objects/world_item.h"
#include "world/world.h"

#include <sstream>
#include <map>

using std::stringstream;
using std::string;

void SmallItemUsage::applyEffect(WorldItem* item, World& world, Unit& unit, const string effect_id) {

    world.add_message("Applying effect");

    string effect = item->strVals[effect_id];
    int value     = item->intVals[effect_id];

    // this handles sanity buffs, health buffs, etc.
    auto it = unit.intVals.find(effect);
    if(it != unit.intVals.end()) {
        it->second += value;
    }

    // special case for lights
    if(effect == "LIGHT") {
        world.visualworld->activateTorch(unit, value);
    }

    // other non-trivial items

}

void SmallItemUsage::tick(WorldItem* item, Unit&) {
    if(--item->intVals["CD"] < 0)
        item->intVals["CD"] = 0;
}


void SmallItemUsage::onActivatePrimary(WorldItem* item, World& world, Unit& unit) {

    if(item->intVals["CD"] > 0)
        return;

    world.add_message("Activating " + item->strVals["NAME"]);
    item->intVals["CD"] = 25;

    int effect_num = 1;
    while(true) {
        stringstream effect; effect << "EFFECT" << effect_num;
        if(item->strVals.find(effect.str()) != item->strVals.end())
            this->applyEffect(item, world, unit, effect.str());
        else
            break;
        ++effect_num;
    }

    item->intVals["AMOUNT"] -= 1;
    if(item->intVals["AMOUNT"] <= 0) {
        unit.destroyInventoryItem(item);
        return;
    }
}


void SmallItemUsage::getDetails(WorldItem* item, std::vector<std::string>& details) {
    stringstream ss_name;
    ss_name << "^YItem: ^G" << item->strVals["NAME"];
    details.push_back(ss_name.str());

    stringstream quantity;
    quantity << "^YQuantity: ^G" << item->intVals["AMOUNT"];
    details.push_back(quantity.str());

    int effect_num = 1;
    while(1)
    {
        stringstream id; id << "EFFECT" << effect_num;
        if(item->strVals.find(id.str()) != item->strVals.end()) {
            stringstream ss_effect;
            ss_effect << "^YEffect: ^G(" << item->strVals[id.str()] << ") +" << item->intVals[id.str()];
            details.push_back(ss_effect.str());
            ++effect_num;
        }
        else
            break;
    }
}
