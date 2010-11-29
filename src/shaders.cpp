#include "shaders.h"
#include "graphics.h"

#include <iostream>
#include <sstream>
#include <string>

GLint unit_color_location = -1;
GLint color_index_location = -1;

GLint bones_location = -1;
GLint bone_index_location = -1;
GLint bone_weight_location = -1;
GLint active_location = -1;

char* readFile(const char *path)
{
	FILE *fd;
	long len,
	r;
	char *str;
	
	if (!(fd = fopen(path, "r")))
	{
		fprintf(stderr, "Can't open file '%s' for reading\n", path);
		return 0;
	}
	
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	
	printf("File '%s' is %ld long\n", path, len);
	
	fseek(fd, 0, SEEK_SET);
	
	str = new char[len+1];
	r = fread(str, 1, len, fd);
	
	str[r] = '\0';
	fclose(fd);
	
	return str;
}

void releaseFile(char* file)
{
	delete[] file;
}

void Graphics::loadVertexShader(const std::string& name, const std::string& filename)
{
	char* data = readFile(filename.c_str());
	
	const char* code = data;
	shaders[name] = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shaders[name], 1, &code, NULL);
	glCompileShader(shaders[name]);
	printLog(shaders[name]);

	releaseFile(data);
}

void Graphics::loadFragmentShader(const std::string& name, const std::string& filename)
{
	char* data = readFile(filename.c_str());

	const char* code = data;
	shaders[name] = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shaders[name], 1, &code, NULL);
	glCompileShader(shaders[name]);
	printLog(shaders[name]);

	releaseFile(data);
}

void Graphics::loadGeometryShader(const std::string& name, const std::string& filename)
{
	char* data = readFile(filename.c_str());

	const char* code = data;
	shaders[name] = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(shaders[name], 1, &code, NULL);
	glCompileShader(shaders[name]);
	printLog(shaders[name]);

	releaseFile(data);
}

void printLog(GLuint obj)
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

void Graphics::initShaders()
{
	if(glewInit() == GLEW_OK)
		std::cerr << "GLEW JIHUU :DD" << std::endl;
	else
		std::cerr << "GLEW VITYYY DD:" << std::endl;
	
	glGetIntegerv(GL_MAX_LIGHTS, &MAX_NUM_LIGHTS);
	std::cerr << "OpenGL MAX_LIGHTS: " << MAX_NUM_LIGHTS << std::endl;
	std::cerr << std::endl;

	loadFragmentShader("level_frag", "shaders/level.fragment");
	loadVertexShader("level_vert", "shaders/level.vertex");
//	loadGeometryShader("level_geom", "shaders/level.geometry");
	shaders["level_program"] = glCreateProgram();
	glAttachShader(shaders["level_program"], shaders["level_frag"]);
	glAttachShader(shaders["level_program"], shaders["level_vert"]);
//	glAttachShader(shaders["level_program"], shaders["level_geom"]);
//	glProgramParameteriEXT(shaders["level_program"], GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
//	glProgramParameteriEXT(shaders["level_program"], GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
//	glProgramParameteriEXT(shaders["level_program"], GL_GEOMETRY_VERTICES_OUT_EXT, 3);
	glLinkProgram(shaders["level_program"]);
	printLog(shaders["level_program"]);

	loadFragmentShader("blur_frag", "shaders/blur_verticalpass.fragment");
	loadVertexShader("blur_vert", "shaders/blur.vertex");
	shaders["blur_program"] = glCreateProgram();
	glAttachShader(shaders["blur_program"], shaders["blur_frag"]);
	glAttachShader(shaders["blur_program"], shaders["blur_vert"]);
	glLinkProgram(shaders["blur_program"]);
	printLog(shaders["blur_program"]);
	
	loadFragmentShader("unit_frag", "shaders/unit.fragment");
	loadVertexShader("unit_vert", "shaders/unit.vertex");
	shaders["unit_program"] = glCreateProgram();
	glAttachShader(shaders["unit_program"], shaders["unit_frag"]);
	glAttachShader(shaders["unit_program"], shaders["unit_vert"]);
	glLinkProgram(shaders["unit_program"]);
	printLog(shaders["unit_program"]);


	loadFragmentShader("grass_frag", "shaders/grass.fragment");
	loadVertexShader("grass_vert", "shaders/grass.vertex");
	loadGeometryShader("grass_geom", "shaders/grass.geometry");
	shaders["grass_program"] = glCreateProgram();
	glAttachShader(shaders["grass_program"], shaders["grass_frag"]);
	glAttachShader(shaders["grass_program"], shaders["grass_vert"]);
	glAttachShader(shaders["grass_program"], shaders["grass_geom"]);
	glProgramParameteriEXT(shaders["grass_program"], GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
	glProgramParameteriEXT(shaders["grass_program"], GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLES);
	glProgramParameteriEXT(shaders["grass_program"], GL_GEOMETRY_VERTICES_OUT_EXT, 3 * 2 * 3);
	glLinkProgram(shaders["grass_program"]);
	printLog(shaders["grass_program"]);



	loadFragmentShader("particle_frag", "shaders/particle.fragment");
	loadVertexShader("particle_vert", "shaders/particle.vertex");
	loadGeometryShader("particle_geom", "shaders/particle.geometry");
	shaders["particle_program"] = glCreateProgram();
	glAttachShader(shaders["particle_program"], shaders["particle_frag"]);
	glAttachShader(shaders["particle_program"], shaders["particle_vert"]);
	glAttachShader(shaders["particle_program"], shaders["particle_geom"]);
	glProgramParameteriEXT(shaders["particle_program"], GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
	glProgramParameteriEXT(shaders["particle_program"], GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLES);
	glProgramParameteriEXT(shaders["particle_program"], GL_GEOMETRY_VERTICES_OUT_EXT, 2 * 3);
	glLinkProgram(shaders["particle_program"]);
	printLog(shaders["particle_program"]);

	glUseProgram(shaders["particle_program"]);
	uniform_locations["particle_particleTexture"] = glGetUniformLocation(shaders["particle_program"], "particleTexture");
	uniform_locations["particle_depthTexture"] = glGetUniformLocation(shaders["particle_program"], "depthTexture");
	uniform_locations["particle_particleScale"] = glGetAttribLocation(shaders["particle_program"], "particleScale");
	glUniform1i(uniform_locations["particle_particleTexture"], 0);
	glUniform1i(uniform_locations["particle_depthTexture"], 1);
	
	glUseProgram(shaders["level_program"]);
	uniform_locations["lvl_baseMap0"] = glGetUniformLocation(shaders["level_program"], "baseMap0");
	uniform_locations["lvl_baseMap1"] = glGetUniformLocation(shaders["level_program"], "baseMap1");
	uniform_locations["lvl_baseMap2"] = glGetUniformLocation(shaders["level_program"], "baseMap2");
	uniform_locations["lvl_baseMap3"] = glGetUniformLocation(shaders["level_program"], "baseMap3");
	glUniform1i(uniform_locations["lvl_baseMap0"], 0);
	glUniform1i(uniform_locations["lvl_baseMap1"], 1);
	glUniform1i(uniform_locations["lvl_baseMap2"], 2);
	glUniform1i(uniform_locations["lvl_baseMap3"], 3);
	uniform_locations["lvl_ambientLight"] = glGetUniformLocation(shaders["level_program"], "ambientLight");
	uniform_locations["lvl_activeLights"] = glGetUniformLocation(shaders["level_program"], "activeLights");
	MAX_NUM_LIGHTS = 71;
	MAX_NUM_ACTIVE_LIGHTS = 1; // Make sure this is the same number as in the shaders.
	for(int i = 0; i < MAX_NUM_LIGHTS*2; ++i)
	{
		std::stringstream ss;
		ss << i;
		uniform_locations["lvl_lights[" + ss.str() + "]"] = glGetUniformLocation(shaders["level_program"], ("lights[" + ss.str() + "]").c_str());
	}
	
	glUseProgram(shaders["unit_program"]);
	unit_color_location = glGetUniformLocation(shaders["unit_program"], "unit_color" );
	bones_location = glGetUniformLocation(shaders["unit_program"], "bones" );
	active_location = glGetUniformLocation(shaders["unit_program"], "active" );
	color_index_location = glGetAttribLocation(shaders["unit_program"], "color_index" );
	bone_weight_location = glGetAttribLocation(shaders["unit_program"], "bone_weight" );
	bone_index_location = glGetAttribLocation(shaders["unit_program"], "bone_index" );


	glUseProgram(shaders["grass_program"]);
	uniform_locations["grass_texture"] = glGetUniformLocation(shaders["grass_program"], "texture");
	uniform_locations["grass_wind"] = glGetAttribLocation(shaders["grass_program"], "wind");
	uniform_locations["grass_scale"] = glGetAttribLocation(shaders["grass_program"], "scale");
	glUniform1i(uniform_locations["grass_texture"], 0);


	glUseProgram(0);

	for(auto it = uniform_locations.begin(); it != uniform_locations.end(); ++it)
	{
		if(it->first.size() < 11 || it->first.substr(0, 11) != "lvl_lights[")
		{
			std::cerr << it->first << ": "  << it->second << std::endl;
		}
	}
	
	std::cerr << std::endl;
	std::cerr << "Unit: unit_color location: " << unit_color_location << std::endl;
	std::cerr << "Unit: color_index location: " << color_index_location << std::endl;
	std::cerr << "Unit: bones location: " << bones_location << std::endl;
	std::cerr << "Unit: bone_index location: " << bone_index_location << std::endl;
	std::cerr << "Unit: bone_weight location: " << bone_weight_location << std::endl;
	std::cerr << "Unit: active location: " << active_location << std::endl;
	std::cerr << std::endl;
}

void Graphics::releaseShaders()
{
	glUseProgram(0);
	glDetachShader(shaders["unit_program"], shaders["unit_frag"]);
	glDetachShader(shaders["unit_program"], shaders["unit_frag"]);
	glDetachShader(shaders["level_program"], shaders["level_vert"]);
	glDetachShader(shaders["level_program"], shaders["level_frag"]);
//	glDetachShader(shaders["level_program"], shaders["level_geom"]);

	glDeleteProgram(shaders["unit_program"]);
	glDeleteProgram(shaders["level_program"]);

	glDeleteShader(shaders["unit_frag"]);
	glDeleteShader(shaders["unit_vert"]);
	glDeleteShader(shaders["level_frag"]);
	glDeleteShader(shaders["level_vert"]);
//	glDeleteShader(shaders["level_geom"]);
}

