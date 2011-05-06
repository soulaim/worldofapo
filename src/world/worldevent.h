#ifndef WORLDEVENT_H
#define WORLDEVENT_H

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

