
#ifndef H_MENUBUTTON
#define H_MENUBUTTON

#include "image.h"
#include "texturehandler.h"
#include <string>
#include <iostream>
#include <sstream>

struct MenuButton
{
	
	MenuButton(const std::string& name_, std::string& strValue, int& intValue): info(strValue), value(intValue)
	{
		name  = name_;
		if(intValue == -1)
		{
			valueType = 0; // string value
		}
		else
		{
			valueType = 1; // int value
			std::stringstream ss;
			ss << value;
			info = ss.str();
		}
		
		selected = 0;
	}
	
	void unloadTexture()
	{
		std::cerr << "WARNING: Calling MenuButton::unloadTexture(), which is deprecated" << std::endl;
	}
	
	~MenuButton()
	{
	}
	
	std::string info;
	int value;
	
	std::string name;
	int valueType;
	int selected;
};

#endif

