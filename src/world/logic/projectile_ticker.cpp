
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

	int num_particles      = 2;
	int death_at_collision = projectile["DEATH_IF_HITS_UNIT"];
	int projectile_owner   = projectile["OWNER"];
	string& projectile_name = projectile("NAME");

    // TODO: Handle ballistic & beam projectiles separately.

	static ParticleSource ps;

	if(num_particles > 0)
	{
        // these ps values don't need to be properties.
		ps.getIntProperty("MAX_LIFE") = 10;
		ps.getIntProperty("CUR_LIFE") = 10;
		ps.getIntProperty("PSP_1000") = projectile["PARTICLE_RAND_1000"];
		ps.getIntProperty("PPF") = num_particles;
		ps.getIntProperty("PLIFE") = projectile["PARTICLE_LIFE"];
		ps.getIntProperty("SCALE") = projectile["PARTICLE_SCALE"];

		ps.setColors(projectile("START_COLOR_START"), projectile("START_COLOR_END"), projectile("END_COLOR_START"), projectile("END_COLOR_END"));
	}

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

		// generate particles
		if(num_particles > 0)
		{
			ps.velocity = projectile.velocity * FixedPoint(projectile["PARTICLE_VELOCITY"], 1000);
			ps.position = projectile.position;
			ps.tick(world.visualworld->particles);
		}

		projectile.tick();

		if(projectile.collidesTerrain(world.lvl))
		{
			// behaviour here should be defined by the projectile..

			projectile.destroyAfterFrame = true;
			// intentional continue of execution
		}


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
			if(u->hitpoints <= 0)
				continue;


			if(projectile["DISTANCE_TEST"])
			{
				if(!friendly_fire && ((*u)["TEAM"] == team))
				{
				}
				else
				{
					// distance test
					Location distance_vector = projectile.position - u->getEyePosition(); // TODO: Point to point distance is maybe not ideal.
					FixedPoint real_distance = distance_vector.length();
					FixedPoint max_distance = FixedPoint(projectile["DISTANCE_MAX"], 1000);

					if(real_distance < max_distance)
					{
						u->takeDamage( ((max_distance - real_distance) / max_distance * projectile["DISTANCE_DAMAGE"]).getInteger() );
						u->last_damage_dealt_by = projectile_owner;
						(*u)("DAMAGED_BY") = projectile_name;
					}
				}
			}

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

				// blood is blood is blood
				auto string_property_it = projectile.strVals.find("AT_DEATH");
				if(string_property_it != projectile.strVals.end())
				{
					if(string_property_it->second == "PUFF")
					{
						projectile.strVals.erase("AT_DEATH");
					}
				}

				projectile.destroyAfterFrame |= death_at_collision;
			}
		}
	}

	if(world.visualworld->isActive())
	{
		assert(model && "projectile model does not exist");
		model->updatePosition(projectile.position.x.getFloat(), projectile.position.y.getFloat(), projectile.position.z.getFloat());
	}

	// as a post frame update, update values of the projectile
	if(projectile["AIR_RESISTANCE"])
	{
		projectile.velocity *= FixedPoint(projectile["AIR_RESISTANCE"], 1000);
	}
	projectile.velocity.y += FixedPoint(projectile["GRAVITY"], 1000);
}
