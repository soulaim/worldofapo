#include "graphics.h"

#include "texturehandler.h"
#include "level.h"
#include "shaders.h"
#include "hud.h"
#include "frustum/matrix4.h"
#include "octree.h"
#include "texturecoordinate.h"
#include "window.h"
#include "menubutton.h"
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

// NOTE: set the y values to be the same. lights that were high above ground were drawn poorly
// because they were never near the player, although the effect is high.
bool near(const Camera& camera, const vec3<float>& position, float dist = 100.0)
{
	vec3<float> copy1 = camera.getPosition();
	vec3<float> copy2 = position;
	
	copy1.y = copy2.y;
	
	return (copy1 - copy2).lengthSquared() < dist * dist;
}

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


void GameView::drawLoadScreen(const string& message, const string& bg_image, const float& percent_done)
{
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	
	float y_plus  = -0.85f;
	float y_minus = -0.95f;
	float plus  = (percent_done - 0.5f) * 2.0f;
	float minus = -1.0f;
	
	Graphics::Framebuffer::unbind();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// DRAW BACKGROUND IMAGE
	TextureHandler::getSingleton().bindTexture(0, bg_image);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	
	glBegin(GL_QUADS);
	glTexCoord2f( 0.0f,  0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f( 1.0f,  0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	glTexCoord2f( 1.0f,  1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
	glTexCoord2f( 0.0f,  1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();
	
	// DRAW LOAD BAR
	TextureHandler::getSingleton().unbindTexture(0);
	
	glColor3f(1.0f - percent_done, percent_done, 0.0f);
	
	glBegin(GL_QUADS);
	glVertex3f(minus, y_minus, -1.0f);
	glVertex3f( plus, y_minus, -1.0f);
	glVertex3f( plus,  y_plus, -1.0f);
	glVertex3f(minus,  y_plus, -1.0f);
	glEnd();
	
	// DRAW LOAD TITLE
	hud.drawString(message, -0.95f, -0.8f, 3.0f, true, 1.0f);
	
	window.swap_buffers();
	return;
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

void GameView::updateLights(const std::map<int, LightObject>& lightsContainer)
{
//	clear_errors();

	// TODO: update only the shader that is currently active. Or update lights when actually drawing.
#ifdef _WIN32
	vector<string> programs;
	programs.push_back("level_program");
	programs.push_back("deferred_lights_program");
	programs.push_back("partitioned_deferred_lights_program");
	programs.push_back("partitioned_deferred_lights_program2");
#else
	vector<string> programs = { "level_program", "deferred_lights_program", "partitioned_deferred_lights_program", "partitioned_deferred_lights_program2" };
#endif

	Matrix4 modelview = camera_p->modelview();

	for(size_t p = 0; p < programs.size(); ++p)
	{
		Shader& shader = shaders.get_shader(programs[p]);
		shader.start();

		int i = 0;
		
		int POSITION = 0;
		int DIFFUSE = 1;
		
		for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); ++iter)
		{
			float rgb[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			
			iter->second.getDiffuse(rgb[0], rgb[1], rgb[2]);
			stringstream ss1;
			ss1 << "lights[" << i*2 + DIFFUSE << "]";
			glUniform4f(shader.uniform(ss1.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
//			check_errors(__FILE__, __LINE__);

			const Location& pos = iter->second.getPosition();
			vec3<float> v(pos.x.getFloat(), pos.y.getFloat(), pos.z.getFloat());
			v = modelview * v;

			stringstream ss2;
			ss2 << "lights[" << i*2 + POSITION << "]";
			glUniform4f(shader.uniform(ss2.str()), v.x, v.y, v.z, 1.0f);
//			check_errors(__FILE__, __LINE__);
			
			++i;
			if(i >= MAX_NUM_LIGHTS)
			{
				// if there are too many lights, just ignore the rest of them
				break;
			}
		}
		
		// always write multiples of four
		for(int k = i % 4; (k % 4) != 0; ++k)
		{
			if(i >= MAX_NUM_LIGHTS)
			{
				// if there are too many lights, just ignore the rest of them
				break;
			}
			
			float rgb[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			stringstream ss1;
			ss1 << "lights[" << i*2 + DIFFUSE << "]";
			glUniform4f(shader.uniform(ss1.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
			
			stringstream ss2;
			ss2 << "lights[" << i*2 + POSITION << "]";
			glUniform4f(shader.uniform(ss2.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
			
			++i;
		}
		
		shader.stop();
	}
}

void GameView::drawSkybox()
{
	Shader& shader = shaders.get_shader("skybox_program");
	shader.start();

	if(intVals["DRAW_DEBUG_LINES"])
	{
		TextureHandler::getSingleton().bindTexture(0, "chessboard");
	}
	else
	{
		TextureHandler::getSingleton().bindTexture(0, strVals["SKYBOX"]);
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	vec3<float> ans = camera_p->getTarget() - camera_p->getPosition();
	gluLookAt(
			0,0,0,
			ans.x, ans.y, ans.z,
			0,1,0);
	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	glDisable(GL_NORMALIZE);
	glDisable(GL_RESCALE_NORMAL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glNormal3f(0.0f, 0.0f, 0.0f);

	double minus = -20.0;
	double plus  =  20.0;
	double epsilon = 0.001;
//	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS); // Front
	glTexCoord2f(1.0/4+epsilon, 2.0/3-epsilon); glVertex3f( plus,  plus, plus-epsilon);
	glTexCoord2f(1.0/4+epsilon, 1.0/3+epsilon); glVertex3f( plus, minus, plus-epsilon);
	glTexCoord2f(2.0/4-epsilon, 1.0/3+epsilon); glVertex3f(minus, minus, plus-epsilon);
	glTexCoord2f(2.0/4-epsilon, 2.0/3-epsilon); glVertex3f(minus,  plus, plus-epsilon);
	glEnd();
//	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_QUADS); // Right
	glTexCoord2f(2.0/4+epsilon, 2.0/3-epsilon); glVertex3f(minus+epsilon,  plus, plus);
	glTexCoord2f(2.0/4+epsilon, 1.0/3+epsilon); glVertex3f(minus+epsilon, minus, plus);
	glTexCoord2f(3.0/4-epsilon, 1.0/3+epsilon); glVertex3f(minus+epsilon, minus, minus);
	glTexCoord2f(3.0/4-epsilon, 2.0/3-epsilon); glVertex3f(minus+epsilon,  plus, minus);
	glEnd();
//	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS); // Left
	glTexCoord2f(0.0/4+epsilon, 2.0/3-epsilon); glVertex3f(plus-epsilon,  plus, minus);
	glTexCoord2f(0.0/4+epsilon, 1.0/3+epsilon); glVertex3f(plus-epsilon, minus, minus);
	glTexCoord2f(1.0/4-epsilon, 1.0/3+epsilon); glVertex3f(plus-epsilon, minus, plus);
	glTexCoord2f(1.0/4-epsilon, 2.0/3-epsilon); glVertex3f(plus-epsilon,  plus, plus);
	glEnd();
//	glColor3f(1.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS); // Top
	glTexCoord2f(1.0/4+epsilon, 2.0/3+epsilon); glVertex3f( plus, plus-epsilon, plus);
	glTexCoord2f(2.0/4-epsilon, 2.0/3+epsilon); glVertex3f(minus, plus-epsilon, plus);
	glTexCoord2f(2.0/4-epsilon, 3.0/3-epsilon); glVertex3f(minus, plus-epsilon, minus);
	glTexCoord2f(1.0/4+epsilon, 3.0/3-epsilon); glVertex3f( plus, plus-epsilon, minus);
	glEnd();
//	glColor3f(0.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS); // Bottom
	glTexCoord2f(1.0/4+epsilon, 1.0/3-epsilon); glVertex3f( plus, minus+epsilon,  plus);
	glTexCoord2f(1.0/4+epsilon, 0.0/3+epsilon); glVertex3f( plus, minus+epsilon, minus);
	glTexCoord2f(2.0/4-epsilon, 0.0/3+epsilon); glVertex3f(minus, minus+epsilon, minus);
	glTexCoord2f(2.0/4-epsilon, 1.0/3-epsilon); glVertex3f(minus, minus+epsilon,  plus);
	glEnd();
//	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_QUADS); // Back
	glTexCoord2f(3.0/4+epsilon, 2.0/3-epsilon); glVertex3f(minus,  plus, minus+epsilon);
	glTexCoord2f(3.0/4+epsilon, 1.0/3+epsilon); glVertex3f(minus, minus, minus+epsilon);
	glTexCoord2f(4.0/4-epsilon, 1.0/3+epsilon); glVertex3f( plus, minus, minus+epsilon);
	glTexCoord2f(4.0/4-epsilon, 2.0/3-epsilon); glVertex3f( plus,  plus, minus+epsilon);
	glEnd();

	glPopAttrib();
	glPopMatrix();

	shader.stop();
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

void GameView::drawParticles(std::vector<Particle>& viewParticles)
{
	string depth_texture = (intVals["DEFERRED_RENDERING"] ? Graphics::Framebuffer::get("deferredFBO").depth_texture() : Graphics::Framebuffer::get("screenFBO").depth_texture());
	
	switch(intVals["DRAW_PARTICLES"])
	{
		case 2: drawParticles_vbo(viewParticles, depth_texture); break;
		case 3: drawParticles_old(viewParticles); break;
		default: drawParticles(viewParticles, depth_texture);
	}
}

// TODO: make this function faster than the current implementation by having the data already in the VBO format.
// TODO: Now it has to do extra copying.
void GameView::drawParticles_vbo(std::vector<Particle>& viewParticles, const std::string& depth_texture)
{
	Shader& shader = shaders.get_shader("particle_program");
	shader.start();
	GLint scale_location = shader.attribute("particleScale");
	glUniform1f(shader.uniform("near"), camera_p->nearP);
	glUniform1f(shader.uniform("far"), camera_p->farP);

	vec3<float> direction_vector = camera_p->getTarget() - camera_p->getPosition();
	depthSortParticles(direction_vector, viewParticles);
	
	TextureHandler::getSingleton().bindTexture(1, depth_texture);
	TextureHandler::getSingleton().bindTexture(0, "particle");
//	TextureHandler::getSingleton().bindTexture(0, "smoke");

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDepthMask(GL_FALSE); // dont write to depth buffer.

	static vector<vec3<float> > particles;
	static vector<float> colors;
	static vector<float> scales;
	static bool buffers_loaded = false;
	static const int BUFFERS = 3;
	static GLuint locations[BUFFERS];
	
	if(!buffers_loaded)
	{
		buffers_loaded = true;
		glGenBuffers(BUFFERS, locations);
	}

	size_t n = viewParticles.size();
	if(particles.size() < n)
	{
		particles.resize(n);
		colors.resize(4*n);
		scales.resize(n);
	}

	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		particles[i] = viewParticles[i].pos;

		viewParticles[i].getColor(&colors[i*4]);
		colors[i*4 + 3] = viewParticles[i].getAlpha();
		scales[i] = viewParticles[i].getScale();
		++QUADS_DRAWN_THIS_FRAME;
	}

	int buffer = 0;
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(vec3<float>), &particles[0], GL_STREAM_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * 4 * sizeof(float), &colors[0], GL_STREAM_DRAW);
	glColorPointer(4, GL_FLOAT, 0, 0);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(float), &scales[0], GL_STREAM_DRAW);
	glVertexAttribPointer(scale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

	assert(buffer == BUFFERS);

	// Draw sent data.
	glEnableVertexAttribArray(scale_location);
	glDrawArrays(GL_POINTS, 0, viewParticles.size());
	glDisableVertexAttribArray(scale_location);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDepthMask(GL_TRUE); // re-enable depth writing.
	glDisable(GL_BLEND);

	TextureHandler::getSingleton().unbindTexture(1);
	TextureHandler::getSingleton().unbindTexture(0);
	
	shader.stop();
}

void GameView::prepareForParticleRendering(const std::string& depth_texture)
{
	Graphics::Framebuffer::get("particlesDownScaledFBO").bind();
	glClear(GL_COLOR_BUFFER_BIT);
	
	TextureHandler::getSingleton().bindTexture(1, depth_texture);
	TextureHandler::getSingleton().bindTexture(0, "particle");
	
	// vec3<float> direction_vector = camera_p->getTarget() - camera_p->getPosition();
	// depthSortParticles(direction_vector, viewParticles);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
}

void GameView::renderParticles(std::vector<Particle>& viewParticles)
{
	Shader& shader = shaders.get_shader("particle_program");
	shader.start();
	GLint scale_location = shader.attribute("particleScale");
	glUniform1f(shader.uniform("near"), camera_p->nearP);
	glUniform1f(shader.uniform("far"), camera_p->farP);
	
	// The geometry shader transforms the points into quads.
	glBegin(GL_POINTS);
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		float px = viewParticles[i].pos.x;
		float py = viewParticles[i].pos.y;
		float pz = viewParticles[i].pos.z;
		
		float color[4];
		viewParticles[i].getColor(color);
		color[3] = viewParticles[i].getAlpha();
		
		// cerr << color[0] << " " << color[1] << " " << color[2] << " " << color[3] << "\n";
		
		float scale = viewParticles[i].getScale();
		
		glVertexAttrib1f(scale_location, scale);
		glColor4fv(color);
		glVertex3f(px, py, pz);
		
		++QUADS_DRAWN_THIS_FRAME;
	}
	glEnd();
	
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	TextureHandler::getSingleton().unbindTexture(1);

	shader.stop();
}

void GameView::drawParticles(std::vector<Particle>& viewParticles, const std::string& depth_texture)
{
	prepareForParticleRendering(depth_texture);
	Graphics::Framebuffer& particlesUpScaledFBO = Graphics::Framebuffer::get("particlesUpScaledFBO");
	
	// create a downscaled depth texture for particle rendering
	renderParticles(viewParticles);
	
	// upscale particle rendering result
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glDisable(GL_BLEND);
	Graphics::Framebuffer::get("particlesUpScaledFBO").bind();

	TextureHandler::getSingleton().bindTexture(0, Graphics::Framebuffer::get("particlesDownScaledFBO").texture(0));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// TODO: Could this perhaps be done (faster?) by directly copying from one framebuffer to another?
	// Perhaps with with glBindGraphics::Framebuffer(GL_READ_FRAMEBUFFER,...); glBindGraphics::Framebuffer(GL_DRAW_FRAMEBUFFER,...); glBlitGraphics::Framebuffer(...);??
	drawFullscreenQuad(); // Draw particlesDownScaledFBO_texture to particlesUpScaledFBO_texture (i.e. upscale).
	
	// blur upscaled particle texture
	if(intVals["PARTICLE_BLUR"])
	{
		// Draw particlesUpScaledFBO_texture to particlesUpScaledFBO_texture (apply blur).
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], particlesUpScaledFBO.texture(0), particlesUpScaledFBO);
		
		// Draw particlesUpScaledFBO_texture to particlesUpScaledFBO_texture (apply blur).
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], particlesUpScaledFBO.texture(0), particlesUpScaledFBO);
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Graphics::Framebuffer::get("screenFBO").bind();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	TextureHandler::getSingleton().bindTexture(0, particlesUpScaledFBO.texture(0));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	drawFullscreenQuad(); // Draw particlesUpScaledFBO_texture to screenFBO_texture (copy directly).
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	TextureHandler::getSingleton().unbindTexture(0);
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
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

void GameView::drawLightsDeferred_multiple_passes(const Camera& camera, const std::map<int, LightObject>& lights)
{
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	TextureHandler::getSingleton().bindTexture(0, Graphics::Framebuffer::get("deferredFBO").texture(0));
	TextureHandler::getSingleton().bindTexture(1, Graphics::Framebuffer::get("deferredFBO").texture(1));
	// TextureHandler::getSingleton().bindTexture(2, deferredFBO.texture(2));
	TextureHandler::getSingleton().bindTexture(3, Graphics::Framebuffer::get("deferredFBO").depth_texture());
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	Shader& shader = shaders.get_shader("partitioned_deferred_lights_program");
	shader.start();
	glUniform1f(shader.uniform("fullscreen"), 0.0f);

	// Draw lights that are not near the camera as point sprites. Lights are blended together one light at a time.
	int pass = 0;
	for(auto it = lights.begin(); it != lights.end(); ++it, ++pass)
	{
		const LightObject& light = it->second;

		Location loc = light.getPosition();
		vec3<float> v = vec3<float>(loc.x.getFloat(), loc.y.getFloat(), loc.z.getFloat());

		// TODO: intensity doesn't accurately reflect (in worl units) how far the light is actually seen.
		// light power is linear,
		// light attenuation is quadratic
		// so, square root of power should be sensible measure? lets approximate it with a constant number.
		float power = light.getIntensity().getFloat();
		power = 1000.0f;
	
		if(near(camera, v, power / intVals["DEFERRED_LIGHT_FIX_SPEED"]))
		{
			continue;
		}

		glUniform1f(shader.uniform("activeLight"), float(pass));
		glUniform1f(shader.uniform("power"), power);

		// TODO: if this thing will someday work really well, then all lights could be passed as varyings
		// here instead of updated through uniforms elsewhere. Not sure which is faster though.

		glBegin(GL_POINTS);
		glVertex3f(v.x, v.y, v.z);
		glEnd();
	}
	shader.stop();

	if(intVals["DEFERRED_LIGHT_FIX"])
	{
		Shader& shader2 = shaders.get_shader("partitioned_deferred_lights_program2");
		shader2.start();
		glUniform1f(shader2.uniform("fullscreen"), 1.0f);
		// Draw lights that are near the camera as fullscreen quads. Lights are blended together one light at a time.
		pass = 0;
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		
		int full_screen_lights = 0;
		
		for(auto it = lights.begin(); it != lights.end(); ++it, ++pass)
		{
			const LightObject& light = it->second;

			Location loc = light.getPosition();
			vec3<float> v = vec3<float>(loc.x.getFloat(), loc.y.getFloat(), loc.z.getFloat());

			// TODO: same as the todo in previous lights loop
			float power = light.getIntensity().getFloat();
			power = 1000.0f;
			
			if(!near(camera, v, power / intVals["DEFERRED_LIGHT_FIX_SPEED"]))
			{
				continue;
			}
			
			full_screen_lights++;
			glUniform1f(shader2.uniform("activeLight"), float(pass));
			drawFullscreenQuad();
		}
		
		stringstream lights_debug_str;
		lights_debug_str << "FS Lights: " << full_screen_lights;
		hud.insertDebugString(lights_debug_str.str());
		
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		shader2.stop();
	}

	glDisable(GL_BLEND);
	
	TextureHandler::getSingleton().unbindTexture(3);
	TextureHandler::getSingleton().unbindTexture(2);
	TextureHandler::getSingleton().unbindTexture(1);
	TextureHandler::getSingleton().unbindTexture(0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void GameView::applyDeferredLights(const std::map<int, LightObject>& lights)
{
	Graphics::Framebuffer::get("screenFBO").bind();

	applyAmbientLight();

	if(intVals["DRAW_DEFERRED_LIGHTS"] == 2)
	{
		drawLightsDeferred_single_pass(lights.size());
	}
	else if(intVals["DRAW_DEFERRED_LIGHTS"] == 1)
	{
		drawLightsDeferred_multiple_passes(*camera_p, lights);
	}
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
	
	if(intVals["DRAW_GRASS"])
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
		Shader& shader = shaders.get_shader("particle_program");
		shader.start();
		glUniform1f(shader.uniform("screen_width"),  intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
		glUniform1f(shader.uniform("screen_height"), intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
		shader.stop();
	}
	
	{
		Shader& shader = shaders.get_shader("ssao_program");
		shader.start();
		glUniform1f(shader.uniform("screen_width"), intVals["RESOLUTION_X"]);
		glUniform1f(shader.uniform("screen_height"), intVals["RESOLUTION_Y"]);
		shader.stop();
	}

	{
		Shader& shader = shaders.get_shader("partitioned_deferred_lights_program");
		shader.start();
		glUniform1f(shader.uniform("screen_width"), intVals["RESOLUTION_X"]);
		glUniform1f(shader.uniform("screen_height"), intVals["RESOLUTION_Y"]);
		shader.stop();
	}

	{
		Shader& shader = shaders.get_shader("partitioned_deferred_lights_program2");
		shader.start();
		glUniform1f(shader.uniform("screen_width"), intVals["RESOLUTION_X"]);
		glUniform1f(shader.uniform("screen_height"), intVals["RESOLUTION_Y"]);
		shader.stop();
	}
}
