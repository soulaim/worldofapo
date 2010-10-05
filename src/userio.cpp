#include "userio.h"
#include <iostream>

using namespace std;

UserIO::UserIO()
{
	mouse_has_been_pressed = 0;
	mouse_right_button = 0;
	mouse = Coord(-1, -1);
}

// this must not be called before SDL has initialized
void UserIO::init()
{
	emptyString = ""; // lol initialisation :D
	keystate = SDL_GetKeyState(&numKeys);
	keyStates.resize(numKeys, 0);
	for(int i=0; i<numKeys; i++)
		keyNames.push_back( SDL_GetKeyName(  SDLKey(i) ) );
}

int UserIO::getGameInput()
{
	checkEvents();
	
	int keyBoard = 0;
	if(keystate[SDLK_a])
		keyBoard |= 1;
	if(keystate[SDLK_d])
		keyBoard |= 2;
	if(keystate[SDLK_w])
		keyBoard |= 4;
	if(keystate[SDLK_s])
		keyBoard |= 8;
	if(keystate[SDLK_SPACE])
		keyBoard |= 16;
	if(keystate[SDLK_q])
		keyBoard |= 32;
	if(keystate[SDLK_e])
		keyBoard |= 64;
	if(keystate[SDLK_HOME])
		keyBoard |= 128;
	if(keystate[SDLK_END])
		keyBoard |= 256;
	if(keystate[SDLK_PAGEUP])
		keyBoard |= 512;
	if(keystate[SDLK_PAGEDOWN])
		keyBoard |= 1024;
	
	if(keystate[SDLK_F1])
		keyBoard |= 1<<11;
	
	if(keystate[SDLK_F5])
		keyBoard |= 1<<12;
	if(keystate[SDLK_F6])
		keyBoard |= 1<<13;
	if(keystate[SDLK_F7])
		keyBoard |= 1<<14;
	if(keystate[SDLK_F8])
		keyBoard |= 1<<15;
	if(keystate[SDLK_F9])
		keyBoard |= 1<<16;
	if(keystate[SDLK_F10])
		keyBoard |= 1<<17;
	
	if(keystate[SDLK_F2])
		keyBoard |= 1<<18;
	if(keystate[SDLK_F3])
		keyBoard |= 1<<19;

/*
	if(keystate[SDLK_a])
		keyBoard |= 1<<20;
	if(keystate[SDLK_d])
		keyBoard |= 1<<21;
	if(keystate[SDLK_w])
		keyBoard |= 1<<22;
	if(keystate[SDLK_s])
		keyBoard |= 1<<23;
	if(keystate[SDLK_e])
		keyBoard |= 1<<24;
*/

	return keyBoard;
}

void UserIO::getMouseChange(int& x, int& y)
{
	checkEvents();
	
	x = mouseMove.x;
	y = mouseMove.y;
	
	mouseMove.x = 0;
	mouseMove.y = 0;
}

Coord UserIO::getMousePoint()
{
	return mouse;
}

int UserIO::getMousePress()
{
	if(mouse_has_been_pressed)
	{
		mouse_has_been_pressed = 0;
		return 1;
	}
	else if(mouse_right_button)
	{
		mouse_right_button = 0;
		return 2;
	}
	return 0;
}


string& UserIO::getSingleKey()
{
	checkEvents();
	for(int i=0; i<numKeys; i++)
	{
		if(keyStates[i] > 0)
		{
			keyStates[i]--;
			return keyNames[i];
		}
	}
	return emptyString;
}

int UserIO::checkEvents()
{
	SDL_Event event;
	
	while( SDL_PollEvent( &event ) )
	{
		if(event.type == SDL_KEYDOWN)
		{
			/*
			if(event.key.keysym.sym == SDLK_ESCAPE)
			{
				cerr << "User pressed ESC, shutting down." << endl;
				SDL_Quit();
				exit(0);
			}
			*/
			keyStates[event.key.keysym.sym] = 1;
		}
		
		if( event.type == SDL_MOUSEMOTION )
		{
			mouseMove.x += event.motion.xrel;
			mouseMove.y += event.motion.yrel;
		}
		
		if( event.type == SDL_MOUSEBUTTONDOWN )
		{
			//If the left mouse button was released
			if( event.button.button == SDL_BUTTON_LEFT )
			{
				mouse = Coord(event.button.x, event.button.y);
				mouse_has_been_pressed = 1;
			}
			
			//If the right mouse button was released
			if( event.button.button == SDL_BUTTON_RIGHT )
			{
				mouse = Coord(event.button.x, event.button.y);
				mouse_right_button = 1;
			}
			
		}
	}
	
	return 1;
}
