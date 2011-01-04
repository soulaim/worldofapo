#ifndef OPENGL_H
#define OPENGL_H

#include "glew/glew.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

class OpenGL
{
public:
	OpenGL();
};

#endif

