#ifndef SHADER_H
#define SHADER_H

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include <string>
#include <map>

class Shader
{
public:
	Shader(const std::string& vertex, const std::string& fragment);
	Shader(const std::string& vertex, const std::string& fragment, const std::string& geometry, GLint input, GLint output, GLint vertices);
	~Shader();

	void set_texture_unit(size_t unit, const std::string& name);
	GLint uniform(const std::string& name);
	GLint attribute(const std::string& name);

	void start();
	void stop();

	GLuint get_program() const;

private:
	Shader() = delete;
	Shader(const Shader& shader) = delete;
	Shader& operator=(const Shader& shader) = delete;
	
	static char* readFile(const char *path);
	static void releaseFile(char* data);

	static void printLog(GLuint obj);

	static GLuint loadVertexShader(const std::string& filename);
	static GLuint loadFragmentShader(const std::string& filename);
	static GLuint loadGeometryShader(const std::string& filename);

	GLuint program;
	GLuint vertex;
	GLuint geometry;
	GLuint fragment;

	bool started;
	static int shaders_in_use;
};

#endif

