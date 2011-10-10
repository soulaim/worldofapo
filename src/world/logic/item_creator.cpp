
#include "world/logic/item_creator.h"
#include "world/random.h"
#include "world/objects/inventory.h"

WorldItem ItemCreator::makeItem(int depth, int item_num, int worldTick) {
    RandomMachine random;

    int max_quality = 1 + depth / 3;
    for(int i=0; i<max_quality; ++i)
        if(random.getInt() & 3)
            --max_quality;

    int quality = 1 + max_quality;
    if(quality > 7)
        quality = 7;
    
    int major_type = (item_num + worldTick) % 8;

    switch(major_type) {
        case Inventory::AMULET_SLOT:
            return createAmulet(quality, item_num, worldTick);
        case Inventory::ARMS_SLOT:
            return createArmsArmor(quality, item_num, worldTick);
        case Inventory::BELT_SLOT:
            return createBeltArmor(quality, item_num, worldTick);
        case Inventory::HEAD_SLOT:
            return createHeadArmor(quality, item_num, worldTick);
        case Inventory::LEGS_SLOT:
            return createLegArmor(quality, item_num, worldTick);
        case Inventory::TORSO_SLOT:
            return createTorsoArmor(quality, item_num, worldTick);
        case Inventory::WEAPON_SLOT1:
            return createWeapon(quality, item_num, worldTick);
        default:
            return createSmallItem(quality, item_num, worldTick);
    }
}


WorldItem ItemCreator::createAmulet(int quality, int num, int worldTick) {
    RandomMachine random; random.setSeed(num + worldTick);
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 10;
    item.strVals["NAME"] = "Power\\sAmulet";
    return item;
}

WorldItem ItemCreator::createArmsArmor(int quality, int num, int worldTick) {
    RandomMachine random; random.setSeed(num + worldTick);
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 9;
    item.strVals["NAME"] = "Arm\\sprotectors";
    return item;
}

WorldItem ItemCreator::createTorsoArmor(int quality, int num, int worldTick) {
    RandomMachine random; random.setSeed(num + worldTick);
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 6;
    item.strVals["NAME"] = "Power\\sVest";
    return item;
}

WorldItem ItemCreator::createHeadArmor(int quality, int num, int worldTick) {
    RandomMachine random; random.setSeed(num + worldTick);
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 5;
    item.strVals["NAME"] = "Cap";
    return item;
}

WorldItem ItemCreator::createLegArmor(int quality, int num, int worldTick) {
    RandomMachine random; random.setSeed(num + worldTick);
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 8;
    item.strVals["NAME"] = "Pants";
    return item;
}

WorldItem ItemCreator::createBeltArmor(int quality, int num, int worldTick) {
    RandomMachine random; random.setSeed(num + worldTick);
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 7;
    item.strVals["NAME"] = "Belt";
    return item;
}

WorldItem ItemCreator::createWeapon(int quality, int num, int worldTick) {
    RandomMachine randomer;
    randomer.setSeed(num + worldTick);
    if(randomer.getInt() & 1)
        return createBallisticWeapon(quality, num, worldTick);
    return createBeamWeapon(quality, num, worldTick);
}

WorldItem ItemCreator::createBallisticWeapon(int quality, int num, int worldTick) {
    quality += 3;
    RandomMachine randomer;
    randomer.setSeed(num + worldTick);
    WorldItem item; item.load("data/items/ballistic_1.dat");
    item.intVals["ITEM_LVL"] = quality - 3;
    item.intVals["DAMAGE"] = (randomer.getInt() % quality) * quality + quality;
    item.intVals["BPS"] = 1 + randomer.getInt() % (quality - 2);
    item.intVals["CLIPSIZE"] = 3 + quality / item.intVals["BPS"];
    item.intVals["COOLDOWN"] = (15 - quality) * item.intVals["BPS"];
    item.intVals["RELOAD_TIME"] = (17 - quality) * 10;
    item.strVals["NAME"] = "Hand\\sCannon";
    return item;
}

WorldItem ItemCreator::createBeamWeapon(int quality, int num, int worldTick) {

    // TODO
    ++quality;
    RandomMachine randomer;
    randomer.setSeed(num + worldTick);

    WorldItem item = this->createBallisticWeapon(quality, num, worldTick);
    return item;

}

WorldItem ItemCreator::createSmallItem(int quality, int num, int worldTick) {
    RandomMachine random; random.setSeed(num + worldTick);
    WorldItem item;

    // for small items, the level marker is not used to show quality, but type.
    item.intVals["ITEM_LVL"] = 0;
    item.intVals["TYPE"] = 4;
    item.strVals["NAME"] = "Antidepressant";
    item.strVals["EFFECT1"] = "SANITY";
    item.intVals["EFFECT1"] = 20; // increases sanity by 20 when consumed.
    item.intVals["AMOUNT"] = quality * 2; // increases sanity by 20 when consumed.

    return item;
}
