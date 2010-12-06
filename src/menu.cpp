#include "menu.h"

#include "graphics.h"
#include "userio.h"
#include "game.h"
#include "image.h"
#include "menubutton.h"

#include <vector>

#ifndef _WIN32
#include <sys/wait.h>
#endif

using namespace std;

#ifndef _WIN32
pid_t serverpid = 0;

void server_killer()
{
	int status;
	cerr << "Killing server..." << endl;
	if(kill(serverpid, SIGKILL) < 0)
	{
		perror("killing server failed");
	}
	pid_t pid = wait(&status);
	cerr << "Server process " << pid << " died: " << status << endl;
}
#endif

Menu::Menu(Graphics* v, UserIO* u):
	view(v),
	userio(u)
{
}

std::string Menu::menu_tick()
{
	// load images & create textures
	vector<MenuButton> buttons;
	
	buttons.push_back(MenuButton("exit", "data/menu/exit.png"));
	buttons.push_back(MenuButton("connect", "data/menu/connect.png"));
	buttons.push_back(MenuButton("host", "data/menu/host.png"));
	
	size_t selected = 2;
	int dont_exit = 1;
	buttons[selected].selected = 1;

	string ret;
	
	while(dont_exit)
	{
		view->drawMenu(buttons);
		
		string key = userio->getSingleKey();
		
		if(key == "")
		{
			SDL_Delay(50); // sleep a bit. don't need anything intensive done anyway.
			continue;
		}
		else if(key == "down")
		{
			buttons[selected].selected = 0;
			if(selected == 0)
				selected = buttons.size() - 1;
			else
				selected--;
			buttons[selected].selected = 1;
		}
		else if(key == "up")
		{
			buttons[selected].selected = 0;
			if(selected == (buttons.size()-1))
				selected = 0;
			else
				selected++;
			buttons[selected].selected = 1;
		}
		else if(key == "return")
		{
			if(buttons[selected].name == "connect")
			{
				// ask for host name and connect.
				ret = connectMenu();
				if(ret != "")
				{
					dont_exit = false;
				}
			}
			else if(buttons[selected].name == "exit")
			{
				return "exit";
			}
#ifndef _WIN32
			else if(buttons[selected].name == "host" && serverpid == 0)
			{
				pid_t pid = fork();
				if(pid == 0)
				{
					execl("./server", "./server", 0);
					perror("Error, starting server failed");
					return "exit";
				}
				serverpid = pid;
				atexit(server_killer);
				cerr << "Started server process with pid " << serverpid << endl;
				sleep(2);
				ret = "localhost";
				dont_exit = false;
			}
#endif
			else
			{
				cerr << "STUPID USER!!" << endl;
			}
		}
		else if(key == "escape")
		{
			return "exit";
		}
	}
	
	for(size_t i = 0; i < buttons.size(); ++i)
	{
		buttons[i].unloadTexture();
	}
	
	return ret;
}

std::string Menu::connectMenu()
{
	cerr << "Type the name of the host machine: " << endl;
	string hostName = "";
	
	while(true)
	{
		string key_hostname = userio->getSingleKey();
		
		if(key_hostname == "")
		{
			SDL_Delay(50); // sleep a bit. don't need anything intensive done anyway.
			continue;
		}
		else if(key_hostname.size() == 1)
		{
			hostName.append(key_hostname);
		}
		else if(key_hostname == "escape")
		{
			cerr << "Out from connecting" << endl;
			break;
		}
		else if(key_hostname == "backspace")
		{
			if(hostName.size() > 0)
			{
				hostName.resize(hostName.size()-1);
			}
		}
		else if(key_hostname == "return")
		{
			return hostName;
		}
		cerr << "Current input: \"" << hostName << "\"" << endl;
	}
	return "";
}

