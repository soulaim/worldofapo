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
	void handle_command(const std::string& command);
	void select_part(const std::string& part);
	void move_part(double dx, double dy, double dz);

	std::map<int, Model> models;
	Unit dummy;

	std::string objectsName;
	std::string modelName;

	bool editing_single_part;
	size_t selected_part;
public:
	Editor();
	void start();
};


#endif

