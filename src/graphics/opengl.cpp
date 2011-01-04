#include "opengl.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

OpenGL::OpenGL()
{
	GLenum status = glewInit();
	if(status == GLEW_OK)
	{
		std::cerr << "GLEW JIHUU :DD" << std::endl;
	}
	else
	{
		std::stringstream ss;
		ss << "GLEW initialization failed: " << glewGetErrorString(status);
		throw std::runtime_error(ss.str());
	}

	if(!GLEW_VERSION_3_0)
	{
		throw std::runtime_error("Hardware does not support OpenGL 3.0");
	}
}

