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

vector<pair<Vec3,Vec3> > LINES;
vector<Vec3> DOTS;
vector<pair<Vec3,string> > STRINGS;

int TRIANGLES_DRAWN_THIS_FRAME = 0;
int QUADS_DRAWN_THIS_FRAME = 0;

// NOTE: set the y values to be the same. lights that were high above ground were drawn poorly
// because they were never near the player, although the effect is high.
bool near(const Camera& camera, const Vec3& position, float dist = 100.0)
{
	Vec3 copy1 = camera.getPosition();
	Vec3 copy2 = position;
	
	copy1.y = copy2.y;
	
	return (copy1 - copy2).lengthSquared() < dist * dist;
}

/*
struct LightDistance
{
	int index;
	FixedPoint squaredDistance;
	
	bool operator<(const LightDistance& rhs) const
	{
		return squaredDistance < rhs.squaredDistance;
	}
};

float isLightUsed(LightDistance& light)
{
	if(light.squaredDistance > 1000)
		return -1.f;
	else
		return light.index;
}
*/
void Graphics::depthSortParticles(Vec3& d, vector<Particle>& viewParticles)
{
	return;
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		viewParticles[i].updateDepthVal(d);
	}
	sort(viewParticles.begin(), viewParticles.end());
}

void Graphics::toggleWireframeStatus()
{
	drawDebugWireframe = !drawDebugWireframe;
}

void Graphics::toggleLightingStatus()
{
	lightsActive   = !lightsActive;
	drawDebuglines = lightsActive;
}

Graphics::Graphics(Window& w, Hud& h):
	window(w),
	hud(h)
{
	cerr << "Loading config file for Graphics.." << endl;
	load("graphics.conf");
	
	window.createWindow(intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
}

Graphics::~Graphics()
{
}

void Graphics::check_framebuffer_status(const std::string& fbo_name)
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Failed to create FBO: " + fbo_name);
	}
}

void Graphics::init(Camera& camera)
{
	cerr << "Graphics::init()" << endl;

	shaders.init();
	
	camera_p = &camera;

	glLineWidth(3.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set the background color.
	glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// Reset The Projection Matrix
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	MAX_NUM_LIGHTS = 71;
	MAX_NUM_ACTIVE_LIGHTS = 1; // Make sure this is the same number as in the shaders.
	
	setInitialShaderValues();
	
	// Init screen framebuffer objects
	glGenFramebuffers(1, &screenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
	TextureHandler::getSingleton().createTexture("screenFBO_texture0", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	TextureHandler::getSingleton().createDepthTexture("screenFBO_depth_texture", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("screenFBO_texture0"), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("screenFBO_depth_texture"), 0);
	check_framebuffer_status("screenFBO");

	glGenFramebuffers(1, &deferredFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, deferredFBO);
	TextureHandler::getSingleton().createTexture("deferredFBO_texture0", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	TextureHandler::getSingleton().createTexture("deferredFBO_texture1", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	TextureHandler::getSingleton().createFloatTexture("deferredFBO_texture2", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	TextureHandler::getSingleton().createDepthTexture("deferredFBO_depth_texture", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("deferredFBO_texture0"), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("deferredFBO_texture1"), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("deferredFBO_texture2"), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("deferredFBO_depth_texture"), 0);
	check_framebuffer_status("deferredFBO");

	glGenFramebuffers(1, &postFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
	TextureHandler::getSingleton().createTexture("postFBO_texture", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("postFBO_texture"), 0);
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("postFBO_depth_texture"), 0);
	check_framebuffer_status("postFBO");
	
	glGenFramebuffers(1, &particlesDownScaledFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, particlesDownScaledFBO);
	TextureHandler::getSingleton().createTexture("particlesDownScaledFBO_texture", intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"], intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("particlesDownScaledFBO_texture"), 0);
	check_framebuffer_status("particlesDownScaledFBO");
	
	glGenFramebuffers(1, &particlesUpScaledFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, particlesUpScaledFBO);
	TextureHandler::getSingleton().createTexture("particlesUpScaledFBO_texture", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("particlesUpScaledFBO_texture"), 0);
	check_framebuffer_status("particlesUpScaledFBO");
	
	/*
	glGenRenderbuffers(1, &screenRB);
	glBindRenderbuffer(GL_RENDERBUFFER, screenRB);
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 800, 600);
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenRB);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	*/

	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// ------------------------------
	
	// do some weird magic i dont understand
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	
	TextureHandler::getSingleton().createTexture("font", "data/fonts/font2.png");
	TextureHandler::getSingleton().createTexture("particle", "data/images/particle.png");
	
	camera_p->aspect_ratio = float(intVals["RESOLUTION_X"]) / float(intVals["RESOLUTION_Y"]);
	gluPerspective(camera_p->fov, camera_p->aspect_ratio, camera_p->nearP, camera_p->farP);
	frustum.setCamInternals(camera_p->fov, camera_p->aspect_ratio, camera_p->nearP, camera_p->farP);
	
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	
	lightsActive = false;
	drawDebuglines = false;
	drawDebugWireframe = false;
}

/*
void drawNormal(const Level& lvl, int x, int z)
{
	Location n = lvl.getNormal(x, z) * 10;
	Location start;
	start.x = FixedPoint(x * Level::BLOCK_SIZE);
	start.y = lvl.getVertexHeight(x, z);
	start.z = FixedPoint(z * Level::BLOCK_SIZE);
	
	Location end = start + n;
	
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); glVertex3f(start.x.getFloat(), start.y.getFloat(), start.z.getFloat());
	glColor3f(0.0, 1.0, 0.0); glVertex3f(end.x.getFloat(), end.y.getFloat(), end.z.getFloat());
	glEnd();
	
	glColor3f(1.0, 1.0, 1.0);
}
*/

void Graphics::clear_errors() const
{
	while(glGetError() != GL_NO_ERROR);
}

bool Graphics::check_errors(const char* filename, int line) const
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

void Graphics::updateLights(const std::map<int, LightObject>& lightsContainer)
{
	clear_errors();

	// TODO: update only the shader that is currently active. Or update lights when actually drawing.
	vector<string> programs = { "level_program", "deferred_lights_program", "partitioned_deferred_lights_program", "partitioned_deferred_lights_program2" };

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
			check_errors(__FILE__, __LINE__);
	//		cerr << shader.uniform(ss1.str()) << " is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
	//		cerr << "DIFFUSE is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;

			const Location& pos = iter->second.getPosition();
			rgb[0] = pos.x.getFloat();
			rgb[1] = pos.y.getFloat();
			rgb[2] = pos.z.getFloat();
			
	//		glLightfv(GL_LIGHT0 + i, GL_POSITION, rgb);

			stringstream ss2;
			ss2 << "lights[" << i*2 + POSITION << "]";
			glUniform4f(shader.uniform(ss2.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
			check_errors(__FILE__, __LINE__);
	//		cerr << shader.uniform(ss2.str()) << " is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
	//		cerr << "POSITION of light " << i << " is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
			
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


void Graphics::drawDebugLines()
{
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	for(size_t i = 0; i < LINES.size(); ++i)
	{
		const Vec3& p1 = LINES[i].first;
		const Vec3& p2 = LINES[i].second;
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
	}
	glEnd();
	
	glPointSize(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for(size_t i = 0; i < DOTS.size(); ++i)
	{
		const Vec3& p1 = DOTS[i];
		glVertex3f(p1.x, p1.y, p1.z);
	}
	glEnd();
}

void Graphics::drawDebugStrings()
{
	for(size_t i = 0; i < STRINGS.size(); ++i)
	{
		hud.draw3Dstring(STRINGS[i].second, STRINGS[i].first, camera_p->getXrot(), camera_p->getYrot());
	}
}

void Graphics::drawSkybox()
{
	Shader& shader = shaders.get_shader("skybox_program");
	shader.start();

	if(drawDebuglines)
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

	Vec3 ans = camera_p->getTarget() - camera_p->getPosition();
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

void Graphics::drawModels(const map<int, Model*>& models)
{
	if(lightsActive)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
	Shader& shader = shaders.get_shader("unit_program");
	shader.start();
	GLint unit_color_location = shader.uniform("unit_color");
	GLint unit_location_location = shader.uniform("unit_location");
	glMatrixMode(GL_MODELVIEW);
	for(map<int, Model*>::const_iterator iter = models.begin(); iter != models.end(); ++iter)
	{
		const Model& model = *iter->second;

		if(frustum.sphereInFrustum(model.currentModelPos, 5) != FrustumR::OUTSIDE)
		{
			glUniform4f(unit_color_location, 0.7, 0.7, 0.7, 0.5);
			glUniform3f(unit_location_location, model.currentModelPos.x, model.currentModelPos.y, model.currentModelPos.z);

			glTranslatef(model.currentModelPos.x, model.currentModelPos.y, model.currentModelPos.z);
			model.draw();
			glTranslatef(-model.currentModelPos.x, -model.currentModelPos.y, -model.currentModelPos.z);
		}
	}
	shader.stop();
}

void Graphics::drawParticles(std::vector<Particle>& viewParticles)
{
	string depth_texture = (intVals["DEFERRED_RENDERING"] ? "deferredFBO_depth_texture" : "screenFBO_depth_texture");

	switch(intVals["DRAW_PARTICLES"])
	{
		case 2: drawParticles_vbo(viewParticles, depth_texture); break;
		case 3: drawParticles_old(viewParticles); break;
		default: drawParticles(viewParticles, depth_texture);
	}
}

// TODO: make this function faster than the current implementation by having the data already in the VBO format.
// TODO: Now it has to do extra copying.
void Graphics::drawParticles_vbo(std::vector<Particle>& viewParticles, const std::string& depth_texture)
{
	glUseProgram(shaders["particle_program"]);
	GLint scale_location = shaders.uniform("particle_particleScale");

	Vec3 direction_vector = camera_p->getTarget() - camera_p->getPosition();
	depthSortParticles(direction_vector, viewParticles);
	
	TextureHandler::getSingleton().bindTexture(1, depth_texture);
	TextureHandler::getSingleton().bindTexture(0, "particle");
//	TextureHandler::getSingleton().bindTexture(0, "smoke");

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDepthMask(GL_FALSE); // dont write to depth buffer.

	static vector<Vec3> particles;
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
		particles[i] = Vec3(viewParticles[i].pos.x.getFloat(), viewParticles[i].pos.y.getFloat(), viewParticles[i].pos.z.getFloat());

		viewParticles[i].getColor(&colors[i*4]);
		colors[i*4 + 3] = viewParticles[i].getAlpha();
		scales[i] = viewParticles[i].getScale();
		++QUADS_DRAWN_THIS_FRAME;
	}

	int buffer = 0;
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(Vec3), &particles[0], GL_STREAM_DRAW);
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

	TextureHandler::getSingleton().bindTexture(1, "");
	TextureHandler::getSingleton().bindTexture(0, "");

	glUseProgram(0);
}

void Graphics::prepareForParticleRendering(const std::string& depth_texture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, particlesDownScaledFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"], intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
	
	TextureHandler::getSingleton().bindTexture(1, depth_texture);
	TextureHandler::getSingleton().bindTexture(0, "particle");
	
	// Vec3 direction_vector = camera_p->getTarget() - camera_p->getPosition();
	// depthSortParticles(direction_vector, viewParticles);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE); // dont write to depth buffer.
	glDisable(GL_DEPTH_TEST);
}

void Graphics::renderParticles(std::vector<Particle>& viewParticles)
{
	glUseProgram(shaders["particle_program"]);
	
	GLint scale_location = shaders.uniform("particle_particleScale");
	
	// The geometry shader transforms the points into quads.
	glBegin(GL_POINTS);
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		float px = viewParticles[i].pos.x.getFloat();
		float py = viewParticles[i].pos.y.getFloat();
		float pz = viewParticles[i].pos.z.getFloat();
		
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
	TextureHandler::getSingleton().bindTexture(1, "");

	glUseProgram(0);
}


void Graphics::drawFullscreenQuad() const
{
	float vertices[4*3] =
	{
		-1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f
	};
	float texcoords[4*2] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
/*
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.0f, +1.0f, -1.0f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f, +1.0f, -1.0f);
	glEnd();
*/
}

void Graphics::drawParticles(std::vector<Particle>& viewParticles, const std::string& depth_texture)
{
	prepareForParticleRendering(depth_texture);
	
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
	glBindFramebuffer(GL_FRAMEBUFFER, particlesUpScaledFBO);
	glViewport(0, 0, intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	TextureHandler::getSingleton().bindTexture(0, "particlesDownScaledFBO_texture");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// TODO: Could this perhaps be done (faster?) by directly copying from one framebuffer to another?
	// Perhaps with with glBindFramebuffer(GL_READ_FRAMEBUFFER,...); glBindFramebuffer(GL_DRAW_FRAMEBUFFER,...); glBlitFramebuffer(...);??
	drawFullscreenQuad(); // Draw particlesDownScaledFBO_texture to particlesUpScaledFBO_texture (i.e. upscale).
	
	// blur upscaled particle texture
	if(intVals["PARTICLE_BLUR"])
	{
		clear_errors();
		// Draw particlesUpScaledFBO_texture to particlesUpScaledFBO_texture (apply blur).
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], "particlesUpScaledFBO_texture", particlesUpScaledFBO);
		check_errors(__FILE__, __LINE__);
		// Draw particlesUpScaledFBO_texture to particlesUpScaledFBO_texture (apply blur).
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], "particlesUpScaledFBO_texture", particlesUpScaledFBO);
		check_errors(__FILE__, __LINE__);
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	TextureHandler::getSingleton().bindTexture(0, "particlesUpScaledFBO_texture");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	drawFullscreenQuad(); // Draw particlesUpScaledFBO_texture to screenFBO_texture (copy directly).
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	TextureHandler::getSingleton().bindTexture(0, "");
	
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

// This function is not in use anymore, would be useful only if geometry shader is not available. Remove if you feel like it.
void Graphics::drawParticles_old(std::vector<Particle>& viewParticles)
{
	Vec3 direction_vector = camera_p->getTarget() - camera_p->getPosition();
	// depthSortParticles(direction_vector, viewParticles);
	
	glBindFramebuffer(GL_FRAMEBUFFER, particlesDownScaledFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"], intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
	
	//TextureHandler::getSingleton().bindTexture(1, depth_texture);
	TextureHandler::getSingleton().bindTexture(0, "particle");
	
	// glUseProgram(shaders["particle_program"]);
	// GLint scale_location = shaders.uniform("particle_particleScale");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE); // dont write to depth buffer.

	glBegin(GL_QUADS);
	
	float x_angle =  camera_p->getXrot();
	float y_angle = -camera_p->getYrot() - 90.f;
	Matrix4 m(y_angle, x_angle, 0.f, 0.f, 0.f, 0.f);
	
	Vec3 s1(-1.0f, -1.0f, 0.0f);
	Vec3 s2(+1.0f, -1.0f, 0.0f);
	Vec3 s3(+1.0f, +1.0f, 0.0f);
	Vec3 s4(-1.0f, +1.0f, 0.0f);
	
	s1 = m * s1;
	s2 = m * s2;
	s3 = m * s3;
	s4 = m * s4;
	
	float color[4];
	
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		
		float px = viewParticles[i].pos.x.getFloat();
		float py = viewParticles[i].pos.y.getFloat();
		float pz = viewParticles[i].pos.z.getFloat();
		
		viewParticles[i].getColor(color);
		color[3] = viewParticles[i].getAlpha();
		glColor4fv(color);
		
		float s = viewParticles[i].getScale();
		glTexCoord2f(0.f, 0.f); glVertex3f(px+s1.x*s, py+s1.y*s, pz+s1.z*s);
		glTexCoord2f(1.f, 0.f); glVertex3f(px+s2.x*s, py+s2.y*s, pz+s2.z*s);
		glTexCoord2f(1.f, 1.f); glVertex3f(px+s3.x*s, py+s3.y*s, pz+s3.z*s);
		glTexCoord2f(0.f, 1.f); glVertex3f(px+s4.x*s, py+s4.y*s, pz+s4.z*s);
		
		++QUADS_DRAWN_THIS_FRAME;
	}
	
	glEnd();
	
	
	
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	TextureHandler::getSingleton().bindTexture(1, "");
	
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
	glViewport(0, 0, intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	
	
	TextureHandler::getSingleton().bindTexture(0, "particlesDownScaledFBO_texture");
	
	glUseProgram(0);
	
	//glBlendFunc(GL_ONE, GL_ONE);
	glBlendFunc(GL_SRC_COLOR, GL_ONE);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	drawFullscreenQuad();
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glUseProgram(0);
	TextureHandler::getSingleton().bindTexture(0, "");
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	glUseProgram(0);
}

void Graphics::updateCamera(const Level& lvl)
{
	Vec3 camStartPos = camera_p->getPosition();
	FixedPoint camX = FixedPoint(camStartPos.x);
	FixedPoint camZ = FixedPoint(camStartPos.z);
	
	float cam_min_y = lvl.getHeight(camX, camZ).getFloat() + 3.f;
	camera_p->setAboveGround(cam_min_y);
}

void Graphics::setupCamera(const Camera& camera)
{
	Vec3 camPos = camera.getPosition();
	Vec3 camTarget = camera.getTarget();
	Vec3 upVector(0.0, 1.0, 0.0);

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

void Graphics::startDrawing()
{
	setupCamera(*this->camera_p);

	if(intVals["DEFERRED_RENDERING"])
	{
		bind_framebuffer(deferredFBO, 3);
	}
	else
	{
		bind_framebuffer(screenFBO, 1);
	}

	glClear(GL_DEPTH_BUFFER_BIT); // Dont clear color buffer, since we're going to rewrite the color everywhere anyway.
//	glClear(GL_COLOR_BUFFER_BIT);

	TRIANGLES_DRAWN_THIS_FRAME = 0;
	QUADS_DRAWN_THIS_FRAME = 0;
}



void Graphics::applySSAO(int power, const string& input_texture, const string& depth_texture, GLuint renderTarget)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, renderTarget);
	TextureHandler::getSingleton().bindTexture(0, input_texture);
	TextureHandler::getSingleton().bindTexture(1, depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	Shader& shader = shaders.get_shader("ssao_program");
	shader.start();
	glUniform1f(shader.uniform("power"), float(power));

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	drawFullscreenQuad();
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	TextureHandler::getSingleton().bindTexture(1, "");
	shader.stop();
}


void Graphics::applyBlur(int blur, const string& input_image, GLuint renderTarget)
{
	blur = min(blur, intVals["MAXIMUM_BLUR"]);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
	TextureHandler::getSingleton().bindTexture(0, input_image);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	glUseProgram(shaders["blur_program1"]);
	glUniform1f(shaders.uniform("blur_amount1"), float(blur));
	drawFullscreenQuad();
	glUseProgram(0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, renderTarget);
	TextureHandler::getSingleton().bindTexture(0, "postFBO_texture");
	
	glUseProgram(shaders["blur_program2"]);
	glUniform1f(shaders.uniform("blur_amount2"), float(blur));
	drawFullscreenQuad();
	glUseProgram(0);
	
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}


void Graphics::drawLightsDeferred_single_pass(int lights)
{
	Shader& shader = shaders.get_shader("deferred_lights_program");
	shader.start();

	glUniform1f(shader.uniform("light_count"), float(lights));

	Vec3 camera_position = camera_p->getPosition();
	glUniform3f(shader.uniform("camera_position"), camera_position.x, camera_position.y, camera_position.z);
	Vec3 camera_target = camera_p->getTarget();
	glUniform3f(shader.uniform("camera_target"), camera_target.x, camera_target.y, camera_target.z);
/*
	cerr << "Camera: " << camera_position << endl;
	cerr << "BL    : " << frustum.fbl << endl;
	cerr << "BR    : " << frustum.fbr << endl;
	cerr << "TL    : " << frustum.ftl << endl;
	cerr << "TR    : " << frustum.ftr << endl;
*/
	TextureHandler::getSingleton().bindTexture(0, "deferredFBO_texture0");
	TextureHandler::getSingleton().bindTexture(1, "deferredFBO_texture1");
	TextureHandler::getSingleton().bindTexture(2, "deferredFBO_texture2");
	TextureHandler::getSingleton().bindTexture(3, "deferredFBO_depth_texture");
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
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, -1.0f); glVertexAttrib3f(attribute_location, frustum.fbl.x, frustum.fbl.y, frustum.fbl.z);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.0f, -1.0f, -1.0f); glVertexAttrib3f(attribute_location, frustum.fbr.x, frustum.fbr.y, frustum.fbr.z);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.0f, +1.0f, -1.0f); glVertexAttrib3f(attribute_location, frustum.ftr.x, frustum.ftr.y, frustum.ftr.z);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f, +1.0f, -1.0f); glVertexAttrib3f(attribute_location, frustum.ftl.x, frustum.ftl.y, frustum.ftl.z);
	glEnd();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	TextureHandler::getSingleton().bindTexture(3, "");
	TextureHandler::getSingleton().bindTexture(2, "");
	TextureHandler::getSingleton().bindTexture(1, "");
	TextureHandler::getSingleton().bindTexture(0, "");
	
	shader.stop();
}

void Graphics::applyAmbientLight()
{
	Shader& shader = shaders.get_shader("deferred_ambientlight_program");
	shader.start();

	TextureHandler::getSingleton().bindTexture(0, "deferredFBO_texture0");
	TextureHandler::getSingleton().bindTexture(1, "deferredFBO_texture1");
	float r = intVals["AMBIENT_RED"]   / 255.0f;
	float g = intVals["AMBIENT_GREEN"] / 255.0f;
	float b = intVals["AMBIENT_BLUE"]  / 255.0f;
	glUniform4f(shader.uniform("ambientLight"), r, g, b, 1.0f);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	drawFullscreenQuad();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	TextureHandler::getSingleton().bindTexture(1, "");
	TextureHandler::getSingleton().bindTexture(0, "");

	shader.stop();
}

Vec3 Graphics::getWorldPosition()
{
	return getWorldPosition(intVals["RESOLUTION_X"]/2, intVals["RESOLUTION_Y"]/2);
}

Vec3 Graphics::getWorldPosition(int screen_x, int screen_y)
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

	bind_framebuffer(deferredFBO, 0);
	glReadPixels( screen_x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

	gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return Vec3(posX, posY, posZ);
}

void Graphics::drawLightsDeferred_multiple_passes(const Camera& camera, const std::map<int, LightObject>& lights)
{
	camera.getPosition();

	clear_errors();

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	TextureHandler::getSingleton().bindTexture(0, "deferredFBO_texture0");
	TextureHandler::getSingleton().bindTexture(1, "deferredFBO_texture1");
	TextureHandler::getSingleton().bindTexture(2, "deferredFBO_texture2");
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
		Vec3 v = Vec3(loc.x.getFloat(), loc.y.getFloat(), loc.z.getFloat());

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
			Vec3 v = Vec3(loc.x.getFloat(), loc.y.getFloat(), loc.z.getFloat());

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
	
	TextureHandler::getSingleton().bindTexture(2, "");
	TextureHandler::getSingleton().bindTexture(1, "");
	TextureHandler::getSingleton().bindTexture(0, "");

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}


// TODO: This doesn't work.
void Graphics::drawLightsDeferred_multiple_passes_with_scissors(const std::map<int, LightObject>& lights)
{
	clear_errors();

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	TextureHandler::getSingleton().bindTexture(0, "deferredFBO_texture0");
	TextureHandler::getSingleton().bindTexture(1, "deferredFBO_texture1");
	TextureHandler::getSingleton().bindTexture(2, "deferredFBO_texture2");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	Shader& shader = shaders.get_shader("partitioned_deferred_lights_program_with_scissors");
	shader.start();


	Matrix4 m; // TODO: get modelview matrix directly from camera.
	glGetFloatv(GL_TRANSPOSE_MODELVIEW_MATRIX, m.T);

	int screen_width = intVals["RESOLUTION_X"];
	int screen_height = intVals["RESOLUTION_Y"];

	unsigned saved = 0;
	unsigned total = 0;
	glEnable(GL_SCISSOR_TEST);
	// Draw one light per pass and blend the results together.
	int pass = 0;
	for(auto it = lights.begin(); it != lights.end(); ++it, ++pass)
	{
		const LightObject& light = it->second;
		glUniform1f(shader.uniform("activeLight"), float(pass));
		if(check_errors(__FILE__, __LINE__))
		{
			cerr << "activeLight: " << pass << endl;
		}
		float power = light.getIntensity().getFloat();
		glUniform1f(shader.uniform("power"), power);
		if(check_errors(__FILE__, __LINE__))
		{
			cerr << "power: " << power << endl;
		}


		Location loc = light.getPosition();
		Vec3 v = Vec3(loc.x.getFloat(), loc.y.getFloat(), loc.z.getFloat());

		// TODO: if this thing will someday work really well, then lights could be passed here
		// instead of updated through uniforms. Not sure which is faster though.
//		float r, g, b;
//		light.getDiffuse(r, g, b);
//		glColor3f(r, g, b);

		total += screen_width*screen_height;
		array<int,4> rect;

		// TODO: light_scissor function does not work if
		// a) the light is behind the camera, or
		// b) the light is just behind the far plane (??)
		// c) ever(?)
		int n = light_scissor(m*v, power/2.0f, screen_width, screen_height, rect);
		if(n == 0)
		{
			saved += screen_width*screen_height;
			continue;
		}
		if(n >= screen_width * screen_height)
		{
			glDisable(GL_SCISSOR_TEST);
		}
		else
		{
			glScissor(rect[0], rect[1], rect[2], rect[3]);
			if(check_errors(__FILE__, __LINE__))
			{
				cout << "pass: " << pass << m*v << " [" << rect[0] << "," << rect[1] << "] x [" << rect[2] << "," << rect[3] << "]" << endl;
			}
			saved += (rect[2]-rect[0])*(rect[3]-rect[1]);
		}

		drawFullscreenQuad();
		/*
		glBegin(GL_POINTS);
		glVertex3f(v.x, v.y, v.z);
		glEnd();
		*/
	}
	glDisable(GL_SCISSOR_TEST);

//	cout << double(saved)/double(total) * 100.0 << "%" << endl;
	shader.stop();

	glDisable(GL_BLEND);
	
	TextureHandler::getSingleton().bindTexture(2, "");
	TextureHandler::getSingleton().bindTexture(1, "");
	TextureHandler::getSingleton().bindTexture(0, "");

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}



void Graphics::bind_framebuffer(GLuint framebuffer, int output_buffers) const
{
	assert(output_buffers >= 0);
	assert(output_buffers <= 3);
	clear_errors();
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	if(!output_buffers)
	{
		glDrawBuffer(GL_NONE);
	}
	else
	{
		GLenum targets[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(output_buffers, targets);
	}
	check_errors(__FILE__, __LINE__);
}

// TODO: this interface for deferred rendering is ugly and unintuitive to use...
void Graphics::geometryDrawn(const std::map<int, LightObject>& lights)
{
	if(intVals["DEFERRED_RENDERING"])
	{
		bind_framebuffer(screenFBO, 1);

		applyAmbientLight();

		if(intVals["DRAW_DEFERRED_LIGHTS"] == 2)
		{
			drawLightsDeferred_single_pass(lights.size());
		}
		else if(intVals["DRAW_DEFERRED_LIGHTS"] == 1)
		{
			drawLightsDeferred_multiple_passes(*camera_p, lights);
//			drawLightsDeferred_multiple_passes_with_scissors(lights);
		}
	}
}

void Graphics::draw(
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

	if(drawDebuglines)
	{
		visualworld.levelDesc.drawDebugLevelNormals();
		drawDebugProjectiles(projectiles);
	}

	if(drawDebugWireframe)
	{
		visualworld.levelDesc.drawDebugHeightDots(camera_p->getPosition());
	}
	else if(intVals["DRAW_LEVEL"])
	{
		if(deferred_rendering)
		{
			visualworld.levelDesc.drawDeferred(shaders);
		}
		else
		{
			visualworld.levelDesc.drawFR(visualworld.lights.size(), shaders);
		}
	}

	if(drawDebuglines)
	{
		drawDebugLines();
	}
	
	if(intVals["DRAW_MODELS"])
	{
		drawModels(visualworld.models);
	}
	
	if(intVals["DRAW_GRASS"])
	{
		drawGrass(visualworld.meadows);
	}

	geometryDrawn(visualworld.lights);
	
	string depth_texture = (deferred_rendering ? "deferredFBO_depth_texture" : "screenFBO_depth_texture");

	if(intVals["SSAO"])
	{
		applySSAO(intVals["SSAO_DISTANCE"], "screenFBO_texture0", depth_texture, screenFBO);
	}
	
	if(intVals["DRAW_PARTICLES"])
	{
		drawParticles(visualworld.particles);
	}
	
	if(drawDebuglines)
	{
		drawBoundingBoxes(units);
		drawOctree(o);
	}
	
	if(intVals["DRAW_NAMES"])
	{
		drawPlayerNames(units, visualworld.models);
	}
	
	drawDebugStrings();
	
	if(intVals["DAMAGE_BLUR"])
	{
		applyBlur(blur, "screenFBO_texture0", screenFBO);
	}
	
	if(intVals["DRAW_HUD"])
	{
		hud.draw(camera_p->mode() == Camera::FIRST_PERSON);
	}

	finishDrawing();
}

void Graphics::drawPlayerNames(const std::map<int, Unit>& units, const map<int, Model*>& models)
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
		Vec3 pos = model.currentModelPos;
		pos.y += 5.0;

		hud.draw3Dstring(iter->second.name, pos, camera_p->getXrot(), camera_p->getYrot(), iter->second["TEAM"]);
	}
}

void Graphics::drawDebugQuad()
{
	string s = strVals["DEBUG_QUAD"];
	if(s == "NULL")
	{
		return;
	}

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	TextureHandler::getSingleton().bindTexture(0, s);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -0.7f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(-0.2f, -0.7f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(-0.2f, +0.0f, -1.0f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f, +0.0f, -1.0f);
	glEnd();

	TextureHandler::getSingleton().bindTexture(0, "");
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void Graphics::renderToBackbuffer()
{
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	TextureHandler::getSingleton().bindTexture(0, "screenFBO_texture0");
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	
	glColor3f(1.0, 1.0, 1.0);
	
	drawFullscreenQuad();

	glEnable(GL_DEPTH_TEST);
}

void Graphics::finishDrawing()
{
	STRINGS.clear();

	renderToBackbuffer();

	drawDebugQuad();

	window.swap_buffers();
}


void Graphics::world_tick(const std::map<int, LightObject>& lights)
{
	updateLights(lights);
}

void Graphics::tick()
{
}

void Graphics::toggleFullscreen()
{
	window.toggle_fullscreen();
}

void Graphics::zoom_in()
{
	camera_p->zoomIn();
}

void Graphics::zoom_out()
{
	camera_p->zoomOut();
}

void Graphics::drawBox(const Location& top, const Location& bot,
	GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	glColor4f(r, g, b, a);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());

	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());

	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glEnd();
}

void Graphics::drawOctree(const std::shared_ptr<Octree>& o)
{
	if (o->n > 0)
		drawBox(o->top, o->bot, 1.0f);
	if (o->hasChildren) {
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
					drawOctree(o->children[i][j][k]);
	}
}

void Graphics::drawBoundingBoxes(const std::map<int,Unit>& units)
{
	for(auto iter = units.begin(); iter != units.end(); iter++)
	{
		drawBox(iter->second.bb_top(), iter->second.bb_bot());
	}
}

void Graphics::drawDebugProjectiles(const std::map<int, Projectile>& projectiles)
{
	glColor3f(0.3, 0.7, 0.5);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for(auto it = projectiles.begin(); it != projectiles.end(); ++it)
	{
		const MovableObject& proj = it->second;
		glVertex3f(proj.position.x.getFloat(), proj.position.y.getFloat(), proj.position.z.getFloat());
	}
	glEnd();
}

void Graphics::drawGrass(const std::vector<GrassCluster>& meadows)
{
	Shader& shader = shaders.get_shader("grass_program");
	shader.start();
	
	if(drawDebuglines)
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
			meadows[i].draw(&shaders);
		}
	}

	shader.stop();
}

void Graphics::drawMenuParticles(const std::vector<MenuParticle>& menuParticles, int front, float scale, const string& color) const
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

void Graphics::drawMenuButtons(const vector<MenuButton>& buttons) const
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
        // float plus  = 2.f * (i+1.f) / buttons.size() - 1.f;
        // TextureHandler::getSingleton().bindTexture(0, buttons[i].name);
        // void drawString(const std::string&, float pos_x = -1.0f, float pos_y = -1.0f, float scale = 1.0f, bool background = false, float alpha = 1.0f) const;
        msg << buttons[i].name;
        info << buttons[i].info;
        hud.drawString(msg.str(), -0.7f, minus + menu_y_offset, 3.0f);
        hud.drawString(info.str(), 0.0f, minus + menu_y_offset, 3.0f);
        /*
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.0f); glVertex3f(-1, minus * menu_height + menu_y_offset, -1);
		glTexCoord2f(1.f, 0.0f); glVertex3f(+0, minus * menu_height + menu_y_offset, -1);
		glTexCoord2f(1.f, 1.0f); glVertex3f(+0, plus  * menu_height + menu_y_offset, -1);
		glTexCoord2f(0.f, 1.0f); glVertex3f(-1, plus  * menu_height + menu_y_offset, -1);
		glEnd();
		*/
    }
}

void Graphics::drawMenuRectangles() const
{
	static list<tuple<float, float, float, float, float, float, float> > rects;
	static int count = 0;
	if(count == 0)
	{
		rects.push_back(make_tuple(0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.4f, 0.4f));
	}
	if(count++ % 30 == 0)
	{
		float r = sin(count / 300.0f) * 0.5 + 0.5;
		float g = cos(count / 300.0f) * 0.5 + 0.5;
		float b = (sin(count / 300.0f) + cos(count / 300.0f))/2 * 0.5 + 0.5;

		float x = sin(count / 300.0f) / 5.0f;
		float y = cos(count / 300.0f) / 5.0f;
		rects.push_back(make_tuple(x, y, 0.007f, 0.004f, r, g, b));
	}
	TextureHandler::getSingleton().bindTexture(0, "");

	for(auto it = rects.begin(); it != rects.end(); )
	{
		float center_x = std::get<0>(*it);
		float center_y = std::get<1>(*it);
		float size_x = std::get<2>(*it);
		float size_y = std::get<3>(*it);
		float r = std::get<4>(*it);
		float g = std::get<5>(*it);
		float b = std::get<6>(*it);

		float left = center_x - size_x;
		float right = center_x + size_x;
		float top = center_y + size_y;
		float bot = center_y - size_y;

		if(top > 1.0f && bot < -1.0f && left < -1.0f && right > 1.0f)
		{
			it = rects.erase(it);
			continue;
		}
		else
		{
			std::get<2>(*it) *= 1.03f;
			std::get<3>(*it) *= 1.03f;
			++it;
		}

		glColor3f(r, g, b);
		glBegin(GL_LINES);
			glVertex3f(left, top, -1.0f);
			glVertex3f(right, top, -1.0f);
			glVertex3f(left, bot, -1.0f);
			glVertex3f(right, bot, -1.0f);
			glVertex3f(left, top, -1.0f);
			glVertex3f(left, bot, -1.0f);
			glVertex3f(right, top, -1.0f);
			glVertex3f(right, bot, -1.0f);
		glEnd();
	}
}

void Graphics::drawMenu(const vector<MenuButton>& buttons, const std::vector<MenuParticle>& menuParticles) const
{
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

    drawMenuParticles(menuParticles, 0, 1.2f, "^r");
    drawMenuParticles(menuParticles, 1, 2.0f, "^R");
	
    drawMenuRectangles();

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

void Graphics::reload_shaders()
{
	shaders.release();
	shaders.init();
	setInitialShaderValues();
}

void Graphics::setInitialShaderValues()
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

