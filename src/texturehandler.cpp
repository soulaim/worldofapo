#include "texturehandler.h"

#ifdef _WIN32
#include <windows.h>
#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <iostream>
#include <cassert>
#include <fstream>

#include "logger.h"

using namespace std;

/*
static TextureHandler* s_TexHandler;
std::map<std::string, unsigned> textures;
*/  

TextureHandler::TextureHandler()
{
	int max_texture_units = 0;
#ifndef _WIN32
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &max_texture_units);
	cerr << "Max texture units: " << max_texture_units << endl;
#else
	max_texture_units = 4; // make a wild guess :DD
#endif
	current_textures.resize(max_texture_units);
}

TextureHandler::~TextureHandler()
{
	deleteAllTextures();
}

TextureHandler& TextureHandler::getSingleton()
{
	static TextureHandler s_TexHandler;
	return s_TexHandler;
}

const string& TextureHandler::getCurrentTexture(size_t texture_unit)
{
	assert(texture_unit < current_textures.size());
	return current_textures[texture_unit];
}

void TextureHandler::createTextures(const std::string& filename)
{
	ifstream in(filename.c_str());

	string name;
	string file;
	while(in >> name >> file)
	{
		createTexture(name, file);
	}
}

unsigned TextureHandler::createTexture(const string& name, const string& filename)
{
	Image img;
	img.loadImage(filename);

	cerr << "Loading texture '" << name << "' from file '" << filename << "'" << endl;
	return createTexture(name, img);
}


unsigned TextureHandler::getTextureID(const std::string& name)
{
	if(textureExists(name))
		return textures[name];
	glGenTextures(1, &(textures[name]));
	return textures[name];
}

namespace
{
	struct Color
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	};
}

void buildDebugMipmaps(size_t x, size_t y)
{
	assert(x == y && "debug mipmaps don't work with non-square textures");

	int lod = 0;
	do
	{
		size_t bit = 1;
		while(bit < x)
			bit <<= 1;

		unsigned char r = 255 - bit * 255 / 8;
		unsigned char g = bit * 255 / 8;
		unsigned char b = 0;
		Color color = { r, g, b, 255 };

		vector<Color> data(x*y, color);
		glTexImage2D(GL_TEXTURE_2D, lod++, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

		x /= 2;
		y /= 2;
	}
	while(x != 0 && y != 0);
}


unsigned TextureHandler::createTexture(const std::string& name, int width, int height)
{
	glGenTextures(1, &(textures[name]));
	glBindTexture(GL_TEXTURE_2D, textures[name]);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	return textures[name];
}

unsigned TextureHandler::createDepthTexture(const std::string& name, int width, int height)
{
	glGenTextures(1, &(textures[name]));
	glBindTexture(GL_TEXTURE_2D, textures[name]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture

#ifndef _WIN32
	// not really sure if this is 100% necessary anyway..
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,GL_NONE);
#endif

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	return textures[name];
}

unsigned TextureHandler::createTexture(const string& name, Image& img)
{
	assert(!name.empty());

	if(img.data == 0)
	{
		cerr << "ERROR: Trying to build texture of image pointer -> 0" << endl;
		return 0;
	}
	
	if(textureExists(name))
	{
		cerr << "Texture for \"" << name << "\" is already loaded!" << endl;
		return textures[name];
	}
	
	Logger log;

	glGenTextures(1, &(textures[name]));
	glBindTexture(GL_TEXTURE_2D, textures[name]);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

	log << "Loading texture '" << name << "' ";
	// 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image, 
	// border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
	if(img.hasAlpha)
	{
		log << "with alpha channel...";
//		glTexImage2D(GL_TEXTURE_2D, 0, 4, img.sizeX, img.sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, img.sizeX, img.sizeY, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
	}
	else
	{
		log << "without alpha channel...";
//		glTexImage2D(GL_TEXTURE_2D, 0, 3, img.sizeX, img.sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
//		buildDebugMipmaps(img.sizeX, img.sizeY);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, img.sizeX, img.sizeY, GL_RGB, GL_UNSIGNED_BYTE, img.data);
	}
	log << " OK, texture loaded.\n";
	
	img.unload();
	
	return textures[name];
}


void TextureHandler::deleteTexture(const std::string& name)
{
	if(textureExists(name))
	{
		glDeleteTextures(1, &textures[name]);
		textures.erase(name);
	}
	else
		cerr << "You tried to delete a texture that doesnt exist: \"" << name << "\"" << endl;
	return;
}



int TextureHandler::bindTexture(size_t texture_unit, const std::string& name)
{
	assert(texture_unit < current_textures.size());
	if(name == "")
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glDisable(GL_TEXTURE_2D);
	}
	else if(textureExists(name))
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[name]);
		current_textures[texture_unit] = name;

		return 1;
	}
	else
	{
		cerr << "Trying to bind to a texture that doesnt exist: \"" << name << "\"" << endl;
	}
	return 0;
}

void TextureHandler::deleteAllTextures()
{
	for(map<string, unsigned>::iterator iter = textures.begin(); iter != textures.end(); iter++)
		glDeleteTextures(1, &textures[(*iter).first]);
	textures.clear();
}


bool TextureHandler::textureExists(const std::string& name)
{
	if(textures.find(name) == textures.end())
		return false;
	return true;
}

