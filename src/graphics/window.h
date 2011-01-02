#ifndef WINDOW_H
#define WINDOW_H

#include <cstddef>

class SDL_Surface;

class Window
{
public:
	Window();
	~Window();

	size_t width() const;
	size_t height() const;

	void toggle_fullscreen() const;
	void swap_buffers() const;
	void screenshot() const;

	void createWindow(int, int);
	
	void enable_grab() const;
	void disable_grab() const;

	bool active() const;
	void hide() const;
private:
	Window& operator=(const Window&); // Disabled.
	Window(const Window&); // Disabled.
	
	size_t width_;
	size_t height_;

	SDL_Surface* drawContext;
};

#endif


