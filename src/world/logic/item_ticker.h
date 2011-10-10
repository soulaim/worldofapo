/*
 * File:   item_ticker.h
 * Author: urtela
 *
 * Created on October 10, 2011, 5:31 PM
 */

#ifndef ITEM_TICKER_H
#define	ITEM_TICKER_H

class World;
class WorldItem;
class Model;

class ItemTicker {
public:
    void tickItem(World&, WorldItem&, Model*);
};

#endif	/* ITEM_TICKER_H */

