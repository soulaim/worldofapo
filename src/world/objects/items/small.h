/*
 * File:   small.h
 * Author: urtela
 *
 * Created on October 10, 2011, 9:59 PM
 */

#ifndef SMALL_H
#define	SMALL_H

#include <string>
#include <vector>

class WorldItem;

class SmallItemUsage {
public:
    void getDetails(WorldItem*, std::vector<std::string>&);
};

#endif	/* SMALL_H */

