
#include "image.h"

using namespace std;

int pngLoad(const char *file, unsigned long *pwidth, unsigned long *pheight, char **image_data_ptr, bool&);
void pngUnload(char* data);

void Image::loadImage(const string& filename)
{
	pngLoad(filename.c_str(), &sizeX, &sizeY, &data, hasAlpha);
}

void Image::unload()
{
	pngUnload(data);
	data = 0;
}

