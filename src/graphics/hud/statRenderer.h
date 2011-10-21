/*
 * File:   statRenderer.h
 * Author: urtela
 *
 * Created on October 21, 2011, 3:32 PM
 */

#ifndef STATRENDERER_H
#define	STATRENDERER_H

#include "graphics/hud/textRenderer.h"

#include <string>
#include <vector>

class Unit;

class StatRenderer {
    std::vector<std::string> names;
    std::vector<std::string> keys;

    TextRenderer textRenderer;
public:
    StatRenderer();
    void drawStatSheet(Unit&);
};


#endif	/* STATRENDERER_H */

