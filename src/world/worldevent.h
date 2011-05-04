#ifndef WORLDEVENT_H
#define WORLDEVENT_H

/*
struct WorldEvent
{
	// Identifications for events where we want to do some SFX.
	enum EventType
	{
		DAMAGE_BULLET,
		DAMAGE_DEVOUR,
		DEATH_PLAYER,
		DEATH_ENEMY,
		CENTER_CAMERA,
		SET_LOCAL_PROPERTY,
		GAME_OVER
	};

	EventType type;
	
	Location t_position;
	Location t_velocity;
	
	Location a_position;
	Location a_velocity;
	
	int actor_id;
	int target_id;
	
	std::string cmd;
};
*/

struct GotMyName
{
	std::string name;
};

struct GotPlayerID
{
	int myID;
};

struct BulletHitEvent
{
	Location t_position;
	Location t_velocity;
	
	Location a_position;
	Location a_velocity;
};

struct DevourEvent
{
	Location t_position;
	Location t_velocity;
	
	int target_id;
};


struct DeathPlayerEvent
{
	Location t_position;
	Location t_velocity;
	
	Location a_position;
	Location a_velocity;
	
	int actor_id;
	int target_id;
};

struct DeathNPCEvent
{
	Location t_position;
	Location t_velocity;
	
	Location a_position;
	Location a_velocity;
	
	int actor_id;
	int target_id;
};

struct CenterCamera
{
	int plr_id;
};

struct SetLocalProperty
{
	std::string cmd;
};

struct GameOver
{
	int win;
};

#endif

