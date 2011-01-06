#include "window.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <SDL.h>

#include "opengl.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include <iomanip>

using namespace std;

Window::Window():
	drawContext(0)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		cerr << "ERROR: SDL init failed." << endl;
		throw std::string("Unable to init SDL");
	}
	
	// createWindow(width, height);
}

Window::~Window()
{
//	SDL_VideoQuit();
	SDL_Quit();
}

void Window::createWindow(int width, int height)
{
	assert(width > 10);
	assert(width <= 4096);
	assert(height > 10);
	assert(height <= 4096);

	width_  = width;
	height_ = height;
	
	assert(!drawContext);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	
	drawContext = SDL_SetVideoMode(width_, height_, 0, SDL_OPENGL); // | SDL_FULLSCREEN);
	if(drawContext == 0)
	{
		cerr << "ERROR: drawContext = " << drawContext << endl;
		throw std::string("Unable to set SDL video mode");
	}
	else
	{
		SDL_WM_SetCaption("World of Apo", "World of Apo");
		
//		cerr << "SUCCESS: Got a drawContext!" << endl;
	}
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

	string filename;
	do
	{
		stringstream ss;
		ss << "screenshots/screenshot" << setw(3) << setfill('0') << next << ".bmp";
		filename = ss.str();
		++next;
	}
	while(ifstream(filename));

	return filename;
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

bool Window::active() const
{
	return SDL_GetAppState() & SDL_APPACTIVE;
}

void Window::hide() const
{
	disable_grab();
	SDL_WM_IconifyWindow();
}

