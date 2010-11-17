/*
* gamesound.cpp
*
*  Created on: 13.5.2009
*      Author: UserXP
*/

#include "gamesound.h"
#include <iterator>
#include <iostream>

using namespace std;

Mix_Music *game_sound_music;

/*
// check music progress
if( game_sound_music == 0  && !game_over)
{
	game_sound_music = Mix_LoadMUS( "music/smb.mp3" );
	Mix_PlayMusic(game_sound_music, 0);
	Mix_HookMusicFinished(musicDone);
	
	Mix_VolumeMusic( static_cast<int> (MIX_MAX_VOLUME * 0.5) );
	}
*/



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
	Mix_HaltMusic();
	if(game_sound_music)
		Mix_FreeMusic(game_sound_music);
	game_sound_music = 0;
	game_over = true;

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
		Mix_AllocateChannels(20);
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
	
	sfx["hit0"] = Mix_LoadWAV("sounds/hit4.wav");
	sfx["hit1"] = Mix_LoadWAV("sounds/hit3.wav");
	sfx["hit2"] = Mix_LoadWAV("sounds/hit2.wav");
	sfx["hit3"] = Mix_LoadWAV("sounds/hit1.wav");
	
	return 1;
}

/* This is the function that we told SDL_Mixer to call when the music
was finished. In our case, we're going to simply unload the music
as though the player wanted it stopped.  In other applications, a
different music file might be loaded and played. */
void musicDone()
{
	Mix_HaltMusic();
	Mix_FreeMusic(game_sound_music);
	game_sound_music = 0;
}


void GameSound::playEffect(const string& requested_sfx, float distance, float magnitude)
{
	if(!soundsystem_ok)
		return;
	
	if(requested_sfx == "")
		return;
	
	// +0.01 to avoid division by zero
	int volume = magnitude / (0.01 + (distance * distance));
	
	if(volume > 128)
		volume = 128;
	
	if(volume < 10)
		return;

	sfx[requested_sfx]->volume = volume;
	Mix_PlayChannel(-1, sfx[requested_sfx], 0);
	
}


void GameSound::levelStart()
{
	if(!soundsystem_ok)
		return;
	
	Mix_HaltMusic();
	if(game_sound_music)
		Mix_FreeMusic(game_sound_music);
	game_sound_music = 0;
	game_over = false;
}


