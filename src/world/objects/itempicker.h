/*
 * File:   itempicker.h
 * Author: urtela
 *
 * Created on October 9, 2011, 3:54 AM
 */

#ifndef ITEMPICKER_H
#define	ITEMPICKER_H

#include "misc/fixed_point.h"

class WorldItem;

class ItemPicker {
public:

    ItemPicker();

    void reset();
    void pushItem(WorldItem& item, FixedPoint dist);
    WorldItem* get();

    FixedPoint distance;
    WorldItem* nearest;
};

#endif	/* ITEMPICKER_H */

