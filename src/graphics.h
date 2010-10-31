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
#include "playerinfo.h"

#include "octree.h"
#include "image.h"
#include "location.h"
#include "particle.h"
#include "primitives.h"

#include "medikit.h"
#include "light_object.h"
#include "projectile.h"

struct Level;
struct MenuButton;
class Hud;

class Graphics
{
	void init();
	void initLight();
	
	void createWindow();
	void destroyWindow();
	
	void startDrawing();
	void drawPartsRecursive(Model&, int, const std::string&, int);
	void drawLevel(const Level&, const std::map<int, LightObject>& lights);
	
	void drawModels(std::map<int, Model*>& models);
	void drawParticles();
	void updateCamera(const Level&);
	void updateParticles();
	void finishDrawing();

	void drawDebugHeightDots(const Level& lvl);
	void drawDebugLines();
	void drawDebugLevelNormals(const Level& lvl);
	void drawDebugProjectiles(const std::map<int, Projectile>& projectiles);

	void updateLights(const std::map<int, LightObject>&); // once per world frame
	void setActiveLights(const std::map<int, LightObject>&, const Location&);

	void loadVertexShader(const std::string& name, const std::string& filename);
	void loadFragmentShader(const std::string& name, const std::string& filename);

	//ParticleSystemManager particleManager;
	std::vector<Particle> viewParticles;
	std::map<std::string, GLuint> shaders;
	std::map<std::string, GLint> uniform_locations;
	
	std::vector<BTT_Triangle> level_triangles;
	
	SDL_Surface* drawContext;
	Camera camera;
	
	GLint MAX_NUM_LIGHTS;
	bool lightsActive;
	bool drawDebuglines;
	bool drawDebugWireframe;
	
public:
	void initShaders(); // Public for debugging.
	void releaseShaders();

	static std::map<std::string, ObjectPart> objects; // TODO: Maybe move this somewhere?
	friend class Editor;

	void bindCamera(Unit* unit);
	void updateInput(int keystate);
	
	void megaFuck(); // this function creates a test animation called "walk" for the test model
	
	void setCamera(const Camera& camera);
	bool loadObjects(const std::string&);
	bool saveObjects(const std::string&);
	
	void draw(std::map<int, Model*>&, const Level& lvl, const std::map<int,Unit>& units,
		const std::map<int, LightObject>& lights, const std::shared_ptr<Octree> o, Hud* hud,
		const std::map<int, Medikit>& medikits,
		const std::map<int, Projectile>& projectiles);
	void drawMenu(const std::vector<MenuButton>&) const;

	void drawBoundingBoxes(const std::map<int,Unit>& units);
	void drawBox(const Location&, const Location&, GLfloat r = 1.0f, GLfloat g = 0, GLfloat b = 0, GLfloat a = 1.0f);
	void drawMedikits(const std::map<int, Medikit>& medikits);
	void drawOctree(const std::shared_ptr<Octree>& o);
	
	void toggleLightingStatus();
	void toggleWireframeStatus();
	void toggleFullscreen();

	void mouseUp();
	void mouseDown();
	void tick();
	void world_tick(Level& lvl, const std::map<int, LightObject>&);
	
	void genParticles(const Location& position, const Location& velocity, int num, float max_rand, float scale, float r, float g, float b);
	void depthSortParticles(Vec3&);

	Graphics();
	~Graphics();
	FrustumR frustum;
};

#endif

