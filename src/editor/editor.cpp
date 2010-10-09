#include "editor.h"
#include "../texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <climits>

using namespace std;

#define RED 255,0,0
#define WHITE 0,0,0
#define GREEN 0,255,0

Editor::Editor()
{
	init();
}

void Editor::init()
{
	editing_single_part = false;
	selected_part = 0;
	userio.init();

//	TextureHandler::getSingleton().createTexture("grass", "data/grass.png");
//	TextureHandler::getSingleton().createTexture("highground", "data/highground.png");
//	TextureHandler::getSingleton().createTexture("mountain", "data/hill.png");

	view.bindCamera(&dummy);

	handle_command("load objects parts.dat");
	handle_command("load model model.bones");

//	view.megaFuck();
}

void Editor::start()
{
	int ticks = SDL_GetTicks();
	static int last_tick = -999999;

	int time_since_last = ticks - last_tick;
	int time_between_ticks = 1000/60;
	if(time_since_last >= time_between_ticks)
	{
		last_tick = ticks;

		tick();
		view.setTime( ticks );
		view.tick();


		stringstream ss;
		if(models[0].parts.size() > selected_part)
		{
			const ModelNode& node = models[0].parts[selected_part];
			ss << "'" << node.name << "' is " << node.wireframe << " at ("
				<< node.offset_x << ", " << node.offset_y << ", " << node.offset_z << ")";
		}
		view.draw(models, ss.str());
	}
	else
	{
		SDL_Delay(time_between_ticks - time_since_last);
	}
}

void Editor::tick()
{
	handle_input();

//	cerr << view.camera.getPosition() << "\n";
}

void Editor::saveModel(const string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Saving model to '" + pathed_file + "'", WHITE);
	if(models[0].save(pathed_file))
	{
		view.pushMessage("Success", GREEN);
		objectsName = file;
	}
	else
	{
		view.pushMessage("Fail", RED);
	}
}

void Editor::saveObjects(const string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Saving objects to '" + pathed_file + "'", WHITE);
	if(view.saveObjects(pathed_file))
	{
		view.pushMessage("Success", GREEN);
		objectsName = file;
	}
	else
	{
		view.pushMessage("Fail", RED);
	}
}

void Editor::loadObjects(const string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Loading objects from '" + pathed_file + "'", WHITE);
	if(view.loadObjects(pathed_file))
	{
		view.pushMessage("Success", GREEN);
		objectsName = file;
		selected_part = 0;
		editing_single_part = false;
	}
	else
	{
		view.pushMessage("Fail", RED);
	}
}

void Editor::loadModel(const string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Loading objects from '" + pathed_file + "'", WHITE);

	Model model;
	bool ok = model.load(pathed_file);
	if(ok)
	{
		selected_part = 0;
		editing_single_part = false;
		models[0] = model;
		view.pushMessage("Success", GREEN);
		modelName = file;
	}
	else
	{
		view.pushMessage("Fail", RED);
	}
}

void Editor::move_part(double dx, double dy, double dz)
{
	if(models[0].parts.size() <= selected_part)
	{
		view.pushMessage("Failed to move part, no selected part", WHITE);
		return;
	}

	ModelNode modelnode = models[0].parts[selected_part];
	modelnode.offset_x += dx;
	modelnode.offset_y += dy;
	modelnode.offset_z += dz;
}

void Editor::select_part(const string& part)
{
	for(size_t i = 0; i < models[0].parts.size(); ++i)
	{
		if(models[0].parts[i].name == part)
		{
			selected_part = i;
			view.pushMessage("Selected part " + part, GREEN);
			return;
		}
	}
	view.pushMessage("Failed to select part " + part, RED);
}

void Editor::remove_part()
{
	if(models[0].parts.size() <= selected_part)
	{
		view.pushMessage("Failed to remove part, no selected part", WHITE);
		return;
	}
	queue<size_t> to_be_removed;
	vector<bool> removed;
	vector<size_t> new_indices;
	removed.resize(models[0].parts.size(), 0);
	new_indices.resize(models[0].parts.size(), UINT_MAX);

	// Find all removed indices.
	to_be_removed.push(selected_part);
	while(!to_be_removed.empty())
	{
		size_t current = to_be_removed.front();
		to_be_removed.pop();
		removed[current] = true;
		for(size_t i = 0; i < models[0].parts[current].children.size(); ++i)
		{
			to_be_removed.push(models[0].parts[current].children[i]);
		}
	}

	// Do actual moving and resizing. Store new indices.
	size_t next = 0;
	for(size_t i = 0; i < models[0].parts.size(); ++i)
	{
		if(!removed[i])
		{
			std::swap(models[0].parts[i], models[0].parts[next]);
			new_indices[i] = next;
			++next;
		}
	}
	models[0].parts.resize(next - 1);

	// Update children's indices.
	for(size_t i = 0; i < models[0].parts.size(); ++i)
	{
		for(size_t j = 0; j < models[0].parts[i].children.size(); ++j)
		{
			size_t& child = models[0].parts[i].children[j];
			child = new_indices[child];
			if(child == UINT_MAX)
			{
				models[0].parts[i].children.erase(models[0].parts[i].children.begin() + j);
				--j;
			}
		}
	}
}

void Editor::add_part(const std::string& part_name, const std::string& part_type)
{
	if(!models[0].parts.empty() && selected_part >= models[0].parts.size())
	{
		view.pushMessage("Add part failed, select part first");
		return;
	}
	ModelNode new_node;
	new_node.name = part_name;
	new_node.wireframe = part_type;
	new_node.offset_x = 0.0f;
	new_node.offset_y = 0.0f;
	new_node.offset_z = 0.0f;
	models[0].parts.push_back(new_node);
	if(!models[0].parts.empty())
	{
		ModelNode& selected_node = models[0].parts[selected_part];
		selected_node.children.push_back(models[0].parts.size()-1);
		view.pushMessage("Added new part '" + part_name + "' of type '" + part_type + "' as child of '" + selected_node.name, GREEN);
	}
	else
	{
		view.pushMessage("Added new part '" + part_name + "' of type '" + part_type + "' as root", GREEN);
	}

}

void Editor::handle_command(const string& command)
{
	view.pushMessage(command, WHITE);
	stringstream ss(command);

	string first_word;
	string second_word;
	string third_word;
	ss >> first_word;


	if(first_word == "load")
	{
		ss >> second_word;
		ss >> third_word;
		if(second_word == "objects")
		{
			loadObjects(third_word);
		}
		else if(second_word == "model")
		{
			loadModel(third_word);
		}
	}
	else if(first_word == "move")
	{
		double dx = 0.0;
		double dy = 0.0;
		double dz = 0.0;
		ss >> dx >> dy >> dz;
		move_part(dx,dy,dz);
	}
	else if(first_word == "select")
	{
		ss >> second_word;
		select_part(second_word);
	}
	else if(first_word == "add")
	{
		ss >> second_word;
		ss >> third_word;
		add_part(second_word, third_word);
	}
	else if(first_word == "remove")
	{
		remove_part();
	}
}

void Editor::handle_input()
{
	string key = userio.getSingleKey();

	static string clientCommand = "";
	static bool writing = false;
	static bool grabbed = false;

	if(key.size() != 0)
	{
		if(key == "f4")
		{
			loadObjects(objectsName);
			loadModel(modelName);
		}
		if(key == "f11")
		{
			view.toggleFullscreen();
		}
		if(key == "f10")
		{
			view.toggleLightingStatus();
		}
		if(key == "f12")
		{
			if(grabbed)
			{
				grabbed = false;
				SDL_WM_GrabInput(SDL_GRAB_OFF);
				SDL_ShowCursor(1);
			}
			else
			{
				grabbed = true;
				SDL_WM_GrabInput(SDL_GRAB_ON);
				SDL_ShowCursor(0);
			}
		}
		
		if(writing)
		{
			string nick = ">";
			
			if(key.size() == 1)
			{
				clientCommand.append(key);
			}
			else if(key == "backspace" && clientCommand.size() > 0)
			{
				clientCommand.resize(clientCommand.size()-1);
			}
			else if(key == "space")
			{
				clientCommand.append(" ");
			}


			if(key == "return")
			{
				writing = false;
				if(clientCommand.size() > 0)
				{
					handle_command(clientCommand);
				}

				clientCommand = "";
				view.setCurrentClientCommand(clientCommand);
			}
			else if(key == "escape")
			{
				writing = false;
				clientCommand = "";
				view.setCurrentClientCommand(clientCommand);
			}
			else
			{
				view.setCurrentClientCommand("> " + clientCommand);
			}
		}
		else
		{
			
			if(key == "return") {
				writing = true;
				view.setCurrentClientCommand("> " + clientCommand);
			}

			if(key == "escape")
			{
				cerr << "User pressed ESC, shutting down." << endl;
				SDL_Quit();
				exit(0);
			}
		}
	}

	int keystate = userio.getGameInput();
	int x, y;
	userio.getMouseChange(x, y);
	int wheel_status = userio.getMouseWheelScrolled();
	if(wheel_status == 1)
	{
		view.mouseUp();
	}
	if(wheel_status == 2)
	{
		view.mouseDown();
	}
	view.updateInput(keystate, x, y);
	dummy.updateInput(0, x, y, 0);
}

