/*
 * File:   itempickrenderer.h
 * Author: urtela
 *
 * Created on October 9, 2011, 4:48 PM
 */

#ifndef ITEMPICKRENDERER_H
#define	ITEMPICKRENDERER_H

#include "graphics/hud/textRenderer.h"

class Inventory;
class ItemPicker;

class ItemPickRenderer {
    float alpha;
public:
    ItemPickRenderer(): alpha(0) {}
    TextRenderer textRenderer;
    void draw(const Inventory&, const ItemPicker&);
};

#endif	/* ITEMPICKRENDERER_H */

