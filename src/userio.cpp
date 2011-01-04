#include "userio.h"
#include <iostream>

using namespace std;

UserIO::UserIO()
{
	mouseButtons = 0;
	mouse = Coord(-1, -1);
	wheel_has_been_scrolled_up = false;
	wheel_has_been_scrolled_down = false;
	capslock_is_down = false;
	keystate = 0;
	numKeys = 0;
}

// this must not be called before SDL has initialized
void UserIO::init()
{
	keystate = SDL_GetKeyState(&numKeys);
	keyStates.resize(numKeys, 0);
	for(int i = 0; i < numKeys; ++i)
	{
		keyNames.push_back( SDL_GetKeyName(  SDLKey(i) ) );
	}
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
	
	if(keystate[SDLK_r])
		keyBoard |= 1<<16;
	
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
		wheel_has_been_scrolled_up = false;
		return SCROLL_UP;
	}
	else if(wheel_has_been_scrolled_down)
	{
		wheel_has_been_scrolled_down = false;
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
			
			if(special_key || (!shift_pressed && !capslock_is_down))
			{
				return keyNames[i];
			}

			char key = keyNames[i][0];
			if(key >= 'a' && key <= 'z')
			{
				key -= 32;
			}

			if(!shift_pressed)
			{
				return string(1, key);
			}
			
			if(key == '+') key = '?';
			if(key == '-') key = '_';
			if(key == '9') key = ')';
			if(key == '8') key = '(';
			if(key == '7') key = '/';
			if(key == '6') key = '&';
			if(key == '5') key = '%';
//			if(key == '4') key = '¤';
			if(key == '3') key = '#';
			if(key == '2') key = '"';
			if(key == '1') key = '!';
			if(key == '0') key = '=';
			
			return string(1, key);
		}
	}
	return "";
}

void UserIO::checkEvents()
{
	SDL_Event event;
	
	while( SDL_PollEvent( &event ) )
	{
		if(event.type == SDL_KEYDOWN)
		{
			keyStates[event.key.keysym.sym] = 1;
			capslock_is_down = bool(event.key.keysym.mod & KMOD_CAPS);
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
				wheel_has_been_scrolled_up = true;
			}

			if( event.button.button == SDL_BUTTON_WHEELDOWN )
			{
				wheel_has_been_scrolled_down = true;
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
}
