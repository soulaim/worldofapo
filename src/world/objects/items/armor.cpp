
#include "world/objects/items/armor.h"
#include "world/objects/world_item.h"

#include <sstream>

void ArmorItemUsage::getDetails(WorldItem* item, std::vector<std::string>& details) {

    std::stringstream ss_name;
    ss_name << item->strVals["NAME"];

    std::stringstream ss_ac;
    ss_ac << "Armor Class: " << item->intVals["ARMOR"];

    details.push_back(ss_name.str());
    details.push_back(ss_ac.str());
    
}