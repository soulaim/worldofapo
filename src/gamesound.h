#ifndef GAMESOUND_H
#define GAMESOUND_H

#include <string>
#include <iostream>
#include <map>

class Mix_Chunk;

class GameSound
{
	std::map<std::string, Mix_Chunk*> sfx;
	bool game_over;
	bool soundsystem_ok;
	
	public:
		GameSound();
		~GameSound();
		
		int init();
		int end_music();
		
		void playEffect(const std::string&, float, float);
		void levelStart();
};

#endif

