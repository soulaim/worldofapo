/*
 * File:   barRenderer.h
 * Author: urtela
 *
 * Created on October 9, 2011, 5:12 PM
 */

#ifndef BARRENDERER_H
#define	BARRENDERER_H

#include <string>
#include "misc/vec3.h"

class BarRenderer {
public:
    void draw3DBar(float percentage, const vec3<float>& pos, float x_angle, float y_angle, const std::string& start_color, const std::string& end_color, float scale) const;
    void drawBar(float size, const std::string& start_color, const std::string& end_color, float min_x, float max_x, float min_y, float max_y) const;
};

#endif	/* BARRENDERER_H */

