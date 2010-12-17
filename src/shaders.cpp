#include "shaders.h"

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

using namespace std;

GLint unit_color_location = -1;
GLint color_index_location = -1;

GLint bones_location = -1;
GLint bone_index_location = -1;
GLint bone_weight_location = -1;
GLint active_location = -1;
GLint model_scale_location = -1;

void Shaders::init()
{
	if(glewInit() == GLEW_OK)
	{
		cerr << "GLEW JIHUU :DD" << endl;
	}
	else
	{
		cerr << "GLEW VITYYY DD:" << endl;
		exit(-1);
	}

	GLint n;
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &n);
	cerr << "Max geometry shader output vertices: " << n << endl;
	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &n);
	cerr << "Max geometry shader output components: " << n << endl;

	shaders["level_program"]    = shared_ptr<Shader>(new Shader("shaders/level.vertex", "shaders/level.fragment"));
	shaders["blur_program1"]    = shared_ptr<Shader>(new Shader("shaders/blur.vertex", "shaders/blur_verticalpass.fragment"));
	shaders["blur_program2"]    = shared_ptr<Shader>(new Shader("shaders/blur.vertex", "shaders/blur_horizontalpass.fragment"));
	shaders["unit_program"]     = shared_ptr<Shader>(new Shader("shaders/unit.vertex", "shaders/unit.fragment"));
	shaders["grass_program"]    = shared_ptr<Shader>(new Shader("shaders/grass.vertex", "shaders/grass.fragment", "shaders/grass.geometry", GL_POINTS, GL_TRIANGLE_STRIP, 3*4));
	shaders["ssao"]             = shared_ptr<Shader>(new Shader("shaders/ssao_simple.vertex", "shaders/ssao_simple.fragment"));
	shaders["particle_program"] = shared_ptr<Shader>(new Shader("shaders/particle.vertex", "shaders/particle.fragment", "shaders/particle.geometry", GL_POINTS, GL_TRIANGLE_STRIP, 4));

	shaders["deferred_lights_program"] = shared_ptr<Shader>(new Shader("shaders/deferred_lights.vertex", "shaders/deferred_lights.fragment"));
	shaders["deferred_level_program"]    = shared_ptr<Shader>(new Shader("shaders/deferred_level.vertex", "shaders/deferred_level.fragment"));

//	shaders["partitioned_deferred_lights_program"] = shared_ptr<Shader>(new Shader("shaders/partitioned_deferred_lights.vertex", "shaders/partitioned_deferred_lights.fragment", "shaders/particle.geometry", GL_POINTS, GL_TRIANGLE_STRIP, 4));
	shaders["partitioned_deferred_lights_program"] = shared_ptr<Shader>(new Shader("shaders/partitioned_deferred_lights.vertex", "shaders/partitioned_deferred_lights.fragment"));
	shaders["deferred_ambientlight_program"] = shared_ptr<Shader>(new Shader("shaders/deferred_lights.vertex", "shaders/deferred_ambientlight.fragment"));

	shaders["deferred_ambientlight_program"]->start();
	shaders["deferred_ambientlight_program"]->set_texture_unit(0, "texture_colors");
	shaders["deferred_ambientlight_program"]->stop();

	shaders["partitioned_deferred_lights_program"]->start();
	shaders["partitioned_deferred_lights_program"]->set_texture_unit(0, "texture_colors");
	shaders["partitioned_deferred_lights_program"]->set_texture_unit(1, "normals");
	shaders["partitioned_deferred_lights_program"]->set_texture_unit(2, "positions");
	shaders["partitioned_deferred_lights_program"]->stop();

	shaders["deferred_lights_program"]->start();
	shaders["deferred_lights_program"]->set_texture_unit(0, "texture_colors");
	shaders["deferred_lights_program"]->set_texture_unit(1, "normals");
	shaders["deferred_lights_program"]->set_texture_unit(2, "positions");
	uniform_locations["deferred_lights_ambientLight"] = shaders["deferred_lights_program"]->uniform("ambientLight");
	uniform_locations["deferred_lights_activeLights"] = shaders["deferred_lights_program"]->uniform("activeLights");
	int MAX_NUM_LIGHTS = 71;
	for(int i = 0; i < MAX_NUM_LIGHTS*2; ++i)
	{
		stringstream ss;
		ss << i;
		uniform_locations["deferred_lights[" + ss.str() + "]"] = shaders["deferred_lights_program"]->uniform(("lights[" + ss.str() + "]").c_str());
	}
	shaders["deferred_lights_program"]->stop();

	shaders["deferred_level_program"]->start();
	shaders["deferred_level_program"]->set_texture_unit(0, "baseMap0");
	shaders["deferred_level_program"]->set_texture_unit(1, "baseMap1");
	shaders["deferred_level_program"]->set_texture_unit(2, "baseMap2");
	shaders["deferred_level_program"]->set_texture_unit(3, "baseMap3");
	shaders["deferred_level_program"]->stop();


	shaders["particle_program"]->start();
	uniform_locations["particle_screen_width"] = shaders["particle_program"]->uniform("screen_width");
	uniform_locations["particle_screen_height"] = shaders["particle_program"]->uniform("screen_height");
	uniform_locations["particle_particleScale"] = shaders["particle_program"]->attribute("particleScale");
	shaders["particle_program"]->set_texture_unit(0, "particleTexture");
	shaders["particle_program"]->set_texture_unit(1, "depthTexture");
	shaders["particle_program"]->stop();
	
	shaders["ssao"]->start();
	uniform_locations["ssao_power"] = shaders["ssao"]->uniform("power");
	shaders["ssao"]->set_texture_unit(0, "imageTexture");
	shaders["ssao"]->set_texture_unit(1, "depthTexture");
	uniform_locations["ssao_height"] = shaders["ssao"]->uniform("screen_height");
	uniform_locations["ssao_width"]  = shaders["ssao"]->uniform("screen_width");
	shaders["ssao"]->stop();

	shaders["level_program"]->start();
	shaders["level_program"]->set_texture_unit(0, "baseMap0");
	shaders["level_program"]->set_texture_unit(1, "baseMap1");
	shaders["level_program"]->set_texture_unit(2, "baseMap2");
	shaders["level_program"]->set_texture_unit(3, "baseMap3");
	uniform_locations["lvl_ambientLight"] = shaders["level_program"]->uniform("ambientLight");
	uniform_locations["lvl_activeLights"] = shaders["level_program"]->uniform("activeLights");
	for(int i = 0; i < MAX_NUM_LIGHTS*2; ++i)
	{
		stringstream ss;
		ss << i;
		uniform_locations["lvl_lights[" + ss.str() + "]"] = shaders["level_program"]->uniform(("lights[" + ss.str() + "]").c_str());
	}
	shaders["level_program"]->stop();
	
	shaders["unit_program"]->start();
	unit_color_location = shaders["unit_program"]->uniform("unit_color");
	bones_location = shaders["unit_program"]->uniform("bones");
	active_location = shaders["unit_program"]->uniform("active");
	model_scale_location = shaders["unit_program"]->uniform("scale");
	color_index_location = shaders["unit_program"]->attribute("color_index");
	bone_weight_location = shaders["unit_program"]->attribute("bone_weight");
	bone_index_location = shaders["unit_program"]->attribute("bone_index");
	shaders["unit_program"]->stop();

	shaders["blur_program1"]->start();
	uniform_locations["blur_amount1"] = shaders["blur_program1"]->uniform("amount");
	shaders["blur_program1"]->stop();

	shaders["blur_program2"]->start();
	uniform_locations["blur_amount2"] = shaders["blur_program2"]->uniform("amount");
	shaders["blur_program2"]->stop();
	
	shaders["grass_program"]->start();
	uniform_locations["grass_texture"] = shaders["grass_program"]->uniform("texture");
	uniform_locations["grass_wind"] = shaders["grass_program"]->uniform("wind");
	uniform_locations["grass_scale"] = shaders["grass_program"]->uniform("scale");
	shaders["grass_program"]->stop();

	for(auto it = uniform_locations.begin(); it != uniform_locations.end(); ++it)
	{
		if(it->second == -1)
		{
			cerr << "WARNING: shader variable " << it->first << " not in use" << endl;
		}
	}
}

void Shaders::release()
{
	shaders.clear();
}

GLint Shaders::uniform(const string& name) const
{
	auto it = uniform_locations.find(name);
	if(it == uniform_locations.end())
		return -1;
	else
		return it->second;
}

GLuint Shaders::operator[](const string& program_name) const
{
	auto it = shaders.find(program_name);
	if(it == shaders.end())
	{
		cerr << "WARNING: program " << program_name << " not found!" << endl;
		return -1;
	}
	else
	{
		return it->second->get_program();
	}
}

Shader& Shaders::get_shader(const std::string& program_name)
{
	auto it = shaders.find(program_name);
	if(it == shaders.end())
	{
		cerr << "WARNING: program " << program_name << " not found!" << endl;
		throw std::logic_error("get_shader(" + program_name + ") failed!");
	}
	else
	{
		return *it->second;
	}
}

