#include "octree.h"

Octree::Octree(Location _bot, Location _top, int _depth):
	top(_top),
	bot(_bot),
	hasChildren(false),
	depth(_depth),
	n(0)
{
	c = Location((top.x + bot.x)/2, (top.y + bot.y)/2, (top.z + bot.z)/2);
}

void Octree::split() {
	for(int x = 0; x < 2; ++x) {
		FixedPoint bot_x;
		FixedPoint top_x;
		if (x == 0) {
			bot_x = bot.x;
			top_x = c.x;
		}
		else {
			bot_x = c.x;
			top_x = top.x;
		}

		for(int y = 0; y < 2; ++y) {
			FixedPoint bot_y;
			FixedPoint top_y;
			if (y == 0) {
				bot_y = bot.y;
				top_y = c.y;
			}
			else {
				bot_y = c.y;
				top_y = top.y;
			}
			
			for(int z = 0; z < 2; ++z) {
				FixedPoint bot_z;
				FixedPoint top_z;
				if (z == 0) {
					bot_z = bot.z;
					top_z = c.z;
				}
				else {
					bot_z = c.z;
					top_z = top.z;
				}
				children[x][y][z].reset(new Octree(Location(bot_x, bot_y, bot_z),
							Location(top_x, top_y, top_z), depth + 1));
			}
		}
	}
	hasChildren = true;
	for(std::vector<Location>::iterator it = objects.begin(); it != objects.end(); ++it) {
		insert(*it);
	}
	objects.clear();
}

void Octree::insert(Location l)
{
	if (!hasChildren) {
		if (n >= MAX_OBJ && depth < MAX_DEPTH) {
			split();
			n = 0;
		} else {
			objects.push_back(l);
			n += 1;
			return;
		}
	}
	int x = (l.x < c.x)? 0 : 1;
	int y = (l.y < c.y)? 0 : 1;
	int z = (l.z < c.z)? 0 : 1;
	children[x][y][z]->insert(l);
}

