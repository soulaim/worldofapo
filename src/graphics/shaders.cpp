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
GLint model_scale_location = -1;

void Shaders::init()
{
	GLint n;
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &n);
	cerr << "Max geometry shader output vertices: " << n << endl;
	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &n);
	cerr << "Max geometry shader output components: " << n << endl;

	shaders["level_program"]    = shared_ptr<Shader>(new Shader("shaders/level.vertex", "shaders/level.fragment"));
	shaders["blur_program1"]    = shared_ptr<Shader>(new Shader("shaders/blur.vertex", "shaders/blur_verticalpass.fragment"));
	shaders["blur_program2"]    = shared_ptr<Shader>(new Shader("shaders/blur.vertex", "shaders/blur_horizontalpass.fragment"));
	shaders["unit_program"]     = shared_ptr<Shader>(new Shader("shaders/unit.vertex", "shaders/unit.fragment"));
	shaders["ssao_program"]     = shared_ptr<Shader>(new Shader("shaders/ssao_simple.vertex", "shaders/ssao_simple.fragment"));

    OpenGL gl;
    if(gl.getGL3bit()) {
        cout << "building with geometry shaders" << endl;
        shaders["grass_program"]    = shared_ptr<Shader>(new Shader("shaders/grass.vertex", "shaders/grass.fragment", "shaders/grass.geometry", GL_POINTS, GL_TRIANGLE_STRIP, 3*4));
        shaders["particle_program"] = shared_ptr<Shader>(new Shader("shaders/particle.vertex", "shaders/particle.fragment", "shaders/particle.geometry", GL_POINTS, GL_TRIANGLE_STRIP, 4));
    } else {
        cout << "building without geometry shaders." << endl;
        shaders["grass_program"]    = shared_ptr<Shader>(new Shader("shaders/grass.vertex", "shaders/grass.fragment"));
        shaders["particle_program"] = shared_ptr<Shader>(new Shader("shaders/particle.vertex", "shaders/particle.fragment"));
    }

    shaders["partitioned_deferred_lights_program"] = shared_ptr<Shader>(new Shader("shaders/partitioned_deferred_lights.vertex", "shaders/partitioned_deferred_lights.fragment"));

	shaders["deferred_lights_program"] = shared_ptr<Shader>(new Shader("shaders/fullscreenquad.vertex", "shaders/deferred_lights.fragment"));
	shaders["deferred_level_program"]    = shared_ptr<Shader>(new Shader("shaders/deferred_level.vertex", "shaders/deferred_level.fragment"));

	shaders["partitioned_deferred_lights_program2"] = shared_ptr<Shader>(new Shader("shaders/fullscreenquad.vertex", "shaders/partitioned_deferred_lights.fragment"));
	shaders["deferred_ambientlight_program"] = shared_ptr<Shader>(new Shader("shaders/fullscreenquad.vertex", "shaders/deferred_ambientlight.fragment"));
	shaders["skybox_program"] = shared_ptr<Shader>(new Shader("shaders/skybox.vertex", "shaders/skybox.fragment"));


	shaders["deferred_ambientlight_program"]->start();
	shaders["deferred_ambientlight_program"]->set_texture_unit(0, "texture_colors");
	shaders["deferred_ambientlight_program"]->set_texture_unit(1, "normals");
	shaders["deferred_ambientlight_program"]->stop();


    shaders["partitioned_deferred_lights_program"]->start();
    shaders["partitioned_deferred_lights_program"]->set_texture_unit(0, "texture_colors");
    shaders["partitioned_deferred_lights_program"]->set_texture_unit(1, "normals");
    // shaders["partitioned_deferred_lights_program"]->set_texture_unit(2, "positions");
    shaders["partitioned_deferred_lights_program"]->set_texture_unit(3, "depthTexture");
    shaders["partitioned_deferred_lights_program"]->stop();


    shaders["particle_program"]->start();
    shaders["particle_program"]->set_texture_unit(0, "particleTexture");
    shaders["particle_program"]->set_texture_unit(1, "depthTexture");
    shaders["particle_program"]->stop();


	shaders["partitioned_deferred_lights_program2"]->start();
	shaders["partitioned_deferred_lights_program2"]->set_texture_unit(0, "texture_colors");
	shaders["partitioned_deferred_lights_program2"]->set_texture_unit(1, "normals");
//	shaders["partitioned_deferred_lights_program2"]->set_texture_unit(2, "positions");
	shaders["partitioned_deferred_lights_program2"]->set_texture_unit(3, "depthTexture");
	shaders["partitioned_deferred_lights_program2"]->stop();

	shaders["deferred_lights_program"]->start();
	shaders["deferred_lights_program"]->set_texture_unit(0, "texture_colors");
	shaders["deferred_lights_program"]->set_texture_unit(1, "normals");
//	shaders["deferred_lights_program"]->set_texture_unit(2, "positions");
	shaders["deferred_lights_program"]->set_texture_unit(3, "depthTexture");
	shaders["deferred_lights_program"]->stop();

	shaders["deferred_level_program"]->start();
	shaders["deferred_level_program"]->set_texture_unit(0, "baseMap0");
	shaders["deferred_level_program"]->set_texture_unit(1, "baseMap1");
	shaders["deferred_level_program"]->set_texture_unit(2, "baseMap2");
	shaders["deferred_level_program"]->set_texture_unit(3, "baseMap3");
	shaders["deferred_level_program"]->stop();

	shaders["ssao_program"]->start();
	shaders["ssao_program"]->set_texture_unit(0, "imageTexture");
	shaders["ssao_program"]->set_texture_unit(1, "depthTexture");
	shaders["ssao_program"]->stop();

	shaders["level_program"]->start();
	shaders["level_program"]->set_texture_unit(0, "baseMap0");
	shaders["level_program"]->set_texture_unit(1, "baseMap1");
	shaders["level_program"]->set_texture_unit(2, "baseMap2");
	shaders["level_program"]->set_texture_unit(3, "baseMap3");
	shaders["level_program"]->stop();

	shaders["unit_program"]->start();
	unit_color_location = shaders["unit_program"]->uniform("unit_color");
	bones_location = shaders["unit_program"]->uniform("bones");
	model_scale_location = shaders["unit_program"]->uniform("scale");
	color_index_location = shaders["unit_program"]->attribute("color_index");
	bone_weight_location = shaders["unit_program"]->attribute("bone_weight");
	bone_index_location = shaders["unit_program"]->attribute("bone_index");
	shaders["unit_program"]->stop();
}

void Shaders::release()
{
	shaders.clear();
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

