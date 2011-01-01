#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "opengl.h"

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
struct MenuParticle;

class Hud;
class Octree;
class Window;

class Graphics: public HasProperties
{
	void createWindow();
	void destroyWindow();
	
	void setupCamera(const Camera& camera);
	void updateCamera(const Level&); // TODO: separate camera and this function away from graphics.
	void startDrawing();
	void geometryDrawn(const std::map<int, LightObject>& lights);
	void finishDrawing();

	void drawSkybox();

	void drawLightsDeferred_single_pass();
	void drawLightsDeferred_multiple_passes(const Camera& camera, const std::map<int, LightObject>& lights);
	void drawLightsDeferred_multiple_passes_with_scissors(const std::map<int, LightObject>& lights);

	void drawModels(const std::map<int, Model*>& models);
	
	void prepareForParticleRendering(const std::string& depth_texture);
	void renderParticles(std::vector<Particle>&);
	void drawParticles(std::vector<Particle>&);
	void drawParticles(std::vector<Particle>&, const std::string& depth_texture);
	void drawParticles_old(std::vector<Particle>&);
	void drawParticles_vbo(std::vector<Particle>&, const std::string& depth_texture);
	void renderToBackbuffer();
	
	void applyAmbientLight();
	void applyBlur(int blur, const std::string& input_texture, GLuint renderTarget);
	void applySSAO(int power, const std::string& input_texture, const std::string& depth_texture, GLuint renderTarget);
	
	void drawDebugLines();
	void drawDebugStrings();
	void drawDebugProjectiles(const std::map<int, Projectile>& projectiles);
	void drawDebugQuad();
	
	void updateLights(const std::map<int, LightObject>&); // once per world frame
	
	void setInitialShaderValues();
	
	Camera* camera_p;
	
	GLint MAX_NUM_LIGHTS;
	GLint MAX_NUM_ACTIVE_LIGHTS;
	
	GLuint postFBO;
	GLuint particlesDownScaledFBO;
	GLuint particlesUpScaledFBO;
	
	GLuint deferredFBO;
	GLuint screenFBO;
	GLuint screenRB;
	
	bool lightsActive;
	bool drawDebuglines;
	bool drawDebugWireframe;
	
	Window& window;
	Hud& hud;
	Shaders shaders;
	
	void clear_errors() const;
	bool check_errors(const char* filename, int line) const;
	void check_framebuffer_status(const std::string& fbo_name);
	void bind_framebuffer(GLuint framebuffer, int output_buffers) const;

	void drawFullscreenQuad() const;
public:
	void init(Camera&);

	void reload_shaders();
	
	friend class Editor;
	
	void draw(
		const Level& lvl,
		const VisualWorld& visualworld,
		const std::shared_ptr<Octree> o, // For debug.
		const std::map<int, Projectile>& projectiles, // For debug.
		const std::map<int, Unit>& units, // For debug.
		int blur
		);

	void drawMenuRectangles() const;
	void drawMenuButtons(const std::vector<MenuButton>& buttons) const;
	void drawMenuParticles(const std::vector<MenuParticle>& menuParticles, int front, float scale, const std::string& color) const;
	void drawMenu(const std::vector<MenuButton>&, const std::vector<MenuParticle>&) const;
	
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
	void world_tick(const std::map<int, LightObject>&);
	
	void depthSortParticles(Vec3&, std::vector<Particle>&);
	
	Graphics(Window& window, Hud& hud);
	~Graphics();
	FrustumR frustum;
};

#endif

