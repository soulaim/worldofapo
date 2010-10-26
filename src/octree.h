#ifndef OCTREE_H
#define OCTREE_H

#include "octree_object.h"
#include "collision.h"
#include "location.h"
#include <iostream>
#include <vector>
#include <memory>
#include <set>
#include <utility>
#include <algorithm>

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
	Location center;

	bool hasChildren;
	std::shared_ptr<Octree> children[2][2][2];
	int depth;
	int n;
	std::set<OctreeObject*> objects;

	void split();
	void insertObject(OctreeObject*);
	void doCollisions();
	const std::set<OctreeObject*>& nearObjects(const Location&) const;
	void getUnitUnitColl(std::vector<std::pair<OctreeObject*,OctreeObject*>>&) const;
	void potUnitUnitColl(std::vector<std::pair<OctreeObject*, OctreeObject*>>&) const;
};


#endif
