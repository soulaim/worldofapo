
#ifndef TEXTURE_HANDLER_SINGLETON_LOL
#define TEXTURE_HANDLER_SINGLETON_LOL

#include "image.h"
#include <map>
#include <string>

class TextureHandler
{
	std::map<std::string, unsigned> textures;
	
	public:
		TextureHandler();
		~TextureHandler();
		TextureHandler(const TextureHandler&);
		
		static TextureHandler& getSingleton();
		
		
		unsigned createTexture(const std::string&, const std::string&);
		unsigned createTexture(const std::string&, Image& img);
		int bindTexture(const std::string&);
		
		void deleteTexture(const std::string&);
		void deleteAllTextures();
		bool textureExists(const std::string&);
};

#endif

