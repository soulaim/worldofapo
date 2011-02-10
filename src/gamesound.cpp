/*
* gamesound.cpp
*
*  Created on: 13.5.2009
*      Author: UserXP
*/

#include "gamesound.h"

#include <SDL.h>
#include <SDL_mixer.h>

#include <iterator>
#include <iostream>

using namespace std;

Mix_Music *game_sound_music;

void MusicTrackFinished()
{
	if(game_sound_music)
		Mix_FreeMusic(game_sound_music);
	game_sound_music = 0;
}

/*
// check music progress
if( game_sound_music == 0)
{
	game_sound_music = Mix_LoadMUS( "music/smb.mp3" );
	Mix_PlayMusic(game_sound_music, 0);
	Mix_HookMusicFinished(MusicTrackFinished);
	
	Mix_VolumeMusic( static_cast<int> (MIX_MAX_VOLUME * 0.5) );
	}
*/

// 
// Mix_PlayingMusic();

GameSound::GameSound()
{
	SDL_Init(SDL_INIT_AUDIO);
	soundsystem_ok = false;
}

GameSound::~GameSound()
{
	Mix_CloseAudio();
}

int GameSound::end_music()
{
	Mix_FadeOutMusic(3000);
	return 1;
}

int GameSound::init()
{
	int audio_rate = 44100;
	Uint16 audio_format = AUDIO_S16SYS;
	int audio_channels = 2;
	int audio_buffers = 4096;
	
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) == -1)
	{
		cerr << "Unable to open audio! Sounds will not play." << endl;
	}
	else
	{
		cerr << "Soundsystem initiated succesfully" << endl;
		soundsystem_ok = true;
		Mix_AllocateChannels(40);
	}
	
	game_sound_music = 0;
	
	sfx["alien_death"]   = Mix_LoadWAV("sounds/alien_death.wav");
	sfx["player_death"]  = Mix_LoadWAV("sounds/player_death.wav");
	sfx["walk"]  = Mix_LoadWAV("sounds/walk.wav");
	sfx["jump"]  = Mix_LoadWAV("sounds/jump.wav");
	sfx["jump_land"] = Mix_LoadWAV("sounds/jump_land.wav");
	sfx["domination"]  = Mix_LoadWAV("sounds/domination.wav");
	
	sfx["shoot"] = Mix_LoadWAV("sounds/shoot.wav");
	sfx["flamethrower"] = Mix_LoadWAV("sounds/flamethrower_licensed.wav");
	sfx["machinegun"] = Mix_LoadWAV("sounds/machinegun_licensed.wav");
	sfx["shotgun"] = Mix_LoadWAV("sounds/shotgun_licensed.wav");
	sfx["rifle"] = Mix_LoadWAV("sounds/rifle_shot_licensed.wav");
	
	/*
	sfx["taunt1"] = Mix_LoadWAV("sounds/taunts/balls.wav");
	sfx["taunt2"] = Mix_LoadWAV("sounds/taunts/powerking.wav");
	sfx["taunt3"] = Mix_LoadWAV("sounds/taunts/upyours.wav");
	*/
	
	sfx["domination"] = Mix_LoadWAV("sounds/domination.wav");
	
	sfx["hit0"] = Mix_LoadWAV("sounds/hit4.wav");
	sfx["hit1"] = Mix_LoadWAV("sounds/hit3.wav");
	sfx["hit2"] = Mix_LoadWAV("sounds/hit2.wav");
	sfx["hit3"] = Mix_LoadWAV("sounds/hit1.wav");
	
	return 1;
}


void GameSound::playEffect(const string& lol, float distance, float magnitude)
{
	string requested_sfx = lol;
	
	if(!soundsystem_ok)
		return;
	
	/*
	if(requested_sfx == "taunt")
	{
		string number = "1";
		number[0] += rand() % 3;
		requested_sfx.append(number);
		distance = 1;
	}
	*/
	
	if(requested_sfx == "domination")
	{
		distance = 1;
	}
	
	if(requested_sfx == "")
		return;
	
	// +0.01 to avoid division by zero
	int volume = magnitude / (0.01 + (distance * distance));
	
	if(volume > 128)
		volume = 128;
	
	if(volume < 30)
		return;
	
	sfx[requested_sfx]->volume = volume;
	Mix_PlayChannel(-1, sfx[requested_sfx], 0);
}


void GameSound::startMusic(string& name)
{
	if(!soundsystem_ok)
		return;
	
	if(name == "NONE")
		return;
	
	Mix_HaltMusic();
	if(game_sound_music)
		Mix_FreeMusic(game_sound_music);
	game_sound_music = 0;
	
	game_sound_music = Mix_LoadMUS(name.c_str());
	Mix_FadeInMusic(game_sound_music, 1, 3000);
	Mix_HookMusicFinished(MusicTrackFinished);
}


