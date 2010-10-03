
#ifndef H_MENUBUTTON
#define H_MENUBUTTON

#include "image.h"
#include "texturehandler.h"
#include <string>
#include <iostream>

struct MenuButton
{
	
	MenuButton(std::string name_, std::string file)
	{
		name = name_;
		TextureHandler::getSingleton().createTexture(name, file);
		selected = 0;
	}
	
	void unloadTexture()
	{
		std::cerr << "BUTTON: deleting my texture.." << std::endl;
		TextureHandler::getSingleton().deleteTexture(name); 
	}
	
	~MenuButton()
	{
	}
	
	std::string name;
	int selected;
};

#endif

