#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "btt.h"
#include "model.h"
#include "frustum/frustumr.h"
#include "camera.h"
#include "location.h"
#include "particle.h"
#include "shaders.h"
#include "light_object.h"
#include "projectile.h"
#include "visualworld.h"

#include "hasproperties.h"

struct Level;
struct MenuButton;
class Hud;
class Octree;
class Window;

class Graphics : public HasProperties
{
	void initLight();
	
	void createWindow();
	void destroyWindow();
	
	void startDrawing();
	void drawLevelFR(const Level&, const std::map<int, LightObject>& lights);
	void drawSkybox();
	
	void drawModels(const std::map<int, Model*>& models);
	
	void prepareForParticleRendering();
	void renderParticles(std::vector<Particle>&);
	
	void drawParticles(std::vector<Particle>&);
	void drawParticles_old(std::vector<Particle>&);
	void drawParticles_vbo(std::vector<Particle>&);
	void updateCamera(const Level&);
	void finishDrawing();
	
	void applyBlur(int blur, std::string inputImg, GLuint renderTarget);
	void applySSAO(int power, std::string inputImg, std::string depthImage, GLuint renderTarget);
	
	void drawDebugHeightDots(const Level& lvl);
	void drawDebugLines();
	void drawDebugStrings();
	void drawDebugLevelNormals(const Level& lvl);
	void drawDebugProjectiles(const std::map<int, Projectile>& projectiles);
	
	void updateLights(const std::map<int, LightObject>&); // once per world frame
//	void setActiveLights(const std::map<int, LightObject>&, const Location&);
	
	std::vector<BTT_Triangle> level_triangles;
	
	Camera* camera_p;
	
	GLint MAX_NUM_LIGHTS;
	GLint MAX_NUM_ACTIVE_LIGHTS;
	
	GLuint postFBO;
	GLuint particlesFBO;
	
	GLuint particlesUpScaledFBO;
	
	GLuint screenFBO;
	GLuint screenRB;
	
	bool lightsActive;
	bool drawDebuglines;
	bool drawDebugWireframe;
	
	Window& window;
	Hud& hud;
	Shaders shaders;
	
public:
	void init(Camera&);

	void reload_shaders();
	
	friend class Editor;
	
	void updateInput(int keystate);
	
	void draw(
		const Level& lvl,
		const VisualWorld& visualworld,
		const std::shared_ptr<Octree> o, // For debug.
		const std::map<int, Projectile>& projectiles, // For debug.
		const std::map<int, Unit>& units // For debug.
		);
	void drawMenu(const std::vector<MenuButton>&) const;
	
	void drawBoundingBoxes(const std::map<int,Unit>& units);
	void drawBox(const Location&, const Location&, GLfloat r = 1.0f, GLfloat g = 0, GLfloat b = 0, GLfloat a = 1.0f);
	void drawOctree(const std::shared_ptr<Octree>& o);
	
	void drawGrass(const std::vector<GrassCluster>& meadows);
	void drawPlayerNames(const std::map<int, Unit>& units, const std::map<int, Model*>& models);
	
	void toggleLightingStatus();
	void toggleWireframeStatus();
	void toggleFullscreen();
	
	void zoom_in();
	void zoom_out();
	void tick();
	void world_tick(Level& lvl, const std::map<int, LightObject>&);
	
	void depthSortParticles(Vec3&, std::vector<Particle>&);
	
	Graphics(Window& window, Hud& hud);
	~Graphics();
	FrustumR frustum;
};

#endif

