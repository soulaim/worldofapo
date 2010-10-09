
#ifndef GAMESOUND_H_
#define GAMESOUND_H_


#include <string>
#include <iostream>
#include <map>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

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
