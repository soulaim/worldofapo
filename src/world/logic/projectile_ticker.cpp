
#include "world/logic/projectile_ticker.h"

#include "world/world.h"
#include "world/objects/unit.h"
#include "world/objects/projectile.h"

#include <string>
#include <map>
#include <cassert>

using std::string;

ProjectileTicker::ProjectileTicker() {
    friendly_fire = 1;
}

void ProjectileTicker::tickProjectile(World& world, Projectile& projectile, Model* model)
{
	int ticks = projectile["TPF"]; assert(ticks > 0);

	int death_at_collision = projectile["DEATH_IF_HITS_UNIT"];
	int projectile_owner   = projectile["OWNER"];
	string& projectile_name = projectile("NAME");

    Location startingPosition = projectile.position;

	int team = -1;
	std::map<int, Unit>::iterator owner_unit_iter = world.units.find(projectile_owner);
	if(owner_unit_iter != world.units.end())
	{
		team = owner_unit_iter->second["TEAM"];
	}

	for(int i=0; i<ticks; i++)
	{
		int lifetime = projectile["LIFETIME"];

		if(lifetime <= 0 || projectile.destroyAfterFrame)
		{
			projectile.destroyAfterFrame = true;
			break;
		}

		projectile.tick();
		if(projectile.collidesTerrain(world.lvl))
			projectile.destroyAfterFrame = true;

		const std::set<OctreeObject*, OctreeObjectLess>& potColl = world.octree->nearObjects(projectile.position);
		for(std::set<OctreeObject*, OctreeObjectLess>::const_iterator it = potColl.begin(); it != potColl.end(); ++it)
		{
			if ((*it)->type != OctreeObject::UNIT)
				continue;

			Unit* u = static_cast<Unit*>(*it);

			// no collision with dead units
			if(!u->exists())
				continue;

			// if the target unit is already dead, just continue.
			if(u->intVals["HEALTH"] <= 0)
				continue;

			// boolean test, hits or doesn't hit
			if(projectile["BALLISTIC"] && projectile.collides(*u))
			{
				{
					BulletHitEvent event;
					event.t_position = u->getEyePosition();
					event.t_velocity = u->velocity;
					event.a_position = projectile.position;
					event.a_velocity = projectile.velocity * projectile["TPF"];
					queueMsg(event);
				}

				u->velocity += projectile.velocity * FixedPoint(projectile["MASS"], 1000) / FixedPoint(u->intVals["MASS"], 1000);

				if(!friendly_fire && ((*u)["TEAM"] == team))
				{
				}
				else
				{
					u->takeDamage(projectile["DAMAGE"]); // does damage according to weapon definition :)
					u->last_damage_dealt_by = projectile_owner;
					(*u)("DAMAGED_BY") = projectile_name;
				}

				projectile.destroyAfterFrame |= death_at_collision;
			}
		}
	}

    Location endingPosition = projectile.position;
    world.visualworld->pushProjectilePath(startingPosition, endingPosition);

	if(world.visualworld->isActive())
	{
		assert(model && "projectile model does not exist");
		model->updatePosition(projectile.position.x.getFloat(), projectile.position.y.getFloat(), projectile.position.z.getFloat());
	}
}
