#include "octree.h"

Octree::Octree(Location _bot, Location _top, int _depth):
	top(_top),
	bot(_bot),
	hasChildren(false),
	depth(_depth),
	n(0)
{
	c = (top + bot)/2;
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
	for(std::vector<Unit*>::iterator it = units.begin(); it != units.end(); ++it) {
		insertUnit(*it);
	}
	for(std::vector<Projectile*>::iterator it = projectiles.begin(); it != projectiles.end(); ++it) {
		insertProjectile(*it);
	}
	units.clear();
	projectiles.clear();
}

void Octree::insertUnit(Unit* u)
{
	if (!hasChildren) {
		if (n >= MAX_OBJ && depth < MAX_DEPTH) {
			split();
			n = 0;
		} else {
			units.push_back(u);
			n += 1;
			return;
		}
	}

	Location top = u->hitbox_top();
	Location bot = u->hitbox_bot();
	int pos = 0;

	if (bot.x <= c.x) pos |= BOT_X;
	if (top.x >= c.x) pos |= TOP_X;

	if (bot.y <= c.y) pos |= BOT_Y;
	if (top.y >= c.y) pos |= TOP_Y;

	if (bot.z <= c.z) pos |= BOT_Z;
	if (top.z >= c.z) pos |= TOP_Z;

	for(int x = 0; x < 2; ++x) {
		if (x == 0 && !(pos & BOT_X))
			continue;
		if (x == 1 && !(pos & TOP_X))
			continue;
		for(int y = 0; y < 2; ++y) {
			if (y == 0 && !(pos & BOT_Y))
				continue;
			if (y == 1 && !(pos & TOP_Y))
				continue;
			for(int z = 0; z < 2; ++z) {
				if (z == 0 && !(pos & BOT_Z))
					continue;
				if (z == 1 && !(pos & TOP_Z))
					continue;
				children[x][y][z]->insertUnit(u);
			}
		}
	}
}

void Octree::insertProjectile(Projectile* p)
{
	if (!hasChildren) {
		if (n >= MAX_OBJ && depth < MAX_DEPTH) {
			split();
			n = 0;
		} else {
			projectiles.push_back(p);
			n += 1;
			return;
		}
	}
	Location l = p->curr_position;
	int x = (l.x < c.x)? 0 : 1;
	int y = (l.y < c.y)? 0 : 1;
	int z = (l.z < c.z)? 0 : 1;
	children[x][y][z]->insertProjectile(p);
}

void Octree::potProjectileUnitColl(std::vector<std::pair<Projectile*, Unit*>>& l) {
	for(std::vector<Projectile*>::iterator p_it = projectiles.begin(); p_it != projectiles.end(); ++p_it)
		for(std::vector<Unit*>::iterator u_it = units.begin(); u_it != units.end(); ++u_it)
			l.push_back(make_pair<Projectile*,Unit*>(*p_it, *u_it));

	if (hasChildren)
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
					children[i][j][k]->potProjectileUnitColl(l);
}

void Octree::potUnitUnitColl(std::vector<std::pair<Unit*, Unit*>>& l) {
	for(std::vector<Unit*>::iterator u_it = units.begin(); u_it != units.end(); ++u_it) {
		std::vector<Unit*>::iterator u2_it = u_it;
		for(++u2_it; u2_it != units.end(); ++u2_it)
			l.push_back(make_pair<Unit*,Unit*>(*u_it, *u2_it));
	}

	if (hasChildren)
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
					children[i][j][k]->potUnitUnitColl(l);
}

