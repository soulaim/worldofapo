#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "graphics/opengl.h"
#include "graphics/models/model.h"
#include "graphics/frustum/frustumr.h"
#include "graphics/camera.h"
#include "graphics/particles/particle.h"
#include "graphics/shaders.h"
#include "graphics/light_object.h"
#include "graphics/visualworld.h"
#include "graphics/framebuffer.h"
#include "graphics/graphics_helper.h"
#include "world/objects/projectile.h"
#include "misc/hasproperties.h"
#include "misc/vec3.h"
#include "graphics/skybox/skybox.h"
#include "graphics/terrain/roof_renderer.h"

#include "graphics/hud/barRenderer.h"
#include "graphics/hud/textRenderer.h"

struct Level;
struct MenuButton;
struct MenuParticle;

class Hud;
class Octree;
class Window;


class GameView: public HasProperties
{
	void createWindow();
	void destroyWindow();

	void setupCamera(const Camera& camera);
	void updateCamera(const Level&); // TODO: separate camera and this function away from graphics.

	void startDrawing();
	void applyDeferredLights(const std::map<int, LightObject>& lights);
	void finishDrawing();

	void drawDeferredDepthBuffer(const VisualWorld& visualworld);

	void drawLightsDeferred_single_pass(int lights);
	void drawLightsDeferred_multiple_passes(const Camera& camera, const std::map<int, LightObject>& lights);

	void drawSolidGeometry(const VisualWorld& visualworld);
	void drawModels(const std::map<int, Model*>& models, const Camera& camera);

	void prepareForParticleRendering(const std::string& depth_texture);
	void renderParticles(std::vector<Particle>&);
	void drawParticles(std::vector<Particle>&);
	void drawParticles(std::vector<Particle>&, const std::string& depth_texture);
	void drawParticles_old(std::vector<Particle>&, const std::string& depth_texture);
	void drawParticles_vbo(std::vector<Particle>&, const std::string& depth_texture);
	void renderToBackbuffer();

	void applyAmbientLight();
	void applyBlur(int blur, const std::string& input_texture, const Graphics::Framebuffer& renderTarget);
	void applySSAO(int power, const std::string& input_texture, const std::string& depth_texture, const Graphics::Framebuffer& renderTarget);

	void drawDebugLines();
	void drawDebugStrings();
	void drawDebugProjectiles(const std::map<int, Projectile>& projectiles);
	void drawDebugQuad();

	void updateLights(const std::map<int, LightObject>&); // once per world frame

	void setInitialShaderValues();

	Camera* camera_p;

	GLint MAX_NUM_LIGHTS;
	GLint MAX_NUM_ACTIVE_LIGHTS;

	Window& window;
	Hud& hud;
	Shaders shaders;

    Skybox skybox;
    RoofRenderer roofRenderer;
    TextRenderer textRenderer;
    BarRenderer barRenderer;

	void clear_errors() const;
	bool check_errors(const char* filename, int line) const;

public:
	void init(Camera&);
	void reload_shaders();
	void drawLoadScreen(const std::string&, const std::string&, const float&);

	friend class Editor;

	void draw(
		const Level& lvl,
		const VisualWorld& visualworld,
		const std::shared_ptr<Octree> o, // For debug.
		const std::map<int, Projectile>& projectiles, // For debug.
        const std::map<int, WorldItem>& items, // For debug.
		std::map<int, Unit>& units, // For debug.
		int blur
		);

	void drawMenuRectangles() const;
	void drawMenuButtons(const std::vector<MenuButton>& buttons) const;
	void drawMenuParticles(const std::vector<MenuParticle>& menuParticles, int front, float scale, const std::string& color) const;
	void drawMenu(const std::vector<MenuButton>&, const std::vector<MenuParticle>&) const;

	void drawBoundingBoxes(const std::map<int,Unit>& units);
    void drawBoundingBoxes(const std::map<int,WorldItem>& items);
	void drawBox(const Location&, const Location&, GLfloat r = 1.0f, GLfloat g = 0, GLfloat b = 0, GLfloat a = 1.0f);
	void drawOctree(const std::shared_ptr<Octree>& o);

	void drawGrass(const std::vector<GrassCluster>& meadows);
	void drawPlayerNames(const std::map<int, Unit>& units, const std::map<int, Model*>& models);

	void toggleDebugStatus();
	void toggleWireframeStatus();
	void toggleFullscreen();

	vec3<float> getWorldPosition();
	vec3<float> getWorldPosition(int screen_x, int screen_y);

	void zoom_in();
	void zoom_out();
	void tick();
	void world_tick(const std::map<int, LightObject>&);

	void depthSortParticles(vec3<float>&, std::vector<Particle>&);

	GameView(Window& window, Hud& hud);
	~GameView();

	FrustumR frustum;
};

#endif

