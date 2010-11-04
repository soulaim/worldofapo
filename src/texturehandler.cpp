
#include "texturehandler.h"
#include <GL/gl.h>

#include <iostream>
#include <cassert>

using namespace std;

/*
static TextureHandler* s_TexHandler;
std::map<std::string, unsigned> textures;
*/  

TextureHandler::TextureHandler()
{
	int max_texture_units = 0;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &max_texture_units);
	cerr << "Max texture units: " << max_texture_units << endl;
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

unsigned TextureHandler::createTexture(const string& name, const string& fileName)
{
	Image img;
	img.loadImage(fileName);
	return createTexture(name, img);
}


unsigned TextureHandler::getTextureID(const std::string& name)
{
	if(textureExists(name))
		return textures[name];
	glGenTextures(1, &(textures[name]));
	return textures[name];
}

unsigned TextureHandler::createTexture(const string& name, Image& img)
{
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
	
	glGenTextures(1, &(textures[name]));
	glBindTexture(GL_TEXTURE_2D, textures[name]);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	
	// 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image, 
	// border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
	if(img.hasAlpha)
		glTexImage2D(GL_TEXTURE_2D, 0, 4, img.sizeX, img.sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, 3, img.sizeX, img.sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
	
	img.unload();
	
	return textures[name];
}


void TextureHandler::deleteTexture(const string& name)
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



int TextureHandler::bindTexture(size_t texture_unit, const string& name)
{
	assert(texture_unit < current_textures.size());
	if(textureExists(name))
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, textures[name]);
		current_textures[texture_unit] = name;
		return 1;
	}
	else
		cerr << "Trying to bind to a texture that doesnt exist: \"" << name << "\"" << endl;
	return 0;
}

void TextureHandler::deleteAllTextures()
{
	for(map<string, unsigned>::iterator iter = textures.begin(); iter != textures.end(); iter++)
		glDeleteTextures(1, &textures[(*iter).first]);
	textures.clear();
}


bool TextureHandler::textureExists(const string& name)
{
	if(textures.find(name) == textures.end())
		return false;
	return true;
}

