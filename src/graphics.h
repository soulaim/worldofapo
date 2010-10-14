#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include <map>
#include <string>

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include <SDL/SDL.h>

#include "model.h"
#include "animation.h"
#include "frustum/FrustumR.h"
#include "camera.h"
#include "viewmessage.h"
#include "playerinfo.h"

#include "image.h"
#include "location.h"
#include "particle.h"

struct Level;
struct MenuButton;

struct ObjectTri
{
	float x[3];
	float y[3];
	float z[3];
};

struct ObjectQuad
{
	float x[4];
	float y[4];
	float z[4];
};

struct ObjectPart
{
	std::vector<ObjectQuad> quads;
	std::vector<ObjectTri> triangles;
	std::map<std::string, Animation> animations;
	
	float end_x;
	float end_y;
	float end_z;
};

class Graphics
{
	GLfloat m[16]; // storage for OGL ModelView matrix
	float terrainDrawn;
	
	void init();
	void initLight();
	
	void createWindow();
	float modelGround(const Model& model);
	
	void startDrawing();
	void drawPartsRecursive(Model&, int, int, const std::string&, int);
	void drawString(const std::string&, float pos_x = -1.0f, float pos_y = -1.0f, float scale = 1.0f, bool background = false);
	void drawLevel(const Level&);
	void drawModels(std::map<int, Model>& models);
	void drawDebugLines();
	void updateCamera(const Level&);


	// HUD Stuff
	void drawHUD();
	void drawMessages();
	void drawCrossHair();
	void drawStatusBar();
	void drawZombiesLeft();
	void drawBanner();
	void drawParticles();
	void drawMinimap();
	void drawFPS();
	
	std::string currentClientCommand;
	std::vector<ViewMessage> viewMessages;
	std::string kills;
	std::string deaths;
	std::string health;
	std::string plr_name;
	std::map<int, PlayerInfo>* Players;
	int zombieCount;
	std::vector<Location> humanPositions;
	

	void loadVertexShader(std::string name, std::string filename);
	void loadFragmentShader(std::string name, std::string filename);


	std::vector<Particle> viewParticles;
	
	std::map<std::string, ObjectPart> objects;
	std::map<std::string, GLuint> shaders;
	
	// define some character widths in our particular font
	std::vector<float> charWidth;
	
	SDL_Surface* drawContext;
	Camera camera;
	
	int world_ticks;
	unsigned currentTime;
	bool lightsActive;
	
public:
	friend class Editor;

	// HUD stuff
	void setLocalPlayerName(const std::string&);
	void setLocalPlayerHP(const int);
	void setPlayerInfo(std::map<int,PlayerInfo>* pInfo);
	void setZombiesLeft(int);
	void setLocalPlayerKills(const int k);
	void setLocalPlayerDeaths(const int d);
	void drawStats();
	void pushMessage(const std::string&, float r = 1.0, float g = 1.0, float b = 1.0);
	void setCurrentClientCommand(const std::string&);
	void setHumanPositions(const std::vector<Location>&);



	void bindCamera(Unit* unit);
	void updateInput(int keystate, int mousex, int mousey);
	
	void megaFuck(); // this function creates a test animation called "walk" for the test model
	
	void setCamera(const Camera& camera);
	bool loadObjects(const std::string&);
	bool saveObjects(const std::string&);
	
	void setTime(unsigned);
	
	void draw(std::map<int, Model>&, const Level& lvl, const std::map<int,Unit>& units);
	void draw(std::map<int, Model>&, const std::string& status_message);
	void drawMenu(std::vector<MenuButton>&);

	void drawHitboxes(const std::map<int,Unit>& units);
	
	void toggleLightingStatus();
	void toggleFullscreen();

	void mouseUp();
	void mouseDown();
	void tick();
	void world_tick();
	
	void genParticles(const Location& position, const Location& velocity, int num, float max_rand, float scale, float r, float g, float b);
	void depthSortParticles(Vec3&);

	Graphics();
	FrustumR frustum;
};

#endif

