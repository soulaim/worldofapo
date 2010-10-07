#ifndef EDITOR_H
#define EDITOR_H

#include "../model.h"
#include "../userio.h"
#include "../graphics.h"

#include <string>
#include <vector>
#include <queue>
#include <map>


class Editor
{
	UserIO userio;
	Graphics view;

	void init();

	void tick();

	std::map<int, Model> models;
public:
	Editor();
	void start();
};


#endif

