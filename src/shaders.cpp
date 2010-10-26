#include "shaders.h"
#include "graphics.h"

#include <iostream>

GLint unit_color_location = -1;
GLint color_index_location = -1;

GLint bones_location = -1;
GLint bone_index_location = -1;
GLint bone_weight_location = -1;

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
		cerr << "GLEW JIHUU :DD" << endl;
	else
		cerr << "GLEW VITYYY DD:" << endl;
	
	loadFragmentShader("level_frag", "shaders/level.fragment");
	loadVertexShader("level_vert", "shaders/level.vertex");
	
	loadFragmentShader("unit_frag", "shaders/unit.fragment");
	loadVertexShader("unit_vert", "shaders/unit.vertex");
	
	shaders["level_program"] = glCreateProgram();
	glAttachShader(shaders["level_program"], shaders["level_frag"]);
	glAttachShader(shaders["level_program"], shaders["level_vert"]);
	glLinkProgram(shaders["level_program"]);
	printLog(shaders["level_program"]);
	
	shaders["unit_program"] = glCreateProgram();
	glAttachShader(shaders["unit_program"], shaders["unit_frag"]);
	glAttachShader(shaders["unit_program"], shaders["unit_vert"]);
	glLinkProgram(shaders["unit_program"]);
	printLog(shaders["unit_program"]);

	glUseProgram(shaders["unit_program"]);
	unit_color_location = glGetUniformLocation(shaders["unit_program"], "unit_color" );
	bones_location = glGetUniformLocation(shaders["unit_program"], "bones" );

	color_index_location = glGetAttribLocation(shaders["unit_program"], "color_index" );
	bone_weight_location = glGetAttribLocation(shaders["unit_program"], "bone_weight" );
	bone_index_location = glGetAttribLocation(shaders["unit_program"], "bone_index" );
	glUseProgram(0);

	cerr << "unit_color location: " << unit_color_location << endl;
	cerr << "color_index location: " << color_index_location << endl;
	cerr << "bones location: " << bones_location << endl;
	cerr << "bone_index location: " << bone_index_location << endl;
	cerr << "bone_weight location: " << bone_weight_location << endl;
}

