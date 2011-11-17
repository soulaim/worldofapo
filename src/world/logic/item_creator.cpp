
#include "world/logic/item_creator.h"
#include "world/random.h"
#include "world/objects/inventory.h"
#include "world/logic/damagetype.h"
#include "world/objects/unit.h"


int getQualityFromDepth(int depth, RandomMachine& random) {
    int max_quality = 1 + depth / 3;
    for(int i=0; i<max_quality; ++i)
        if(random.getInt() & 3)
            --max_quality;

    int quality = max_quality;
    if(quality > 7)
        quality = 7;

    return quality;
}

void ItemCreator::fillWithItems(World& world, Unit& unit, int depth, Inventory& inventory, RandomMachine& random) {
    {
        int weaponQuality = getQualityFromDepth(depth, random);
        if(weaponQuality < 1) weaponQuality = 1;
        WorldItem item = this->createBallisticWeapon(weaponQuality, random);
        inventory.pickUp(world, unit, &item, true);
    }

    int torso = getQualityFromDepth(depth, random);
    if(torso) {
        WorldItem item = this->createTorsoArmor(torso, random);
        inventory.pickUp(world, unit, &item, true);
    }

    int arms = getQualityFromDepth(depth, random);
    if(arms) {
        WorldItem item = this->createArmsArmor(arms, random);
        inventory.pickUp(world, unit, &item, true);
    }

    int head = getQualityFromDepth(depth, random);
    if(head) {
        WorldItem item = this->createHeadArmor(head, random);
        inventory.pickUp(world, unit, &item, true);
    }

    int belt = getQualityFromDepth(depth, random);
    if(belt) {
        WorldItem item = this->createBeltArmor(belt, random);
        inventory.pickUp(world, unit, &item, true);
    }

    int legs = getQualityFromDepth(depth, random);
    if(legs) {
        WorldItem item = this->createLegArmor(legs, random);
        inventory.pickUp(world, unit, &item, true);
    }

    int pendant = getQualityFromDepth(depth, random);
    if(pendant) {
        WorldItem item = this->createAmulet(pendant, random);
        inventory.pickUp(world, unit, &item, true);
    }

    for(int i=0; i<3; ++i) {
        int small = getQualityFromDepth(depth, random);
        if(!small) break;

        WorldItem item = this->createSmallItem(small, random);
        inventory.pickUp(world, unit, &item, true);
    }
}


WorldItem ItemCreator::makeItem(int depth, int item_num, int worldTick) {
    RandomMachine random;
    random.setSeed(item_num + worldTick);

    int quality = getQualityFromDepth(depth, random);
    if(quality == 0) quality = 1;

    int major_type = random.getInt() & 15;

    switch(major_type) {
        case Inventory::AMULET_SLOT:
            return createAmulet(quality, random);
        case Inventory::ARMS_SLOT:
            return createArmsArmor(quality, random);
        case Inventory::BELT_SLOT:
            return createBeltArmor(quality, random);
        case Inventory::HEAD_SLOT:
            return createHeadArmor(quality, random);
        case Inventory::LEGS_SLOT:
            return createLegArmor(quality, random);
        case Inventory::TORSO_SLOT:
            return createTorsoArmor(quality, random);
        case Inventory::WEAPON_SLOT1:
            return createWeapon(quality, random);
        default:
            return createSmallItem(quality, random);
    }
}


WorldItem ItemCreator::createAmulet(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.strVals["NAME"] = "Power\\sAmulet";
    item.intVals["TYPE"] = 10;
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createArmsArmor(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 9;
    item.strVals["NAME"] = "Arm\\sprotectors";
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createTorsoArmor(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 6;
    item.strVals["NAME"] = "Power\\sVest";
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createHeadArmor(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 5;
    item.strVals["NAME"] = "Cap";
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createLegArmor(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 8;
    item.strVals["NAME"] = "Pants";
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createBeltArmor(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = quality;
    item.intVals["AC"] = quality + random.getInt() % quality;
    item.intVals["TYPE"] = 7;
    item.strVals["NAME"] = "Belt";
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createWeapon(int quality, RandomMachine& random) {
    if(random.getInt() & 1)
        return createBallisticWeapon(quality, random);
    return createBeamWeapon(quality, random);
}

WorldItem ItemCreator::createBallisticWeapon(int quality, RandomMachine& randomer) {
    quality += 3;
    WorldItem item;
    item.intVals["DAMAGE_TYPE"] = DamageType::PHYSICAL;
    item.intVals["TYPE"] = 1;
    item.intVals["ITEM_LVL"] = quality - 3;
    item.intVals["DAMAGE"] = (randomer.getInt() % quality) * quality + quality;
    item.intVals["BPS"] = 1 + randomer.getInt() % (quality - 2);
    item.intVals["CLIPSIZE"] = 3 + quality / item.intVals["BPS"];
    item.intVals["COOLDOWN"] = (15 - quality) * item.intVals["BPS"];
    item.intVals["RELOAD_TIME"] = (17 - quality) * 10;
    item.strVals["NAME"] = "Hand\\sCannon";
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::WEAPON_MODEL;
    return item;
}

WorldItem ItemCreator::createBeamWeapon(int quality, RandomMachine& random) {

    // TODO
    // item.intVals["TYPE"] = 0;

    ++quality;
    WorldItem item = this->createBallisticWeapon(quality, random);
    return item;

}

WorldItem ItemCreator::createTorch(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = 3;
    item.intVals["TYPE"] = 4;
    item.strVals["NAME"] = "Torch";
    item.strVals["EFFECT1"] = "LIGHT";
    item.intVals["EFFECT1"] = 2 + (random.getInt() & 7);
    item.intVals["AMOUNT"] = quality * 2;
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createMediKit(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = 4;
    item.intVals["TYPE"] = 4;
    item.strVals["NAME"] = "RegeDrug";
    item.strVals["EFFECT1"] = "HEALTH";
    item.intVals["EFFECT1"] = 10 + 4 * (random.getInt() % quality);
    item.intVals["AMOUNT"] = quality * 2;
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createAntiDepressant(int quality, RandomMachine& random) {
    WorldItem item;
    item.intVals["ITEM_LVL"] = 2;
    item.intVals["TYPE"] = 4;
    item.strVals["NAME"] = "Antidepressant";
    item.strVals["EFFECT1"] = "SANITY";
    item.intVals["EFFECT1"] = 10 + random.getInt() % 20;
    item.intVals["AMOUNT"] = quality * 2 + (random.getInt() % (quality * 2));
    item.intVals["MODEL_TYPE"] = VisualWorld::ModelType::ITEM_MODEL;
    return item;
}

WorldItem ItemCreator::createSmallItem(int quality, RandomMachine& random) {
    WorldItem item;

    int val = random.getInt() & 127;
    if(val < 40) {
        return createAntiDepressant(quality, random);
    }
    else if(val < 80) {
        return createMediKit(quality, random);
    }
    else {
        return createTorch(quality, random);
    }
}
