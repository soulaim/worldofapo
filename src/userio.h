#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL/SDL.h>

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
  int keyBoard;
  
  Coord mouseMove;
  Coord mouse;
  int mouse_has_been_pressed;
  int mouse_right_button;
  
public:
  
  int getKeyChange();
  int getMousePress();
  Coord getMousePoint();
  void getMouseChange(int& x, int& y);
  
  int checkEvents();
  std::string getSingleKey();
  UserIO();
};


#endif
