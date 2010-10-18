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
	static const int MAX_OBJ = 8;
	static const int MAX_DEPTH = 6;

	enum {
		BOT_X = 1,
		TOP_X = 2,
		BOT_Y = 4,
		TOP_Y = 8,
		BOT_Z = 16,
		TOP_Z = 32
	};

public:
	Octree(Location _bot, Location _top, int _depth = 1);
	Location top;
	Location bot;
	Location c;

	bool hasChildren;
	std::shared_ptr<Octree> children[2][2][2];
	int depth;
	int n;
	std::vector<Unit*> units;

	void split();
	void insertUnit(Unit* u);
	const std::vector<Unit*>& potProjectileUnitColl(const Projectile&) const;
	void potUnitUnitColl(std::vector<std::pair<Unit*,Unit*>>& l);
};

#endif
