#ifndef OPENGL_H
#define OPENGL_H


#ifdef _WIN32
#include <GL\glew.h>
#else
#include "glew/glew.h"
#endif

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
        bool& getGL3bit();
        void init();
};

#endif

