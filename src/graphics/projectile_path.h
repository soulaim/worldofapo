/*
 * File:   projectile_path.h
 * Author: urtela
 *
 * Created on November 2, 2011, 4:11 PM
 */

#ifndef PROJECTILE_PATH_H
#define	PROJECTILE_PATH_H

#include "misc/vec3.h"

class ProjectilePath {
    Location start;
    Location end;

    float r, g, b, a;
    float scale;

    int time_to_live;

public:
    ProjectilePath(const Location&, const Location&);
    void tick(int);
    void draw() const;
    bool alive() const;
};

#endif	/* PROJECTILE_PATH_H */

