
#ifndef GAME_IMAGE_H
#define GAME_IMAGE_H

#include <string>

struct Image {
	Image(): data(0) {}
	
	//  void constructTexture();
	void loadImage(const std::string& filename);
	void unload();
	
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
	bool hasAlpha;
};

#endif

