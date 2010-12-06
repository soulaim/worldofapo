#ifndef SHADERS_H
#define SHADERS_H

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include <map>
#include <string>
#include <memory>

#include "shader.h"

extern GLint unit_color_location;
extern GLint color_index_location;
extern GLint bones_location;
extern GLint bone_index_location;
extern GLint bone_weight_location;
extern GLint active_location;

class Shaders
{
public:
	void init();
	void release();

	GLint uniform(const std::string& name) const;
	GLuint operator[](const std::string& program_name) const;

private:
	std::map<std::string, std::shared_ptr<Shader>> shaders;
	std::map<std::string, GLint> uniform_locations;
};

#endif

