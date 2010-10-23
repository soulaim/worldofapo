#include "octree.h"

Octree::Octree(Location _bot, Location _top, int _depth):
	top(_top),
	bot(_bot),
	hasChildren(false),
	depth(_depth),
	n(0)
{
	center = (top + bot)/2;
}

void Octree::split() {
	for(int x = 0; x < 2; ++x) {
		FixedPoint bot_x;
		FixedPoint top_x;
		if (x == 0) {
			bot_x = bot.x;
			top_x = center.x;
		}
		else {
			bot_x = center.x;
			top_x = top.x;
		}
		for(int y = 0; y < 2; ++y) {
			FixedPoint bot_y;
			FixedPoint top_y;
			if (y == 0) {
				bot_y = bot.y;
				top_y = center.y;
			}
			else {
				bot_y = center.y;
				top_y = top.y;
			}
			
			for(int z = 0; z < 2; ++z) {
				FixedPoint bot_z;
				FixedPoint top_z;
				if (z == 0) {
					bot_z = bot.z;
					top_z = center.z;
				}
				else {
					bot_z = center.z;
					top_z = top.z;
				}
				children[x][y][z].reset(new Octree(Location(bot_x, bot_y, bot_z),
							Location(top_x, top_y, top_z), depth + 1));
			}
		}
	}
	hasChildren = true;
	for(auto it = units.begin(); it != units.end(); ++it) {
		insertObject(*it);
	}
	units.clear();
}

void Octree::insertObject(OctreeObject* o)
{
	if (!hasChildren) {
		if (n >= MAX_OBJ && depth < MAX_DEPTH) {
			split();
			n = 0;
		} else {
			units.insert(o);
			n += 1;
			return;
		}
	}

	Location top = o->bb_top();
	Location bot = o->bb_bot();
	int pos = 0;

	if (bot.x <= center.x) pos |= BOT_X;
	if (top.x >= center.x) pos |= TOP_X;

	if (bot.y <= center.y) pos |= BOT_Y;
	if (top.y >= center.y) pos |= TOP_Y;

	if (bot.z <= center.z) pos |= BOT_Z;
	if (top.z >= center.z) pos |= TOP_Z;

	if ((pos & BOT_X) && (pos & BOT_Y) && (pos & BOT_Z)) children[0][0][0]->insertObject(o);
	if ((pos & BOT_X) && (pos & BOT_Y) && (pos & TOP_Z)) children[0][0][1]->insertObject(o);
	if ((pos & BOT_X) && (pos & TOP_Y) && (pos & BOT_Z)) children[0][1][0]->insertObject(o);
	if ((pos & BOT_X) && (pos & TOP_Y) && (pos & TOP_Z)) children[0][1][1]->insertObject(o);
	if ((pos & TOP_X) && (pos & BOT_Y) && (pos & BOT_Z)) children[1][0][0]->insertObject(o);
	if ((pos & TOP_X) && (pos & BOT_Y) && (pos & TOP_Z)) children[1][0][1]->insertObject(o);
	if ((pos & TOP_X) && (pos & TOP_Y) && (pos & BOT_Z)) children[1][1][0]->insertObject(o);
	if ((pos & TOP_X) && (pos & TOP_Y) && (pos & TOP_Z)) children[1][1][1]->insertObject(o);
}

const std::set<OctreeObject*>& Octree::nearObjects(const Location& p) const {
	if (!hasChildren)
		return units;
	int x = (p.x < center.x) ? 0 : 1;
	int y = (p.y < center.y) ? 0 : 1;
	int z = (p.z < center.z) ? 0 : 1;
	return children[x][y][z]->nearObjects(p);
}

std::vector<std::pair<OctreeObject*, OctreeObject*>> Octree::potUnitUnitColl() const {
	std::vector<std::pair<OctreeObject*, OctreeObject*>> l;
	getUnitUnitColl(l);
	sort(l.begin(), l.end());
	l.erase(std::unique(l.begin(), l.end()), l.end());
	return l;
}

void Octree::getUnitUnitColl(std::vector<std::pair<OctreeObject*, OctreeObject*> >& l) const {
	for(auto u_it = units.begin(); u_it != units.end(); ++u_it) {
		auto u2_it = u_it;
		for(++u2_it; u2_it != units.end(); ++u2_it)
			l.push_back(std::pair<OctreeObject*,OctreeObject*>(*u_it, *u2_it));
	}

	if (hasChildren)
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
					children[i][j][k]->getUnitUnitColl(l);
}

void Octree::doCollisions() {
	auto l = potUnitUnitColl();
	for(auto it = l.begin(); it != l.end(); ++it)
	{
		auto o = it->first;
		auto o2 = it->second;
		if (Collision::boxBox(o->bb_bot(), o->bb_top(), o2->bb_bot(), o2->bb_top()))
		{
			o->collides(*o2);
		}
	}
}

