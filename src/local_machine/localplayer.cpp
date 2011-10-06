#include "local_machine/localplayer.h"
#include "local_machine/game.h"
#include "local_machine/userio.h"

#include "graphics/graphics.h"
#include "graphics/texturehandler.h"
#include "graphics/models/animation.h"
#include "graphics/models/modelfactory.h"
#include "graphics/graphics.h"
#include "graphics/hud/hud.h"
#include "graphics/window.h"

#include "world/ordercontainer.h"
#include "world/order.h"
#include "world/playerinfo.h"
#include "world/world.h"

#include "misc/fps_manager.h"

#include "gamesound.h"

#include "net/socket.h"
#include "net/socket_handler.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <sstream>
#include <pthread.h>

using namespace std;

Localplayer::Localplayer(GameView* g, UserIO* u, Hud* h, Window* w):
	client_input_state(0),
	hud(h),
	view(g),
	userio(u),
	window(w),
	game(&world),
	world(&visualworld)
{
	cerr << "Localplayer(...)" << endl;

	// to fix cases where no drawing happens
	// Note the changes to how the sync variable is handled, this syncs fps and tps for the first 5 frames.
	intVals["SYNC_FPS_AND_TPS"] = 5;

	hud->setLevelSize(world.lvl.max_x(), world.lvl.max_z());
	need_to_tick_world = false;

	visualworld.levelDesc.setLevel(&world.lvl);
}

void Localplayer::reload_confs()
{
	cerr << "Reloading conf files.." << endl;

	world.load("configs/world.conf");
	visualworld.load("configs/visualworld.conf");
	game.readConfig();
	this->load("configs/localplayer.conf");
	hud->load("configs/hud.conf");
	view->load("configs/graphics.conf");
}


void Localplayer::startMusic(string name)
{
	// TODO: some more control over this
	soundsystem.startMusic(name);
}

void Localplayer::endMusic()
{
	soundsystem.end_music();
}

void Localplayer::setMusicVolume()
{
	// TODO: Implement this
	return;
}


// returns true on success.
bool Localplayer::internetGameGetHeroes(const std::string& hostname, map<string, CharacterInfo>& heroes)
{
	// TODO: Should also obtain level generation parameters and possibility for other variables as well.
	return game.internetGameGetHeroes(hostname, heroes);
}

// this operation can't fail, if value is selected from "heroes" map
void Localplayer::internetGameSelectHero(const std::string& hero)
{
	game.internetGameSelectHero(hero);
}

void Localplayer::endGame()
{
	game.endGame();
}


void Localplayer::reset()
{
	game.reset();
	client_input_state = 0;
	clientCommand = "";
}

void Localplayer::init()
{
	cerr << "Localplayer::init()" << endl;

	load("configs/localplayer.conf");

	userio->init();
	view->init(visualworld.camera);
	soundsystem.init();

	// TODO: Should not be done here? FIX
	// TODO: move model loading things to config files
	ModelFactory::load(VisualWorld::ModelType::ITEM_MODEL, "models/item_bag.sm2", "itembag");
	ModelFactory::load(VisualWorld::ModelType::STONEBEAST_MODEL, "models/stone_beast.sm2", "stonebeast");
	ModelFactory::load(VisualWorld::ModelType::BULLET_MODEL, "models/bullet_lowpoly.sm2", "bullet_tex");
	ModelFactory::load(VisualWorld::ModelType::PLAYER_MODEL, "models/marine.sm2", "marine");     // TODO: Texture
	ModelFactory::load(VisualWorld::ModelType::ZOMBIE_MODEL, "models/imp_apodus.sm2", "marine"); // TODO: Texture
	ModelFactory::load(VisualWorld::ModelType::WEAPON_MODEL, "models/big_pistol.sm2", "big_pistol");
	ModelFactory::load(VisualWorld::ModelType::INVISIBLE_MODEL, "", "");

	game.init();


	hud->setPlayerInfo(&game.Players);
	hud->setUnitsMap(&world.units);

	Animation::load("models/model.animation");
	Animation::load("models/skeleani.animation");

	TextureHandler::getSingleton().createTextures("data/textures.txt");
}

struct LoadScreenInfo
{
	GameView* view;
	World* world;
	float* percentage;
	string* task_name;
	int world_seed;
};

void* generateTerrain(void* loadScreenInfo)
{
	LoadScreenInfo* lsi = static_cast<LoadScreenInfo*>(loadScreenInfo);

	float& worldGenPercent = *(lsi->percentage);
	string& taskName       = *(lsi->task_name);

	worldGenPercent = 0.0f;
	taskName = "^GGenerating terrain..";

	lsi->world->buildTerrain(lsi->world_seed, worldGenPercent);

	worldGenPercent = 1.1f;

	return 0;
}

void Localplayer::handleMetaEvent(HasProperties& event)
{
	if(event.strVals["EVENT_TYPE"] == "WORLD_GEN_PARAM")
	{
		int world_seed   = event.intVals["WORLD_GEN_SEED"];
		string area_name = event.strVals["WORLD_NAME"];

		world.strVals["AREA_NAME"] = area_name;

		world.terminate();

		// shared memory for processes to determine how far terrain generation has progressed.
		float done_percent = 0.0f;
		//float prev_done_percent = -1.0f;
		string task_name = "Preparing to load world..";

		LoadScreenInfo info;
		info.view = view;
		info.world = &world;
		info.percentage = &done_percent;
		info.task_name = &task_name;
		info.world_seed = world_seed;


		pthread_t worldGen;


		pthread_create( &worldGen, NULL, generateTerrain,  (void*) &info);

		string bg_image;

		if(rand() % 2)
		{
			bg_image = "loading2";
		}
		else
		{
			bg_image = "loading1";
		}

		while(true)
		{
			if(done_percent > 1.0f)
				break;

			// if(done_percent > prev_done_percent)
			{
				view->drawLoadScreen(task_name, bg_image, done_percent);
				//prev_done_percent = done_percent;
			}

#ifndef _WIN32
			usleep(1000);
#else
			// sleep(10);
#endif
		}

		task_name = "Decorating terrain..";
		visualworld.decorate(world.lvl);


		// and then send WORLD_GEN_READY message!
		stringstream ss_world_gen_ready;
		ss_world_gen_ready << "-2 WORLD_GEN_READY #";
		game.write(Game::SERVER_ID, ss_world_gen_ready.str());

		// wait until game starts for me..
		cerr << "world gen finished, waiting for game content.." << endl;

	}
	else
	{
		world.add_message("^RUnkown game metaevent occurred: " + event.strVals["EVENT_TYPE"]);
	}
}

void Localplayer::sendCheckSumMessage()
{
	stringstream checksum_msg;
	checksum_msg << "-2 CSMSG " << (world.currentWorldFrame);

	vector<World::CheckSumType> checksums;
	world.checksum(checksums);

	for(size_t k = 0; k < checksums.size(); k++)
	{
		checksum_msg << " " << checksums[k];
	}

	checksum_msg << "#";
	game.write(Game::SERVER_ID, checksum_msg.str());
}

bool Localplayer::client_tick()
{
    this->userio->tick();

	for(size_t i=0; i<game.meta_events.size(); ++i)
	{
		HasProperties& event = game.meta_events[i];
		handleMetaEvent(event);
	}
	game.meta_events.clear();

	bool stop = game.check_messages_from_server();
	game.processClientMsgs();

	stop = stop || !handleClientLocalInput();

	if(!stop && !game.paused())
	{
		if(game.client_tick_local())
		{
			need_to_tick_world = true;

			process_sent_game_input();
			game.process_received_game_input();

			game.TICK();
			handleWorldEvents();

			hud->world_tick();
			view->world_tick(visualworld.lights);

			visualworld.levelDesc.world_tick(view->frustum);

			sendCheckSumMessage();
		}
		else
		{
			need_to_tick_world = false;
		}
	}
	return stop;
}

void Localplayer::draw()
{
	hud->setTime( SDL_GetTicks() );
	if((world.units.find(game.myID) != world.units.end()) && (game.myID >= 0)) // TODO: why do we need myID?
	{
		visualworld.viewTick(world.units, world.projectiles, world.items, world.currentWorldFrame);
		view->tick();

		float fps = hud->getFPS();
		if( (fps < 5.0f) || (!need_to_tick_world && window->active()) )
		{
			int blur = world.units.find(game.myID)->second["D"];
			if(visualworld.camera.mode() == Camera::STATIC)
				blur = 0;

			// TODO: kinda silly to call this every frame, but what the hell..
			hud->setAreaName(world.strVals["AREA_NAME"]);

			// if we didn't need to tick the world right now, then there should be time to draw the scene.
			view->draw(world.lvl, visualworld, world.octree, world.projectiles, world.units, blur);
		}
	}
}

void Localplayer::playSound(const std::string& name, const Location& position)
{
	// play sounds!
	if(game.myID >= 0)
	{
		const Location& reference_point = world.units.find(game.myID)->second.getPosition();

		FixedPoint distance = (reference_point - position).length();

		if(distance < FixedPoint(1))
			distance = FixedPoint(1);


		// play local player's unit's sound effect
		if(name == "walk")
		{
			static int frame = 0;
			++frame;
			if(frame % 15 == 0)
			{
				soundsystem.playEffect(name, distance.getFloat(), 100000);
			}
		}
		else
		{
			soundsystem.playEffect(name, distance.getFloat(), 100000);
		}
	}
}


void Localplayer::camera_handling()
{
	int wheel_status = userio->getMouseWheelScrolled();
	if(wheel_status == UserIO::SCROLL_UP)
	{
		view->zoom_in();
	}

	if(wheel_status == UserIO::SCROLL_DOWN)
	{
		view->zoom_out();
	}
}

void Localplayer::process_sent_game_input()
{
	static int prevMousePress = -1;
	static int prevKeyState = -1;

	int keyState = userio->getGameInput();
	if (client_input_state & 2)
		keyState = 0;
	int x, y;

	userio->getMouseChange(x, y);
	int mousePress = userio->getMousePress();

	int& no_input = world.intVals["NO_INPUT"];
	if((no_input > 0) || (world.units.find(game.myID) == world.units.end()))
	{
		keyState = 0;
		x = 0;
		y = 0;
		mousePress = 0;

		if(no_input < 1000)
			--no_input;
	}

	visualworld.camera.updateInput(keyState, x, y); // Make only "small" local changes like change the camera angle.
	hud->setShowStats(keyState & (1 << 31));

	x *= intVals["sensitivity"];
	y *= intVals["sensitivity"];

	if(keyState != prevKeyState)
		game.sendKeyState(keyState);
	if(x != 0 || y != 0)
		game.sendMouseMove(x, y);
	if(mousePress != prevMousePress)
		game.sendMousePress(mousePress);

	prevMousePress = mousePress;
	prevKeyState = keyState;
}

bool setVariable(HasProperties& properties, string var_name, string word, int val)
{
	if(properties.intVals.find(var_name) != properties.intVals.end())
	{
		properties.intVals[var_name] = val;
		return true;
	}

	if(properties.strVals.find(var_name) != properties.strVals.end())
	{
		properties.strVals[var_name] = word;
		return true;
	}

	return false;
}

bool Localplayer::set_local_variable(const std::string& clientCommand)
{
	stringstream msg_ss(clientCommand);
	string word1;
	string word2;
	msg_ss >> word1 >> word2;

	int value = -1;
	stringstream word2_ss(word2);
	word2_ss >> value;

	if(!setVariable(world, word1, word2, value))
	if(!setVariable(*view, word1, word2, value))
	if(!setVariable(*this, word1, word2, value))
	if(!setVariable(visualworld, word1, word2, value))
	if(!setVariable(*hud, word1, word2, value))
	if(!setVariable(visualworld.levelDesc, word1, word2, value))
	{
		// handle special cases
		if(word1 == "AMBIENT")
		{
			view->intVals["AMBIENT_RED"] = value;
			view->intVals["AMBIENT_GREEN"] = value;
			view->intVals["AMBIENT_BLUE"] = value;
		}
		else
		{
			return false;
		}
	}

	world.add_message("^Gvalue set");
	return true;
}

bool Localplayer::handleClientLocalInput()
{
	camera_handling();

	std::string key = userio->getSingleKey();

	if(key.size() == 0)
		return true;

	if(key == "return")
		client_input_state ^= 2;
	else if(key == "f1")
		view->intVals["HELP"] ^= 1;
	else if(key == "f2")
		visualworld.camera.setMode(Camera::THIRD_PERSON);
	else if(key == "f3")
		visualworld.camera.setMode(Camera::FIRST_PERSON);
	else if(key == "f4")
		visualworld.camera.setMode(Camera::STATIC);
	else if(key == "f11")
		view->toggleFullscreen();
	else if(key == "f10")
		view->toggleDebugStatus();
	else if(key == "f8")
		view->toggleWireframeStatus();
	else if(key == "f9")
		world.show_errors ^= 1;
	else if(key == "f7")
	{
		std::cerr << "RELOADING SHADERS" << std::endl;
		view->reload_shaders();
	}

	if(client_input_state & 2) // chat message
	{
		std::string nick;
		nick.append("<");
		nick.append(game.Players[game.myID].name);
		nick.append("> ");

		if(key.size() == 1)
		{
			clientCommand.append(key);
		}
		else if(key == "backspace" && clientCommand.size() > 0)
			clientCommand.resize(clientCommand.size()-1);

		else if(key == "escape")
		{
			client_input_state ^= 2;
			clientCommand = "";
			nick = "";
		}
		else if(key == "space")
			clientCommand.append(" ");

		nick.append(clientCommand);
		hud->setCurrentClientCommand(nick);
	}
	else
	{
		if(key == "pause")
		{
			window->screenshot();
		}
		else if(key == "return") // handle client local command
		{
			if(clientCommand.size() > 0)
			{
				if(!set_local_variable(clientCommand))
				{
					game.send_chat_message(clientCommand);
				}
			}

			clientCommand = "";
			hud->setCurrentClientCommand(clientCommand);
		}
		else if(key == "escape")
		{
			// could send a message instead :)
			game.endGame();
			world.terminate();
			hud->reset();

			// then proceed with local shutdown.
			std::cerr << "User pressed ESC, shutting down." << std::endl;
			return false;
		}
		else if(key == "g")
		{
			if(client_input_state & 4)
			{
				window->enable_grab();
			}
			else
			{
				window->disable_grab();
			}
			client_input_state ^= 4;
		}
		else if(key == "n")
		{
			auto iter = world.units.find(visualworld.camera.unit_id);
			if(iter == world.units.end())
				iter = world.units.begin();
			else
			{
				iter++;
				if(iter == world.units.end())
					iter = world.units.begin();
			}

			if(iter != world.units.end())
			{
				visualworld.bindCamera(&iter->second);
				std::stringstream ss_msg;
				ss_msg << "Bound camera to " << iter->second.name << " with unitID " << iter->first;
				world.add_message(ss_msg.str());
			}
		}
		else if(key == "p")
		{
			auto iter = world.units.find(visualworld.camera.unit_id);
			if(iter != world.units.begin())
			{
				iter--;
				visualworld.bindCamera(&iter->second);
				std::stringstream ss_msg;
				ss_msg << "Bound camera to " << iter->second.name << " with unitID " << iter->first;
				world.add_message(ss_msg.str());
			}
		}
		else if(key == "N")
		{
			auto iter = world.units.find(visualworld.camera.unit_id);
			if(iter == world.units.end())
				iter = world.units.begin();
			else
			{
				iter++;
				if(iter == world.units.end())
					iter = world.units.begin();
			}

			if(iter != world.units.end())
			{
				Location loc = iter->second.getPosition();
				vec3<float> v = vec3<float>(loc.x.getFloat(), loc.y.getFloat(), loc.z.getFloat());
				visualworld.camera.unit_id = iter->second.id;
				visualworld.camera.setMode(Camera::STATIC);
				visualworld.camera.setTarget(v);
				std::stringstream ss_msg;
				ss_msg << "Looking at " << v;
				world.add_message(ss_msg.str());
			}
		}
		else if(key == "h")
		{
			cerr << "Hiding window" << endl;
			window->hide();
		}
		else if(key == "o")
		{
			stringstream ss;
			ss << "Camera is looking at " << view->getWorldPosition();
			world.add_message(ss.str());
		}

	}
	return true;
}



void Localplayer::handle(const BulletHitEvent& event)
{
	int scale = 1000;
	int max_rand = 500;

	std::stringstream ss;
	int x = (rand() % 4);
	ss << "hit" << x;
	playSound(ss.str(), event.t_position);

	Location bulletDirection = event.a_velocity;
	bulletDirection.normalize();

	visualworld.genParticleEmitter(event.a_position, bulletDirection, 5, max_rand, scale, "DARK_RED", "DARK_RED", "DARK_RED", "DARK_RED");
}

void Localplayer::handle(const DevourEvent& event)
{
	int scale = 1000;
	int max_rand = 500;

	playSound("hit0", event.t_position);
	visualworld.genParticleEmitter(event.t_position, event.t_velocity, 5, max_rand, scale, "DARK_RED", "DARK_RED", "DARK_RED", "DARK_RED");
}


void Localplayer::handle(const DeathPlayerEvent& event)
{
	int scale = 1000;
	int max_rand = 500;

	if(event.target_id == game.myID)
	{
		// we want the player who dies to always hear his own screams
		playSound("player_death", world.units[game.myID].position);
	}
	else
	{
		playSound("player_death", event.t_position);
	}

	visualworld.genParticleEmitter(event.t_position, event.t_velocity, 15, max_rand, scale, "DARK_RED", "DARK_RED", "DARK_RED", "DARK_RED", 2000, 25);

	auto unit_it = world.units.find(event.actor_id);
	if( (unit_it != world.units.end()) && unit_it->second.human())
	{
		game.Players[event.actor_id].kills++;
	}

	auto target_it = world.units.find(event.target_id);
	if( (target_it != world.units.end()) && target_it->second.human() )
	{
		game.Players[event.target_id].deaths++;
	}

	hud->setLocalPlayerKills(game.Players[game.myID].kills);
	hud->setLocalPlayerDeaths(game.Players[game.myID].deaths);
}

void Localplayer::handle(const DeathNPCEvent& event)
{
	int scale = 1000;
	int max_rand = 500;

	playSound("alien_death", event.t_position);

	visualworld.genParticleEmitter(event.t_position, event.t_velocity, 15, max_rand, scale, "DARK_RED", "DARK_RED", "DARK_RED", "DARK_RED", 2000, 25);

	auto unit_it = world.units.find(event.actor_id);
	if( (unit_it != world.units.end()) && unit_it->second.human())
	{
		game.Players[event.actor_id].kills++;
	}

	hud->setLocalPlayerKills(game.Players[game.myID].kills);
	hud->setLocalPlayerDeaths(game.Players[game.myID].deaths);
}

void Localplayer::handle(const CenterCamera& event)
{
	if( (world.units.find(event.plr_id) != world.units.end()) )
	{
		std::cerr << "Binding camera to unit " << event.plr_id << std::endl;
		visualworld.bindCamera(&world.units.find(event.plr_id)->second);
	}
}

void Localplayer::handle(const SetLocalProperty& event)
{
	if(set_local_variable(event.cmd))
	{
		world.add_message("^GSomething happened");
	}
	else
	{
		world.add_message("^RSomething did not happen");
	}
}

void Localplayer::handle(const GameOver& event)
{
	// ...
	if(event.win)
	{
		cerr << "YAY, I won!" << endl;
	}
	else
	{
		cerr << "NAY! I lost!" << endl;
	}
}

void Localplayer::handle(const GotPlayerID& event)
{
	hud->setLocalPlayerID(event.myID);
}

void Localplayer::handle(const GotMyName& event)
{
	hud->setLocalPlayerName(event.name);
}


void Localplayer::deliverMessages()
{
	MessagingSystem<BulletHitEvent>::deliverMessages();
	MessagingSystem<DevourEvent>::deliverMessages();
	MessagingSystem<DeathPlayerEvent>::deliverMessages();
	MessagingSystem<DeathNPCEvent>::deliverMessages();
}

// TODO: Replace this stuff with messaging system completely.
// world events are local events generated by the world simulation.
void Localplayer::handleWorldEvents()
{
	if(game.myID >= 0)
	{
		hud->setLocalPlayerHP(world.units.find(game.myID)->second.hitpoints);
	}

	hud->setZombiesLeft(world.getUnitCount());

	// TODO: THIS MAKES NO FUCKING SENSE!! USE THE MESSAGE SENDING SYSTEM INSTEAD
	// deliver any world message events to graphics structure, and erase them from world data.
	for(size_t i = 0; i < visualworld.worldMessages.size(); ++i)
	{
		hud->pushMessage(visualworld.worldMessages[i]);
	}
	visualworld.worldMessages.clear();

	deliverMessages();

	// TODO: This could also be implemented with message passing.
	for(auto iter = world.units.begin(); iter != world.units.end(); iter++)
	{
		playSound(iter->second.soundInfo, iter->second.getPosition());
	}
}

