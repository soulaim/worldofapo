/*
 * File:   inventoryrenderer.h
 * Author: urtela
 *
 * Created on October 6, 2011, 4:33 PM
 */

#ifndef INVENTORYRENDERER_H
#define	INVENTORYRENDERER_H

class Inventory;

class InventoryRenderer {
private:
    float x_pos, y_pos;
    void drawItem(const Inventory&, int slot, float dx, float dy, float hw, float hh);

public:
    InventoryRenderer();
    void draw(const Inventory& inventory);
};

#endif	/* INVENTORYRENDERER_H */

