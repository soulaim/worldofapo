#ifndef EDITOR_H
#define EDITOR_H

#include "../model.h"
#include "../userio.h"
#include "../graphics.h"
#include "../unit.h"

#include <string>
#include <vector>
#include <queue>
#include <map>


class Editor
{
	UserIO userio;
	Graphics view;

	void init();
	
	void handle_input();
	void tick();
	void loadObjects(const std::string& file);
	void loadModel(const std::string& file);
	void saveModel(const std::string& file);
	void saveObjects(const std::string& file);
	bool type_exists(const std::string& type);
	void handle_command(const std::string& command);
	void select_part(const std::string& part);
	void move_part(double dx, double dy, double dz);
	void rotate_part(double dx, double dy, double dz);
	void remove_part();
	void add_part(const std::string& part_name, const std::string& type);
	void edit_model();
	void type_helper(const std::string& type);
	void edit_type(const std::string& type);
	void add_type(const std::string& type);
	void print_types();
	void print_model();

	Unit dummy;

	std::string objectsName;
	std::string modelName;

	bool editing_single_part;
	size_t selected_part;

	Model edited_model;
	ObjectPart* edited_part;
	Model stored_model;

	float speed;
	float rotate_speed;

	std::vector<std::string> commands;
	size_t current_command;
public:
	Editor();
	void start();
};


#endif

