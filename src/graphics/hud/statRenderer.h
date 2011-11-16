/*
 * File:   statRenderer.h
 * Author: urtela
 *
 * Created on October 21, 2011, 3:32 PM
 */

#ifndef STATRENDERER_H
#define	STATRENDERER_H

#include "graphics/hud/textRenderer.h"
#include "world/logic/stats_names.h"

#include <string>
#include <vector>

class Unit;

class StatRenderer {

    float cursor_x;
    float cursor_y;

    int mouseButtons;
    int selected;

    StatsNames statsNames;
    TextRenderer textRenderer;
public:
    StatRenderer();
    void drawStatExplanation();
    void drawStatSheet(Unit&);
    int input(int dx, int dy, int mousePress, int keyState);
};


#endif	/* STATRENDERER_H */

