#include "menu.h"

#include "graphics.h"
#include "userio.h"
#include "game.h"
#include "image.h"
#include "menubutton.h"

#include <vector>
#include <csignal>

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


std::string Menu::handleMainMenu(vector<MenuButton>& buttons, size_t selected)
{
	if(buttons[selected].name == "connect")
	{
		// ask for host name and connect.
		return getInput(buttons, selected);
	}
	else if(buttons[selected].name == "exit")
	{
		return "exit";
	}
	else if(buttons[selected].name == "options")
	{
		// hmm.
		vector<MenuButton> options_buttons;
		options_buttons.push_back(MenuButton("Nick name", ""));
		options_buttons.push_back(MenuButton("Tard", ""));
		options_buttons.push_back(MenuButton("Crap", ""));
		options_buttons.push_back(MenuButton("Lol", ""));
		
		run_menu(options_buttons, "options");
		return "";
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
		return "localhost";
	}
	#endif
	else
	{
		buttons[selected].info = "^RDoes not work";
		return "";
	}
}

std::string Menu::run_menu(vector<MenuButton>& buttons, string menu_name)
{
	
	size_t selected = buttons.size() - 1;
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
			string ans;
			if(menu_name == "main")
				ans = handleMainMenu(buttons, selected);
			else if(menu_name == "options")
			{
				cerr << "OPTIONS!" << endl;
				ans = handleMainMenu(buttons, selected);
			}
			if(ans == "")
				continue;
			ret = ans;
			dont_exit = false;
		}
		else if(key == "escape")
		{
			ret = "exit";
			dont_exit = false;
		}
	}
	
	for(size_t i = 0; i < buttons.size(); ++i)
	{
		buttons[i].unloadTexture();
	}
	
	cerr << "returning from menu." << endl;
	return ret;
}

std::string Menu::menu_tick()
{
	// load images & create textures
	vector<MenuButton> buttons;
	
	buttons.push_back(MenuButton("exit", "data/menu/exit.png"));
	buttons.push_back(MenuButton("host", "data/menu/host.png"));
	buttons.push_back(MenuButton("options", "data/menu/host.png"));
	buttons.push_back(MenuButton("connect", "data/menu/connect.png"));
	
	return run_menu(buttons, "main");
	
}

std::string Menu::getInput(vector<MenuButton>& buttons, int i)
{
	buttons[i].info = "_";
	string& hostName = buttons[i].info;
	
	while(true)
	{
		string key_hostname = userio->getSingleKey();
		view->drawMenu(buttons);
		
		if(key_hostname == "")
		{
			SDL_Delay(50); // sleep a bit. don't need anything intensive done anyway.
			continue;
		}
		else if(key_hostname.size() == 1)
		{
			hostName.resize(hostName.size() - 1);
			hostName.append(key_hostname);
			hostName.append("_");
		}
		else if(key_hostname == "escape")
		{
			break;
		}
		else if(key_hostname == "backspace")
		{
			if(hostName.size() > 1)
			{
				hostName.resize(hostName.size()-2);
				hostName.append("_");
			}
		}
		else if(key_hostname == "return")
		{
			string copy = buttons[i].info;
			buttons[i].info = "";
			copy.resize(copy.size() - 1);
			return copy;
		}
	}
	
	// good idea?
	buttons[i].info = "";
	return "";
}

