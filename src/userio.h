#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL.h>

#include <vector>
#include <string>
#include <cstdlib>


struct Coord
{
	Coord(int a, int b): x(a), y(b) {}
	Coord():x(0), y(0) {}
	int x;
	int y;
};

class UserIO
{
	Uint8* keystate;
	int numKeys;
	
	Coord mouseMove;
	Coord mouse;
	int mouseButtons;
	bool wheel_has_been_scrolled_up;
	bool wheel_has_been_scrolled_down;
	bool capslock_is_down;
	
	std::vector<int> keyStates;
	std::vector<std::string> keyNames;
public:
	enum MouseScrollStatus
	{
		NO_SCROLL = 0,
		SCROLL_UP = 1,
		SCROLL_DOWN = 2
	};

	void init();
	
	int getGameInput();
	int isPressed(int);
	
	void getMouseChange(int& x, int& y);
	int getMousePress();
	MouseScrollStatus getMouseWheelScrolled();
	int ismousepressed();
	
	Coord getMousePoint();
	
	void checkEvents();
	std::string getSingleKey();
	
	UserIO();
};


#endif
