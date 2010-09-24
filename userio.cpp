
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
  int val = keyBoard;
  keyBoard = 0;
  return val;
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


char UserIO::getSingleChar()
{
  SDL_Event event;

  for(int i=0; i<20; i++)
  {
    if( SDL_PollEvent( &event ) )
    {
      if(event.type == SDL_KEYDOWN)
      {
	if(event.key.keysym.sym == SDLK_ESCAPE)
	{
	  cerr << "User pressed ESC, shutting down." << endl;
	  SDL_Quit();
	  exit(0);
	}
	
	if(event.key.keysym.sym == SDLK_SPACE)
	{
	  return ' ';
	}
	
	if(event.key.keysym.sym == SDLK_RETURN)
	{
	  return '#';
	}
	
	
	return *(SDL_GetKeyName (event.key.keysym.sym));
      }
    }
  }
  
  return 0;
}

int UserIO::checkEvents()
{
  SDL_Event event;

  for(int i=0; i<20; i++)
  {
    if( SDL_PollEvent( &event ) )
    {
      if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      {
	if(event.key.keysym.sym == SDLK_ESCAPE)
	{
	  cerr << "User pressed ESC, shutting down." << endl;
	  SDL_Quit();
	  exit(0);
	}

	if(event.key.keysym.sym == SDLK_LEFT)
	  keyBoard |= 1;
	if(event.key.keysym.sym == SDLK_RIGHT)
	  keyBoard |= 2;
	if(event.key.keysym.sym == SDLK_UP)
	  keyBoard |= 4;
	if(event.key.keysym.sym == SDLK_DOWN)
	  keyBoard |= 8;
	if(event.key.keysym.sym == SDLK_SPACE)
	  keyBoard |= 16;
      }
      
      if( event.type == SDL_MOUSEMOTION )
      {
	mouseMove.x += event.motion.xrel;
	mouseMove.y += event.motion.yrel;
      }
      
      if( event.type == SDL_MOUSEBUTTONUP )
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
  }

  return 1;
}
