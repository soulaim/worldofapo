#include "userio.h"
#include <iostream>

using namespace std;

UserIO::UserIO()
{
	mouseButtons = 0;
	mouse = Coord(-1, -1);
	wheel_has_been_scrolled_up = false;
	wheel_has_been_scrolled_down = false;
	keystate = 0;
	numKeys = 0;
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

int UserIO::isPressed(int key)
{
	checkEvents();
	return keystate[key];
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
	
	/*
	if(keystate[SDLK_F9])
		keyBoard |= 1<<16;
	if(keystate[SDLK_F10])
		keyBoard |= 1<<17;
	*/
	
	if(keystate[SDLK_F2])
		keyBoard |= 1<<18;
	if(keystate[SDLK_F3])
		keyBoard |= 1<<19;

	if(keystate[SDLK_1])
		keyBoard |= 1<<20;
	if(keystate[SDLK_2])
		keyBoard |= 1<<21;
	if(keystate[SDLK_3])
		keyBoard |= 1<<22;
	if(keystate[SDLK_4])
		keyBoard |= 1<<23;
	if(keystate[SDLK_5])
		keyBoard |= 1<<24;
/*
	if(keystate[SDLK_w])
		keyBoard |= 1<<22;
	if(keystate[SDLK_s])
		keyBoard |= 1<<23;
	if(keystate[SDLK_e])
		keyBoard |= 1<<24;
*/

	if(keystate[SDLK_TAB])
		keyBoard |= 1<<31;

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
	return mouseButtons;
}

UserIO::MouseScrollStatus UserIO::getMouseWheelScrolled()
{
	if(wheel_has_been_scrolled_up)
	{
		wheel_has_been_scrolled_up = 0;
		return SCROLL_UP;
	}
	else if(wheel_has_been_scrolled_down)
	{
		wheel_has_been_scrolled_down = 0;
		return SCROLL_DOWN;
	}
	return NO_SCROLL;
}

string UserIO::getSingleKey()
{
	checkEvents();
	for(int i=0; i<numKeys; i++)
	{
		if(keyStates[i] > 0)
		{
			keyStates[i]--;
			
			bool shift_pressed = isPressed(SDLK_LSHIFT) || isPressed(SDLK_RSHIFT);
			bool special_key = keyNames[i].size() != 1;
			
			if(keyNames[i] == "left ctrl") return "^";
			
			if(!shift_pressed || special_key )
			{
				return keyNames[i];
			}

			string key = keyNames[i];
			if(key[0] >= 'a' && key[0] <= 'z')
			{
				key[0] -= 32;
			}
			
			if(key[0] == '+') key[0] = '?';
			if(key[0] == '-') key[0] = '_';
			if(key[0] == '9') key[0] = ')';
			if(key[0] == '8') key[0] = '(';
			if(key[0] == '7') key[0] = '/';
			if(key[0] == '6') key[0] = '&';
			if(key[0] == '5') key[0] = '%';
//			if(key[0] == '4') key[0] = '¤';
			if(key[0] == '3') key[0] = '#';
			if(key[0] == '2') key[0] = '"';
			if(key[0] == '1') key[0] = '!';
			if(key[0] == '0') key[0] = '=';
			
			return key;
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
				mouseButtons |= 1;
			}
			
			//If the right mouse button was released
			if( event.button.button == SDL_BUTTON_RIGHT )
			{
				mouse = Coord(event.button.x, event.button.y);
				mouseButtons |= 2;
			}

			if( event.button.button == SDL_BUTTON_WHEELUP )
			{
				wheel_has_been_scrolled_up = 1;
			}

			if( event.button.button == SDL_BUTTON_WHEELDOWN )
			{
				wheel_has_been_scrolled_down = 1;
			}
			
		}
		else if( event.type == SDL_MOUSEBUTTONUP )
		{
			if( event.button.button == SDL_BUTTON_LEFT )
			{
				mouse = Coord(event.button.x, event.button.y);
				mouseButtons &= ~0 ^ 1;
			}
			if( event.button.button == SDL_BUTTON_RIGHT )
			{
				mouse = Coord(event.button.x, event.button.y);
				mouseButtons &= ~0 ^ 2;
			}
		}
	}
	
	return 1;
}
