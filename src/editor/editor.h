#ifndef EDITOR_H
#define EDITOR_H

#include "../model.h"
#include "../apomodel.h"
#include "../skeletalmodel.h"
#include "../userio.h"
#include "../graphics.h"
#include "../unit.h"
#include "../hud.h"
#include "../particle.h"
#include "../particle_source.h"
#include "../visualworld.h"
#include "../window.h"

#include <string>
#include <vector>
#include <queue>
#include <map>


class Editor
{
	Window window;
	UserIO userio;
	Graphics view;
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
	void select_part(const std::string& part);
	void move_part(double dx, double dy, double dz);
	void move_dot(double dx, double dy, double dz);
	void move(double dx, double dy, double dz);
	void rotate_part(double dx, double dy, double dz);
	void remove_part();
	void add_part(const std::string& part_name, const std::string& type);
	void edit_model();
	void type_helper(const std::string& type);
	void edit_type(const std::string& type);
	void add_type(const std::string& type);
	void print_types();
	void print_model();
	void print_animations();
	void dot();
	void undot();
	void next_dot();
	void prev_dot();
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

	ApoModel edited_model;
	std::string edited_type_name;
	ObjectPart* edited_type;
	ApoModel stored_model;

	bool skele;
	SkeletalModel skeletal_model;
	std::map<int, Model*> models;

	float speed;
	float rotate_speed;

	std::vector<std::string> commands;
	size_t current_command;

	size_t selected_dot;
	Vec3 current_dot;
	std::vector<Vec3> new_dots;

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

