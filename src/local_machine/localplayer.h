#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "local_machine/game.h"
#include "world/world.h"
#include "world/order.h"
#include "world/playerinfo.h"
#include "world/ordercontainer.h"
#include "graphics/visualworld.h"
#include "misc/fps_manager.h"
#include "misc/hasproperties.h"
#include "misc/messaging_system.h"
#include "gamesound.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>

class UserIO;
class GameView;
class Hud;
class Window;

class Localplayer: private HasProperties, public MessagingSystem<BulletHitEvent>, public MessagingSystem<DevourEvent>, public MessagingSystem<DeathPlayerEvent>, public MessagingSystem<DeathNPCEvent>, public MessagingSystem<CenterCamera>, public MessagingSystem<SetLocalProperty>, public MessagingSystem<GameOver>, public MessagingSystem<GotMyName>, public MessagingSystem<GotPlayerID>
{
public:
	Localplayer(GameView*, UserIO*, Hud*, Window* window);

	void handle(const BulletHitEvent& event);
	void handle(const DevourEvent& event);
	void handle(const DeathPlayerEvent& event);
	void handle(const DeathNPCEvent& event);
	void handle(const CenterCamera& event);
	void handle(const SetLocalProperty& event);
	void handle(const GameOver& event);
	void handle(const GotPlayerID& event);
	void handle(const GotMyName& event);

	void deliverMessages();

	bool client_tick();

	void draw();
	void init();

	bool internetGameGetHeroes(const std::string&, std::map<std::string, CharacterInfo>&);
	void internetGameSelectHero(const std::string&);

	void reload_confs();

	void startMusic(std::string name);
	void endMusic();
	void setMusicVolume();
	void reset();

private:
	void endGame();

	void handleMetaEvent(HasProperties& event);
	void handleWorldEvents();
	void camera_handling();

	bool handleClientLocalInput();
	void process_sent_game_input();
	void sendCheckSumMessage();

	bool set_local_variable(const std::string& clientCommand);

private:
	bool need_to_tick_world;

	int client_input_state;
	std::string clientCommand;

	Hud* hud;
	GameView* view;
	UserIO* userio;
	Window* window;

	Game game;
	GameSound soundsystem;
	VisualWorld visualworld;
	World world;
};


#endif

