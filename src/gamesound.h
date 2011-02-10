#ifndef GAMESOUND_H
#define GAMESOUND_H

#include <string>
#include <iostream>
#include <map>

struct Mix_Chunk;

class GameSound
{
	std::map<std::string, Mix_Chunk*> sfx;
	bool soundsystem_ok;
	
public:
	GameSound();
	~GameSound();
	
	int init();
	
	void playEffect(const std::string&, float, float);
	void startMusic(std::string& name);
	int end_music();
};

#endif

