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

// TODO: All input that is not relevant for the update of the world should be somewhere else.
//       this stuff is sent over network. for local needs, handle key press with getSingleKey()
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

    if(getSingleKey() == "e")
		keyBoard |= 32;

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

	if(keystate[SDLK_TAB])
		keyBoard |= 1<<31;

	return keyBoard;
}

void UserIO::getMouseChange(int& x, int& y)
{
	checkEvents();
	x = mouseMove.x;
	y = mouseMove.y;
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

void UserIO::tick() {
    mouseMove.x = 0;
	mouseMove.y = 0;
    updateSingleKey();
}

const string& UserIO::getSingleKey() {
    return singleKeyPressStorage;
}

void UserIO::updateSingleKey()
{
	checkEvents();
	for(int i=0; i<numKeys; i++)
	{
		if(keyStates[i] > 0)
		{
			keyStates[i]--;

			bool shift_pressed = isPressed(SDLK_LSHIFT) || isPressed(SDLK_RSHIFT);
			bool special_key = keyNames[i].size() != 1;

			if(keyNames[i] == "left ctrl") {
                this->singleKeyPressStorage = "^";
                return;
            }

			if(special_key || (!shift_pressed && !capslock_is_down))
			{
				this->singleKeyPressStorage = keyNames[i];
                return;
			}

			char key = keyNames[i][0];
			if(key >= 'a' && key <= 'z')
			{
				key -= 32;
			}

			if(!shift_pressed)
			{
				singleKeyPressStorage = string(1, key);
                return;
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

			singleKeyPressStorage = string(1, key);
            return;
		}
	}
	singleKeyPressStorage = "";
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
			//If the left mouse button was pressed
			if( event.button.button == SDL_BUTTON_LEFT )
			{
				mouse = Coord(event.button.x, event.button.y);
				mouseButtons |= 1;
			}

			//If the right mouse button was pressed
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
