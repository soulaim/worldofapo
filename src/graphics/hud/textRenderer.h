/*
 * File:   textRenderer.h
 * Author: urtela
 *
 * Created on October 9, 2011, 5:05 PM
 */

#ifndef TEXTRENDERER_H
#define	TEXTRENDERER_H

#include <string>
#include "misc/vec3.h"

class TextRenderer {
public:
    int draw3Dstring(const std::string& message, const vec3<float>& pos, float x_angle, float y_angle, int team = 0) const;
    int drawString(const std::string& msg, float pos_x, float pos_y, float scale, bool background = false, float alpha = 1.0f) const;
};

#endif	/* TEXTRENDERER_H */

