#ifndef FONT_H
#define FONT_H

#include <vector>

#include "texturecoordinate.h"

struct TextureCoordinates
{
	TextureCoordinate corner[4];
};

class Font
{
public:
	Font();

	static float width(char c);
	static TextureCoordinates texture_coordinates(char c);
private:
	// Define some character widths in our particular font.
	static std::vector<float> charWidth;
};

#endif

