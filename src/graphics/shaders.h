#ifndef SHADERS_H
#define SHADERS_H

#include "opengl.h"

#include <map>
#include <string>
#include <memory>

#include "shader.h"

// TODO: get rid of these.
extern GLint unit_color_location;
extern GLint color_index_location;
extern GLint bones_location;
extern GLint bone_index_location;
extern GLint bone_weight_location;
extern GLint model_scale_location;

class Shaders
{
public:
	void init();
	void release();

	GLuint operator[](const std::string& program_name) const;

	Shader& get_shader(const std::string& program_name);
private:
	std::map<std::string, std::shared_ptr<Shader> > shaders;
};

#endif

