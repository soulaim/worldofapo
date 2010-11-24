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
#include "frustum/frustumr.h"
#include "camera.h"
#include "location.h"
#include "particle.h"

#include "medikit.h"
#include "light_object.h"
#include "projectile.h"


#include "world.h" // TODO: change to visualworld.h

struct Level;
struct MenuButton;
class Hud;
class Octree;

class Graphics
{
	void init();
	void initLight();
	
	void createWindow();
	void destroyWindow();
	
	void startDrawing();
	void drawLevel(const Level&, const std::map<int, LightObject>& lights);
	void drawSkybox();
	
	void drawModels(const std::map<int, Model*>& models);
	void drawParticles(std::vector<Particle>&);
	void drawParticles_old(std::vector<Particle>&);
	void drawParticles_vbo(std::vector<Particle>&);
	void updateCamera(const Level&);
	void finishDrawing();

	void drawDebugHeightDots(const Level& lvl);
	void drawDebugLines();
	void drawDebugLevelNormals(const Level& lvl);
	void drawDebugProjectiles(const std::map<int, Projectile>& projectiles);

	void updateLights(const std::map<int, LightObject>&); // once per world frame
//	void setActiveLights(const std::map<int, LightObject>&, const Location&);

	void loadVertexShader(const std::string& name, const std::string& filename);
	void loadFragmentShader(const std::string& name, const std::string& filename);
	void loadGeometryShader(const std::string& name, const std::string& filename);
	
	std::map<std::string, GLuint> shaders;
	std::map<std::string, GLint> uniform_locations;
	
	std::vector<BTT_Triangle> level_triangles;
	
	SDL_Surface* drawContext;
	Camera camera;
	
	GLint MAX_NUM_LIGHTS;
	GLint MAX_NUM_ACTIVE_LIGHTS;
	bool lightsActive;
	bool drawDebuglines;
	bool drawDebugWireframe;
	
public:
	void initShaders(); // Public for debugging.
	void releaseShaders();

	friend class Editor;

	void bindCamera(Unit* unit);
	void updateInput(int keystate);
	
	void setCamera(const Camera& camera);
	
	void draw(
		const Level& lvl,
		const VisualWorld& visualworld,
		Hud* hud,
		const std::shared_ptr<Octree> o, // For debug.
		const std::map<int, Projectile>& projectiles, // For debug.
		const std::map<int, Unit>& units // For debug.
		);
	void drawMenu(const std::vector<MenuButton>&) const;

	void drawBoundingBoxes(const std::map<int,Unit>& units);
	void drawBox(const Location&, const Location&, GLfloat r = 1.0f, GLfloat g = 0, GLfloat b = 0, GLfloat a = 1.0f);
	void drawMedikits(const std::map<int, Medikit>& medikits);
	void drawOctree(const std::shared_ptr<Octree>& o);

	void drawGrass(const std::vector<Vec3>& locations, const std::vector<Vec3>& winds);
	
	void toggleLightingStatus();
	void toggleWireframeStatus();
	void toggleFullscreen();

	void zoom_in();
	void zoom_out();
	void tick();
	void world_tick(Level& lvl, const std::map<int, LightObject>&);
	
	void depthSortParticles(Vec3&, std::vector<Particle>&);

	Graphics();
	~Graphics();
	FrustumR frustum;
};

#endif

