
#include "world/objects/items/small.h"
#include "world/objects/world_item.h"

#include <sstream>

using std::stringstream;

void SmallItemUsage::getDetails(WorldItem* item, std::vector<std::string>& details) {
    stringstream ss_name;
    ss_name << "^YItem: ^G" << item->strVals["NAME"];

    details.push_back(ss_name.str());

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
