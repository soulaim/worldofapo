
#include "world/logic/unitdeath.h"
#include "world/world.h"
#include "world/objects/unit.h"

#include <vector>
#include <string>
#include <sstream>

using std::string;

void UnitDeathHandler::updateComponents(World& world, Unit& unit) {
    target_colour = world.getTeamColour(unit);
	killer_colour = "";
    killer = "an unknown entity";

	causeOfDeath = unit.last_damage_dealt_by;
	actor_id  = -1;
	target_id = unit.id;

    std::map<int, Unit>::iterator killer_it = world.units.find(causeOfDeath);
	if(killer_it != world.units.end())
	{
		killer_colour = world.getTeamColour(killer_it->second);
		killer = killer_it->second.name;
		actor_id = killer_it->second.id;
	}
}

void UnitDeathHandler::doDeathFor(World& world, Unit& unit) {

    updateComponents(world, unit);


	// deleted units don't deserve a burial!
	if(unit["DELETED"] == 1)
	{
		world.unitHasDied(unit.id);
		return;
	}

	Location t_position = unit.getEyePosition();
	Location t_velocity = unit.velocity; t_velocity.y += FixedPoint(200,1000);
	Location a_position;
	Location a_velocity;

	if(actor_id != -1)
	{
		std::map<int, Unit>::iterator it = world.units.find(actor_id);
		a_position = it->second.getEyePosition();
		a_velocity = it->second.velocity;
	}

	bool verbose = unit.human();

    if(verbose) {
        std::stringstream msg;

        if(actor_id != target_id)
        {
            msg << target_colour << unit.name << " ^Wwas killed by " << killer_colour << killer << " ^g(" << unit("DAMAGED_BY") << ")";
            world.add_message(msg.str());
        }
        else
        {
            actor_id  = -1; // For a suicide, no points are to be awarded.
            msg << target_colour << killer << " ^Whas committed suicide by ^g" << unit("DAMAGED_BY");
            world.add_message(msg.str());
        }
    }

	if(unit.human())
	{
		{
			DeathPlayerEvent event;
			event.t_position = t_position;
			event.t_velocity = t_velocity;
			event.a_position = a_position;
			event.a_velocity = a_velocity;
			event.actor_id = actor_id;
			event.target_id = target_id;
			queueMsg(event);
		}

        playerDeath.handle(world, unit);
	}
	else
	{
        DeathNPCEvent event;
        event.t_position = t_position;
        event.t_velocity = t_velocity;
        event.a_position = a_position;
        event.a_velocity = a_velocity;
        event.actor_id = actor_id;
        event.target_id = target_id;
        queueMsg(event);

		world.unitHasDied(unit.id);
        world.awardExperience(unit);
	}

}
