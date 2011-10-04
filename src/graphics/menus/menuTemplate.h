/*
 * File:   menuTemplate.h
 * Author: urtela
 *
 * Created on October 4, 2011, 6:35 PM
 *
 * This is the new menu template that all game menus (both in-game and
 * main menus) should use.
 */

#ifndef MENUTEMPLATE_H
#define	MENUTEMPLATE_H

class World;
class Localplayer;
class UserIO;

class AbstractMenu {
    virtual void draw(World& w) = 0;
    virtual void tick(Localplayer& localplayer, UserIO* io) = 0;
};

#endif	/* MENUTEMPLATE_H */

