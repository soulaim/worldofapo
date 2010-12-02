#include "window.h"

#include <SDL.h>
#include <GL/gl.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

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

std::string next_screenshot_filename()
{
	static int next = 0;

	stringstream ss;
	do
	{
		ss << "screenshot" << next << ".bmp";
		++next;
	}
	while(ifstream(ss.str().c_str()));

	return ss.str();
}

void Window::screenshot() const
{
	std::string filename = next_screenshot_filename();

	SDL_Surface* screen = drawContext;
	SDL_Surface* temp = 0;

	if(!(screen->flags & SDL_OPENGL))
	{
		cerr << "Weird, saving non-opengl screenshot to '" << filename << "'?" << endl;
		SDL_SaveBMP(screen, filename.c_str());
		return;
	}

	vector<unsigned char> pixels(3 * screen->w * screen->h);

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
			0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
			);
	if(!temp)
	{
		cerr << "ERROR: screenshot failed" << endl;
		return;
	}

	glReadPixels(0, 0, screen->w, screen->h, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);

	for(int i = 0; i < screen->h; ++i)
	{
		memcpy(((char *) temp->pixels) + temp->pitch * i, &pixels[0] + 3*screen->w * (screen->h-i-1), screen->w*3);
	}

	cerr << "Saving screenshot to '" << filename << "'" << endl;
	SDL_SaveBMP(temp, filename.c_str());

	SDL_FreeSurface(temp);
}


void Window::enable_grab() const
{
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(0);
}

void Window::disable_grab() const
{
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(1);
}

