
#include "userio.h"
#include <iostream>


using namespace std;

UserIO::UserIO()
{
  keyBoard = 0;
  mouse_has_been_pressed = 0;
  mouse_right_button = 0;
  mouse = Coord(-1, -1);
}

int UserIO::getKeyChange()
{
  Uint8* keystate = SDL_GetKeyState(NULL);
  
  keyBoard = 0;
  if(keystate[SDLK_LEFT])
    keyBoard |= 1;
  if(keystate[SDLK_RIGHT])
    keyBoard |= 2;
  if(keystate[SDLK_UP])
    keyBoard |= 4;
  if(keystate[SDLK_DOWN])
    keyBoard |= 8;
  if(keystate[SDLK_SPACE])
    keyBoard |= 16;
  if(keystate[SDLK_INSERT])
    keyBoard |= 32;
  if(keystate[SDLK_DELETE])
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
  
  return keyBoard;
}

void UserIO::getMouseChange(int& x, int& y)
{
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


string UserIO::getSingleKey()
{
  SDL_Event event;

  while(SDL_PollEvent( &event ))
  {
    if(event.type == SDL_KEYDOWN)
    {
      if(event.key.keysym.sym == SDLK_ESCAPE)
      {
	cerr << "User pressed ESC, shutting down." << endl;
	SDL_Quit();
	exit(0);
      }   
      
      return string(SDL_GetKeyName (event.key.keysym.sym));
    }
  }
  
  return "";
}

int UserIO::checkEvents()
{
  SDL_Event event;

  while( SDL_PollEvent( &event ) )
  {
    if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
      if(event.key.keysym.sym == SDLK_ESCAPE)
      {
	cerr << "User pressed ESC, shutting down." << endl;
	SDL_Quit();
	exit(0);
      }
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
