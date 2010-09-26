
#include "image.h"

using namespace std;

int pngLoad(const char *file, unsigned long *pwidth, unsigned long *pheight, char **image_data_ptr);

void Image::loadImage(string& filename)
{
  pngLoad(filename.c_str(), &sizeX, &sizeY, &data);
}

