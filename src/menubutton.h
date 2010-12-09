
#ifndef H_MENUBUTTON
#define H_MENUBUTTON

#include "image.h"
#include "texturehandler.h"
#include <string>
#include <iostream>

struct MenuButton
{
	
	MenuButton(const std::string& name_, const std::string&)
	{
		name = name_;
		
		// TextureHandler::getSingleton().createTexture(name, file);
		selected = 0;
	}
	
	void unloadTexture()
	{
		std::cerr << "BUTTON: deleting my texture.." << std::endl;
		// TextureHandler::getSingleton().deleteTexture(name);
	}
	
	~MenuButton()
	{
	}
	
	std::string info;
	std::string name;
	int selected;
};

#endif

