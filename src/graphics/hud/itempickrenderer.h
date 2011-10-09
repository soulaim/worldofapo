/*
 * File:   itempickrenderer.h
 * Author: urtela
 *
 * Created on October 9, 2011, 4:48 PM
 */

#ifndef ITEMPICKRENDERER_H
#define	ITEMPICKRENDERER_H

class Inventory;
class ItemPicker;

class ItemPickRenderer {
public:
    void draw(const Inventory&, const ItemPicker&);
};

#endif	/* ITEMPICKRENDERER_H */

