
#ifndef H_MENUBUTTON
#define H_MENUBUTTON

#include "graphics/image.h"
#include "graphics/texturehandler.h"
#include <string>
#include <iostream>
#include <sstream>

struct MenuParticle
{
	MenuParticle()
	{
		reset();
	}

	float x;
	float y;

	float vx;
	float vy;

	int val;
	int front;

	void tick()
	{
		x += vx;
		y += vy;
	}

	void reset()
	{
		x = randf(-1.0f, 1.0f);
		y = 1.2f;
		vx = 0.0f;

		val = 0;

		if(randf(0.0f, 1.0f) > 0.5f)
			val = 1;

		if(randf(0.0f, 1.0f) > 0.5f)
		{
			// front
			front = 1;
			vy = -0.005f;
		}
		else
		{
			// back
			front = 0;
			vy = -0.003f;
		}

		vy *= randf(0.9f, 1.1f);
	}

	bool alive()
	{
		if(x < -1.5f || x > 1.5f)
			return false;
		if(y < -1.5f || y > 1.5f)
			return false;
		return true;
	}
};

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

		reserved = 0;
		selected = 0;
	}

	void unloadTexture()
	{
		std::cerr << "WARNING: Calling MenuButton::unloadTexture(), which is deprecated" << std::endl;
	}

	bool editing() const
	{
		return (reserved > 0);
	}

	void reserve()
	{
		reserved = 1;
	}

	void release()
	{
		reserved = 0;
	}

	~MenuButton()
	{
	}

	std::string info;
	int value;

	std::string name;
	int valueType;
	int selected;
	int reserved;
};

#endif

