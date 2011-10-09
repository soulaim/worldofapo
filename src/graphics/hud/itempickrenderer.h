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
public:
    TextRenderer textRenderer;
    void draw(const Inventory&, const ItemPicker&);
};

#endif	/* ITEMPICKRENDERER_H */

