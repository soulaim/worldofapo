#ifndef GAMESOUND_H
#define GAMESOUND_H

#include "world/worldevent.h"
#include "misc/messaging_system.h"
#include <string>
#include <iostream>
#include <map>

struct Mix_Chunk;

class GameSound : public MessagingSystem<PlaySoundEvent>
{
	std::map<std::string, Mix_Chunk*> sfx;
	bool soundsystem_ok;

public:
	GameSound();
	~GameSound();

	int init();

	void handle(const PlaySoundEvent&);
	void startMusic(std::string& name);
	int end_music();
};

#endif

