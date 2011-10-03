/*
 * File:   roof_renderer.h
 * Author: urtela
 *
 * Created on October 3, 2011, 5:19 PM
 */

#ifndef ROOF_RENDERER_H
#define	ROOF_RENDERER_H

#include <string>

class Level;

class RoofRenderer {
public:
    RoofRenderer();

    void draw(Level* level);
    void setTexture(const std::string& texName);
    void enable();
    void disable();

    const std::string roofTexture1;
    const std::string roofTexture2;

private:
    std::string roofTexture;
    bool enabled;
};

#endif	/* ROOF_RENDERER_H */

