#ifndef SHADERS_H
#define SHADERS_H

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

extern GLint unit_color_location;
extern GLint color_index_location;
extern GLint bones_location;
extern GLint bone_index_location;
extern GLint bone_weight_location;
extern GLint active_location;

char* readFile(const char *path);
void releaseFile(char* data);

void initShaders();
void printLog(GLuint obj);

#endif
