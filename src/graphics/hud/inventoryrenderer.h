/*
 * File:   inventoryrenderer.h
 * Author: urtela
 *
 * Created on October 6, 2011, 4:33 PM
 */

#ifndef INVENTORYRENDERER_H
#define	INVENTORYRENDERER_H

#include "graphics/hud/textRenderer.h"

class Inventory;

class InventoryRenderer {
private:
    TextRenderer textRenderer;
    float x_pos, y_pos;
    
    void drawArmorItem(const Inventory&, int slot, float dx, float dy, float hw, float hh, int tex_x = 0);
    void drawWeaponItem(const Inventory& inventory, int slot, int itemType);

public:
    InventoryRenderer();
    void draw(const Inventory& inventory);
};

#endif	/* INVENTORYRENDERER_H */

