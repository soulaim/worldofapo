#ifndef WORLDEVENT_H
#define WORLDEVENT_H

struct WorldEvent
{
	// Identifications for events where we want to do some SFX.
	enum EventType
	{
		DAMAGE_BULLET,
		DAMAGE_DEVOUR,
		DEATH_PLAYER,
		DEATH_ENEMY,
		CENTER_CAMERA
	};

	EventType type;
	
	Location t_position;
	Location t_velocity;
	
	Location a_position;
	Location a_velocity;
	
	int actor_id;
	int target_id;
};

#endif

