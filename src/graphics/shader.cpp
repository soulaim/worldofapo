#include "shader.h"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdexcept>

using namespace std;

int Shader::shaders_in_use = 0;

char* Shader::readFile(const std::string& path)
{
	FILE *fd;
	long len, r;
	char *str;
	
	if(!(fd = fopen(path.c_str(), "r")))
	{
		throw runtime_error("Can't open file '" + path + "' for reading");
	}
	
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	
	cerr << "File '" << path << "' is " << len << " long" << endl;
	
	fseek(fd, 0, SEEK_SET);
	
	str = new char[len+1];
	r = fread(str, 1, len, fd);
	
	str[r] = '\0';
	fclose(fd);
	
	return str;
}

void Shader::releaseFile(char* file)
{
	delete[] file;
}

GLuint Shader::loadVertexShader(const string& filename)
{
	char* data = readFile(filename);
	
	const char* code = data;
	GLuint ret = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(ret, 1, &code, NULL);
	glCompileShader(ret);

	releaseFile(data);

	return ret;
}

GLuint Shader::loadFragmentShader(const string& filename)
{
	char* data = readFile(filename);

	const char* code = data;
	GLuint ret = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(ret, 1, &code, NULL);
	glCompileShader(ret);

	releaseFile(data);

	return ret;
}

GLuint Shader::loadGeometryShader(const string& filename)
{
	char* data = readFile(filename);

	const char* code = data;
	GLuint ret = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(ret, 1, &code, NULL);
	glCompileShader(ret);

	releaseFile(data);

	return ret;
}

void Shader::printLog(GLuint obj)
{
	int infologLength = 0;
	char infoLog[1024];
	
	if (glIsShader(obj))
		glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);
	
	if (infologLength > 0)
		printf("%s\n", infoLog);
}


Shader::Shader(const std::string& vert, const std::string& frag):
	program(0),
	vertex(0),
	geometry(0),
	fragment(0),
	started(false)
{
	assert(!vert.empty());
	assert(!frag.empty());

	vertex = loadVertexShader(vert);
	fragment = loadFragmentShader(frag);

	assert(vertex != 0);
	assert(fragment != 0);

	program = glCreateProgram();
	assert(program != 0);

	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	printLog(program);
}

Shader::Shader(const std::string& vert, const std::string& frag, const std::string& geom, GLint input, GLint output, GLint vertices):
	program(0),
	vertex(0),
	geometry(0),
	fragment(0),
	started(false)
{
	assert(!vert.empty());
	assert(!geom.empty());
	assert(!frag.empty());

	vertex = loadVertexShader(vert);
	geometry = loadGeometryShader(geom);
	fragment = loadFragmentShader(frag);

	assert(vertex != 0);
	assert(fragment != 0);

	program = glCreateProgram();
	assert(program != 0);

	glAttachShader(program, vertex);
	glAttachShader(program, geometry);
	glAttachShader(program, fragment);
	glProgramParameteriEXT(program, GL_GEOMETRY_INPUT_TYPE_EXT, input);
	glProgramParameteriEXT(program, GL_GEOMETRY_OUTPUT_TYPE_EXT, output);
	glProgramParameteriEXT(program, GL_GEOMETRY_VERTICES_OUT_EXT, vertices);
	glLinkProgram(program);

	printLog(program);
}

Shader::~Shader()
{
	if(vertex != 0)
	{
		glDetachShader(program, vertex);
	}
	if(geometry != 0)
	{
		glDetachShader(program, geometry);
	}
	if(fragment != 0)
	{
		glDetachShader(program, fragment);
	}
	
	if(program != 0)
	{
		glDeleteProgram(program);
	}

	if(vertex != 0)
	{
		glDeleteShader(vertex);
	}
	if(geometry != 0)
	{
		glDeleteShader(geometry);
	}
	if(fragment != 0)
	{
		glDeleteShader(fragment);
	}
}

void Shader::set_texture_unit(size_t unit, const std::string& name)
{
	assert(started);
	GLint texture_unit_location = uniform(name);
	if(texture_unit_location == -1)
	{
		cerr << "WARNING: shader has no sampler with name " << name << endl;
	}
	else
	{
		glUniform1i(texture_unit_location, unit);
	}
}

GLint Shader::uniform(const std::string& name)
{
	assert(started);
	auto it = uniform_locations.find(name);
	if(it == uniform_locations.end())
	{
		GLint location = glGetUniformLocation(program, name.c_str());
		uniform_locations.insert(it, {name, location});
		if(location == -1)
		{
			cerr << "WARNING: shader has no uniform with name " << name << endl;
		}
		return location;
	}
	else
	{
		return it->second;
	}
}

GLint Shader::attribute(const std::string& name)
{
	assert(started);
	auto it = uniform_locations.find(name);
	if(it == uniform_locations.end())
	{
		GLint location = glGetAttribLocation(program, name.c_str());
		uniform_locations.insert(it, {name, location});
		if(location == -1)
		{
			cerr << "WARNING: shader has no attribute with name " << name << endl;
		}
		return location;
	}
	else
	{
		return it->second;
	}
}

void Shader::start()
{
	assert(program != 0);
	assert(!started);
	assert(!shaders_in_use);
	++shaders_in_use;
	started = true;
	glUseProgram(program);
}

void Shader::stop()
{
	assert(started);
	assert(shaders_in_use == 1);
	glUseProgram(0);
	--shaders_in_use;
	started = false;
}

GLuint Shader::get_program() const
{
	return program;
}

