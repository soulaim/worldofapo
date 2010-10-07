
#include "image.h"
#include "location.h"
#include "particle.h"

#include <vector>
#include <map>
#include <string>

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>

#include "model.h"
#include "animation.h"
#include "frustum/FrustumR.h"
#include "camera.h"
#include "viewmessage.h"

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
	
	void init();
	void initLight();
	
	void createWindow();
	float modelGround(const Model& model);
	
	void startDrawing();
	void drawPartsRecursive(Model&, int, int, const std::string&, int);
	void drawMessages();
	void drawString(const std::string&, float pos_x = -1.0f, float pos_y = -1.0f, float scale = 1.0f, bool background = false);
	void drawLevel(const Level&);
	void drawModels(std::map<int, Model>& models);
	void drawDebugLines();
	void drawCrossHair();
	void drawStatusBar();
	void drawZombiesLeft();
	void drawBanner();
	void drawParticles();
	void drawHUD();
	
	std::string currentClientCommand;
	std::vector<ViewMessage> viewMessages;
	std::vector<Particle> viewParticles;
	std::map<std::string, ObjectPart> objects;
	
	// define some character widths in our particular font
	std::vector<float> charWidth;
	
	std::string kills;
	std::string deaths;
	std::string health;
	std::string plr_name;
	
	SDL_Surface* drawContext;
	Camera camera;
	
	unsigned currentTime;
	bool lightsActive;
	int zombieCount;
	std::vector<Location> humanPositions;
	
public:
	
	void setLocalPlayerName(const std::string&);
	void setLocalPlayerHP(const int);
	
	void bindCamera(Unit* unit);
	void updateInput(int keystate, int mousex, int mousey);
	
	void megaFuck(); // this function creates a test animation called "walk" for the test model

	void setZombiesLeft(int);
	void setLocalPlayerKills(const int k);
	void setLocalPlayerDeaths(const int d);
	
	void setCamera(const Camera& camera);
	void loadObjects(std::string);
	
	void pushMessage(std::string, float r = 1.0, float g = 1.0, float b = 1.0);
	void setTime(unsigned);
	void setCurrentClientCommand(std::string&);
	
	void draw(std::map<int, Model>&, const Level& lvl);
	void draw(std::map<int, Model>&);
	void drawMenu(std::vector<MenuButton>&);
	
	void toggleLightingStatus();
	void toggleFullscreen();

	void mouseUp();
	void mouseDown();
	void tick();
	
	void genParticles(const Location& position, const Location& velocity, int num, float max_rand, float scale, float r, float g, float b);
	void depthSortParticles(Vec3&);

	void drawMinimap();
	void setHumanPositions(std::vector<Location>);
	
	Graphics();
	FrustumR frustum;
};


