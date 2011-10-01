
#include "graphics/graphics.h"
#include "graphics/texturehandler.h"
#include "graphics/shaders.h"
#include "graphics/hud.h"
#include "graphics/frustum/matrix4.h"
#include "graphics/texturecoordinate.h"
#include "graphics/window.h"
#include "graphics/menubutton.h"

#include "physics/octree.h"
#include "world/level.h"
#include "algorithms.h"


#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <algorithm>
#include <numeric>
#include <map>
#include <stdexcept>
#include <list>
#include <tuple>

using namespace std;

int TRIANGLES_DRAWN_THIS_FRAME = 0;
int QUADS_DRAWN_THIS_FRAME = 0;



void GameView::depthSortParticles(vec3<float>& d, vector<Particle>& viewParticles)
{
	return;
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		viewParticles[i].updateDepthVal(d);
	}
	sort(viewParticles.begin(), viewParticles.end());
}



GameView::GameView(Window& w, Hud& h):
	window(w),
	hud(h)
{
	cerr << "Loading config file for Graphics.." << endl;
	load("configs/graphics.conf");

	window.createWindow(intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
}

GameView::~GameView()
{
}

void GameView::init(Camera& camera)
{
	cerr << "GameView::init()" << endl;

	intVals["HELP"] = 1;

	OpenGL opengl;
	shaders.init();

	camera_p = &camera;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set the background color.
	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glLoadIdentity();

	// glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	MAX_NUM_LIGHTS = 71;
	MAX_NUM_ACTIVE_LIGHTS = 1; // Make sure this is the same number as in the shaders.

	setInitialShaderValues();

	// Init screen framebuffer objects
	{
		size_t x = intVals["RESOLUTION_X"];
		size_t y = intVals["RESOLUTION_Y"];
		size_t downscale = intVals["PARTICLE_RESOLUTION_DIVISOR"];

		Graphics::Framebuffer::create("screenFBO", x, y, true, 1);
		Graphics::Framebuffer::create("deferredFBO", x, y, true, 2);

		// TODO: Should these use renderbuffer targets instead of textures?
		Graphics::Framebuffer::create("postFBO", x, y, false, 1);
		Graphics::Framebuffer::create("particlesDownScaledFBO", x / downscale, y / downscale, false, 1);
		Graphics::Framebuffer::create("particlesUpScaledFBO", x, y, false, 1);
	}


	Graphics::Framebuffer::unbind();

	TextureHandler::getSingleton().createTexture("font", "data/fonts/font2.png");
	TextureHandler::getSingleton().createTexture("particle", "data/images/particle.png");
	TextureHandler::getSingleton().createTexture("help_layer", "data/images/help_overlay_dark.png");

	TextureHandler::getSingleton().createTexture("loading1", "data/images/loading_screen1.png");
	TextureHandler::getSingleton().createTexture("loading2", "data/images/loading_screen2.png");

	camera_p->aspect_ratio = float(intVals["RESOLUTION_X"]) / float(intVals["RESOLUTION_Y"]);

	frustum.setCamInternals(camera_p->fov, camera_p->aspect_ratio, camera_p->nearP, camera_p->farP);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(camera_p->fov, camera_p->aspect_ratio, camera_p->nearP, camera_p->farP);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_CULL_FACE);
}

void GameView::clear_errors() const
{
	while(glGetError() != GL_NO_ERROR);
}

bool GameView::check_errors(const char* filename, int line) const
{
	bool ret = false;
	GLenum error;
	while((error = glGetError()) != GL_NO_ERROR)
	{
		cerr << "ERROR: " << gluErrorString(error) << " at " << filename << ":" << line << "\n";
		ret = true;
	}
	return ret;
}





void GameView::drawModels(const map<int, Model*>& models, const Camera& camera)
{
	Shader& shader = shaders.get_shader("unit_program");
	shader.start();
	GLint unit_color_location = shader.uniform("unit_color");
	GLint unit_location_location = shader.uniform("unit_location");
	int skipped_unit_id = -2;
	if(camera.mode() == Camera::FIRST_PERSON)
	{
		skipped_unit_id = camera.unit_id;
	}
	for(map<int, Model*>::const_iterator iter = models.begin(); iter != models.end(); ++iter)
	{
		const Model& model = *iter->second;

		if(iter->first == skipped_unit_id)
		{
			continue;
		}

		if(frustum.sphereInFrustum(model.currentModelPos, 5) != FrustumR::OUTSIDE)
		{
			glUniform4f(unit_color_location, 0.7, 0.7, 0.7, 0.5);
			glUniform3f(unit_location_location, model.currentModelPos.x, model.currentModelPos.y, model.currentModelPos.z);

			model.draw();
		}
	}
	shader.stop();
}




void GameView::updateCamera(const Level& lvl)
{
	vec3<float> camStartPos = camera_p->getPosition();
	FixedPoint camX = FixedPoint(camStartPos.x);
	FixedPoint camZ = FixedPoint(camStartPos.z);

	float cam_min_y = lvl.getHeight(camX, camZ).getFloat() + 3.f;
	camera_p->setAboveGround(cam_min_y);
}

void GameView::setupCamera(const Camera& camera)
{
	vec3<float> camPos = camera.getPosition();
	vec3<float> camTarget = camera.getTarget();
	vec3<float> upVector(0.0, 1.0, 0.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// TODO: how about changing these to our own functions, just because we can.
	gluPerspective(camera.fov, camera.aspect_ratio, camera.nearP, camera.farP);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// TODO: how about changing these to our own functions, just because we can.
	gluLookAt(camPos.x, camPos.y, camPos.z,
			  camTarget.x, camTarget.y, camTarget.z,
			  upVector.x, upVector.y, upVector.z);

	frustum.setCamDef(camPos, camTarget, upVector);
}

void GameView::startDrawing()
{
	setupCamera(*this->camera_p);

	if(intVals["DEFERRED_RENDERING"])
	{
		Graphics::Framebuffer::get("deferredFBO").bind();
	}
	else
	{
		Graphics::Framebuffer::get("screenFBO").bind();
	}

	glClear(GL_DEPTH_BUFFER_BIT); // Dont clear the color buffer, since we're going to rewrite the color everywhere anyway.

	TRIANGLES_DRAWN_THIS_FRAME = 0;
	QUADS_DRAWN_THIS_FRAME = 0;
}



void GameView::applySSAO(int power, const string& input_texture, const string& depth_texture, const Graphics::Framebuffer& renderTarget)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	renderTarget.bind();

	TextureHandler::getSingleton().bindTexture(0, input_texture);
	TextureHandler::getSingleton().bindTexture(1, depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	Shader& shader = shaders.get_shader("ssao_program");
	shader.start();
	glUniform1f(shader.uniform("power"), float(power));

	drawFullscreenQuad();

	shader.stop();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	TextureHandler::getSingleton().unbindTexture(1);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}


void GameView::applyBlur(int blur, const string& input_image, const Graphics::Framebuffer& renderTarget)
{
	blur = min(blur, intVals["MAXIMUM_BLUR"]);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glColor3f(1.0f, 1.0f, 1.0f);

	Graphics::Framebuffer& postFBO = Graphics::Framebuffer::get("postFBO");
	postFBO.bind();

	TextureHandler::getSingleton().bindTexture(0, input_image);
	{
		Shader& shader = shaders.get_shader("blur_program1");
		shader.start();
		glUniform1f(shader.uniform("amount"), float(blur));
		drawFullscreenQuad();
		shader.stop();
	}

	renderTarget.bind();
	TextureHandler::getSingleton().bindTexture(0, postFBO.texture(0));
	{
		Shader& shader = shaders.get_shader("blur_program2");
		shader.start();
		glUniform1f(shader.uniform("amount"), float(blur));
		drawFullscreenQuad();
		shader.stop();
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}


void GameView::drawLightsDeferred_single_pass(int lights)
{
	Shader& shader = shaders.get_shader("deferred_lights_program");
	shader.start();

	glUniform1f(shader.uniform("light_count"), float(lights));

	vec3<float> camera_position = camera_p->getPosition();
	glUniform3f(shader.uniform("camera_position"), camera_position.x, camera_position.y, camera_position.z);
	vec3<float> camera_target = camera_p->getTarget();
	glUniform3f(shader.uniform("camera_target"), camera_target.x, camera_target.y, camera_target.z);
/*
	cerr << "Camera: " << camera_position << endl;
	cerr << "BL    : " << frustum.fbl << endl;
	cerr << "BR    : " << frustum.fbr << endl;
	cerr << "TL    : " << frustum.ftl << endl;
	cerr << "TR    : " << frustum.ftr << endl;
*/
	TextureHandler::getSingleton().bindTexture(0, Graphics::Framebuffer::get("deferredFBO").texture(0));
	TextureHandler::getSingleton().bindTexture(1, Graphics::Framebuffer::get("deferredFBO").texture(1));
//	TextureHandler::getSingleton().bindTexture(2, deferredFBO.texture(2));
	TextureHandler::getSingleton().bindTexture(3, Graphics::Framebuffer::get("deferredFBO").depth_texture());
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	GLint attribute_location = shader.attribute("frustum_corner_position");
//	drawFullscreenQuad();
	glBegin(GL_QUADS);
//	glUniform3fv(shader.uniform("ftl"), 1, (float*)&frustum.ftl);
//	glUniform3fv(shader.uniform("ftr"), 1, (float*)&frustum.ftr);
//	glUniform3fv(shader.uniform("fbl"), 1, (float*)&frustum.fbl);
//	glUniform3fv(shader.uniform("fbr"), 1, (float*)&frustum.fbr);
	glTexCoord2f(0.f, 0.f); glVertexAttrib3f(attribute_location, frustum.fbl.x, frustum.fbl.y, frustum.fbl.z); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertexAttrib3f(attribute_location, frustum.fbr.x, frustum.fbr.y, frustum.fbr.z); glVertex3f(+1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertexAttrib3f(attribute_location, frustum.ftr.x, frustum.ftr.y, frustum.ftr.z); glVertex3f(+1.0f, +1.0f, -1.0f);
	glTexCoord2f(0.f, 1.f); glVertexAttrib3f(attribute_location, frustum.ftl.x, frustum.ftl.y, frustum.ftl.z); glVertex3f(-1.0f, +1.0f, -1.0f);
	glEnd();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	TextureHandler::getSingleton().unbindTexture(3);
	TextureHandler::getSingleton().unbindTexture(2);
	TextureHandler::getSingleton().unbindTexture(1);
	TextureHandler::getSingleton().unbindTexture(0);

	shader.stop();
}

void GameView::applyAmbientLight()
{
	Shader& shader = shaders.get_shader("deferred_ambientlight_program");
	shader.start();

	TextureHandler::getSingleton().bindTexture(0, Graphics::Framebuffer::get("deferredFBO").texture(0));
	TextureHandler::getSingleton().bindTexture(1, Graphics::Framebuffer::get("deferredFBO").texture(1));
	float r = intVals["AMBIENT_RED"]   / 255.0f;
	float g = intVals["AMBIENT_GREEN"] / 255.0f;
	float b = intVals["AMBIENT_BLUE"]  / 255.0f;
	glUniform4f(shader.uniform("ambientLight"), r, g, b, 1.0f);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	drawFullscreenQuad();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	TextureHandler::getSingleton().unbindTexture(1);
	TextureHandler::getSingleton().unbindTexture(0);

	shader.stop();
}

vec3<float> GameView::getWorldPosition()
{
	return getWorldPosition(intVals["RESOLUTION_X"]/2, intVals["RESOLUTION_Y"]/2);
}

vec3<float> GameView::getWorldPosition(int screen_x, int screen_y)
{
	setupCamera(*camera_p);

	GLint viewport[4] = { 0, 0, intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"] };
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	// TODO: DON'T glGet* these, get directly from camera instead!!
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
//	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (float)screen_x;
	winY = (float)viewport[3] - (float)screen_y;

	Graphics::Framebuffer::get("deferredFBO").bind();
	glReadPixels( screen_x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

	gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return vec3<float>(posX, posY, posZ);
}



void GameView::drawSolidGeometry(const VisualWorld& visualworld)
{
	if(intVals["DRAW_LEVEL"])
	{
		if(intVals["DEFERRED_RENDERING"])
		{
			visualworld.levelDesc.drawDeferred(shaders);
		}
		else
		{
			visualworld.levelDesc.drawFR(visualworld.lights.size(), shaders);
		}
	}

	if(intVals["DRAW_MODELS"])
	{
		drawModels(visualworld.models, *camera_p);
	}

        OpenGL gl;
	if(intVals["DRAW_GRASS"] && gl.getGL3bit())
	{
		drawGrass(visualworld.meadows);
	}
}

void GameView::drawDeferredDepthBuffer(const VisualWorld& visualworld)
{
	Graphics::Framebuffer::get("deferredFBO").bind(0);
	glColorMask(false, false, false, false);
	drawSolidGeometry(visualworld);
	glColorMask(true, true, true, true);
	Graphics::Framebuffer::get("deferredFBO").bind();
}

void GameView::draw(
	const Level& lvl,
	const VisualWorld& visualworld,
	const std::shared_ptr<Octree> o, // For debug.
	const std::map<int, Projectile>& projectiles, // For debug.
	const std::map<int, Unit>& units, // For debug and names.
	int blur
	)
{
	bool deferred_rendering = intVals["DEFERRED_RENDERING"];

	updateCamera(lvl);

	startDrawing();

	if(intVals["DRAW_SKYBOX"])
	{
		drawSkybox();
	}

	if(intVals["DRAW_DEBUG_LINES"])
	{
		drawBoundingBoxes(units);
		drawOctree(o);
		visualworld.levelDesc.drawDebugLevelNormals();
		drawDebugProjectiles(projectiles);
		drawDebugLines();
	}

	if(intVals["DRAW_DEBUG_WIREFRAME"])
	{
		visualworld.levelDesc.drawDebugHeightDots(camera_p->getPosition());
	}

	if(deferred_rendering && intVals["EARLY_Z_CULLING"])
	{
		// TODO: for optimal performance, performs this pass with empty fragment shaders.
		glDepthMask(GL_TRUE);
		drawDeferredDepthBuffer(visualworld);
		glDepthFunc(GL_EQUAL);
		glDepthMask(GL_FALSE);
	}

	drawSolidGeometry(visualworld);

	if(deferred_rendering && intVals["EARLY_Z_CULLING"])
	{
		glDepthFunc(GL_LESS);
	}

	if(deferred_rendering)
	{
		applyDeferredLights(visualworld.lights);
	}

	string depth_texture = (deferred_rendering ? Graphics::Framebuffer::get("deferredFBO").depth_texture() : Graphics::Framebuffer::get("screenFBO").depth_texture());

	if(intVals["SSAO"])
	{
		applySSAO(intVals["SSAO_DISTANCE"], Graphics::Framebuffer::get("screenFBO").texture(0), depth_texture, Graphics::Framebuffer::get("screenFBO"));
	}

	if(intVals["DRAW_PARTICLES"])
	{
		drawParticles(visualworld.particles);
	}

	if(intVals["DRAW_NAMES"])
	{
		drawPlayerNames(units, visualworld.models);
	}

	drawDebugStrings();

	if(intVals["DAMAGE_BLUR"])
	{
		applyBlur(blur, Graphics::Framebuffer::get("screenFBO").texture(0), Graphics::Framebuffer::get("screenFBO"));
	}

	if(intVals["DRAW_HUD"])
	{
		hud.draw(camera_p->mode() == Camera::FIRST_PERSON);
	}

	finishDrawing();
}

void GameView::drawPlayerNames(const std::map<int, Unit>& units, const map<int, Model*>& models)
{
	for(auto iter = units.begin(); iter != units.end(); ++iter)
	{
		/*
		if(!iter->second.human())
		{
			continue;
		}
		*/
		const Model& model = *models.find(iter->first)->second;
		vec3<float> posName = model.currentModelPos;
		posName.y += 5.5f * model.myScale;
		hud.draw3Dstring(iter->second.name, posName, camera_p->getXrot(), camera_p->getYrot(), iter->second["TEAM"]);

		float hp_percent = float(iter->second.hitpoints) / float(iter->second.getMaxHP());

		if(&(iter->second) == camera_p->getUnitPointer())
		{
			hud.drawBar(hp_percent, "GREEN", "DARK_RED", -1.0f, 0.0f, -1.0f, -0.9f);
		}
		else
		{
			vec3<float> posHP = model.currentModelPos;
			posHP.y += 5.0f * model.myScale;
			hud.draw3DBar(hp_percent, posHP, camera_p->getXrot(), camera_p->getYrot(), "GREEN", "DARK_RED", 5.0f);
		}
	}

}

void GameView::renderToBackbuffer()
{
	glDisable(GL_DEPTH_TEST);
	Graphics::Framebuffer::unbind();
	TextureHandler::getSingleton().bindTexture(0, Graphics::Framebuffer::get("screenFBO").texture(0));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(1.0, 1.0, 1.0);

	drawFullscreenQuad();

	if(intVals["HELP"])
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		TextureHandler::getSingleton().bindTexture(0, "help_layer");
		drawFullscreenQuad();
		glDisable(GL_BLEND);
	}

	glEnable(GL_DEPTH_TEST);
}

void GameView::finishDrawing()
{
	renderToBackbuffer();

	drawDebugQuad();

	window.swap_buffers();
}


void GameView::world_tick(const std::map<int, LightObject>& lights)
{
	updateLights(lights);
}

void GameView::tick()
{
}

void GameView::toggleFullscreen()
{
	window.toggle_fullscreen();
}

void GameView::zoom_in()
{
	camera_p->zoomIn();
}

void GameView::zoom_out()
{
	camera_p->zoomOut();
}



void GameView::drawGrass(const std::vector<GrassCluster>& meadows)
{
	Shader& shader = shaders.get_shader("grass_program");
	shader.start();

	if(intVals["DRAW_DEBUG_LINES"])
	{
		TextureHandler::getSingleton().bindTexture(0, "chessboard");
	}
	else
	{
		string texture = strVals["MEADOW"];
		TextureHandler::getSingleton().bindTexture(0, texture);
	}

	for(size_t i = 0; i < meadows.size(); ++i)
	{
		if(frustum.sphereInFrustum(meadows[i].center, meadows[i].radius) != FrustumR::OUTSIDE)
		{
			meadows[i].draw(shader);
		}
	}

	shader.stop();
}

void GameView::drawMenuParticles(const std::vector<MenuParticle>& menuParticles, int front, float scale, const string& color) const
{
    // render a menu background scene! Falling particles would be awesome.
    for(size_t i = 0;i < menuParticles.size();i++)
    {
        stringstream menuparticle;
        if(menuParticles[i].front == front)
        {
            menuparticle << color << menuParticles[i].val;
            hud.drawString(menuparticle.str(), menuParticles[i].x, menuParticles[i].y, scale);
        }
    }
}

void GameView::drawMenuButtons(const vector<MenuButton>& buttons) const
{
    float button_height = -0.10f;
    float menu_y_offset = 0.2f;
    for(size_t i = 0;i < buttons.size();++i)
    {
        stringstream msg;
        stringstream info;
        if(buttons[i].selected == 1)
        {
            if(buttons[i].editing())
            {
                msg << "^Y";
                info << "^G";
            }
            else
            {
                msg << "^G";
                info << "^Y";
            }
        }
        else
        {
            msg << "^W";
            info << "^W";
        }
        float minus = i * button_height;

        msg << buttons[i].name;
        info << buttons[i].info;
        hud.drawString(msg.str(), -0.7f, minus + menu_y_offset, 3.0f);
        hud.drawString(info.str(), 0.0f, minus + menu_y_offset, 3.0f);
    }
}

void GameView::drawMenu(const vector<MenuButton>& buttons, const std::vector<MenuParticle>& menuParticles) const
{
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	if(intVals.find("MENU_FALLING_NUMBERS")->second)
	{
		drawMenuParticles(menuParticles, 0, 1.2f, "^r");
		drawMenuParticles(menuParticles, 1, 2.0f, "^R");
	}

	if(intVals.find("MENU_RECTANGLES")->second)
	{
		drawMenuRectangles();
	}

	// blur the effect?

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	// render menu buttons on top of the scene.
	drawMenuButtons(buttons);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	window.swap_buffers();
}

void GameView::reload_shaders()
{
	shaders.release();
	shaders.init();
	setInitialShaderValues();
}

void GameView::setInitialShaderValues()
{
	{
            OpenGL gl;
            if(gl.getGL3bit()) {
		Shader& shader = shaders.get_shader("particle_program");
		shader.start();
		glUniform1f(shader.uniform("screen_width"),  intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
		glUniform1f(shader.uniform("screen_height"), intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
		shader.stop();
            }
	}

	{
		Shader& shader = shaders.get_shader("ssao_program");
		shader.start();
		glUniform1f(shader.uniform("screen_width"), intVals["RESOLUTION_X"]);
		glUniform1f(shader.uniform("screen_height"), intVals["RESOLUTION_Y"]);
		shader.stop();
	}

	{
            OpenGL gl;
            if(gl.getGL3bit()) {
		Shader& shader = shaders.get_shader("partitioned_deferred_lights_program");
		shader.start();
		glUniform1f(shader.uniform("screen_width"), intVals["RESOLUTION_X"]);
		glUniform1f(shader.uniform("screen_height"), intVals["RESOLUTION_Y"]);
		shader.stop();
            }
	}

	{
		Shader& shader = shaders.get_shader("partitioned_deferred_lights_program2");
		shader.start();
		glUniform1f(shader.uniform("screen_width"), intVals["RESOLUTION_X"]);
		glUniform1f(shader.uniform("screen_height"), intVals["RESOLUTION_Y"]);
		shader.stop();
	}
}
