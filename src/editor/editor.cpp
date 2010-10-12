#include "editor.h"
#include "../texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <climits>
#include <locale>
#include <iomanip>

using namespace std;

std::string red(const std::string& s)
{
	return "^r" + s;
}

std::string green(const std::string& s)
{
	return "^g" + s;
}

Editor::Editor()
{
	init();
}

void Editor::init()
{
	speed = 0.1f;
	rotate_speed = 45.0f/2.0f;
	current_command = 0;

	editing_single_part = false;
	edited_part = 0;
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
		if(edited_model.parts.size() > selected_part)
		{
			const ModelNode& node = edited_model.parts[selected_part];
			ss << "'" << node.name << "' is " << node.wireframe << " at ("
				<< fixed << setprecision(2) << node.offset_x << ", " << node.offset_y << ", " << node.offset_z << "), ["
				<< fixed << setprecision(2) << node.rotation_x << ", " << node.rotation_y << ", " << node.rotation_z << "]";
		}

		std::map<int,Model> models;
		models[0] = edited_model;
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

bool Editor::type_exists(const std::string& type)
{
	return view.objects.count(type) > 0;
}

void Editor::saveModel(const std::string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Saving model to '" + pathed_file + "'");
	if(edited_model.save(pathed_file))
	{
		view.pushMessage(green("Success"));
		objectsName = file;
	}
	else
	{
		view.pushMessage(red("Fail"));
	}
}

void Editor::saveObjects(const std::string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Saving objects to '" + pathed_file + "'");
	if(view.saveObjects(pathed_file))
	{
		view.pushMessage(green("Success"));
		objectsName = file;
	}
	else
	{
		view.pushMessage(red("Fail"));
	}
}

void Editor::loadObjects(const string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Loading objects from '" + pathed_file + "'");
	if(view.loadObjects(pathed_file))
	{
		view.pushMessage(green("Success"));
		objectsName = file;
		selected_part = 0;
		editing_single_part = false;
	}
	else
	{
		view.pushMessage(red("Fail"));
	}
}

void Editor::loadModel(const string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Loading objects from '" + pathed_file + "'");

	Model model;
	bool ok = model.load(pathed_file);
	if(ok)
	{
		selected_part = 0;
		editing_single_part = false;
		edited_model = model;
		view.pushMessage(green("Success"));
		modelName = file;
	}
	else
	{
		view.pushMessage(red("Fail"));
	}
}

void Editor::move_part(double dx, double dy, double dz)
{
	if(edited_model.parts.size() <= selected_part)
	{
		view.pushMessage("Failed to move part, no selected part");
		return;
	}

	ModelNode& modelnode = edited_model.parts[selected_part];
	modelnode.offset_x += dx;
	modelnode.offset_y += dy;
	modelnode.offset_z += dz;

	stringstream ss;
	ss << "(" << dx << ", " << dy << ", " << dz << ")";
	view.pushMessage(green("Moved " + modelnode.name + ss.str()));
}

void Editor::rotate_part(double dx, double dy, double dz)
{
	if(edited_model.parts.size() <= selected_part)
	{
		view.pushMessage("Failed to rotate part, no selected part");
		return;
	}

	ModelNode& modelnode = edited_model.parts[selected_part];
	modelnode.rotation_x += dx;
	modelnode.rotation_y += dy;
	modelnode.rotation_z += dz;

	stringstream ss;
	ss << "(" << dx << ", " << dy << ", " << dz << ")";
	view.pushMessage(green("Rotated " + modelnode.name + ss.str()));
}

void Editor::select_part(const string& part)
{
	for(size_t i = 0; i < edited_model.parts.size(); ++i)
	{
		if(edited_model.parts[i].name == part)
		{
			selected_part = i;
			view.pushMessage(green("Selected part " + part));
			return;
		}
	}
	view.pushMessage(red("Failed to select part " + part));
}

void Editor::remove_part()
{
	if(edited_model.parts.size() <= selected_part)
	{
		view.pushMessage("Failed to remove part, no selected part");
		return;
	}
	queue<size_t> to_be_removed;
	vector<bool> removed;
	vector<size_t> new_indices;
	removed.resize(edited_model.parts.size(), 0);
	new_indices.resize(edited_model.parts.size(), UINT_MAX);

	// Find all removed indices.
	to_be_removed.push(selected_part);
	while(!to_be_removed.empty())
	{
		size_t current = to_be_removed.front();
		to_be_removed.pop();
		removed[current] = true;
		for(size_t i = 0; i < edited_model.parts[current].children.size(); ++i)
		{
			to_be_removed.push(edited_model.parts[current].children[i]);
		}
	}

	if(removed[edited_model.root])
	{
		edited_model.root = -1;
	}

	// Do actual moving and resizing. Store new indices.
	size_t next = 0;
	for(size_t i = 0; i < edited_model.parts.size(); ++i)
	{
		if(!removed[i])
		{
			std::swap(edited_model.parts[i], edited_model.parts[next]);
			new_indices[i] = next;
			++next;
		}
	}
	edited_model.parts.resize(next);

	// Update children's indices.
	for(size_t i = 0; i < edited_model.parts.size(); ++i)
	{
		for(size_t j = 0; j < edited_model.parts[i].children.size(); ++j)
		{
			size_t& child = edited_model.parts[i].children[j];
			child = new_indices[child];
			if(child == UINT_MAX)
			{
				edited_model.parts[i].children.erase(edited_model.parts[i].children.begin() + j);
				--j;
			}
		}
	}
	selected_part = 0;
}

void Editor::add_part(const std::string& part_name, const std::string& type)
{
	if(!edited_model.parts.empty() && selected_part >= edited_model.parts.size())
	{
		view.pushMessage(red("Add part failed, select part first"));
		return;
	}
	if(!type_exists(type))
	{
		view.pushMessage(red("Add part failed, type '" + type + "' doesn't exist"));
		return;
	}
	ModelNode new_node;
	new_node.name = part_name;
	new_node.wireframe = type;
	new_node.offset_x = 0.0f;
	new_node.offset_y = 0.0f;
	new_node.offset_z = 0.0f;
	edited_model.parts.push_back(new_node);
	if(edited_model.parts.size() > 1)
	{
		ModelNode& selected_node = edited_model.parts[selected_part];
		selected_node.children.push_back(edited_model.parts.size()-1);
		view.pushMessage(green("Added new part '" + part_name + "' of type '" + type + "' as child of '" + selected_node.name));
	}
	else
	{
		view.pushMessage(green("Added new part '" + part_name + "' of type '" + type + "' as root"));
		edited_model.root = 1;
	}

}

void Editor::print_model()
{
	for(size_t i = 0; i < edited_model.parts.size(); ++i)
	{
		view.pushMessage(edited_model.parts[i].name);
	}
}

void Editor::print_types()
{
	for(auto it = view.objects.begin(); it != view.objects.end(); ++it)
	{
		const std::string& name = it->first;
		view.pushMessage(name);
	}
}

void Editor::type_helper(const std::string& type)
{
	stored_model = edited_model;
	Model dummy;
	dummy.root = 0;
	dummy.updatePosition(0.0f,0.0f,0.0f);
	dummy.currentModelPos = Vec3();
	dummy.animation_time = 0;
	ModelNode node;
	node.name = "dummy";
	node.wireframe = type;
	node.offset_x = 0.0f;
	node.offset_y = 0.0f;
	node.offset_z = 0.0f;
	dummy.parts.push_back(node);
	edited_model = dummy;

	edited_part = &view.objects[type];

	editing_single_part = true;
	view.pushMessage("Editing parttype '" + type + "'");
}

void Editor::edit_type(const std::string& type)
{
	if(!type_exists(type))
	{
		view.pushMessage(red("Edit part failed, type '" + type + "' doesn't exist"));
		return;
	}
	type_helper(type);
}

void Editor::add_type(const std::string& type)
{
	if(type_exists(type))
	{
		view.pushMessage(red("Add part type failed, type '" + type + "' exists already"));
		return;
	}
	type_helper(type);
}

void Editor::edit_model()
{
	if(!editing_single_part)
	{
		view.pushMessage(red("Already editing model."));
		return;
	}
	editing_single_part = false;
	edited_part = 0;

	edited_model = stored_model;
}

void Editor::handle_command(const string& command)
{
	view.pushMessage(command);
	stringstream ss(command);

	string word1;
	string word2;
	string word3;
	string word4;
	ss >> word1;
	ss >> word2;
	ss >> word3;
	ss >> word4;


	if(word1 == "load")
	{
		if(word2 == "objects")
		{
			loadObjects(word3);
		}
		else if(word2 == "model")
		{
			loadModel(word3);
		}
	}
	if(word1 == "save")
	{
		if(word2 == "objects")
		{
			saveObjects(word3);
		}
		else if(word2 == "model")
		{
			saveModel(word3);
		}
	}
	else if(word1 == "move")
	{
		stringstream ss(command);
		ss >> word1;
		double dx = 0.0;
		double dy = 0.0;
		double dz = 0.0;
		ss >> dx >> dy >> dz;
		move_part(dx,dy,dz);
	}
	else if(word1 == "rotate")
	{
		stringstream ss(command);
		ss >> word1;
		double dx = 0.0;
		double dy = 0.0;
		double dz = 0.0;
		ss >> dx >> dy >> dz;
		rotate_part(dx,dy,dz);
	}
	else if(word1 == "select")
	{
		select_part(word2);
	}
	else if(word1 == "add")
	{
		if(word2 == "part")
		{
			add_part(word3, word4);
		}
		else if(word2 == "type")
		{
			add_type(word3);
		}
	}
	else if(word1 == "remove")
	{
		remove_part();
	}
	else if(word1 == "edit")
	{
		if(word2 == "type")
		{
			edit_type(word3);
		}
		else if(word2 == "model")
		{
			edit_model();
		}
	}
	else if(word1 == "print")
	{
		if(word2 == "types")
		{
			print_types();
		}
		else if(word2 == "model")
		{
			print_model();
		}
	}
	else if(word1 == "speed")
	{
		stringstream ss1(command);
		ss1 >> word1 >> speed;
		stringstream ss2;
		ss2 << speed;
		view.pushMessage("Move speed set to " + ss2.str());
	}
	else if(word1 == "rotate_speed")
	{
		stringstream ss1(command);
		ss1 >> word1 >> rotate_speed;
		stringstream ss2;
		ss2 << rotate_speed;
		view.pushMessage("Rotate speed set to " + ss2.str());
	}

	commands.push_back(command);
	current_command = commands.size();
}

void Editor::handle_input()
{
	string key = userio.getSingleKey();
	cerr << key << "\n";

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
		if(key == "g")
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

			if(key == "up")
			{
				if(current_command > 0)
				{
					--current_command;
					clientCommand = commands[current_command];
				}
			}
			if(key == "down")
			{
				if(current_command < commands.size()-1)
				{
					++current_command;
					clientCommand = commands[current_command];
				}
				else if(current_command == commands.size()-1)
				{
					++current_command;
					clientCommand = "";
				}
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
			if(key == "insert")
			{
				rotate_part(0, 0, rotate_speed);
			}
			if(key == "delete")
			{
				rotate_part(0, 0, -rotate_speed);
			}
			if(key == "home")
			{
				rotate_part(rotate_speed, 0, 0);
			}
			if(key == "end")
			{
				rotate_part(-rotate_speed, 0, 0);
			}
			if(key == "page up")
			{
				rotate_part(0, rotate_speed, 0);
			}
			if(key == "page down")
			{
				rotate_part(0, -rotate_speed, 0);
			}


			if(key == "[8]")
			{
				move_part(0, 0, speed);
			}
			if(key == "[5]")
			{
				move_part(0, 0, -speed);
			}
			if(key == "[4]")
			{
				move_part(speed, 0, 0);
			}
			if(key == "[6]")
			{
				move_part(-speed, 0, 0);
			}
			if(key == "[9]")
			{
				move_part(0, speed, 0);
			}
			if(key == "[3]")
			{
				move_part(0, -speed, 0);
			}

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

