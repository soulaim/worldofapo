/*
 * File:   projectile_ticker.h
 * Author: urtela
 *
 * Created on October 10, 2011, 4:22 PM
 */

#ifndef PROJECTILE_TICKER_H
#define	PROJECTILE_TICKER_H

class World;
class Projectile;
class Model;

class ProjectileTicker {
    int friendly_fire;
public:
    ProjectileTicker();
    void tickProjectile(World&, Projectile&, Model*);
};


#endif	/* PROJECTILE_TICKER_H */

