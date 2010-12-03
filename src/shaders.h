#ifndef SHADERS_H
#define SHADERS_H

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include <map>
#include <string>

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
	char* readFile(const char *path);
	void releaseFile(char* data);

	void printLog(GLuint obj);

	void loadVertexShader(const std::string& name, const std::string& filename);
	void loadFragmentShader(const std::string& name, const std::string& filename);
	void loadGeometryShader(const std::string& name, const std::string& filename);

	std::map<std::string, GLuint> shaders;
	std::map<std::string, GLint> uniform_locations;
};

#endif

