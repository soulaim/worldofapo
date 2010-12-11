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
	options_files.push_back("user.conf");
	options_files.push_back("localplayer.conf");
	
	for(size_t i=0; i<options_files.size(); i++)
	{
		options.push_back(HasProperties());
		options.back().load(options_files[i]);
	}
}



std::string Menu::handleOptionsMenu(vector<MenuButton>& buttons, size_t selected)
{
	if(buttons[selected].valueType == 0) // string value
	{
		// ask for player name
		getInput(buttons, selected);
		return "";
	}
	else // int value
	{
		changeValue(buttons, selected);
		return "";
	}
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
		
		for(size_t i=0; i<options.size(); i++)
		{
			for(auto iter = options[i].intVals.begin(); iter != options[i].intVals.end(); iter++)
			{
				string dummy;
				options_buttons.push_back(MenuButton(iter->first, dummy, iter->second));
			}
			
			for(auto iter = options[i].strVals.begin(); iter != options[i].strVals.end(); iter++)
			{
				int dummy = -1;
				options_buttons.push_back(MenuButton(iter->first, iter->second, dummy));
			}
		}
		
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
		buttons[selected].info = "^RMeaning undefined";
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
			{
				ans = handleMainMenu(buttons, selected);
			}
			else if(menu_name == "options")
			{
				ans = handleOptionsMenu(buttons, selected);
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
	
	cerr << "returning from menu." << endl;
	
	if(menu_name == "options")
	{
		for(size_t k = 0; k < buttons.size(); k++)
		{
			for(size_t i = 0; i < options.size(); i++)
			{
				if(options[i].intVals.find(buttons[k].name) != options[i].intVals.end())
				{
					options[i].intVals[buttons[k].name] = buttons[k].value;
				}
				if(options[i].strVals.find(buttons[k].name) != options[i].strVals.end())
				{
					options[i].strVals[buttons[k].name] = buttons[k].info;
				}
			}
		}
		
		for(size_t i = 0; i < options.size(); i++)
		{
			options[i].save(options_files[i]);
		}
	}
	
	return ret;
}

std::string Menu::menu_tick()
{
	// load images & create textures
	vector<MenuButton> buttons;
	
	int dummy = -1;
	string conTarget;
	
	buttons.push_back(MenuButton("exit", conTarget, dummy));
	buttons.push_back(MenuButton("host", conTarget, dummy));
	buttons.push_back(MenuButton("options", conTarget, dummy));
	buttons.push_back(MenuButton("connect", conTarget, dummy));
	
	return run_menu(buttons, "main");
	
}


int Menu::changeValue(vector<MenuButton>& buttons, int i)
{
	buttons[i].info.append("_");
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
			if((key_hostname[0] >= '0') && (key_hostname[0] <= '9'))
			{
				hostName.resize(hostName.size() - 1);
				hostName.append(key_hostname);
				hostName.append("_");
			}
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
			buttons[i].info.resize(buttons[i].info.size() - 1);
			
			// don't allow empty values;
			if(buttons[i].info.size() == 0)
			{
				buttons[i].info = "0";
			}
			
			stringstream ss_int(buttons[i].info);
			ss_int >> buttons[i].value;
			return buttons[i].value;
		}
	}
	
	// good idea?
	buttons[i].info.resize(buttons[i].info.size() - 1);
	stringstream ss(buttons[i].info);
	ss >> buttons[i].value;
	return -1;
}


std::string Menu::getInput(vector<MenuButton>& buttons, int i)
{
	buttons[i].info.append("_");
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
			buttons[i].info.resize(buttons[i].info.size() - 1);
			
			// don't allow empty values
			if(buttons[i].info.size() == 0)
			{
				buttons[i].info = "empty";
			}
			
			return buttons[i].info;
		}
	}
	
	// good idea?
	buttons[i].info.resize(buttons[i].info.size() - 1);
	return "";
}

