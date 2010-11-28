#include "window.h"

#include <SDL.h>

#include <iostream>

using namespace std;

Window::Window(size_t width, size_t height):
	width_(width),
	height_(height)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		cerr << "ERROR: SDL init failed." << endl;
		throw std::string("Unable to init SDL");
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	
	drawContext = SDL_SetVideoMode(width, height, 0, SDL_OPENGL); // | SDL_FULLSCREEN);
	if(drawContext == 0)
	{
		cerr << "ERROR: drawContext = " << drawContext << endl;
		throw std::string("Unable to set SDL video mode");
	}
	else
	{
		cerr << "SUCCESS: Got a drawContext!" << endl;
	}
}

Window::~Window()
{
//	SDL_VideoQuit();
	SDL_Quit();
}

size_t Window::width() const
{
	return width_;
}

size_t Window::height() const
{
	return height_;
}

void Window::toggle_fullscreen() const
{
	SDL_WM_ToggleFullScreen(drawContext);
}

void Window::swap_buffers() const
{
	SDL_GL_SwapBuffers();
}

