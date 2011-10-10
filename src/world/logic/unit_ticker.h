/*
 * File:   unit_ticker.h
 * Author: urtela
 *
 * Created on October 10, 2011, 4:57 PM
 */

#ifndef UNIT_TICKER_H
#define	UNIT_TICKER_H

class World;
class Unit;
class Model;

#include "unit_ai.h"

class UnitTicker {
    UnitAI unit_ai;
public:
    void tickUnit(World&, Unit&, Model*);
};

#endif	/* UNIT_TICKER_H */

