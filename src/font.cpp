#include "font.h"

std::vector<float> Font::charWidth;
Font charWidth_initializer;

Font::Font()
{
	charWidth.resize(255, 1.f);
	
	for(char symbol = 'A'; symbol <= 'Z'; symbol++)
		charWidth[symbol] = 0.22f;
	for(char symbol = 'a'; symbol <= 'z'; symbol++)
		charWidth[symbol] = 0.19f;
	for(char symbol = '0'; symbol <= '9'; symbol++)
		charWidth[symbol] = 0.16f;
	
	charWidth['9'] = 0.20f;
	charWidth['8'] = 0.20f;
	charWidth['4'] = 0.20f;
	charWidth['0'] = 0.23f;
	charWidth['l'] = 0.1f;
	charWidth['r'] = 0.1f;
	charWidth['f'] = 0.1f;
	charWidth['!'] = 0.1f;
	charWidth['t'] = 0.15f;
	charWidth['>'] = 0.15f;
	charWidth['<'] = 0.15f;
	charWidth['i'] = 0.1f;
	charWidth['w'] = 0.25f;
	charWidth['m'] = 0.25f;
	charWidth['j'] = 0.12f;
	charWidth['o'] = 0.19f;
	charWidth['s'] = 0.12f;
	charWidth['I'] = 0.1f;
	charWidth['J'] = 0.12f;
	charWidth['.'] = 0.1f;
	charWidth[','] = 0.1f;
	charWidth[':'] = 0.1f;
	charWidth['?'] = 0.15f;
	charWidth[' '] = 0.1f;
	charWidth[']'] = 0.1f;
	charWidth['['] = 0.1f;
	charWidth[')'] = 0.1f;
	charWidth['('] = 0.1f;
	charWidth['\''] = 0.1f;
	charWidth['-'] = 0.1f;
	charWidth['+'] = 0.1f;
	charWidth['|'] = 0.1f;
	charWidth['/'] = 0.1f;
	charWidth['_'] = 0.1f;
	charWidth['%'] = 0.13f;
	charWidth['&'] = 0.12f;
	charWidth['='] = 0.13f;
	charWidth['"'] = 0.1f;
	
	charWidth['S'] = 0.17;
	charWidth['T'] = 0.19;
	charWidth['W'] = 0.24;
	charWidth['Q'] = 0.24;
	charWidth['O'] = 0.3;
	charWidth['Z'] = 0.1;
	charWidth['M'] = 0.3;
}

float Font::width(char c)
{
	return charWidth[c];
}

TextureCoordinates Font::texture_coordinates(char c)
{
	TextureCoordinates coords;

	int x = c % 16;
	int y = 15 - (c / 16);

	float edge_size = 1./16.;

	coords.corner[0] = TextureCoordinate( x     * edge_size,       y * edge_size );
	coords.corner[1] = TextureCoordinate( (x+1) * edge_size,       y * edge_size );
	coords.corner[2] = TextureCoordinate( (x+1) * edge_size, (y + 1) * edge_size );
	coords.corner[3] = TextureCoordinate(     x * edge_size, (y + 1) * edge_size );

	return coords;
}

