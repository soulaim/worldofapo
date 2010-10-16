#ifndef OCTREE_H
#define OCTREE_H

#include "location.h"
#include "unit.h"
#include "projectile.h"
#include <iostream>
#include <vector>
#include <memory>


class Octree
{
	static const int MAX_OBJ = 6;
	static const int MAX_DEPTH = 10;
public:
	Octree(Location _bot, Location _top, int _depth = 1);
	Location top;
	Location bot;
	Location c;

	bool hasChildren;
	std::shared_ptr<Octree> children[2][2][2];
	int depth;
	int n;
	std::vector<Projectile> projectiles;
	std::vector<Unit> units;

	void split();
	void insertUnit(const Unit& u);
	void insertProjectile(const Projectile& p);
};

#endif
