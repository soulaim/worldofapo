
#include "string2rgb.h"

using namespace std;

// TODO: this should be a bit smarter maybe? oh well.
void getColor(const string& a, int& r, int& g, int& b)
{
	if(a == "WHITE")
	{
		r = 255;
		g = 255;
		b = 255;
		return;
	}
	else if(a == "ORANGE")
	{
		r = 255;
		g = 165;
		b = 0;
		return;
	}
	else if(a == "GREEN")
	{
		r = 0;
		g = 255;
		b = 0;
	}
	else if(a == "DARK_RED")
	{
		r = 100;
		g = 0;
		b = 0;
		return;
	}
	else
	{
		r = 0;
		g = 255;
		b = 255;
		return;
	}
}

void getColor(const string& a, float* rgb)
{
	float& r = (rgb[0]);
	float& g = (rgb[1]);
	float& b = (rgb[2]);
	
	if(a == "WHITE")
	{
		r = 1.0f;
		g = 1.0f;
		b = 1.0f;
		return;
	}
	else if(a == "ORANGE")
	{
		r = 1.0f;
		g = 0.6f;
		b = 0.0f;
		return;
	}
	else if(a == "GREEN")
	{
		r = 0.0f;
		g = 1.0f;
		b = 0.0f;
	}
	else if(a == "DARK_RED")
	{
		r = 0.4f;
		g = 0.0f;
		b = 0.0f;
		return;
	}
	else
	{
		r = 0.0f;
		g = 1.0f;
		b = 1.0f;
		return;
	}
}