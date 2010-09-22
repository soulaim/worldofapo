
#ifndef GAME_IMAGE_H
#define GAME_IMAGE_H

#include <GL/gl.h>
#include <string>

struct Image {
  Image(): id(0), data(0) {}
  
//  void constructTexture();
  void loadImage(std::string& filename);
  
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
  GLuint id;
};

#endif

