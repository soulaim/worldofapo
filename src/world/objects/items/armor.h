/*
 * File:   armor.h
 * Author: urtela
 *
 * Created on October 9, 2011, 7:17 PM
 */

#ifndef ARMOR_H
#define	ARMOR_H

#include <vector>
#include <string>

class WorldItem;

class ArmorItemUsage {
public:
    void getDetails(WorldItem*, std::vector<std::string>&);
};

#endif	/* ARMOR_H */

