
#ifndef H_MENUBUTTON
#define H_MENUBUTTON

#include "image.h"
#include <string>

struct MenuButton
{

  MenuButton(std::string name_, std::string file)
  {
    name = name_;
    image.loadImage(file);
    selected = 0;
  }
  
  unsigned texture;
  std::string name;
  Image image;
  int selected;
};


#endif
