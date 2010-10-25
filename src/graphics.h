#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include <SDL/SDL.h>

#include "btt.h"
#include "model.h"
#include "skeletalmodel.h"
#include "animation.h"
#include "frustum/FrustumR.h"
#include "camera.h"
#include "viewmessage.h"
#include "playerinfo.h"

#include "octree.h"
#include "image.h"
#include "location.h"
#include "particle.h"
#include "primitives.h"

#include "medikit.h"
#include "light_object.h"

struct Level;
struct MenuButton;

class Graphics
{
	void init();
	void initLight();
	
	void createWindow();
	float modelGround(const Model& model);
	
	void startDrawing();
	void drawPartsRecursive(Model&, int, const std::string&, int);
	void drawString(const std::string&, float pos_x = -1.0f, float pos_y = -1.0f, float scale = 1.0f, bool background = false);
	// void drawLevel(const Level&, const std::map<int, LightObject>& lights);
	void drawLevel(const Level&, const std::map<int, LightObject>& lights);
	
	void drawModels(std::map<int, Model*>& models);
	void drawDebugLines();
	void updateCamera(const Level&);
	void updateParticles();
	void finishDrawing();

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
	

	void loadVertexShader(const std::string& name, const std::string& filename);
	void loadFragmentShader(const std::string& name, const std::string& filename);
	void initShaders();


	std::vector<Particle> viewParticles;

	std::map<std::string, GLuint> shaders;
	
	// define some character widths in our particular font
	std::vector<float> charWidth;
	std::vector<BTT_Triangle> level_triangles;
	
	SDL_Surface* drawContext;
	Camera camera;
	
	int world_ticks;
	unsigned currentTime;
	bool lightsActive;
	bool drawDebuglines;
	
public:
	static std::map<std::string, ObjectPart> objects; // TODO: Maybe move this somewhere?
	friend class Editor;

	// HUD stuff
	void setLocalPlayerName(const std::string&);
	void setLocalPlayerHP(const int);
	void setPlayerInfo(std::map<int,PlayerInfo>* pInfo);
	void setZombiesLeft(int);
	void setLocalPlayerKills(const int k);
	void setLocalPlayerDeaths(const int d);
	void drawStats();
	void pushMessage(const std::string&);
	void setCurrentClientCommand(const std::string&);
	void setHumanPositions(const std::vector<Location>&);


	void bindCamera(Unit* unit);
	void updateInput(int keystate);
	
	void megaFuck(); // this function creates a test animation called "walk" for the test model
	
	void setCamera(const Camera& camera);
	bool loadObjects(const std::string&);
	bool saveObjects(const std::string&);
// 	
	void setTime(unsigned);
	
	void draw(std::map<int, Model*>&, const Level& lvl, const std::map<int,Unit>& units,
		const std::map<int, LightObject>& lights, const std::shared_ptr<Octree> o,
		const std::map<int, Medikit>& medikits);
	void draw(std::map<int, Model*>&, const std::string& status_message);
	void drawMenu(std::vector<MenuButton>&);

	void drawBoundingBoxes(const std::map<int,Unit>& units);
	void drawBox(const Location&, const Location&, GLfloat r = 1.0f, GLfloat g = 0, GLfloat b = 0, GLfloat a = 1.0f);
	void drawMedikits(const std::map<int, Medikit>& medikits);
	void drawOctree(const std::shared_ptr<Octree>& o);
	
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

