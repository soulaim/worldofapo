
#include "world/objects/items/armor.h"
#include "world/objects/world_item.h"

#include <sstream>

void ArmorItemUsage::getDetails(WorldItem* item, std::vector<std::string>& details) {
    std::stringstream ss_name;
    ss_name << "^G" << item->strVals["NAME"];

    std::stringstream ss_ac;
    ss_ac << "^YArmor Class: ^R" << item->intVals["AC"];

    details.push_back(ss_name.str());
    details.push_back(ss_ac.str());
}
