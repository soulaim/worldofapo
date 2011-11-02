#ifndef WORLDEVENT_H
#define WORLDEVENT_H

#include <string>
#include <misc/vec3.h>

struct GotMyName
{
	std::string name;
};

struct ChatMessage {
    std::string line;
};

struct SoundEvent {
    SoundEvent(const std::string& s, int mag, Location location) : sound(s), source(location), magnitude(mag) {}
    std::string sound;
    Location source;
    float magnitude;
};

struct PlaySoundEvent
{
    std::string sound;
    float distance;
    float magnitude;
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

