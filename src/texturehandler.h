
#ifndef TEXTURE_HANDLER_SINGLETON_LOL
#define TEXTURE_HANDLER_SINGLETON_LOL

#include "image.h"
#include <map>
#include <string>
#include <vector>

class TextureHandler
{
	public:
		TextureHandler();
		~TextureHandler();
		static TextureHandler& getSingleton();
		
		void createTextures(const std::string& filename);
		const std::string& getCurrentTexture(size_t texture_unit);
		unsigned createTexture(const std::string&, const std::string&);
		unsigned createTexture(const std::string&, Image& img);
		unsigned createTexture(const std::string&, int width, int height);
		
		int bindTexture(size_t texture_unit, const std::string&);
		unsigned getTextureID(const std::string&);
		
		void deleteTexture(const std::string& name);
		void deleteAllTextures();
		bool textureExists(const std::string& name);

	private:
		TextureHandler(const TextureHandler&); // Disabled.
		TextureHandler& operator=(const TextureHandler&); // Disabled.
		
		std::map<std::string, unsigned> textures;
		std::vector<std::string> current_textures;

};

#endif

