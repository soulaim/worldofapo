
#include "string2rgb.h"
#include <iostream> // for errors

using namespace std;

// TODO: this should be a bit smarter maybe? oh well.
void getColor(const string& a, int& r, int& g, int& b)
{
	if(a == "WHITE")
	{
		r = 255;
		g = 255;
		b = 255;
	}
	else if(a == "ORANGE")
	{
		r = 255;
		g = 165;
		b = 0;
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
	}
	else if(a == "BLUE")
	{
		r = 0;
		g = 0;
		b = 255;
	}
	else if(a == "GREY")
	{
		r = 100;
		g = 100;
		b = 100;
	}
	else if(a == "BLACK")
	{
		r = 0;
		g = 0;
		b = 0;
	}
	else
	{
		std::cerr << "UNKOWN COLOR: " << a << std::endl;
		r = 0;
		g = 255;
		b = 255;
	}
}

void getColor(const string& a, float* rgb)
{
	int r, g, b;
	getColor(a, r, g, b);
	
	rgb[0] = r / 255.;
	rgb[1] = g / 255.;
	rgb[2] = b / 255.;
}