#ifndef EDITOR_H
#define EDITOR_H

#include "graphics/model.h"
#include "graphics/skeletalmodel.h"
#include "local_machine/userio.h"
#include "graphics/graphics.h"
#include "world/unit.h"
#include "graphics/hud.h"
#include "graphics/particle.h"
#include "graphics/particle_source.h"
#include "graphics/visualworld.h"
#include "graphics/window.h"

#include <string>
#include <vector>
#include <queue>
#include <map>


class Editor
{
	Window window;
	UserIO userio;
	GameView view;
	Hud hud;

	bool handle_input();
	void loadObjects(const std::string& file);
	void loadModel(const std::string& file);
	void loadSkeletalModel(const std::string& file);
	void loadAnimations(const std::string& file);
	void saveModel(const std::string& file);
	void saveSkeletalModel(const std::string& file);
	void saveObjects(const std::string& file);
	void saveAnimations(const std::string& file);
	bool type_exists(const std::string& type);
	void handle_command(const std::string& command);

	void move_dot(double dx, double dy, double dz);

	void print_model();
	void print_animations();
	void undot();
	void play_animation(const std::string& animation);
	void record_step(size_t time);
	void record_animation(const std::string& animation);
	void reset();
	void scale(float scalar);
	void calculate_nearest_bones();
	void swarm(int X, int Y, int Z);
	void release_swarm();
	void swarm_particles(int X, int Y, int Z);
	void release_particles();
	void swarm_meadows(int X, int Z, int y);
	void reorder_triangles();

	Unit dummy;

	std::string objectsFile;
	std::string modelFile;
	std::string animationsFile;

	bool editing_single_part;
	size_t selected_part;
	
	bool skele;
	SkeletalModel skeletal_model;
	std::map<int, Model*> models;

	float speed;
	float rotate_speed;

	std::vector<std::string> commands;
	size_t current_command;

	size_t selected_dot;
	vec3<float> current_dot;
	std::vector<vec3<float> > new_dots;

	std::string animation_name;

	bool drawing_model;
	bool drawing_skeleton;
	bool skybox;

	bool do_tick();

	VisualWorld visualworld;
public:
	Editor();
	~Editor();
	bool tick();
};


#endif

