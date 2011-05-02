#include "graphics/texturehandler.h"
#include "graphics/opengl.h"
#include "misc/logger.h"

#include <iostream>
#include <cassert>
#include <fstream>

using namespace std;

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

const string& TextureHandler::getCurrentTexture(size_t texture_unit) const
{
	assert(texture_unit < current_textures.size());
	return current_textures[texture_unit];
}

void TextureHandler::createTextures(const std::string& filename)
{
	ifstream in(filename.c_str());

	string line;
	while(getline(in, line))
	{
		if(line.empty() || line[0] == '#')
		{
			continue;
		}
		stringstream ss(line);
		string name;
		string file;
		ss >> name >> file;
		createTexture(name, file);
	}
}

unsigned TextureHandler::createTexture(const string& name, const string& filename)
{
	assert(!name.empty());

	Image img;
	img.loadImage(filename);
	
	if((img.sizeX == 0) || (img.sizeY == 0))
	{
		cerr << "Failed to load texture '" << name << "' from file '" << filename << "'. File doesn't exist?" << endl;
		return 0;
	}
	
	cerr << "Loading " << img.sizeX << "x" << img.sizeY << " texture '" << name << "' from file '" << filename << "'" << endl;
	return createTexture(name, img);
}


unsigned TextureHandler::getTextureID(const std::string& name) const
{
	assert(!name.empty());
	auto it = textures.find(name);
	assert(it != textures.end());
	return it->second;
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


unsigned TextureHandler::createFloatTexture(const std::string& name, int width, int height)
{
	assert(!name.empty());

	glGenTextures(1, &(textures[name]));
	glBindTexture(GL_TEXTURE_2D, textures[name]);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR); // scale linearly when image smalled than texture
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	int value;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &value);
	stringstream ss;
	ss << hex << uppercase << value;
	Logger log;
	log << "Created float texture '" << name << "' with internal format: " << ss.str() << "\n";

	return textures[name];
}

unsigned TextureHandler::createTexture(const std::string& name, int width, int height)
{
	assert(!name.empty());

	glGenTextures(1, &(textures[name]));
	glBindTexture(GL_TEXTURE_2D, textures[name]);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	int value;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &value);
	stringstream ss;
	ss << hex << uppercase << value;
	Logger log;
	log << "Created texture '" << name << "' with internal format: " << ss.str() << "\n";

	return textures[name];
}

unsigned TextureHandler::createDepthTexture(const std::string& name, int width, int height)
{
	assert(!name.empty());

	glGenTextures(1, &(textures[name]));
	glBindTexture(GL_TEXTURE_2D, textures[name]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture

#ifndef _WIN32
	// not really sure if this is 100% necessary anyway..
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,GL_NONE);
#endif

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	int value;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &value);
	stringstream ss;
	ss << hex << uppercase << value;
	Logger log;
	log << "Created depth texture '" << name << "' with internal format: " << ss.str() << "\n";

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
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smaller than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // Deprecated in OpenGL >= 3.0.

	log << "Loading texture '" << name << "' ";
	// 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image, 
	// border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
	if(img.hasAlpha)
	{
		log << "with alpha channel...";
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.sizeX, img.sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
	}
	else
	{
		log << "without alpha channel...";
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img.sizeX, img.sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
//		buildDebugMipmaps(img.sizeX, img.sizeY);
	}
	log << " OK, texture loaded.\n";
	
	img.unload();
	
	return textures[name];
}


void TextureHandler::deleteTexture(const std::string& name)
{
	assert(!name.empty());

	if(textureExists(name))
	{
		glDeleteTextures(1, &textures[name]);
		textures.erase(name);
	}
	else
	{
		cerr << "WARNING: tried to delete a texture that doesnt exist: \"" << name << "\"" << endl;
	}
}


void TextureHandler::unbindTexture(size_t texture_unit)
{
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glDisable(GL_TEXTURE_2D);
	current_textures[texture_unit] = "";
}

int TextureHandler::bindTexture(size_t texture_unit, const std::string& name)
{
	assert(texture_unit < current_textures.size());
	assert(!name.empty());

	if(textureExists(name))
	{
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[name]);
		current_textures[texture_unit] = name;

		return 1;
	}
	else
	{
		cerr << "Trying to bind to a texture that does not exist: \"" << name << "\"" << endl;
		assert(!textures.empty() && "please load at least one texture...");
		std::string default_texture = textures.begin()->first;
		cerr << "Loading default texture '" << default_texture << "'" << endl;
		assert(!default_texture.empty());
		textures[name] = textures[default_texture];
		bindTexture(texture_unit, default_texture);
		return 0;
	}
}

void TextureHandler::deleteAllTextures()
{
	for(map<string, unsigned>::iterator iter = textures.begin(); iter != textures.end(); iter++)
	{
		glDeleteTextures(1, &iter->second);
	}
	textures.clear();
}


bool TextureHandler::textureExists(const std::string& name) const
{
	return textures.find(name) != textures.end();
}

