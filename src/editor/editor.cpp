
#include "editor.h"
#include "../texturehandler.h"

#include <iostream>
#include <sstream>
#include <algorithm>

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
//	view.loadObjects("data/parts.dat");
//	view.megaFuck(); // blah..
	
	userio.init();
	
	// load some textures.
	// should not be done here. FIX
	TextureHandler::getSingleton().createTexture("grass", "data/grass.png");
	TextureHandler::getSingleton().createTexture("highground", "data/highground.png");
	TextureHandler::getSingleton().createTexture("mountain", "data/hill.png");

	view.bindCamera(&dummy);

	handle_command("load objects parts.dat");
	handle_command("load model model.bones");
}

void Editor::start()
{
	tick();

	view.setTime( SDL_GetTicks() );

	view.tick();

	view.draw(models);
}

void Editor::tick()
{
	handle_input();

//	cerr << view.camera.getPosition() << "\n";
}

void Editor::loadObjects(const string& file)
{
	string pathed_file = "data/" + file;
	view.pushMessage("Loading objects from '" + pathed_file + "'", WHITE);
	if(view.loadObjects(pathed_file))
	{
		view.pushMessage("Success", GREEN);
		objectsName = file;
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
		models[0] = model;
		view.pushMessage("Success", GREEN);
		modelName = file;
	}
	else
	{
		view.pushMessage("Fail", RED);
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
	ss >> second_word;
	ss >> third_word;


	if(first_word == "load")
	{
		if(second_word == "objects")
		{
			loadObjects(third_word);
		}
		else if(second_word == "model")
		{
			loadModel(third_word);
		}
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

//	cerr << "keystate: " << keystate << "\n";
	view.updateInput(keystate, x, y);
	dummy.updateInput(0, x, y, 0);
}

